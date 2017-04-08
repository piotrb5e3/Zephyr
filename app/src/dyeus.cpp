#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Arduino.h>
#include <SHA256.h>
#include <Base64.h>
#include <ZephyrConfig.h>

#include "dyeus.h"
#include "screen.h"
#include "wifi.h"


char toHex[] = "0123456789ABCDEF";

void Dyeus::begin() {
	this->buff_ptr = 0;
}

bool Dyeus::addSensorDataS(const char *name, const char *value) {
	int ctr = 0;
	if (this->buff_ptr >= 220)
		return false;

	ctr = sprintf(&(this->buffer[buff_ptr]), "%s", name);
	if(ctr < 0)return false;
	this->buff_ptr += ctr;
	this->buffer[buff_ptr] = '=';
	this->buff_ptr += 1;

	ctr = sprintf(&(this->buffer[buff_ptr]), "%s", value);
	if(ctr < 0)return false;
	this->buff_ptr += ctr;
	this->buffer[buff_ptr] = ',';
	this->buff_ptr += 1;

	return true;
}

bool Dyeus::addSensorDataI(const char *name, int value) {
	char tmpval[SD_VALUE_LEN + 1];
	if(sprintf(tmpval, "%d", value)<0)
		return false;
	return addSensorDataS(name, tmpval);
}

bool Dyeus::addSensorDataD(const char *name, double value) {
	char tmpval[SD_VALUE_LEN + 1];
	dtostrf(value, 1, 4, tmpval);
	return addSensorDataS(name, tmpval);
}

bool Dyeus::sendData() {
	if (!appendTime() || !authenticate())
		return false;

	if (!connectToServer() || !sendDataToServer()) {
		disconnectWiFi();
		return false;
	}
	disconnectWiFi();

	this->buff_ptr = 0;
	return true;
}

bool Dyeus::authenticate() {
	uint8_t key[32];
	uint8_t mac[32];
	uint16_t b64mac_len;
	if (!ZephyrConfig::getHMACKey(key))
		return false;

	SHA256 hmac;
	hmac.resetHMAC(key, 32);
	hmac.update(this->buffer, strlen(this->buffer));
	hmac.finalizeHMAC(key, 32, mac, 32);

	b64mac_len = Base64.encode(this->b64mac, mac, 32);
	this->b64mac[b64mac_len] = '\0';
	return true;
}

bool Dyeus::appendTime() {
	int ctr = 0;
	uint8_t year, month, day, hour, minute, second;

	if (!ZephyrConfig::getConsistentTime(&year, &month, &day, &hour, &minute,
			&second))
		return false;

	ctr = sprintf(&(this->buffer[buff_ptr]),
			"20%2.2d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d", year, month, day, hour,
			minute, second);
	this->buff_ptr += ctr;
	this->buffer[buff_ptr] = '\0';
	return true;
}

bool Dyeus::connectToServer() {
	uint8_t ssid[32], passwd[32], location[32];
	uint8_t ip[4];
	uint16_t port;
	char hoststr[24];
	uint32_t id;
	char s_id[10];

	if (!(ZephyrConfig::getSSID(ssid) && ZephyrConfig::getPassword(passwd)
			&& ZephyrConfig::getLocation(location) && ZephyrConfig::getIP(ip)
			&& ZephyrConfig::getPort(&port) && ZephyrConfig::getId(&id)))
		return false;

	sprintf(s_id, "%d", id);
	sprintf(hoststr, "%d.%d.%d.%d:%d", ip[0], ip[1], ip[2], ip[3], port);

	uint16_t content_len = 26 + strlen(s_id) + strlen(this->b64mac)
			+ strlen(this->buffer);

	if(!(wifiReset() && connect(ssid, passwd) && openConnection(ip, port)))
		return false;
	delay(500);

	return start(location, content_len, hoststr);
}

bool Dyeus::sendDataToServer() {
	char tmp[128];
	size_t tmplen = 127;
	char s_id[10];
	uint32_t id;
	if (!ZephyrConfig::getId(&id))
		return false;

	sprintf(s_id, "%d", id);

	// 6 + 9 + 9 + 2 =26 extra chars
	if (!(cont("{\"id\":") && cont(s_id) && cont(",\"data\":\"")
			&& cont(this->buffer) && cont("\",\"mac\":\"") && cont((char*)this->b64mac)
			&& cont("\"}")))
		return false;

	int ret = expectResponse(tmp, &tmplen);
	if (ret / 100 != 2)
		return false;
	return true;
}

bool Dyeus::disconnectWiFi() {
	return disconnect();
}

