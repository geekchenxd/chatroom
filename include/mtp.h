#ifndef __MTP__H__
#define __MTP__H__
#include <stdint.h>

#define MTP_MAX_PDU 2048

struct mtp {
	int max_pdu;
	void *private_data;
	int (*init)(struct mtp *);
	int (*exit)(struct mtp *);
	int (*send_msg)(struct mtp *mtp, const uint8_t *msg, size_t len);
	int (*recv_msg)(struct mtp *mtp, uint8_t *msg, size_t len);
};


#endif
