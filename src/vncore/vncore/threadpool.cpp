#include "threadpool.hpp"

#include <fmt/ostream.h>
#include <spdlog/spdlog.h>

namespace selwonk::core {

ThreadPool::ThreadPool(unsigned int threadCount) {

  SPDLOG_INFO("Spawning {} worker threads", threadCount);
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

void ThreadPool::finalise() {
  std::unique_lock lock(doneMtx);
  for (auto& job : finishedJobs) {
    job->finalise();
  }
  finishedJobs.clear();
}

void ThreadPool::threadFunc() {
  while (true) {
    auto job = getJob();
    if (job != nullptr) {
      job->execute();
      completeJob(std::move(job));
    } else {
      SPDLOG_INFO("Worker thread {} exiting",
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

void ThreadPool::completeJob(std::unique_ptr<Job> job) {
  {
    std::lock_guard lock(doneMtx);
    finishedJobs.push_back(std::move(job));
    incompleteJobs--;
  }
  // Let the main thread know something's changed
  // notify_one cannot be used as it could be consumed by
  // a worker without anything to do, and would not fall back
  jobsCv.notify_all();
}

} // namespace selwonk::core
