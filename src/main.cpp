#include <M5StickCPlus.h>
#undef min
#undef max

#include <WiFi.h>
#include <EEPROM.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <ActivityLog.h>

#include "local_ssid_define.h"
#include "local_secrets.h"

#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"

// put function declarations here:
void showStatus(time_t now);
bool isPowerAvailable();
bool record(time_t now, uint8_t status);

static ActivityLog activityLog;

static time_t lastSent = 0;
const int SEND_INTERVAL_SEC = 60;

static time_t bootTime = 0;
static auto net = WiFiClientSecure();
static auto client = MQTTClient();

void setup()
{
  M5.begin(
    true, // LCD
    true, // Power
    false // Serial
  );

  // Lcd
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setRotation(3);
  M5.Lcd.printf("Initializing ...\n");

  // Power
  M5.Axp.begin();

  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // ActivityLog
  delay(1000);

  // MQTT
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
  client.begin(AWS_IOT_ENDPOINT, 8883, net);


  M5.Lcd.printf("Initialized.\n");
  delay(500);
}

void loop()
{
  M5.update();
  auto wifiStatus = WiFi.status();
  auto now = time(NULL);

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);

  if (!isPowerAvailable())
  {
    record(now, STATUS_SHUTDOWN);
    // sleep 1 min
    M5.Axp.DeepSleep(1*60*1000*1000);
    return;
  }

  if (wifiStatus == WL_CONNECTED) {
    // time_t: 1000000000 = 2001-09-09 01:46:40 UTC
    if (now < 1000000000) {
      M5.Lcd.printf("Syncing time ... \n");
      configTime(9 * 3600L, 0, "ntp.nict.jp", "time.google.com", "ntp.jst.mfeed.ad.jp");
      delay(1000);
      return;
    }
  }

  if (wifiStatus == WL_CONNECTED) {
    if (now - lastSent > SEND_INTERVAL_SEC)
    {
      // TOBE send to sever
      M5.Lcd.printf("Send to server ... \n");
      lastSent = now;
    }
  }

  if (wifiStatus != WL_CONNECTED) {
    // time_t: 1000000000 = 2001-09-09 01:46:40 UTC
    if (now < 1000000000) {
      M5.Lcd.printf("Connecting to WiFi ... \n");
      delay(1000);
      return;
    }
  }

  if (bootTime == 0) {
    bootTime = now;
    record(now, STATUS_BOOT);
  }

  showStatus(now);
  delay(1000);

}

static const uint8_t POWER_STATUS_ACIN_AVAILABLE = 0x40;
static const uint8_t POWER_STATUS_ACIN_EXISTS = 0x80;
static const uint8_t POWER_STATUS_VBUS_AVAILABLE = 0x10;
static const uint8_t POWER_STATUS_VBUS_EXISTS = 0x20;

bool isPowerAvailable()
{
  auto powerStatus = M5.Axp.Read8bit(0x00);
  return powerStatus & POWER_STATUS_ACIN_AVAILABLE || powerStatus & POWER_STATUS_VBUS_AVAILABLE;
}

// put function definitions here:s
void showStatus(time_t now)
{
  char timeStr[sizeof("YYYY-MM-DD HH:MM:SS")] = {0};
  M5.Lcd.setCursor(0, 0);

  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));
  M5.Lcd.printf("Time: %s\n", timeStr);
  M5.Lcd.printf("WiFi Status: %d\n", WiFi.status());

  // Read Input Power Status
  // auto powerStatus = M5.Axp.GetInputPowerStatus();
  auto powerStatus = M5.Axp.Read8bit(0x00);
  M5.Lcd.printf("Power VBUS: a:%d e:%d\n", powerStatus & POWER_STATUS_VBUS_AVAILABLE ? 1 : 0, powerStatus & POWER_STATUS_VBUS_EXISTS ? 1 : 0);
  M5.Lcd.printf("Power ACIN: a:%d e:%d\n", powerStatus & POWER_STATUS_ACIN_AVAILABLE ? 1 : 0, powerStatus & POWER_STATUS_ACIN_EXISTS ? 1 : 0);

  activityLog.snoop(M5.Lcd, 10);
}

bool record(time_t now, uint8_t status)
{
  activityLog.addEntry(now, status);

  if (WiFi.status() == WL_CONNECTED) {
    // send to server
    M5.Lcd.printf("Send to server ... \n");
    activityLog.flush([](const char* data, int len) {
      client.publish(AWS_IOT_PUBLISH_TOPIC, data, len);
    });
  }

  return true;
}