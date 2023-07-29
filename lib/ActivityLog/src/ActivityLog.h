#ifndef __LOG_H__
#define __LOG_H__

#include <vector>
#include <queue>
#include <deque>

#include <FS.h>
#include <ArduinoJson.h>

struct ActivityLogEntry
{
  time_t time;
  byte status;
};

struct ActivityLogHeader
{
  uint16_t magic;
  uint16_t size;
};

// 200,000 * sizeof(ActivityLogEntry) = 1,600,000 bytes
const int MAX_LOG_ENTRIES = 200000;

const byte STATUS_BOOT = 0;
const byte STATUS_ALIVE = 1;
const byte STATUS_SHUTDOWN = 2;
const byte STATUS_BTN_PRESSED = 3;

template <typename T, int MaxLen, typename Container = std::deque<T>>
class FixedQueue : public std::queue<T, Container>
{
public:
  void push(const T &value)
  {
    if (this->size() == MaxLen)
    {
      this->c.pop_front();
    }
    std::queue<T, Container>::push(value);
  }

  bool emtpy()
  {
    return this->size() == 0;
  }

  typedef typename Container::iterator iterator;
  typedef typename Container::const_iterator const_iterator;

  iterator begin() { return this->c.begin(); }
  iterator end() { return this->c.end(); }
  const_iterator begin() const { return this->c.begin(); }
  const_iterator end() const { return this->c.end(); }
};

class ActivityLog
{
public:
  ActivityLog();
  void addEntry(time_t t, byte status);
  bool save(File &file);
  bool load(File &file);
  void snoop(Print &display, int max_entries);

  template <class PublisherFn>
  bool flush(PublisherFn publisherFn);

private:
  FixedQueue<ActivityLogEntry, MAX_LOG_ENTRIES> entries;
};

static inline const char *statusNameOf(byte status)
{
  switch (status)
  {
  case STATUS_BOOT:
    return "boot";
  case STATUS_ALIVE:
    return "alive";
  case STATUS_SHUTDOWN:
    return "shutdown";
  case STATUS_BTN_PRESSED:
    return "btn_pressed";
  default:
    return "unknown";
  }
}

template <typename PublisherFn>
bool ActivityLog::flush(PublisherFn fn)
{
  char buffer[128];
  while (!this->entries.empty())
  {
    auto entry = this->entries.front();
    StaticJsonDocument<128> doc;
    doc["time"] = entry.time;
    doc["status"] = statusNameOf(entry.status);

    size_t len = serializeJson(doc, buffer, sizeof(buffer));
    if (!fn(buffer, len))
    {
      return false;
    }
    this->entries.pop();
  }

  return true;
}

#endif // __LOG_H__
