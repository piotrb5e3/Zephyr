#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Arduino.h>
#include <SHA256.h>
#include <ZephyrConfig.h>

#include "dyeus.h"
#include "screen.h"

char toHex[] = "0123456789ABCDEF";

Dyeus::Dyeus() {
	this->sd_count = 0;
}

Dyeus::~Dyeus() {
	this->sd_count = 0;
}

bool Dyeus::addSensorData(const char *name, const char *value) {
	if (this->sd_count >= 10)
		return false;

	if (strlen(name) >= SD_NAME_LEN || strlen(value) >= SD_VALUE_LEN)
		return false;

	strncpy(this->sd[this->sd_count].name, name, SD_NAME_LEN - 1);
	this->sd[this->sd_count].name[SD_NAME_LEN - 1] = '\0';

	strncpy(this->sd[this->sd_count].value, value, SD_VALUE_LEN - 1);
	this->sd[this->sd_count].value[SD_VALUE_LEN - 1] = '\0';

	this->sd_count++;
	return true;
}

bool Dyeus::addSensorData(const char *name, int value) {
	char tmpval[SD_VALUE_LEN];
	int ret = snprintf(tmpval, SD_VALUE_LEN - 1, "%d", value);
	if (ret >= SD_VALUE_LEN || ret < 0)
		return false;
	return addSensorData(name, tmpval);
}

bool Dyeus::addSensorData(const char *name, double value) {
	char tmpval[SD_VALUE_LEN];
	dtostrf(value, 1, 4, tmpval);
	return addSensorData(name, tmpval);
}

bool Dyeus::sendData() {
	uint16_t datalen;
	uint8_t year, month, day, hour, minute, second;
	char *buff_ptr = buffer;

	if (this->sd_count <= 0) {
		displayFatalErr("No sensor data to send!");
		return false;
	}

	if (!ZephyrConfig::getConsistentTime(&year, &month, &day, &hour, &minute,
			&second))
		return false;

	for (uint8_t i = 0; i < this->sd_count; i++) {
		int ct = sprintf(buff_ptr, "%s=%s,", this->sd[i].name,
				this->sd[i].value);
		buff_ptr += ct;
		datalen += ct;
	}

	sprintf(buff_ptr, "20%2.2d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d", year, month, day,
			hour, minute, second);

	if (!authenticate())
		return false;

	Serial.print(buffer);
	Serial.print("\n");

	for (int i = 0; i < 32; i++) {
		Serial.print(toHex[mac[i] / 16]);
		Serial.print(toHex[mac[i] % 16]);
	}
	Serial.print("\n");
	this->sd_count = 0;
	return true;
}

bool Dyeus::authenticate() {
	uint8_t key[32];
	if (!ZephyrConfig::getHMACKey(key))
		return false;

	SHA256 hmac;
	hmac.resetHMAC(key, 16);
	hmac.update(buffer, strlen(buffer));
	hmac.finalizeHMAC(key, 16, mac, 32);
	return true;
}

bool Dyeus::connectToWiFi() {

}

bool Dyeus::sendDataToServer() {

}

bool Dyeus::disconnectWiFi() {

}

