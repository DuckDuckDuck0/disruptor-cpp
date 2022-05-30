#pragma once

#include "disruptor/ring_buffer.h"

namespace disruptor {

template <class T>
class EventPublisher {
 public:
  explicit EventPublisher(std::shared_ptr<RingBuffer<T>> ring_buffer)
      : ring_buffer_(ring_buffer) {}

  template <class Translator>
  void Publish(Translator&& event_translator) {
    auto sequence = ring_buffer_->Next();
    event_translator(&(*ring_buffer_)[sequence], sequence);
    ring_buffer_->Publish(sequence);
  }

 private:
  std::shared_ptr<RingBuffer<T>> ring_buffer_;
};

}  // namespace disruptor
