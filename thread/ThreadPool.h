// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_THREADPOOL_H
#define MUDUO_BASE_THREADPOOL_H

#include "Thread.h"
#include "stdint.h"
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
		typedef boost::function<int ()> Task; // function object
		typedef boost::function<int ()> UserTaskCallBack; // Added by xiaoli

		explicit ThreadPool(const string& name = string());
		~ThreadPool();

		void start(int32_t numThreads);
		void stop();
		void wait();
		void SetStop();

		void setUserTaskCallBack(UserTaskCallBack cb); // Added by xiaoli
		void run(const Task& f);

	private:
		void runInThread();
		Task take();

		// The User Task Before IO Thread Running
		UserTaskCallBack user_tast_cb_; // Added by xiaoli
		boost::mutex mutex_;
		boost::condition_variable  cond_;
		string name_;
		boost::ptr_vector<Thread> threads_;
		std::deque<Task> queue_;
		bool running_;
	};

}

#endif
