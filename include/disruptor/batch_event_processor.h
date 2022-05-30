#pragma once

#include "disruptor/event_handler.h"
#include "disruptor/ring_buffer.h"
#include "disruptor/sequence_barrier.h"

namespace disruptor {

template <class T>
class BatchEventProcessor {
 public:
  BatchEventProcessor(std::shared_ptr<RingBuffer<T>> ring_buffer,
                      std::shared_ptr<EventHandler<T>> event_handler,
                      const SequencePtrVector& sequences_to_track = {})
      : ring_buffer_(ring_buffer),
        event_handler_(event_handler),
        sequence_barrier_(ring_buffer->NewBarrier(sequences_to_track)) {}

  void ProcessEvents() {
    int64_t next_sequence = sequence_->Load() + 1;

    for (;;) {
      int64_t available_sequence = sequence_barrier_->WaitFor(next_sequence);
      while (next_sequence <= available_sequence) {
        auto& event = (*ring_buffer_)[next_sequence];
        event_handler_->OnEvent(&event, next_sequence,
                                next_sequence == available_sequence);
        next_sequence++;
      }

      sequence_->Store(available_sequence);
    }
  }

  const auto GetSequence() const { return sequence_; }
  auto GetSequenceBarrier() { return sequence_barrier_; }

 private:
  std::shared_ptr<RingBuffer<T>> ring_buffer_{nullptr};
  std::shared_ptr<EventHandler<T>> event_handler_;
  SequencePtr sequence_{Sequence::Create()};
  SequenceBarrierPtr sequence_barrier_;
};

}  // namespace disruptor
