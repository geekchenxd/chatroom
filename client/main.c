#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

static struct client *client;
static struct link_data client_link_data;
static struct user user = {
	.registerd = false,
	.login = false,
	.client = NULL,
};

static void client_user_register(void)
{
	char buf[128];
	int len = 0;
	int ch;
	int ret;

	if (user.registered)
		return;
	printf("*Please input username:\n");
	while (ch = getchar() != '\n')
		buf[len++] = ch;
	buf[len] = '\0';
	user.name = strdup(buf);

	printf("*Please input passwd\n");
	len = 0;
	while (ch = getchar() != '\n')
		buf[len++] = ch;
	buf[len] = '\0';
	user.passwd = strdup(buf);

	ret = user_register(&user);
	if (ret) {
		printf("user register failed!ret=%d\n", ret);
		return;
	}
}

static void client_user_login(void)
{
	char buf[128];
	int len = 0;
	int ch;
	int ret;

	if (user.login)
		return;
	printf("*Please input username:\n");
	while (ch = getchar() != '\n')
		buf[len++] = ch;
	buf[len] = '\0';
	if (strcmp(user.name, buf)) {
		printf("Invalid user name\n");
		return;
	}

	printf("*Please input passwd\n");
	len = 0;
	while (ch = getchar() != '\n')
		buf[len++] = ch;
	buf[len] = '\0';
	if (strcmp(user.passwd, buf)) {
		printf("Invalid passwd\n");
		return;
	}

	ret = user_login(&user);
	if (ret) {
		printf("user register failed!ret=%d\n", ret);
		return;
	}
}

static void client_user_logout(void)
{
	if (user_logout(&user)) {};
}

static void client_send_message(void)
{
	uint8_t buf[2048] = {0x0};
	int len;
	int ch;

	if (!user.login) {
		printf("user not login\n");
		return;
	}

	while (ch = getchar() != '\n')
		buf[len++] = ch;

	ret = user_send_message(&user, buf, len);
	if (ret) {
		printf("User send message failed, ret=%d\n", ret);
		return;
	}
}

int main(int argc, char *argv[])
{
	int ret;
	int op;

	if (argc != 3)  {
		printf("Usage:%s <serverip> <server port>\n", argv[0]);
		return -1;
	}

	memcpy(client_link_data.ipaddr, argv[1], strlen(argv[1]));
	client_link_data.port = atoi(argv[2]);

	printf("serverip:%s server port:%u\n",
			client_link_data.ipaddr, client_link_data.port);

	client = client_create(&client_link_data);
	if (!client) {
		printf("create client failed\n");
		return -1;
	}

	ret = mtp_run(client->mtp);

	printf("*****************************************\n");
	printf("* (1)register user\n");
	printf("* (2)user login\n");
	printf("* (3)user logout\n");
	printf("* (4)send message\n");
	printf("*****************************************\n");

	while (1) {
		scanf("%d", &op);
		switch (op) {
		case 1:
			client_user_register();
			break;
		case 2:
			client_user_login();
			break;
		case 3:
			client_user_logout();
			break;
		case 4:
			client_send_message();
			break;
		default:
			printf("Unrecognized option <%d>\n", op);
			break;
	}

	return 0;
}


