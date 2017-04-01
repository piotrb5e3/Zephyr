#include <LiquidCrystal_I2C.h>

#include "screen.h"

LiquidCrystal_I2C lcd(0x27, 20, 4);

void displayInit() {
	lcd.init();
	lcd.backlight();
}

void display(const char *msg) {
	lcd.setCursor(0, 0);
	lcd.print(msg);
}

void displayErr(const char *msg) {
	char s[81];
	snprintf(s, 81, "ERR:%s", msg);
	lcd.setCursor(0, 0);
	lcd.print(s);
}

void displayFatalErr(const char *msg) {
	displayErr(msg);
	while (1) {
	}
}
