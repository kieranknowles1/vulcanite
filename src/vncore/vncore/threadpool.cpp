#include "fmt/format.h"
#include "threadpool.hpp"
#include "vncore/platform.hpp"

#include <fmt/ostream.h>
#include <spdlog/spdlog.h>

namespace selwonk::core {

ThreadPool::ThreadPool(unsigned int threadCount) {
  if (threadCount > 0) {
    SPDLOG_INFO("Spawning {} worker threads", threadCount);
  } else {
    SPDLOG_INFO("Running single-threaded");
  }

  for (int i = 0; i < threadCount; i++) {
    std::thread worker(&ThreadPool::threadFunc, this);
    auto name = fmt::format("VN Worker #{}", i);
    Platform::setThreadName(worker, name.c_str());
    mWorkerThreads.push_back(std::move(worker));
  }
}

ThreadPool::~ThreadPool() {
  quitting = true;
  mJobsCv.notify_all();
  for (auto& thread : mWorkerThreads) {
    thread.join();
  }
}

void ThreadPool::awaitAll() {
  std::unique_lock lock(mJobsMtx);
  mJobsCv.wait(lock, [&] { return mIncompleteJobCount == 0; });
}

void ThreadPool::finalise() {
  std::unique_lock lock(mDoneMtx);
  for (auto& job : mFinishedJobs) {
    job->finalise();
  }
  mFinishedJobs.clear();
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
  std::unique_lock lock(mJobsMtx);
  mJobsCv.wait(lock, [&] { return quitting || !mJobs.empty(); });

  if (quitting) {
    return nullptr;
  }

  auto j = std::move(mJobs.back());
  mJobs.pop_back();
  return j;
}

void ThreadPool::completeJob(std::unique_ptr<Job> job) {
  {
    std::lock_guard lock(mDoneMtx);
    mFinishedJobs.push_back(std::move(job));
    mIncompleteJobCount--;
  }
  // Let the main thread know something's changed
  // notify_one cannot be used as it could be consumed by
  // a worker without anything to do, and would not fall back
  mJobsCv.notify_all();
}

} // namespace selwonk::core
