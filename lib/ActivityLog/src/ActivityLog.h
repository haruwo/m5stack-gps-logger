#ifndef __LOG_H__
#define __LOG_H__

#include <vector>
#include <queue>
#include <deque>
#include <M5Stack.h>

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

const int MAX_LOG_ENTRIES = 100;
const uint16_t EEPROM_SIZE = sizeof(ActivityLogHeader) + sizeof(ActivityLogEntry) * MAX_LOG_ENTRIES;
const uint16_t EEPROM_ADDR_BOOTLOG = 0;

const byte STATUS_BOOT = 1;
const byte STATUS_SHUTDOWN = 2;

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
  uint16_t maxBytes();
  void addEntry(time_t t, byte status);
  bool save();
  bool load();
  void snoop(Print &display, int max_entries);

private:
  FixedQueue<ActivityLogEntry, MAX_LOG_ENTRIES> entries;
};

#endif