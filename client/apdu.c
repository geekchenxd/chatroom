
int client_apdu_encode_login(uint8_t buf, size_t len,
		const char *name, const char *passwd)
{
	int encode_len = 0;
	buf[0] = APDU_REQUEST;
	buf[1] = APDU_REQUEST_LOGIN;
	encode_len = 2;
	memcpy(&buf[encode_len], name, strlen(name));
	encode_len += strlen(name);
	buf[encode_len++] = ':';
	memcpy(&buf[encode_len], passwd, strlen(passwd));
	encode_len += strlen(passwd);

	return encode_len;
}

int client_apdu_encode_register(uint8_t buf, size_t len,
		const char *name, const char *passwd)
{
	int encode_len = 0;
	buf[0] = APDU_REQUEST;
	buf[1] = APDU_REQUEST_REGISTER;
	encode_len = 2;
	memcpy(&buf[encode_len], name, strlen(name));
	encode_len += strlen(name);
	buf[encode_len++] = ':';
	memcpy(&buf[encode_len], passwd, strlen(passwd));
	encode_len += strlen(passwd);

	return encode_len;
}

int client_apdu_encode_message(uint8_t buf, size_t len,
		const uint8_t *msg, size_t size)
{
	int encode_len = 0;
	buf[0] = APDU_REQUEST;
	buf[1] = APDU_REQUEST_MESSAGE;
	encode_len = 2;
	memcpy(&buf[encode_len], name, strlen(name));
	encode_len += strlen(name);
	buf[encode_len++] = ':';
	memcpy(&buf[encode_len], msg, size);
	encode_len += size;

	return encode_len;
}

