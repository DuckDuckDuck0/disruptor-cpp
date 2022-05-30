#pragma once

#include "disruptor/constants.h"

#define LHS_PADDING_FIELD(type, name, initial_value...)    \
  char lhs_padding_##name_[kCachelineSize - sizeof(type)]; \
  type name{initial_value};

#define RHS_PADDING_FIELD(type, name, initial_value...) \
  type name{initial_value};                             \
  char rhs_padding_##name_[kCachelineSize - sizeof(type)];

#define PADDING_FIELD(type, name, initial_value...)        \
  char lhs_padding_##name_[kCachelineSize - sizeof(type)]; \
  type name{initial_value};                                \
  char rhs_padding_##name_[kCachelineSize - sizeof(type)];

#define ADD_CACHELINE_PADDING(name) char name[kCachelineSize];
