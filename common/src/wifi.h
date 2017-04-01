#ifndef __WIFI_H__
#define __WIFI_H__

bool connect(const char *ssid, const char *passwd);
bool disconnect();
bool openConnection(uint8_t ip[4], uint16_t port);

class HttpSend {
public:
	HttpSend();

	bool start(uint16_t totalDataLen);
	bool cont(const char *data);
	bool cont(const char *data, size_t datalen);
	bool cont(char data);

	bool waitForResponse();

	// Returns length of recvd data, <0 on error
	int expectData(char *data, size_t maxDatalen);

private:
	uint16_t bytesLeft;
};

#endif /* __WIFI_H__ */
