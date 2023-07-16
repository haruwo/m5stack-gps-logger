#ifndef __LOG_H__
#define __LOG_H__

#include <vector>
#include <M5Stack.h>

struct LogEntry {
  time_t time;
  byte status;
};

const int MAX_LOG_ENTRIES = 100;
const uint16_t EEPROM_SIZE = sizeof(LogEntry) * MAX_LOG_ENTRIES;
const uint16_t EEPROM_ADDR_BOOTLOG = 0;

const byte STATUS_BOOT = 1;
const byte STATUS_SHUTDOWN = 2;

class Log {
  public:
    Log();
    void begin();
    void addEntry(byte status);
    void save();
    void load();
  private:
    std::vector<LogEntry> entries;
};

#endif