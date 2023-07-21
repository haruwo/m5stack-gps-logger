#ifndef __LOG_H__
#define __LOG_H__

#include <vector>
#include <queue>
#include <deque>

#include <FS.h>

#include "ActivityLog.pb.h"

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

const byte STATUS_BOOT = 1;
const byte STATUS_ALIVE = 2;
const byte STATUS_SHUTDOWN = 3;

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

class ActivityLog {
public:
  ActivityLog();
  void addEntry(time_t t, byte status);
  bool save(File& file);
  bool load(File& file);
  void snoop(Print &display, int max_entries);
  
  template<class PublisherFn> bool flush(PublisherFn publisherFn);

private:
  FixedQueue<ActivityLogEntry, MAX_LOG_ENTRIES> entries;
};

template <class PublisherFn>
bool ActivityLog::flush(PublisherFn fn)
{
  pb_byte_t buffer[16];
  while (!this->entries.empty())
  {
    ActivityLogEntry entry = this->entries.front();

    auto msg = _LogEntry();
    msg.time = entry.time;
    switch (entry.status) {
      case STATUS_BOOT:
        msg.status = Status_BOOT;
        break;
      case STATUS_SHUTDOWN:
        msg.status = Status_SHUTDOWN;
        break;
      default:
        msg.status = Status_UNKNONW;
        break;
    }

    auto stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if (!pb_encode(&stream, LogEntry_fields, &msg))
    {
      return false;
    }

    if (!fn((const char *)buffer, (int)stream.bytes_written))
    {  
      return false;
    }
    this->entries.pop();
  }

  return true;
}

#endif // __LOG_H__
