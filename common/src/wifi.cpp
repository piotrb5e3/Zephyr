#include <Arduino.h>

#include "wifi.h"

static bool runCmd(const char *cmd, size_t cmdlen, const char *await,
		uint16_t timeoutMs) {
	const char *aw_ptr = await;
	for (int i = 0; i < cmdlen; i++) {
		Serial.print(cmd[i]);
	}
	uint32_t targetMillis = millis() + timeoutMs;
	while (*aw_ptr != '\0' && millis() <= targetMillis) {
		if (Serial.available() > 0) {
			char c = Serial.read();
			if (c == *aw_ptr)
				aw_ptr++;
			else
				aw_ptr = await;
		}
	}
	return millis() <= targetMillis;
}

static bool runCmd(const char *cmd, const char *await, uint16_t timeoutMs) {
	return runCmd(cmd, strlen(cmd), await, timeoutMs);
}

bool connect(const char *ssid, const char *passwd) {
	char tmp[64];
	snprintf(tmp, 63, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, passwd);
	tmp[63] = '\0';
	return (runCmd(tmp, "OK\r\n", 10000)
			&& runCmd("AT+CWJAP?\r\n", "OK\r\n", 10000)
			&& runCmd("AT+CIFSR\r\n", "OK\r\n", 10000)
			&& runCmd("AT+CIPMUX=0\r\n", "OK\r\n", 10000));
}

bool disconnect() {
	//TODO
	return true;
}

bool openConnection(uint8_t ip[4], uint16_t port) {
	char tmp[64];
	snprintf(tmp, 63, "AT+CIPSTART=\"TCP\",\"%d.%d.%d.%d\",%d\r\n", ip[0],
			ip[1], ip[2], ip[3], port);
	tmp[63] = '\0';
	return runCmd(tmp, "OK\r\n", 10000);
}

HttpSend::HttpSend() {
	this->bytesLeft = 0;
}

bool HttpSend::start(uint16_t totalDataLen) {
	return false;
}
bool HttpSend::cont(const char *data) {
	return false;
}

bool HttpSend::cont(const char *data, size_t datalen) {
	return false;
}

bool HttpSend::cont(char data) {
	return false;
}

bool HttpSend::waitForResponse() {
	return false;
}

// Returns length of recvd data, <0 on error
int HttpSend::expectData(char *data, size_t maxDatalen) {
	return -1;
}

