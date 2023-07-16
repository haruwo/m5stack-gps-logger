#include <M5Stack.h>
#include <WiFi.h>
#include <local_ssid_define.h>

// put function declarations here:

void setup() {
  M5.begin();
  M5.Lcd.init();
  M5.Lcd.printf("Hello world\n");
  // setup wifi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  // TOBE
}

void loop() {
  M5.update();

  float batCur = M5.Power.getBatteryLevel();

  M5.Lcd.setCursor(0, 156);
  M5.Lcd.printf("Baterry: %.3f\n", batCur);

  // if (batCur < 0) {
  //   delay(500);
  //   M5.Lcd.setBrightness(0);
  //   M5.Lcd.sleep();
  //   delay(500);
  //   M5.Lcd.wakeup();
  //   M5.Lcd.setBrightness(128);
  // }
  // put your ain code here, to run repeatedly:
}

// put function definitions here:s