
struct user {
	const char *name;
	const char *passwd;
	bool registerd;
	bool login;
	struct client *client;
};

struct user *user_create(const char *name, const char *passwd, struct client *cli)
{
	if (!cli || !passwd || !name) {
		printf("Invalid arguments\n");
		return NULL;
	}

	struct user *user = malloc(sizeof(struct user));
	if (!user) {
		perror("user malloc");
		return NULL;
	}
	memset(user, 0x0, sizeof(struct user));

	user->name = name;
	user->passwd = passwd;
	user->client = cli;

	return user;
}

static int send_message(struct user *user, uint8_t *msg, size_t size)
{
	size_t ret;

	if (!user || !msg || !size)
		return -EINVAL;

	ret = datalink_send(user->client->mtp, msg, size);
	if (ret < 0) {
		printf("user_send_message failed, ret=%d\n", ret);
		return ret;
	}

	return 0;
}

int user_send_message(struct user *user, uint8_t *msg, size_t size)
{
	uint8_t buf[2048] = {0x0};
	int ret;

	if (!user->login)
		return -ENODEV;

	ret = client_apdu_encode_message(buf, 2048, msg, size);
	if (ret < 0) {
		printf("apdu encode message failed, ret=%d\n", ret);
		return ret;
	}

	return send_message(user, buf, ret);
}

int user_register(struct user *user)
{
	uint8_t buf[2048] = {0x0};
	int ret;

	if (user->registered)
		return -EEXIST;

	ret = client_apdu_encode_register(buf, 2048, user->name, user->passwd);
	if (ret < 0) {
		printf("apdu encode register failed, ret=%d\n", ret);
		return ret;
	}

	return send_message(user, buf, ret);
}

int user_login(struct user *user)
{
	uint8_t buf[2048] = {0x0};
	int ret;

	if (!user->registered)
		return -ENODEV;

	if (user->login)
		return -EEXIST;

	ret = client_apdu_encode_login(buf, sizeof(buf), user->name, user->passwd);
	if (ret < 0) {
		printf("apdu encode login failed, ret=%d\n", ret);
		return ret;
	}

	return send_message(user, buf, ret);
}

int user_logout(struct user *user)
{
	return 0;
}

int user_unregister(struct user *user)
{
	return 0;
}
