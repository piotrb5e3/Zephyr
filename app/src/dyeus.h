#include <stdint.h>
#ifndef __DYEUS_H__
#define __DYEUS_H__

#define SD_NAME_LEN 10
#define SD_VALUE_LEN 10

class Dyeus {
public:
	void begin();

	bool addSensorDataS(const char *name, const char *value);
	bool addSensorDataI(const char *name, int value);
	bool addSensorDataD(const char *name, double value);
	bool sendData();

private:
	bool appendTime();
	bool authenticate();
	bool connectToServer();
	bool sendDataToServer();
	bool disconnectWiFi();

	char buffer[256];
	size_t buff_ptr;
	uint8_t b64mac[44];
};

#endif /* __DYEUS_H__ */
