#include <Wire.h>
#include <Arduino.h>
#include "ZephyrConfig.h"

#define EEPROM_I2C_ADDR 0b1010000
#define EEPROM_ID_ADDR 0x0
#define EEPROM_IP_ADDR 0x4
#define EEPROM_PORT_ADDR 0x8
#define EEPROM_HMAC_KEY_ADDR 0x10
#define EEPROM_LOC_ADDR 0x30
#define EEPROM_SSID_ADDR 0x50
#define EEPROM_PASSWD_ADDR 0x70

#define RTC_I2C_ADDR 0b1101000
#define RTC_SEC_REG 0x03
#define TIMEOUT_MS 1000

uint8_t toBCD(uint8_t in) {
	return ((in / 10) << 4) | (in % 10);
}

uint8_t fromBCD(uint8_t in) {
	return (in >> 4) * 10 + (in & 15);
}

bool wireTimeout() {
	uint32_t st = millis();
	while (millis() - st < TIMEOUT_MS && Wire.available() <= 0) {
	}
	if (millis() - st >= TIMEOUT_MS)
		return true;
	return false;
}

bool ZephyrConfig::configureRTC() {
	Wire.beginTransmission(RTC_I2C_ADDR);
	Wire.write(0); // Select ctrl register 1
	Wire.write(0b00000000); // Def: 7pf, no interrupts, 24h mode
	Wire.write(0b00000000); // Def: no interrupts, no watchdog, no timers
	Wire.write(0b00100000); // Enable direct switchover to battery power, enable battery low detection, clean all interrupts
	Wire.endTransmission();

	// Verify
	Wire.beginTransmission(RTC_I2C_ADDR);
	Wire.write(0);
	Wire.endTransmission();

	Wire.requestFrom(RTC_I2C_ADDR, 3);
	if (wireTimeout())
		return false;
	if (Wire.read() != 0)
		return false;
	if (wireTimeout())
		return false;
	if (Wire.read() != 0)
		return false;
	if (wireTimeout())
		return false;
	if (Wire.read() != 0b00100000)
		return false;
	return true;
}

// To be called at the beginning of a minute
// Resets seconds counter to zero
bool ZephyrConfig::setRTC(uint8_t yr, uint8_t mth, uint8_t day, uint8_t weekday,
		uint8_t hr, uint8_t mins) {
	if (yr > 99 || mth > 12 || mth < 1 || day > 31 || day < 1 || weekday > 6
			|| hr > 23 || mins > 59) {
		return false;
	}
	Wire.beginTransmission(RTC_I2C_ADDR);
	Wire.write(RTC_SEC_REG);
	Wire.write(0); // Reset seconds register
	Wire.write(toBCD(mins)); // Write minutes
	Wire.write(toBCD(hr)); // Write hours
	Wire.write(toBCD(day)); // Write days
	Wire.write(weekday); // Write weekdays
	Wire.write(toBCD(mth)); // Write months
	Wire.write(toBCD(yr)); // Write years
	Wire.endTransmission();

	// Verify
	uint8_t _yr, _month, _day, _hr, _mn, _;
	if (!getTimeOnce(&_yr, &_month, &_day, &_hr, &_mn, &_))
		return false;

	if (_mn != mins)
		return false;
	if (_hr != hr)
		return false;
	if (_day != day)
		return false;
	if (_month != mth)
		return false;
	if (_yr != yr)
		return false;
	return true;
}

bool ZephyrConfig::setId(uint32_t id) {
	uint8_t id_[4];
	uint32_t tmp = id;
	for (int i = 3; i >= 0; i--) {
		id_[i] = tmp % 256;
		tmp /= 256;
	}
	Wire.beginTransmission(EEPROM_I2C_ADDR);
	Wire.write(EEPROM_ID_ADDR);
	for (int i = 0; i < 4; i++) {
		Wire.write(id_[i]);
	}
	Wire.endTransmission();
	delay(10);

	// Verify
	uint32_t act_id = 0;
	if (!getId(&act_id))
		return false;
	if (act_id != id)
		return false;
	return true;
}

