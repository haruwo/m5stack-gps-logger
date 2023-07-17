#include <ActivityLog.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <local_ssid_define.h>

// put function declarations here:
void showStatus(time_t now);

static ActivityLog activityLog;

static time_t lastSaved = 0;
const int SAVE_INTERVAL_SEC = 5;

static time_t lastSent = 0;
const int SEND_INTERVAL_SEC = 60;


void setup()
{
  M5.begin();
  M5.Lcd.init();
  M5.Lcd.clear();
  M5.Lcd.printf("Initializing ...\n");
  M5.Power.begin();
  // setup wifi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  activityLog.load();
  M5.Lcd.printf("Initialized.\n");
}

void loop()
{
  M5.update();
  M5.Lcd.clear();
  M5.Lcd.setCursor(0, 0);

  auto now = time(NULL);

  auto wifiStatus = WiFi.status();
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

  if (now - lastSaved > SAVE_INTERVAL_SEC)
  {
    activityLog.save();
    lastSaved = now;
  }

  if (!M5.Power.isCharging())
  {
    // shutdown
    M5.Lcd.printf("Shutdown ... \n");
    activityLog.addEntry(STATUS_SHUTDOWN);
    activityLog.save();
    delay(3000);
    // M5.Lcd.printf("Poweroff.\n");
    M5.Power.lightSleep(0);
  }

  showStatus(now);
  delay(1000);
}

// put function definitions here:s
void showStatus(time_t now)
{
  char timeStr[sizeof("YYYY-MM-DD HH:MM:SS")] = {0};
  M5.Lcd.setCursor(0, 0);

  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));
  M5.Lcd.printf("Time: %s\n", timeStr);
  M5.Lcd.printf("WiFi Status: %d\n", WiFi.status());
  M5.Lcd.printf("Buttery: %d\n", M5.Power.getBatteryLevel());
  M5.Lcd.printf("Charging: %s\n", M5.Power.isCharging() ? "yes" : "no");

  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&lastSaved));
  M5.Lcd.printf("Last saved: %s\n", timeStr);

  activityLog.snoop(M5.Lcd, 10);
}