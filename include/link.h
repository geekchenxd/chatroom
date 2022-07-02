#ifndef __LINK_H__
#define __LINK_H__

enum {
	LINK_TYPE_TCP_SERVER = 0,
	LINK_TYPE_TCP_CLIENT = 1,
};

struct link_data {
	int sock;
	char ipaddr[16];
	uint16_t port;
	int type;
	pthread_t listen_tid;
	void (*accept_callback)(int, struct sockaddr_in *);
};

struct mtp *datalink_init(struct link_data *data);
void datalink_exit(struct mtp *mtp);
size_t datalink_recv(struct mtp *mtp,
		uint8_t *msg, size_t len);
size_t datalink_send(struct mtp *mtp,
		const uint8_t *msg, size_t size);

#endif