bool ZephyrConfig::setIP(uint8_t *ip) {
	uint8_t ip_[4];
	Wire.beginTransmission(EEPROM_I2C_ADDR);
	Wire.write(EEPROM_IP_ADDR);
	for (int i = 0; i < 4; i++) {
		Wire.write(ip[i]);
	}
	Wire.endTransmission();
	delay(10);

	// Verify
	if (!getIP(ip_))
		return false;
	for (int i = 0; i < 4; i++) {
		if (ip[i] != ip_[i])
			return false;
	}
	return true;
}

bool ZephyrConfig::setPort(uint16_t port) {
	uint8_t hi, lo;
	lo = port % 256;
	hi = (port / 256) % 256;
	Wire.beginTransmission(EEPROM_I2C_ADDR);
	Wire.write(EEPROM_PORT_ADDR);
	Wire.write(hi);
	Wire.write(lo);
	Wire.endTransmission();
	delay(10);

	// Verify
	uint16_t p;
	if (!getPort(&p) || p != port)
		return false;
	return true;
}

void write2Pages(uint8_t addr, uint8_t *data) {
	for (int page_num = 0; page_num < 2; page_num++) {
		Wire.beginTransmission(EEPROM_I2C_ADDR);
		Wire.write(addr + page_num * 16);

		for (int i = 0; i < 16; i++) {
			Wire.write(data[i + page_num * 16]);
		}
		Wire.endTransmission();
		delay(10);
	}
}

bool ZephyrConfig::setHMACKey(uint8_t *key) {
	uint8_t actkey[32];
	write2Pages(EEPROM_HMAC_KEY_ADDR, key);

	// Verify
	if (!getHMACKey(actkey))
		return false;
	for (int i = 0; i < 32; i++) {
		if (actkey[i] != key[i])
			return false;
	}
	return true;
}

bool ZephyrConfig::setLocation(uint8_t *loc) {
	uint16_t len = strlen(loc) + 1;
	if (len >= 32)
		return false;

	write2Pages(EEPROM_LOC_ADDR, loc);

	// Verify
	uint8_t actloc[32];
	if (!getLocation(actloc) || strlen(actloc) != strlen(loc))
		return false;
	for (unsigned int i = 0; i < len; i++) {
		if (actloc[i] != loc[i])
			return false;
	}
	return true;
}

bool ZephyrConfig::setSSID(uint8_t *ssid) {
	uint16_t len = strlen(ssid) + 1;
	if (len >= 32)
		return false;

	write2Pages(EEPROM_SSID_ADDR, ssid);

	// Verify
	uint8_t actssid[32];
	if (!getSSID(actssid) || strlen(actssid) != strlen(ssid))
		return false;
	for (unsigned int i = 0; i < len; i++) {
		if (actssid[i] != ssid[i])
			return false;
	}
	return true;
}

bool ZephyrConfig::setPassword(uint8_t *passwd) {
	uint16_t len = strlen(passwd) + 1;
	if (len >= 32)
		return false;

	write2Pages(EEPROM_PASSWD_ADDR, passwd);

	// Verify
	uint8_t actpasswd[32];
	if (!getPassword(actpasswd) || strlen(actpasswd) != strlen(passwd))
		return false;
	for (unsigned int i = 0; i < len; i++) {
		if (actpasswd[i] != passwd[i])
			return false;
	}
	return true;
}

bool ZephyrConfig::getId(uint32_t *id) {
	(*id) = 0;
	Wire.beginTransmission(EEPROM_I2C_ADDR);
	Wire.write(EEPROM_ID_ADDR);
	Wire.endTransmission();

	Wire.requestFrom(EEPROM_I2C_ADDR, 4);

	for (int i = 0; i < 4; i++) {
		if (wireTimeout())
			return false;
		uint8_t c = Wire.read();
		(*id) <<= 8;
		(*id) |= c;
	}
	return true;
}

