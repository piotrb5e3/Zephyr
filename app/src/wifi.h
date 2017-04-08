#ifndef __WIFI_H__
#define __WIFI_H__

bool wifiReset();

bool connect(const char *ssid, const char *passwd);
bool disconnect();
bool openConnection(uint8_t ip[4], uint16_t port);

bool start(const char *location, uint16_t totalDataLen, const char *host);
bool cont(const char *data);
bool cont(const char *data, size_t datalen);
bool contC(char data);

// Returns HTTP status code, <0 on error
int expectResponse(char *data, size_t *datalen);

#endif /* __WIFI_H__ */
