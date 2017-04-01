#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ZephyrConfig.h>
#include "screen.h"
#include "dyeus.h"

Dyeus d;

void setup() {
	Serial.begin(115200);
	Wire.begin();
	displayInit();
	display("Zephyr ready!");
	delay(2000);
	if (!d.addSensorData("s0", "12"))
		displayFatalErr("S0 set failed!");
	
	if (!d.addSensorData("s1", 7))
		displayFatalErr("S1 set failed!");
	
	if (!d.addSensorData("s2", 0.123))
		displayFatalErr("S2 set failed!");
	
	if (!d.sendData())
		displayFatalErr("SendData failed!");
}

void loop() {

	uint8_t yr, month, day, hr, mn, sc;
	char tm[32];
	if (!ZephyrConfig::getConsistentTime(&yr, &month, &day, &hr, &mn, &sc)) {
		displayErr("Time read failed!");
	}
	sprintf(tm, "20%2.2d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d", yr, month, day, hr, mn,
			sc);
	display(tm);
	delay(100);
}
