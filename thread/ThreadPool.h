#ifndef BASE_THREADPOOL_H
#define BASE_THREADPOOL_H

#include "Thread.h"
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <deque>
#include <string>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

using namespace std;

namespace BaseLib
{
class ThreadPool : boost::noncopyable
{
public:
    typedef boost::function<void ()> Task;

    explicit ThreadPool(const string& name = string());
    ~ThreadPool();

    void start(int numThreads);
    void stop();

    void run(const Task& f);

private:
    void runInThread();
    Task take();

    boost::mutex mutex_;
    boost::condition_variable  cond_;
    string name_;
    boost::ptr_vector<Thread> threads_;
    std::deque<Task> queue_;
    bool running_;
};

}

#endif
