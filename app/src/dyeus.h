#include <stdint.h>
#ifndef __DYEUS_H__
#define __DYEUS_H__

#define SD_NAME_LEN 10
#define SD_VALUE_LEN 10

struct sensorData {
	char name[SD_NAME_LEN];
	char value[SD_VALUE_LEN];
};

class Dyeus {
public:
	Dyeus();
	virtual ~Dyeus();

	bool addSensorData(const char *name, const char *value);
	bool addSensorData(const char *name, int value);
	bool addSensorData(const char *name, double value);
	bool sendData();

private:
	bool authenticate();
	bool connectToWiFi();
	bool sendDataToServer();
	bool disconnectWiFi();

	sensorData sd[10];
	uint16_t sd_count;
	char buffer[256];
	uint8_t mac[32];
};

#endif /* __DYEUS_H__ */
