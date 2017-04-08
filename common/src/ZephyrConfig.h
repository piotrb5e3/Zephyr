#ifndef __ZEPHYR_COMMON_CONFIG_H__
#define __ZEPHYR_COMMON_CONFIG_H__

class ZephyrConfig {
public:
	static bool configureRTC();

	// To be called at the beginning of a minute
	// Resets seconds counter to zero
	static bool setRTC(uint8_t yr, uint8_t mth, uint8_t day, uint8_t weekday,
			uint8_t hr, uint8_t mins);

	// len(key) == 32
	static bool setHMACKey(uint8_t *key);

	static bool setId(uint32_t id);

	// len(ip) == 4
	static bool setIP(uint8_t *ip);

	static bool setPort(uint16_t port);

	// len(loc) <= 31
	static bool setLocation(uint8_t *loc);

	// len(ssid) <= 32
	static bool setSSID(uint8_t *ssid);

	// len(passwd) <= 32
	static bool setPassword(uint8_t *passwd);

	static bool getId(uint32_t *id);

	static bool getIP(uint8_t *ip);

	static bool getPort(uint16_t *port);

	static bool getHMACKey(uint8_t *binkey);

	static bool getLocation(uint8_t *loc);

	static bool getSSID(uint8_t *ssid);

	static bool getPassword(uint8_t *passwd);

	static bool getTimeOnce(uint8_t *yr, uint8_t *month, uint8_t *day,
			uint8_t *hr, uint8_t *mn, uint8_t *sc);

	static bool getConsistentTime(uint8_t *yr, uint8_t *month, uint8_t *day,
			uint8_t *hr, uint8_t *mn, uint8_t *sc);
};

#endif // __ZEPHYR_COMMON_CONFIG_H__
