#pragma once

#include <vector>

#include "disruptor/constants.h"
#include "disruptor/multi_producer_sequencer.h"
#include "disruptor/single_producer_sequencer.h"
#include "disruptor/utils.h"

namespace disruptor {

template <class T>
class RingBuffer {
 public:
  static auto Create(int buffer_size, ProducerType producer_type) {
    SequencerPtr sequencer;
    if (producer_type == ProducerType::kSingle) {
      sequencer = std::make_shared<SingleProducerSequencer>(buffer_size);
    } else if (producer_type == ProducerType::kMulti) {
      sequencer = std::make_shared<MultiProducerSequencer>(buffer_size);
    } else {
      throw std::invalid_argument("illegal producer_type");
    }
    return std::make_shared<RingBuffer>(sequencer);
  }

  explicit RingBuffer(SequencerPtr sequencer)
      : sequencer_(sequencer),
        entries_(sequencer->buffer_size(), T{}),
        mask_(sequencer->buffer_size() - 1) {}

  int64_t Next(int n = 1) { return sequencer_->Next(n); }

  void Publish(int64_t sequence, int n = 1) {
    sequencer_->Publish(sequence, n);
  }

  T& operator[](int64_t sequence) { return entries_[sequence & mask_]; }
  const T& operator[](int64_t sequence) const {
    return const_cast<RingBuffer*>(this)->operator[](sequence);
  }

  T& at(int64_t sequence) { return entries_.at(sequence & mask_); }
  const T& at(int64_t sequence) const {
    return const_cast<RingBuffer*>(this)->at(sequence);
  }

  auto NewBarrier(const SequencePtrVector& sequence_to_track = {}) const {
    return sequencer_->NewBarrier(sequence_to_track);
  }

  void set_dependent_sequences(const SequencePtrVector& dependent_sequences) {
    sequencer_->set_dependent_sequences(dependent_sequences);
  }

  void AddDependentSequence(const SequencePtr& sequence) {
    sequencer_->AddDependentSequence(sequence);
  }

  int64_t GetCursor() const { return sequencer_->GetCursor(); }

 private:
  ADD_CACHELINE_PADDING(padding0_);
  SequencerPtr sequencer_;
  std::vector<T> entries_{nullptr};
  int64_t mask_{0};
  ADD_CACHELINE_PADDING(padding1_);
};

}  // namespace disruptor
