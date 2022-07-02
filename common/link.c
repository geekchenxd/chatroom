#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>

static void *tcp_server_listen(void *arg)
{
	struct link_data *link = arg;
	struct sockaddr_in addr;
	socklen_t sock_len;
	int ret;

	while (1) {
		ret = accept(link->sock, (struct sockaddr *)&addr,
				&sock_len, 0);
		if (ret == -1) {
			continue;
		}
		if (link->accept)
			link->accept_callback(ret, &addr);
	}

	return NULL;
}

static int link_tcp_init(struct mtp *mtp)
{
	int ret;
	struct sockaddr_in addr;

	struct link_data *link = mtp->private_data;
	if (link == NULL)
		return -EINVAL;

	ret = socket(AF_INET, SOCK_STREAM, 0);
	if (ret == -1) {
		perror("socket");
		return -errno;
	}

	addr.sin_port = link->port;
	addr.sin_addr.s_addr = inet_aton(link->ip);

	link->sock = ret;

	if (link->type == LINK_TYPE_TCP_CLIENT) {
		ret = connect(mtp->sock, (struct sockaddr *)&addr,
				(socklen_t)sizeof(addr));
		if (ret == -1) {
			printf("connect server %s:%d error\n",
					link->ip, link->port);
			ret = -errno;
			goto close_socket;
		}
		printf("connect server %s:%d successed\n",
				link->ip, link->port);
		return 0;
	}

	ret = bind(mtp->sock, (struct sockaddr *)&addr,
			(socklen_t)sizeof(addr));
	if (bind) {
		perror("bind");
		ret = -errno;
		goto close_socket;
	}

	ret = listen(mtp->sock, 100);
	if (ret == -1) {
		perror("listen");
		ret = -errno;
		goto close_socket;
	}

	ret = pthread_create(&link->listen_tid, NULL,
			tcp_server_linsten, link);
	if (ret) {
		perror("pthread_create");
		ret = -errno;
		goto close_socket;
	}

	return 0;
close_socket:
	close(mtp->sock);
	mtp->sock = -1;
	return ret;

}

static int link_tcp_send_msg(struct mtp *mtp, const uint8_t *msg, size_t len)
{
	ssize_t size;
	struct link_data *link;

	if (!mtp || !msg || !len)
		return -EINVAL;

       	link = mtp->private_data;
	if (link == NULL)
		return -EINVAL;

	size = send(link->sock, msg, len);
	if (size == -1)
		return -errno;

	return size;
}

static uint8_t checksum(const uint8_t *data, size_t len)
{
	uint8_t sum = 0;
	int i;

	for (i = 0; i < len; i++)
		sum += sum ^= data[i];

	return sum;
}

static int tcp_receive_timeout(int sock, uint8_t *buf, size_t size, uint32_t timeout)
{
	size_t length_to_read = size;
	size_t len;
	size_t received = 0;
	int ret;
	fd_set rfds;
	struct timeval tv;

	while (length_to_read > 0) {
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);

		/* Wait up to five seconds. */

		tv.tv_sec = 0;
		tv.tv_usec = timeout;

		ret = select(sock + 1, &rfds, NULL, NULL, &tv);
		if (ret < 0)
			return -errno;

		if (ret == 0)
			break;

		len = recv(sock, buf + received, length_to_read);
		if (len < 0)
			return -errno;

		length_to_read -= len;
		received += len;
	}

	return received;
}

static int tcp_recv_fsm(struct mtp *mtp, uint8_t *msg, size_t len)
{
	size_t length_to_read;
	uint8_t pdu[MTP_MAX_PDU];
	size_t size;
	int fsm_state = FSM_STATE_HEAD;
	struct link_data *link = mtp->private_data;
	bool invalid_frame = false;
	size_t datalen = 0;
	uint8_t tmp;

	if (!link)
		return -ENODEV;

	while (1) {
		switch (fsm_state) {
		case FSM_STATE_HEAD:
			size = tcp_receive_timeout(link->sock,
					pdu, 1, 10000);
			if (size < 0) {
				invalid_frame = true;
				break;
			}
			if (size == 0)
				return 0;
			if (size == 1 && pdu[0] == 0x68)
				fsm_state = FSM_STATE_LEN;
			break;
		case FSM_STATE_LEN:
			size = tcp_receive_timeout(link->sock,
					pdu, 2, 5000);
			if (size < 0) {
				invalid_frame = true;
				break;
			}

			if (size == 0)
				return 0;

			datalen = link_data_len(pdu);
			if (datalen <= 0 || datalen > mtp->max_pdu) {
				invalid_frame = true;
				break;
			}
			fsm_state = FSM_STATE_DATA;
			break;
		case FSM_STATE_DATA:
			size = tcp_receive_timeout(link->sock,
					pdu, datalen, 5000);
			if (size < 0) {
				invalid_frame = true;
				break;
			}

			if (size == 0)
				return 0;

			fsm_state = FSM_STATE_CHECK;
			break;
		case FSM_STATE_CHECK:
			size = tcp_receive_timeout(link->sock,
					&tmp, 1, 5000);
			if (size < 0) {
				invalid_frame = true;
				break;
			}

			if (size == 0)
				return 0;

			if (tmp != checksum((const uint8_t *)pdu, datalen)) {
				invalid_frame = true;
				break;
			}
			fsm_state = FSM_STATE_TAIL;
			break;
		case FSM_STATE_TAIL:
			size = tcp_receive_timeout(link->sock,
					&tmp, 1, 5000);
			if (size < 0) {
				invalid_frame = true;
				break;
			}

			if (size == 0)
				return 0;

			if (tmp != 0x16) {
				invalid_frame = true;
				break;
			}
			break;
		default:
			break;
		}

		if (invalid_frame)
			return -errno;
	}

	return datalen;
}

static int link_tcp_recv_msg(struct mtp *mtp, uint8_t *msg, size_t len)
{
	if (!mtp || !msg || !len)
		return -EINVAL；

	return tcp_recv_fsm(mtp, msg, len);
}

static struct mtp mtp_tcp = {
	.max_pdu = 2048,
	.private_data = ,
	.init = link_tcp_init,
	.exit = link_tcp_exit,
	.send_msg = link_tcp_send_msg,
	.recv_msg = link_tcp_recv_msg,
};

struct mtp *datalink_init(struct link_data *data)
{
	int ret;
	struct mtp *mtp = &mtp_tcp;

	if (!data)
		return -EINVAL;

	mtp->private_data = data;
	ret = mtp->init(mtp);
	if (ret) {
		printf("init mtp tcp link failed,ret = %d\n", ret);
		return NULL;
	}

	return mtp;
}

void datalink_exit(struct mtp *mtp)
{
	if (mtp)
		mtp->exit(mtp);
}

size_t datalink_recv(struct mtp *mtp, uint8_t *msg, size_t len)
{
	if (!msg || !len)
		return -EINVAL;

	if (!mtp || !mtp->recv_msg)
		return -ENODEV;

	return mtp->recv_msg(mtp, msg, len);
}

size_t datalink_send(struct mtp *mtp, const uint8_t *msg,
		size_t size)
{
	if (!msg || !size)
		return -EINVAL;

	if (!mtp || !mtp->send_msg)
		return -ENODEV;

	return mtp->send_msg(mtp, msg, size);
}

