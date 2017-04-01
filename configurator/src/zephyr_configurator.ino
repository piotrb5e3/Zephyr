#include <Wire.h>
#include <ZephyrConfig.h>

#define TIMEOUT_MS 1000
#define CMD_START_CHAR '@'
#define CMD_END_CHAR ';'
#define FAILSTR "!"
#define OKSTR "+"
#define SEPSTR ","
#define SET_RTC_CMD 'T'
#define CONFIGURE_RTC_CMD 'R'
#define SET_KEY_CMD 'K'
#define SET_ID_CMD 'D'
#define SET_IP_CMD 'A'
#define SET_PORT_CMD 'P'
#define SYNC_CMD 'S'
#define SET_LOCATION_CMD 'L'
#define SET_SSID_CMD 'N'
#define SET_PASSWD_CMD '~'
#define GET_CONF_CMD 'V'

char toHex[] = "0123456789ABCDEF";
void setup() {
	Serial.begin(115200);
	Wire.begin();
}

void loop() {
	if (Serial.available() > 1) {
		char c = Serial.read();
		if (c == CMD_START_CHAR) {
			c = Serial.read();
			switch (c) {
			case SET_RTC_CMD:
				setRTCCmd();
				break;
			case CONFIGURE_RTC_CMD:
				configureRTCCmd();
				break;
			case SET_KEY_CMD:
				setKeyCmd();
				break;
			case SET_ID_CMD:
				setIdCmd();
				break;
			case SET_IP_CMD:
				setIPCmd();
				break;
			case SET_PORT_CMD:
				setPortCmd();
				break;
			case SET_LOCATION_CMD:
				setLocationCmd();
				break;
			case SET_SSID_CMD:
				setSSIDCmd();
				break;
			case SET_PASSWD_CMD:
				setPasswordCmd();
				break;
			case SYNC_CMD:
				syncCmd();
				break;
			case GET_CONF_CMD:
				getConfCmd();
				break;
			default:
				Serial.print(FAILSTR);
				Serial.println("BAD CMD");
				break;
			}
		}
	}
}

void setRTCCmd() {
	uint32_t yr, mth, day, weekday, hr, mins;

	if (!readInt(&yr) || !readInt(&mth) || !readInt(&day) || !readInt(&weekday)
			|| !readInt(&hr) || !readInt(&mins) || yr > 99 || mth > 12
			|| day > 31 || weekday > 7 || hr > 23 || mins > 59) {
		Serial.print(FAILSTR);
		Serial.println("BAD READ");
		return;
	}
	if (!ZephyrConfig::setRTC(yr, mth, day, weekday, hr, mins)) {
		Serial.print(FAILSTR);
		Serial.println("BAD SET");
		return;
	}
	Serial.println(OKSTR);
}

void configureRTCCmd() {
	if (!readCmdEnd()) {
		Serial.print(FAILSTR);
		Serial.println("BAD READ");
		return;
	}
	if (!ZephyrConfig::configureRTC()) {
		Serial.print(FAILSTR);
		Serial.println("BAD SET");
		return;
	}
	Serial.println(OKSTR);
}

void setKeyCmd() {
	uint8_t key[32];
	if (!readKey(key)) {
		Serial.print(FAILSTR);
		Serial.println("BAD KEY READ");
		return;
	}
	if (!readCmdEnd()) {
		Serial.print(FAILSTR);
		Serial.println("BAD READ");
		return;
	}
	if (!ZephyrConfig::setHMACKey(key)) {
		Serial.print(FAILSTR);
		Serial.println("BAD SET");
		return;
	}
	Serial.println(OKSTR);
}

void setIdCmd() {
	uint32_t id;
	if (!readInt(&id)) {
		Serial.print(FAILSTR);
		Serial.println("BAD READ");
		return;
	}
	if (!ZephyrConfig::setId(id)) {
		Serial.print(FAILSTR);
		Serial.println("BAD SET");
		return;
	}
	Serial.println(OKSTR);
}

void setIPCmd() {
	uint8_t ip[4];
	for (int i = 0; i < 4; i++) {
		uint32_t tmp;
		if (!readInt(&tmp) || tmp > 255) {
			Serial.print(FAILSTR);
			Serial.println("BAD READ");
			return;
		}
		ip[i] = tmp;
	}
	if (!ZephyrConfig::setIP(ip)) {
		Serial.print(FAILSTR);
		Serial.println("BAD SET");
		return;
	}
	Serial.println(OKSTR);
}

void setPortCmd() {
	uint32_t port;
	if (!readInt(&port)) {
		Serial.print(FAILSTR);
		Serial.println("BAD READ");
		return;
	}
	if (!ZephyrConfig::setPort(port)) {
		Serial.print(FAILSTR);
		Serial.println("BAD SET");
		return;
	}
	Serial.println(OKSTR);
}

void setLocationCmd() {
	uint8_t loc[32];
	if (!readStr(31, loc)) {
		Serial.print(FAILSTR);
		Serial.println("BAD READ");
		return;
	}
	if (!ZephyrConfig::setLocation(loc)) {
		Serial.print(FAILSTR);
		Serial.println("BAD SET");
		return;
	}
	Serial.println(OKSTR);
}

