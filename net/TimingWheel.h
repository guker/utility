#ifndef _H_TIMINGWHEEL_H_
#define _H_TIMINGWHEEL_H_

#include <boost/circular_buffer.hpp>
#include <boost/unordered_set.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/version.hpp>
#if BOOST_VERSION < 104700
namespace boost
{
	template <typename T>
	inline size_t hash_value(const boost::shared_ptr<T>& x)
	{
		return boost::hash_value(x.get());
	}
}
#endif

template <class T>
class TimeOutCallBackT
{
public:
	explicit TimeOutCallBackT(boost::function<void (boost::shared_ptr<T>&)> cb)
		: timeoutcb_(cb)
	{}
	boost::function<void (boost::shared_ptr<T>&)>& Func(){return timeoutcb_;}
private:
	boost::function<void (boost::shared_ptr<T>&)> timeoutcb_;
};


template<class T>
class WheelEntry
{
public:
	explicit WheelEntry(boost::shared_ptr<T> sharedPtr, TimeOutCallBackT<T> cb)
		: weakPtr_(sharedPtr)
		, cb_(cb)
	{ }
	~WheelEntry()
	{
		boost::shared_ptr<T> TPtr = weakPtr_.lock();
		if (TPtr)
		{
			cb_.Func()(boost::ref(TPtr));
		}
	}
private:
	boost::weak_ptr<T> weakPtr_;
	TimeOutCallBackT<T> cb_;
};

template<class T>
class TimingWheel
{
public:
	explicit TimingWheel(boost::asio::io_service& ios, unsigned int nTimeoutSec)
		: secInterval_(boost::posix_time::seconds((nTimeoutSec/10)>1?(nTimeoutSec/10):1))
		, timerPtr_(new boost::asio::deadline_timer(ios,secInterval_))
		, wheel_(10)
	{
		OnTime();
		bstart_ = true;
	}

	~TimingWheel(void)
	{
		if (bstart_)
		{
			Stop();
		}
	}

	boost::weak_ptr<WheelEntry<T> > Register(boost::shared_ptr<T> t, TimeOutCallBackT<T> cb)
	{
		boost::shared_ptr<WheelEntry<T> > entryptr(new WheelEntry<T>(t,cb));
		{
			boost::lock_guard<boost::mutex> lock(mutex_);
			wheel_.back().insert(entryptr);
		}
		boost::weak_ptr<WheelEntry<T> > weakEntry(entryptr);
		return weakEntry;
	}

	void Active(boost::weak_ptr<WheelEntry<T> > weakEntry)
	{
		boost::shared_ptr<WheelEntry<T> > entry(weakEntry.lock());
		if (entry)
		{
			boost::lock_guard<boost::mutex> lock(mutex_);
			wheel_.back().insert(entry);
		}
	}

	void Stop()
	{
		boost::lock_guard<boost::mutex> lock(mutex_);
		timerPtr_->cancel();
		timerPtr_.reset();
		wheel_.clear();
		bstart_ = false;
	}

protected:

	void OnTime()
	{
		boost::lock_guard<boost::mutex> lock(mutex_);
		boost::unordered_set<boost::shared_ptr<WheelEntry<T> > > Bucket;
		wheel_.push_back(Bucket);
		if(timerPtr_)
		{
			timerPtr_->expires_from_now(secInterval_);
			timerPtr_->async_wait(boost::bind(&TimingWheel<T>::OnTime,this));
		}
	}

private:
	boost::posix_time::seconds secInterval_;
	boost::shared_ptr<boost::asio::deadline_timer> timerPtr_;
	boost::circular_buffer<boost::unordered_set<boost::shared_ptr<WheelEntry<T> > > > wheel_;
	boost::mutex mutex_;
	bool bstart_;
};


#endif	// _H_TIMINGWHEEL_H_
