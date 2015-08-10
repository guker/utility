
#ifndef BASE_THREAD_H
#define BASE_THREAD_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <string>

using namespace std;

namespace BaseLib
{

	class Thread : boost::noncopyable
	{
	public:
		typedef boost::function<void ()> ThreadFunc;
		typedef boost::shared_ptr<boost::thread> ThreadPtr;

		explicit Thread(const ThreadFunc&, const string& name = string());
		~Thread();

		void start();

		void join();

		bool started() const { return started_; }

		const string& name() const { return name_; }

	private:
		bool       started_;
		bool       joined_;

		ThreadFunc func_;
		string     name_;

		ThreadPtr pThread_;

	};

}
#endif
