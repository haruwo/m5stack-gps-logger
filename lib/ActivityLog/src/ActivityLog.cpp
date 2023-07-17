#include <ActivityLog.h>
#include <EEPROM.h>

ActivityLog::ActivityLog()
{
}

uint16_t ActivityLog::maxBytes()
{
  return EEPROM_SIZE;
}

void ActivityLog::addEntry(byte status)
{
  ActivityLogEntry entry;
  entry.time = time(NULL);
  entry.status = status;
  this->entries.push(entry);
}

bool ActivityLog::save()
{
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    return false;
  }

  int cur = 0;

  ActivityLogHeader header = {
      .magic = 0xDEAD,
      .size = this->entries.size(),
  };
  EEPROM.put(cur, header);
  cur += sizeof(ActivityLogHeader);

  for (auto it = this->entries.begin(); it != this->entries.end(); ++it)
  {
    EEPROM.put(cur, *it);
    cur += sizeof(ActivityLogEntry);
  }
  if (!EEPROM.commit())
  {
    return false;
  }
  EEPROM.end();
  return true;
}

bool ActivityLog::load()
{
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    return false;
  }

  int cur = 0;
  ActivityLogHeader header;
  EEPROM.get(cur, header);
  cur += sizeof(ActivityLogHeader);
  if (header.magic != 0xDEAD)
  {
    return false;
  }
  if (header.size > MAX_LOG_ENTRIES)
  {
    return false;
  }
  if (!this->entries.emtpy())
  {
    this->entries = FixedQueue<ActivityLogEntry, MAX_LOG_ENTRIES>();
  }
  for (int i = 0; i < header.size; i++)
  {
    ActivityLogEntry entry;
    EEPROM.get(cur, entry);
    cur += sizeof(ActivityLogEntry);
    this->entries.push(entry);
  }
  EEPROM.end();
  return true;
}

void ActivityLog::snoop(Print &display, int max_entries)
{
  int count = 0;
  for (auto it = this->entries.begin(); it != this->entries.end(); ++it)
  {
    count++;
    if (count > max_entries)
    {
      break;
    }

    char timeStr[sizeof("YYYY-MM-DD HH:MM:SS")] = {0};
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&it->time));
    display.printf("time: %s, status: %d\n", timeStr, it->status);
  }
}
