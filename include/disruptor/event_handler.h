#pragma once

#include <cstdint>

namespace disruptor {

template <class T>
class EventHandler {
 public:
  virtual ~EventHandler() {}

  virtual void OnEvent(T* event, int64_t sequence, bool end_of_batch) = 0;
};

}  // namespace disruptor
