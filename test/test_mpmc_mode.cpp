#include <iostream>
#include <thread>

#include "disruptor/batch_event_processor.h"
#include "disruptor/event_publisher.h"
#include "disruptor/ring_buffer.h"
#include "disruptor/sequence_barrier.h"
#include "disruptor/single_producer_sequencer.h"

using namespace disruptor;

const int64_t iteration_per_prod = 1'000'000 * 1;
const int64_t prod_num = 1;
const int64_t cons_num = 1;

struct StubEvent {
  ADD_CACHELINE_PADDING(padding0);
  int64_t value = 0;
  ADD_CACHELINE_PADDING(padding1);
};

class StubEventHandler : public EventHandler<StubEvent> {
 public:
  StubEventHandler(int handler_id) : handler_id_(handler_id) {
    results.resize(prod_num * iteration_per_prod, 0);
  }

  void OnEvent(StubEvent* event, int64_t sequence, bool end_of_batch) final {
    results.at(sequence)++;
  }

  bool Check() {
    for (auto count : results) {
      if (count != 1) {
        return false;
      }
    }
    return true;
  }

 private:
  int handler_id_{0};
  StubEvent event_;
  std::vector<int> results;
};

int main() {
  using RB = RingBuffer<StubEvent>;
  using Processor = BatchEventProcessor<StubEvent>;
  auto rb = RB::Create(8192, ProducerType::kMulti);

  std::vector<std::jthread> cons_threads;
  std::vector<std::shared_ptr<StubEventHandler>> handlers;
  for (int i = 0; i < cons_num; ++i) {
    auto handler = std::make_shared<StubEventHandler>(i);
    handlers.emplace_back(handler);

    auto processor = std::make_shared<Processor>(rb, handler);
    rb->AddDependentSequence(processor->GetSequence());

    cons_threads.emplace_back([=] { processor->ProcessEvents(); });
  }

  auto prod_func = [=](int64_t start, int id) {
    EventPublisher<StubEvent> publisher(rb);
    for (int64_t i = start; i < start + iteration_per_prod; ++i) {
      publisher.Publish(
          [=](StubEvent* event, int64_t sequence) { event->value = i; });
    }
  };

  std::vector<std::jthread> prod_threads;
  for (auto i = 0; i < prod_num; ++i) {
    prod_threads.emplace_back(prod_func, i * iteration_per_prod, i);
  }

  while (rb->GetCursor() < iteration_per_prod * prod_num - 1) {
    std::this_thread::yield();
    // std::cout << rb->GetCursor() << std::endl;
  }

  for (auto& handler : handlers) {
    if (!handler->Check()) {
      std::cout << "failed\n";
      std::exit(EXIT_FAILURE);
    }
  }

  std::cout << "passed\n";

  std::exit(EXIT_SUCCESS);
}
