#pragma once

#include <atomic>
#include <memory>
#include <vector>

#include "disruptor/misc.h"
#include "disruptor/padding.h"

namespace disruptor {

class Sequence {
 public:
  constexpr static int64_t kInitialValue = -1;

  static auto Create(int64_t initial_value = kInitialValue) {
    return std::make_shared<Sequence>(initial_value);
  }

  Sequence(int64_t initial_value = kInitialValue) : value_(initial_value) {}

  void Store(int64_t new_value) {
    value_.store(new_value, std::memory_order_release);
  }

  int64_t Load() const { return value_.load(std::memory_order_acquire); }

  int64_t FetchAdd(int64_t n) {
    return value_.fetch_add(n, std::memory_order_release);
  }

  bool CompareAndExchange(int64_t old_value, int64_t new_value) {
    return value_.compare_exchange_weak(old_value, new_value,
                                        std::memory_order_release);
  }

 private:
  PADDING_FIELD(std::atomic<int64_t>, value_, kInitialValue);
};

using SequencePtr = std::shared_ptr<Sequence>;
using SequencePtrVector = std::vector<SequencePtr>;

}  // namespace disruptor
