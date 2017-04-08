#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ZephyrConfig.h>
#include "screen.h"
#include "dyeus.h"
#include "wifi.h"

Dyeus d;

void setup() {
	Serial.begin(9600);
	d.begin();
	Wire.begin();
	displayInit();

	display("Zephyr ready!");
	delay(2000);
}

void loop() {
	display("Loop START!");
	if (!d.addSensorDataI("s0", analogRead(A0))) {
		displayErr("SET s0 FAILED!");
	} else if (!d.addSensorDataD("s1", analogRead(A1))) {
		displayErr("SET s1 FAILED!");
	} else if (!d.addSensorDataD("zz", analogRead(A2))) {
		displayErr("SET zz FAILED!");
	} else if (!d.sendData()) {
		displayErr("SEND DATA FAILED!");
	} else {
		display("Loop FIN!");
	}
	delay(60000);
}
