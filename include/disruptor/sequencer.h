#pragma once

#include "disruptor/padding.h"
#include "disruptor/sequence.h"
#include "disruptor/sequence_barrier.h"
#include "disruptor/utils.h"

namespace disruptor {

class Sequencer {
 public:
  Sequencer(int buffer_size) : buffer_size_(buffer_size) {
    if (buffer_size <= 0) {
      throw std::invalid_argument("buffer_size must be greater than 0");
    }

    if (!utils::IsPowerOf2(buffer_size)) {
      throw std::invalid_argument("buffer_size must be a power of 2");
    }
  }

  virtual ~Sequencer() {}

  virtual int64_t Next(int n = 1) = 0;

  virtual void Publish(int64_t sequence, int n = 1) = 0;

  SequenceBarrierPtr NewBarrier(
      const SequencePtrVector& sequence_to_track = {}) const {
    return std::make_shared<SequenceBarrier>(cursor_, sequence_to_track);
  }

  int64_t GetCursor() const { return cursor_->Load(); }

  void AddDependentSequence(const SequencePtr& dependent_sequence) {
    dependent_sequences_.emplace_back(dependent_sequence);
  }

  void set_dependent_sequences(const SequencePtrVector& dependent_sequences) {
    dependent_sequences_ = dependent_sequences;
  }

  int buffer_size() const { return buffer_size_; }

 protected:
  int64_t GetMinimumDependentSequence() const {
    return utils::GetMinimumSequence(dependent_sequences_);
  }

 protected:
  ADD_CACHELINE_PADDING(padding0_);
  int buffer_size_{0};
  SequencePtr cursor_{std::make_shared<Sequence>()};
  SequencePtrVector dependent_sequences_;
  ADD_CACHELINE_PADDING(padding1_);
};

using SequencerPtr = std::shared_ptr<Sequencer>;

}  // namespace disruptor
