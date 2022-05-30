#include <chrono>
#include <iostream>
#include <thread>

#include "disruptor/batch_event_processor.h"
#include "disruptor/event_publisher.h"
#include "disruptor/ring_buffer.h"
#include "disruptor/sequence_barrier.h"
#include "disruptor/single_producer_sequencer.h"

using namespace disruptor;
using namespace std::chrono;

struct StubEvent {
  ADD_CACHELINE_PADDING(padding0);
  int64_t value = 0;
  ADD_CACHELINE_PADDING(padding1);
};

class StubEventHandler : public EventHandler<StubEvent> {
 public:
  StubEventHandler(int handler_id) : handler_id_(handler_id) {}
  void OnEvent(StubEvent* event, int64_t sequence, bool end_of_batch) final {
    event_.value = event->value;
  }

 private:
  int handler_id_{0};
  StubEvent event_;
};

int main() {
  int64_t iteration = 1'000'000 * 100;

  using RB = RingBuffer<StubEvent>;
  using Processor = BatchEventProcessor<StubEvent>;
  auto rb = RB::Create(8192, ProducerType::kSingle);

  auto processor_1 =
      std::make_shared<Processor>(rb, std::make_shared<StubEventHandler>(1));
  auto processor_2 = std::make_shared<Processor>(
      rb, std::make_shared<StubEventHandler>(2),
      SequencePtrVector{processor_1->GetSequence()});
  auto processor_3 = std::make_shared<Processor>(
      rb, std::make_shared<StubEventHandler>(3),
      SequencePtrVector{processor_2->GetSequence()});
  rb->AddDependentSequence(processor_3->GetSequence());

  std::jthread worker_thread_1([=] { processor_1->ProcessEvents(); });
  std::jthread worker_thread_2([=] { processor_2->ProcessEvents(); });
  std::jthread worker_thread_3([=] { processor_3->ProcessEvents(); });

  EventPublisher<StubEvent> publisher(rb);
  auto tic = steady_clock::now();
  for (int64_t i = 0; i < iteration; ++i) {
    const auto translator = [](StubEvent* event, int64_t sequence) {
      event->value = sequence;
    };
    publisher.Publish(translator);
  }

  auto expected_sequence = rb->GetCursor();
  while (processor_3->GetSequence()->Load() < expected_sequence) {
    std::this_thread::yield();
  }
  auto toc = steady_clock::now();
  std::cout << iteration / ((toc - tic).count() / 1e9) << " ops/s\n";

  exit(0);
}
