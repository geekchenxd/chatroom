#include <errno.h>

struct mtp *mtp_init(enum LINK_TYPE type, struct link_data *data)
{
	return NULL;
}

void mtp_exit(struct mtp *mtp)
{
}

void mtp_set_apdu_handler(struct mtp *mtp,
		int (*handler)(uint8_t *apdu, size_t len))
{
	mtp->apdu_handler = handler;
}

int mtp_run(struct mtp *mtp)
{
	int ret;

	if (!mtp)
		return -EINVAL;
}

void mtp_stop(struct mtp *mtp)
{
}

