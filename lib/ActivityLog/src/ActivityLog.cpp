#include "ActivityLog.h"
#include <FS.h>
#include <pb.h>

ActivityLog::ActivityLog()
{
}

void ActivityLog::addEntry(time_t t, byte status)
{
  ActivityLogEntry entry;
  entry.time = t;
  entry.status = status;
  this->entries.push(entry);
}

bool ActivityLog::save(File &file)
{
  ActivityLogHeader header = {0xDEAD, (uint16_t)this->entries.size()};
  file.write((uint8_t *)&header, sizeof(header));
  for (auto it = this->entries.begin(); it != this->entries.end(); ++it)
  {
    file.write((uint8_t *)&(*it), sizeof(ActivityLogEntry));
  }
  return true;
}

bool ActivityLog::load(File &file)
{
  ActivityLogHeader header;
  file.read((uint8_t *)&header, sizeof(header));
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
    file.read((uint8_t *)&entry, sizeof(ActivityLogEntry));
    this->entries.push(entry);
  }
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

    const char *statusStr;
    switch (it->status)
    {
    case STATUS_BOOT:
      statusStr = "boot";
      break;
    case STATUS_SHUTDOWN:
      statusStr = "shutdown";
      break;
    default:
      statusStr = "unknown";
      break;
    }

    display.printf("time: %s, status: %s\n", timeStr, statusStr);
  }
}