bool ZephyrConfig::getIP(uint8_t *ip) {
	Wire.beginTransmission(EEPROM_I2C_ADDR);
	Wire.write(EEPROM_IP_ADDR);
	Wire.endTransmission();

	Wire.requestFrom(EEPROM_I2C_ADDR, 4);

	for (int i = 0; i < 4; i++) {
		if (wireTimeout())
			return false;
		ip[i] = Wire.read();
	}
	return true;
}

bool ZephyrConfig::getPort(uint16_t *port) {
	*port = 0;
	Wire.beginTransmission(EEPROM_I2C_ADDR);
	Wire.write(EEPROM_PORT_ADDR);
	Wire.endTransmission();

	Wire.requestFrom(EEPROM_I2C_ADDR, 2);

	if (wireTimeout())
		return false;
	*port = Wire.read();
	*port <<= 8;
	if (wireTimeout())
		return false;
	*port |= Wire.read();
	return true;
}

bool read2Pages(uint8_t addr, uint8_t *data) {
	for (int page_num = 0; page_num < 2; page_num++) {
		Wire.beginTransmission(EEPROM_I2C_ADDR);
		Wire.write(addr + page_num * 16);
		Wire.endTransmission();

		Wire.requestFrom(EEPROM_I2C_ADDR, 16);

		for (int i = 0; i < 16; i++) {
			if (wireTimeout())
				return false;
			data[i + page_num * 16] = Wire.read();
		}
	}
	return true;
}

bool ZephyrConfig::getHMACKey(uint8_t *binkey) {
	return read2Pages(EEPROM_HMAC_KEY_ADDR, binkey);
}

bool ZephyrConfig::getLocation(uint8_t *loc) {
	if (!read2Pages(EEPROM_LOC_ADDR, loc))
		return false;
	loc[31] = '\0';
	return true;
}

bool ZephyrConfig::getSSID(uint8_t *ssid) {
	if (!read2Pages(EEPROM_SSID_ADDR, ssid))
		return false;
	ssid[31] = '\0';
	return true;
}

bool ZephyrConfig::getPassword(uint8_t *passwd) {
	if (!read2Pages(EEPROM_PASSWD_ADDR, passwd))
		return false;
	passwd[31] = '\0';
	return true;
}

bool ZephyrConfig::getTimeOnce(uint8_t *yr, uint8_t *month, uint8_t *day,
		uint8_t *hr, uint8_t *mn, uint8_t *sc) {
	Wire.beginTransmission(RTC_I2C_ADDR);
	Wire.write(RTC_SEC_REG);
	Wire.endTransmission();

	Wire.requestFrom(RTC_I2C_ADDR, 7);

	if (wireTimeout())
		return false;
	*sc = fromBCD(Wire.read());
	if (wireTimeout())
		return false;
	*mn = fromBCD(Wire.read());
	if (wireTimeout())
		return false;
	*hr = fromBCD(Wire.read());
	if (wireTimeout())
		return false;
	*day = fromBCD(Wire.read());
	if (wireTimeout())
		return false;
	uint8_t _ = Wire.read(); // Dummy read of weekday register
	if (wireTimeout())
		return false;
	*month = fromBCD(Wire.read());
	if (wireTimeout())
		return false;
	*yr = fromBCD(Wire.read());
	return true;
}

bool ZephyrConfig::getConsistentTime(uint8_t *yr, uint8_t *month, uint8_t *day,
		uint8_t *hr, uint8_t *mn, uint8_t *sc) {
	uint8_t yr_ = 0, month_, day_, hr_, mn_, sc_;
	*yr = 1;

	while (*yr != yr_ || *month != month_ || *day != day_ || *hr != hr_
			|| *mn != mn_ || *sc != sc_) {
		if ((!getTimeOnce(yr, month, day, hr, mn, sc))
				|| !getTimeOnce(&yr_, &month_, &day_, &hr_, &mn_, &sc_))
			return false;
	}
	return true;
}
