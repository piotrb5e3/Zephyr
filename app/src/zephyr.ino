#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ZephyrConfig.h>

LiquidCrystal_I2C lcd(0x27,20,4);

void setup() {
  Serial.begin(9600);
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("PlantDuino ready!");
  delay(2000);
}

void loop() {
  uint8_t yr, month, day, hr, mn, sc;
  char tm[32];
  if(!ZephyrConfig::getConsistentTime(&yr, &month, &day, &hr, &mn, &sc)) {
    displayErr("Time read failed!");
  }
  sprintf(tm, "20%2.2d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d", yr, month, day, hr, mn, sc);
  lcd.setCursor(0,0);
  lcd.print(tm);
  delay(100);
}

void displayErr(const char *msg) {
  lcd.setCursor(0,0);
  lcd.print(msg);
  while(1);
}

