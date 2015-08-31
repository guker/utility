#include "Thread.h"

namespace BaseLib
{
using namespace boost;

Thread::Thread(const ThreadFunc& func, const string& n)
    : started_(false),
      joined_(false),
      func_(func),
      name_(n)
{

}

Thread::~Thread()
{
    if (started_ && !joined_)
    {
        pThread_->detach();
    }
}

void Thread::start()
{
    if(!started_)
    {
        pThread_.reset(new boost::thread(func_) );
        started_ = true;
    }
}

void Thread::join()
{
    if ( !joined_)
    {
        pThread_->join();
        joined_ = true;
    }
}

}
