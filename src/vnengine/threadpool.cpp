#include "threadpool.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace selwonk {

ThreadPool::ThreadPool(unsigned int threadCount) {

  fmt::println("Spawning {} worker threads", threadCount);
  for (int i = 0; i < threadCount; i++) {
    workerThreads.emplace_back(&ThreadPool::threadFunc, this);
  }
}

ThreadPool::~ThreadPool() {
  quitting = true;
  jobsCv.notify_all();
  for (auto& thread : workerThreads) {
    thread.join();
  }
}

void ThreadPool::awaitAll() {
  std::unique_lock lock(jobsMtx);
  jobsCv.wait(lock, [&] { return incompleteJobs == 0; });
}

void ThreadPool::threadFunc() {
  while (true) {
    auto job = getJob();
    if (job != nullptr) {
      (*job)();
      completeJob();
    } else {
      fmt::println("Worker thread {} exiting",
                   fmt::streamed(std::this_thread::get_id()));
      return; // We are quitting
    }
  }
}

std::unique_ptr<ThreadPool::Job> ThreadPool::getJob() {
  std::unique_lock lock(jobsMtx);
  jobsCv.wait(lock, [&] { return quitting || !jobs.empty(); });

  if (quitting) {
    return nullptr;
  }

  auto j = std::move(jobs.back());
  jobs.pop_back();
  return j;
}

void ThreadPool::completeJob() {
  {
    std::lock_guard lock(jobsMtx);
    incompleteJobs--;
  }
  // Let the main thread know something's changed
  // notify_one cannot be used as it could be consumed by
  // a worker without anything to do, and would not fall back
  jobsCv.notify_all();
}

} // namespace selwonk
