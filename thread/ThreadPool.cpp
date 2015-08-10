#include "ThreadPool.h"
#include <boost/bind.hpp>
#include <assert.h>
#include <stdio.h>
#include <boost/thread/locks.hpp>

using namespace BaseLib;

namespace BaseLib
{
	ThreadPool::ThreadPool(const string& name)
		: mutex_()
		, name_(name)
		, running_(false)
	{
	}

	ThreadPool::~ThreadPool()
	{
		if (running_)
		{
			stop();
		}
	}

	void ThreadPool::start(int32_t numThreads)
	{
		assert(threads_.empty());
		running_ = true;
		threads_.reserve(numThreads);
		for (int32_t i = 0; i < numThreads; ++i)
		{
			char id[32];
#ifdef WIN32
            sprintf_s(id,sizeof id, "%d",i);
#else
			snprintf(id, sizeof id, "%d", i);
#endif // WIN32

			threads_.push_back(new Thread(
				boost::bind(&ThreadPool::runInThread, this), name_+id));
			threads_[i].start();
		}
	}

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

	void ThreadPool::wait()
	{
		for_each(threads_.begin(),
			threads_.end(),
			boost::bind(&Thread::join, _1));
	}

	void ThreadPool::SetStop()
	{
		{
			boost::lock_guard<boost::mutex> lock(mutex_);
			running_ = false;
			cond_.notify_all();
		}
	}

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

	// Added by xiaoli
	void ThreadPool::setUserTaskCallBack(UserTaskCallBack cb)
	{
		user_tast_cb_ = cb;
	}
	// End

	void ThreadPool::runInThread()
	{
		// Added by xiaoli
		if (user_tast_cb_)
		{
			user_tast_cb_();
		}
		// End

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
