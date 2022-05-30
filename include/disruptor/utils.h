#pragma once

#include <limits>

#include "disruptor/constants.h"
#include "disruptor/sequence.h"

namespace disruptor::utils {

inline int64_t GetMinimumSequence(const SequencePtrVector& sequences) {
  int64_t min_sequence = std::numeric_limits<int64_t>::max();
  for (auto& seq_ptr : sequences) {
    int64_t value = seq_ptr->Load();
    min_sequence = std::min(value, min_sequence);
  }
  return min_sequence;
}

inline bool IsPowerOf2(int x) { return (x & (x - 1)) == 0; }

}  // namespace disruptor::utils
