#include "ThreadPool.h"
#include <boost/bind.hpp>
#include <assert.h>
#include <stdio.h>
#include <boost/thread/locks.hpp>

using namespace BaseLib;

namespace BaseLib
{
	/// <summary>
	/// Initializes a new instance of the <see cref="ThreadPool"/> class.
	/// </summary>
	/// <param name="name">The name.</param>
	ThreadPool::ThreadPool(const string& name)
		: mutex_()
		, name_(name)
		, running_(false)
	{
	}

	/// <summary>
	/// Finalizes an instance of the <see cref="ThreadPool"/> class.
	/// </summary>
	ThreadPool::~ThreadPool()
	{
		if (running_)
		{
			stop();
		}
	}

	/// <summary>
	/// Starts the specified num threads.
	/// </summary>
	/// <param name="numThreads">The num of threads.</param>
	void ThreadPool::start(int numThreads)
	{
		assert(threads_.empty());
		running_ = true;
		threads_.reserve(numThreads);
		for (int i = 0; i < numThreads; ++i)
		{
			char id[32];
#ifdef WIN32
			sprintf_s(id,sizeof id, "%s",i);
#else
			snprintf(id, sizeof id, "%d", i);
#endif // WIN32

			threads_.push_back(new Thread(
				boost::bind(&ThreadPool::runInThread, this), name_+id));
			threads_[i].start();
		}
	}

	/// <summary>
	/// Stops this instance.
	/// </summary>
	void ThreadPool::stop()
	{
		{
			boost::lock_guard<boost::mutex> lock(mutex_);
			running_ = false;
			cond_.notify_all();
		}
		for_each(threads_.begin(),
			threads_.end(),
			boost::bind(&Thread::join, _1));
	}

	/// <summary>
	/// Runs the specified task.
	/// </summary>
	/// <param name="task">The task obj.</param>
	void ThreadPool::run(const Task& task)
	{
		if (threads_.empty())
		{
			task();
		}
		else
		{
			boost::lock_guard<boost::mutex> lock(mutex_);
			queue_.push_back(task);
			cond_.notify_one();
		}
	}

	/// <summary>
	/// 从任务队列中获取一个任务.
	/// </summary>
	/// <returns>ThreadPool.Task.</returns>
	ThreadPool::Task ThreadPool::take()
	{
		boost::unique_lock<boost::mutex> lock(mutex_);
		// always use a while-loop, due to spurious wakeup
		while (queue_.empty() && running_)
		{
			cond_.wait(lock);
		}
		Task task;
		if(!queue_.empty())
		{
			task = queue_.front();
			queue_.pop_front();
		}
		return task;
	}

	void ThreadPool::runInThread()
	{
		try
		{
			while (running_)
			{
				Task task(take());
				if (task)
				{
					task();
				}
			}
		}
		catch (const std::exception& ex)
		{
			fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
			fprintf(stderr, "reason: %s\n", ex.what());
			abort();
		}
		catch (...)
		{
			fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
			throw; // rethrow
		}
	}

}
