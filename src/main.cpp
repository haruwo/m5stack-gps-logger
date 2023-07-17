#include <ActivityLog.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <local_ssid_define.h>

// put function declarations here:
void showStatus();

static ActivityLog activityLog;
static time_t lastSaved = 0;
const int SAVE_INTERVAL_SEC = 5;

void setup()
{
  M5.begin();
  M5.Lcd.init();
  M5.Lcd.printf("Initializing ...\n");
  M5.Power.begin();
  // setup wifi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  activityLog.load();
  activityLog.addEntry(STATUS_BOOT);
  M5.Lcd.printf("Initialized.\n");
}

void loop()
{
  M5.update();

  showStatus();

  auto now = time(NULL);
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
    M5.Power.powerOFF();
  }
}

// put function definitions here:s
void showStatus()
{
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("WiFi Status: %d\n", WiFi.status());
  M5.Lcd.printf("Buttery: %d\n", M5.Power.getBatteryLevel());
  M5.Lcd.printf("Charging: %d\n", M5.Power.isCharging());
  M5.Lcd.printf("EEPROM: %d\n", EEPROM.read(0));

  char timeStr[sizeof("YYYY-MM-DD HH:MM:SS")] = {0};
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&lastSaved));
  M5.Lcd.printf("Last saved: %s\n", timeStr);

  activityLog.snoop(M5.Lcd, 10);
}