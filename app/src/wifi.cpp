#include <Arduino.h>
#include "screen.h"
#include "wifi.h"


static bool serialTimeout() {
	uint32_t st = millis();
	while (millis() - st < 1000 && Serial.available() == 0) {
	}
	if (millis() - st >= 1000)
		return true;
	return false;
}

static int readInt() {
	char c = '0';
	int ret = 0;
	while (isdigit(c)) {
		ret *= 10;
		ret += c - '0';

		if (serialTimeout())
			return -1;

		c = Serial.read();
	}
	return ret;
}

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
	bool ret = millis() <= targetMillis;
	return ret;
}

static bool runCmd(const char *cmd, const char *await, uint16_t timeoutMs) {
	return runCmd(cmd, strlen(cmd), await, timeoutMs);
}

static bool sendData(const char *data, size_t datalen) {
	char tmp[64];
	snprintf(tmp, 63, "AT+CIPSEND=%d\r\n", datalen);
	return runCmd(tmp, ">", 10000) && runCmd(data, datalen, "SEND OK", 10000);
}

static bool sendData(const char *data) {
	return sendData(data, strlen(data));
}

bool wifiReset() {
	pinMode(2, OUTPUT);
	digitalWrite(2, HIGH);
	delay(100);
	digitalWrite(2, LOW);
	delay(10000);
	for (int i = 0; i < 5; i++) {
		if (runCmd("AT\r\n", "OK\r\n", 2000))
			return true;
	}
	return false;
}

bool connect(const char *ssid, const char *passwd) {
	char tmp[64];
	int retries = 5;
	snprintf(tmp, 63, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, passwd);
	tmp[63] = '\0';
	runCmd("ATE0\r\n", "OK\r\n", 2000);
	runCmd("AT+CWMODE=1\r\n", "no change\r\n", 2000);
	while (!runCmd(tmp, "OK\r\n", 10000)) {
		retries--;
		if (retries <= 0)
			return false;
	}
	return (runCmd("AT+CWJAP?\r\n", "OK\r\n", 10000)
			&& runCmd("AT+CIFSR\r\n", "OK\r\n", 10000)
			&& runCmd("AT+CIPMUX=0\r\n", "OK\r\n", 10000));
}

bool disconnect() {
	return runCmd("AT+CWQAP\r\n", "OK\r\n", 10000);
}

bool openConnection(uint8_t ip[4], uint16_t port) {
	char tmp[64];
	int retries = 5;
	snprintf(tmp, 63, "AT+CIPSTART=\"TCP\",\"%d.%d.%d.%d\",%d\r\n", ip[0],
			ip[1], ip[2], ip[3], port);
	tmp[63] = '\0';

	while (!runCmd(tmp, "OK\r\nLinked\r\n", 10000)) {
		retries--;
		if (retries <= 0)
			return false;
	}
	delay(2000);
	return true;
}

bool start(const char *location, uint16_t totalDataLen, const char *host) {
	char s_len[20];
	sprintf(s_len, "%d", totalDataLen);
	return (sendData("POST ") && sendData(location) && sendData(" HTTP/1.1\r\n")
			&& sendData("Host: ") && sendData(host) && sendData("\r\n")
			&& sendData("Accept: */*\r\n"
					"User-Agent: Zephyr/0.1\r\n"
					"Accept-Encoding: deflate\r\n")
			&& sendData("Content-Length: ") && sendData(s_len)
			&& sendData("\r\n")
			&& sendData("Content-Type: application/json\r\n")
			&& sendData("\r\n"));
}
bool cont(const char *data) {
	return sendData(data);
}

bool cont(const char *data, size_t datalen) {
	return sendData(data, datalen);
}

bool contC(char data) {
	return sendData(&data, 1);
}

// Returns HTTP status code, <0 on error
int expectResponse(char *data, size_t *datalen) {
	int ret_len = 0;
	int status = -1;
	uint16_t max_datalen = *datalen;
	*datalen = 0;

	if (!runCmd("", "+IPD,", 10000))
		return -1;

	if (!runCmd("", "HTTP/1.1 ", 10000))
		return -1;

	// Read status
	status = readInt();

	if (status < 0)
		return -1;

	if (!runCmd("", "Content-Length: ", 10000))
		return -1;

	// Read returned data length
	ret_len = readInt();

	if (ret_len < 0)
		return -1;

	if (!runCmd("", "\r\n\r\n", 10000))
		return -1;

	for (int i = 0; i < ret_len && i < max_datalen; i++) {
		if (serialTimeout())
			return -1;
		data[*datalen] = Serial.read();
		(*datalen)++;
	}
	return status;
}
