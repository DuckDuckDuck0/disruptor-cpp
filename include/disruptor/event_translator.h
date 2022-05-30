#pragma once

#include <functional>

namespace disruptor {

// T* event, int64_t sequence
template <class T>
using EventTranslator = std::function<void(T*, int64_t)>;

}  // namespace disruptor
