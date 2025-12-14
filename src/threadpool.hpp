#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace selwonk {

// Basic thread pool for running short-lived tasks in parallel
// Order of operations is not specified, do not use for time-sensitive actions
// There are no means for prioritizing jobs, cancelling jobs, or waiting for a
// specific job to complete. None of these were needed for the original use case
// (part of a resource manager class)
//
// A number of worker threads are allocated according to threadCount
//
// If destroyed while jobs are still pending, they will be discarded
class ThreadPool {
public:
  // Base class for any job. Extend this to use
  struct Job {
    virtual ~Job() = default;
    virtual void operator()() = 0;
  };

  ThreadPool(unsigned int threadCount);
  ~ThreadPool();

  // Wait for all jobs to complete. Be weary of deadlocks
  void awaitAll();

  void addJob(std::unique_ptr<Job> job) {
    {
      std::lock_guard lock(jobsMtx);
      jobs.push_back(std::move(job));
      incompleteJobs++;
    }
    // Wake up a worker thread to complete the job
    jobsCv.notify_one();
  }

protected:
  bool quitting = false;
  std::vector<std::thread> workerThreads;
  std::vector<std::unique_ptr<Job>> jobs;
  // This is NOT the same as jobs.size(), as the latter includes jobs
  // that have been accepted, but are yet to complete
  int incompleteJobs = 0;
  std::mutex jobsMtx;
  std::condition_variable jobsCv;

  // Entry point for worker threads
  // Fetch and execute jobs until exit
  void threadFunc();

  // Get a job once one is ready, or return nullptr when quitting
  std::unique_ptr<Job> getJob();

  // Callback for when a job is completed
  void completeJob();
};

} // namespace selwonk
