#include <M5Stack.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <local_ssid_define.h>
#include <Log.h>

// put function declarations here:
void showStatus();

Log log;

void setup() {
  M5.begin();
  M5.Lcd.init();
  M5.Lcd.printf("Initializing ...\n");
  M5.Power.begin();
  // setup wifi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  log.begin();
  M5.Lcd.printf("Initialized.\n");
}

void loop() {
  M5.update();

  showStatus();

  if (!M5.Power.isCharging()) {
    // shutdown
    M5.Lcd.printf("Shutdown ... \n");
    delay(3000);
    // M5.Lcd.printf("Poweroff.\n");
    M5.Power.powerOFF();
  }
}

// put function definitions here:s
void showStatus() {
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("WiFi Status: %d\n", WiFi.status());
  M5.Lcd.printf("Buttery: %d\n", M5.Power.getBatteryLevel());
  M5.Lcd.printf("Charging: %d\n", M5.Power.isCharging());
  M5.Lcd.printf("EEPROM: %d\n", EEPROM.read(0));
}