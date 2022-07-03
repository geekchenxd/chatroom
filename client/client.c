
struct client {
	bool initialized;
	struct mtp *mtp;
};

static int client_apdu_handler(struct mtp *mtp, uint8_t *apdu, size_t len)
{
	int i;
	printf("received apdu:\n");
	for (i = 0; i < len; i++)
		printf("0x%x ", apdu[i]);
	printf("\n");
	return 0;
}

struct client *client_create(struct link_data *link)
{
	struct client *cli;

	cli = malloc(sizeof(struct client));
	if (!cli) {
		perror("client malloc");
		return NULL;
	}
	memset(cli, 0x0, sizeof(struct client));

	cli->mtp = datalink_init(link);
	if (!cli->mtp) {
		printf("client mtp init failed\n");
		free(cli);
	}
	mtp_set_apdu_handler(mtp, client_apdu_handler);

	cli->initialized = true;

	return cli;
}

void client_destroy(struct client *cli)
{
	datalink_exit(cli->mtp);
	free(cli);
}