void setSSIDCmd() {
	uint8_t ssid[32];
	if (!readStr(31, ssid)) {
		Serial.print(FAILSTR);
		Serial.println("BAD READ");
		return;
	}
	if (!ZephyrConfig::setSSID(ssid)) {
		Serial.print(FAILSTR);
		Serial.println("BAD SET");
		return;
	}
	Serial.println(OKSTR);
}

void setPasswordCmd() {
	uint8_t passwd[32];
	if (!readStr(31, passwd)) {
		Serial.print(FAILSTR);
		Serial.println("BAD READ");
		return;
	}
	if (!ZephyrConfig::setPassword(passwd)) {
		Serial.print(FAILSTR);
		Serial.println("BAD SET");
		return;
	}
	Serial.println(OKSTR);
}

void syncCmd() {
	if (!readCmdEnd()) {
		Serial.print(FAILSTR);
		Serial.println("BAD READ");
		return;
	}
	Serial.println(OKSTR);
}

void getConfCmd() {
	uint8_t binkey[32];
	uint8_t hexkey[64];
	uint8_t location[32];
	uint8_t ssid[32];
	uint8_t passwd[32];
	uint32_t id;
	uint16_t port;
	uint8_t ip[4];
	char tm[32];
	uint8_t yr, month, day, hr, mn, sc;

	if (!readCmdEnd()) {
		Serial.print(FAILSTR);
		Serial.println("BAD READ");
		return;
	}

	if (!ZephyrConfig::getId(&id) || !ZephyrConfig::getIP(ip)
			|| !ZephyrConfig::getPort(&port)
			|| !ZephyrConfig::getHMACKey(binkey)
			|| !ZephyrConfig::getLocation(location)
			|| !ZephyrConfig::getSSID(ssid)
			|| !ZephyrConfig::getPassword(passwd)) {
		Serial.print(FAILSTR);
		Serial.println("BAD EEPROM READ");
		return;
	}

	if (!ZephyrConfig::getConsistentTime(&yr, &month, &day, &hr, &mn, &sc)) {
		Serial.print(FAILSTR);
		Serial.println("BAD RTC READ");
		return;
	}

	for (int i = 0; i < 32; i++) {
		hexkey[2 * i] = toHex[binkey[i] / 16];
		hexkey[2 * i + 1] = toHex[binkey[i] % 16];
	}

	sprintf(tm, "20%2.2d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d", yr, month, day, hr, mn,
			sc);

	Serial.print(OKSTR);
	//Id
	Serial.print(id);
	Serial.print(SEPSTR);
	//Ip
	for (int i = 0; i < 3; i++) {
		Serial.print(ip[i]);
		Serial.print(".");
	}
	Serial.print(ip[3]);
	Serial.print(SEPSTR);
	//Port
	Serial.print(port);
	Serial.print(SEPSTR);
	//Key
	for (int i = 0; i < 64; i++) {
		Serial.print((char) hexkey[i]);
	}
	Serial.print(SEPSTR);
	//Location
	Serial.print((char*) location);
	Serial.print(SEPSTR);
	//SSID
	Serial.print((char*) ssid);
	Serial.print(SEPSTR);
	//Password
	Serial.print((char*) passwd);
	Serial.print(SEPSTR);
	//Time
	Serial.println(tm);
}

bool readCmdEnd() {
	if (serialTimeout())
		return false;
	return Serial.read() == CMD_END_CHAR;
}

bool readInt(uint32_t *res) {
	*res = 0;
	if (serialTimeout())
		return false;
	char c = Serial.read();
	while (isdigit(c)) {
		(*res) *= 10;
		(*res) += c - '0';
		if (serialTimeout())
			return false;
		c = Serial.read();
	}
	return true;
}

bool readStr(int maxLen, uint8_t *str) {
	char len;

	if (serialTimeout())
		return false;

	len = Serial.read();

	if (len > maxLen)
		return false;

	for (int i = 0; i < len; i++) {
		if (serialTimeout())
			return false;
		str[i] = Serial.read();
	}
	if (serialTimeout())
		return false;
	if (Serial.read() != CMD_END_CHAR)
		return false;
	str[len] = '\0';
	return true;
}

bool readKey(uint8_t *binkey) {
	for (int i = 0; i < 32; i++) {
		if (serialTimeout())
			return false;
		binkey[i] = Serial.read();
	}
	return true;
}

bool hexToByte(char c1, char c0, uint8_t *result) {
	int d1 = hexDigitToInt(c1);
	int d0 = hexDigitToInt(c0);
	if (d1 == -1 || d0 == -1)
		return false;
	*result = 16 * d1 + d0;
	return true;
}

int hexDigitToInt(char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

bool serialTimeout() {
	unsigned long int st = millis();
	while (millis() - st < TIMEOUT_MS && Serial.available() == 0)
		;
	if (millis() - st >= TIMEOUT_MS)
		return true;
	return false;
}

