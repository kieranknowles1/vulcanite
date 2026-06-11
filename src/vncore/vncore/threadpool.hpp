#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

namespace selwonk::core {

// Basic thread pool for running short-lived tasks in parallel
// Order of operations is not specified, do not use for time-sensitive actions
// There are no means for prioritizing jobs, cancelling jobs, or waiting for a
// specific job to complete. None of these were needed for the original use case
// (part of a resource manager class)
//
// A number of worker threads are allocated according to threadCount
//
// If destroyed while jobs are still pending, they will be discarded. Incomplete
// jobs will not be finalised
class ThreadPool {
public:
  struct Job {
    virtual ~Job() = default;
    // Execute job on worker thread
    virtual void execute() = 0;
    // Finish job on main thread
    virtual void finalise() {}
  };

  ThreadPool(unsigned int threadCount);
  ~ThreadPool();

  // Wait for all jobs to complete. Be weary of deadlocks
  void awaitAll();

  // Finalise anything that needs to run on the main thread
  void finalise();

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
  std::vector<std::unique_ptr<Job>> finishedJobs;
  // This is NOT the same as jobs.size(), as the latter includes jobs
  // that have been accepted, but are yet to complete
  int incompleteJobs = 0;
  std::mutex jobsMtx;
  // TODO: Does this need to be the same mtx
  std::mutex doneMtx;
  std::condition_variable jobsCv;

  // Entry point for worker threads
  // Fetch and execute jobs until exit
  void threadFunc();

  // Get a job once one is ready, or return nullptr when quitting
  std::unique_ptr<Job> getJob();

  // Callback for when a job is completed
  void completeJob(std::unique_ptr<Job> job);
};

} // namespace selwonk::core
