#pragma once

#include <cassert>

#include "disruptor/sequencer.h"

namespace disruptor {

class MultiProducerSequencer : public Sequencer {
 public:
  using Sequencer::Sequencer;

  int64_t Next(int n = 1) final {
    if (n <= 0 || n > buffer_size()) {
      throw std::invalid_argument("n must be > 0 and <= buffer_size");
    }

    int64_t current_sequence;
    int64_t next_sequence;
    int64_t wrap_point;
    int64_t min_seq;
    bool success = false;

    do {
      current_sequence = sequence_.Load();
      next_sequence = current_sequence + n;
      wrap_point = next_sequence - buffer_size();
      min_seq = cached_min_dependent_seq_.Load();

      if (wrap_point > min_seq || min_seq > current_sequence) {
        min_seq = GetMinimumDependentSequence();
        if (wrap_point > min_seq) {
          continue;
        }

        cached_min_dependent_seq_.Store(min_seq);

      } else {
        success = sequence_.CompareAndExchange(current_sequence, next_sequence);
      }
    } while (!success);

    return next_sequence;
  }

  void Publish(int64_t sequence, int n = 1) final {
    int64_t expected_sequence = sequence - n;
    while (cursor_->Load() != expected_sequence) {
      continue;
    }
    cursor_->Store(sequence);
  }

 private:
  Sequence sequence_;
  Sequence cached_min_dependent_seq_;
};

}  // namespace disruptor
