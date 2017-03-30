#ifndef __ZEPHYR_COMMON_CONFIG_H__
#define __ZEPHYR_COMMON_CONFIG_H__

class ZephyrConfig {
 public:
    static bool configureRTC();

    // To be called at the beginning of a minute
    // Resets seconds counter to zero
    static bool setRTC(uint8_t yr, uint8_t mth, uint8_t day, uint8_t weekday,
                 uint8_t hr, uint8_t mins);

    // len(key) == 16
    static bool setAESKey(uint8_t *key);

    static bool setId(uint32_t id);

    // len(ip) == 4
    static bool setIP(uint8_t *ip);

    static bool setPort(uint16_t port);

    static bool setLocation(uint8_t *loc);

    static bool resetIvCtr();

    static bool getId(uint32_t *id);

    static bool getIP(uint8_t *ip);

    static bool getPort(uint16_t *port);

    static bool getIV(uint8_t *iv);

    static bool getKey(uint8_t *binkey);

    static bool getLocation(uint8_t *loc, uint16_t *len);

    static bool getTimeOnce(uint8_t *yr, uint8_t *month, uint8_t *day, uint8_t *hr, uint8_t *mn, uint8_t *sc);

    static bool getConsistentTime(uint8_t *yr, uint8_t *month, uint8_t *day, uint8_t *hr, uint8_t *mn, uint8_t *sc);
};

#endif // __ZEPHYR_COMMON_CONFIG_H__
