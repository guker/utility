//
// IoServicePool.cpp
// ~~~~~~~~~~~~~~~~~~~
//
#include "ioservicepool.h"
#include <stdexcept>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

using namespace boost;

namespace AsioModel{

	/// <summary>
	/// Initializes a new instance of the <see cref="IoServicePool"/> class.
	/// </summary>
	/// <param name="pool_size">The pool_size.</param>
	IoServicePool::IoServicePool(std::size_t pool_size)
		: next_io_service_(0)
		, pool_size_(pool_size)
		, running_(false)
	{
		// Create a pool of threads to run all of the io_services.
		if (pool_size_ == 0)
			throw std::runtime_error("IoServicePool size is 0");

		// Give all the io_services work to do so that their run() functions will not
		// exit until they are explicitly stopped.
		for (std::size_t i = 0; i < pool_size_; ++i)
		{
			io_service_ptr io_service(new asio::io_service);
			work_ptr work(new asio::io_service::work(*io_service));
			io_services_.push_back(io_service);
			work_.push_back(work);
		}
	}

	/// <summary>
	/// Initializes a new instance of the <see cref="IoServicePool"/> class.
	/// </summary>
	IoServicePool::IoServicePool()
		: next_io_service_(0)
	{

	}

	/// <summary>
	/// Runs this instance.
	/// </summary>
	void IoServicePool::run()
	{
		// 生效后，不可修改poolsize
		for (std::size_t i = 0; i < io_services_.size(); ++i)
		{
			boost::shared_ptr<boost::thread> thread(new boost::thread(
				boost::bind(&asio::io_service::run, io_services_[i])));
			threads_.push_back(thread);
		}
		running_ = true;
	}


	/// <summary>
	/// Stops this instance.
	/// </summary>
	void IoServicePool::stop()
	{
		// Explicitly stop all io_services.
		for (std::size_t i = 0; i < io_services_.size(); ++i)
			io_services_[i]->stop();

		// Wait for all threads in the pool to exit.
		for (std::size_t i = 0; i < threads_.size(); ++i)
			threads_[i]->join();

		threads_.clear();
		running_ = false;
	}

	/// <summary>
	/// Get one io_service obj from this pool.
	/// </summary>
	/// <returns>boost.asio.io_service &.</returns>
	asio::io_service& IoServicePool::get_io_service()
	{
		// Use a round-robin scheme to choose the next io_service to use.
		asio::io_service& io_service = *io_services_[next_io_service_];
		++next_io_service_;
		if (next_io_service_ == io_services_.size())
			next_io_service_ = 0;
		return io_service;
	}

	/// <summary>
	/// Set the specified pool_size.
	/// </summary>
	/// <param name="pool_size">The pool_size.</param>
	void IoServicePool::setpoolsize( std::size_t pool_size )
	{
		if(running_)
			return;

		std::size_t newsize = pool_size;
		if (pool_size_ <=  newsize)
		{
			pool_size -= pool_size_;
			for (std::size_t i = 0; i < pool_size; ++i)
			{
				io_service_ptr io_service(new asio::io_service);
				work_ptr work(new asio::io_service::work(*io_service));
				io_services_.push_back(io_service);
				work_.push_back(work);
			}
			pool_size_ = newsize;
		}
		else
		{
			threads_.resize(newsize);			
			work_.resize(newsize);
		}
		
	}


} //namespace AsioModel
