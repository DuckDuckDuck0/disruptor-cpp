#pragma once

#include <cassert>

#include "disruptor/sequencer.h"

namespace disruptor {

class SingleProducerSequencer : public Sequencer {
 public:
  using Sequencer::Sequencer;

  int64_t Next(int n = 1) final {
    if (n <= 0 || n > buffer_size()) {
      throw std::invalid_argument("n must be > 0 and <= buffer_size");
    }

    auto next_sequence = sequence_.Load() + n;
    auto wrap_point = next_sequence - buffer_size();

    if (wrap_point > cached_min_dependent_seq_) {
      int64_t min_dependent_seq;
      while (wrap_point > (min_dependent_seq = GetMinimumDependentSequence())) {
        // TODO: wait policy
        continue;
      }

      cached_min_dependent_seq_ = min_dependent_seq;
    }

    sequence_.Store(next_sequence);
    return next_sequence;
  }

  void Publish(int64_t sequence, int n = 1) final { cursor_->Store(sequence); }

 private:
  Sequence sequence_;
  int64_t cached_min_dependent_seq_ = Sequence::kInitialValue;
};

}  // namespace disruptor
