#pragma once

#include "disruptor/sequence.h"
#include "disruptor/utils.h"

namespace disruptor {

class SequenceBarrier {
 public:
  SequenceBarrier(SequencePtr cursor,
                  const SequencePtrVector& dependent_sequences)
      : cursor_(cursor), dependent_sequences_(dependent_sequences) {}

  int64_t WaitFor(int64_t sequence) {
    int64_t available_sequence;
    if (dependent_sequences_.empty()) {
      do {
        available_sequence = cursor_->Load();
      } while (available_sequence < sequence);
    } else {
      do {
        available_sequence = utils::GetMinimumSequence(dependent_sequences_);
      } while (available_sequence < sequence);
    }

    return available_sequence;
  }

 private:
  SequencePtr cursor_;
  SequencePtrVector dependent_sequences_;
};

using SequenceBarrierPtr = std::shared_ptr<SequenceBarrier>;

}  // namespace disruptor
