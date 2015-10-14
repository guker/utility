#ifndef _IoServicePool_HPP
#define _IoServicePool_HPP

#include <boost/asio/io_service.hpp>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "boost/thread.hpp"

namespace AsioModel{

	/// A pool of io_service objects.
	class IoServicePool
		: private boost::noncopyable
	{
	public:
		/// Construct the io_service pool.
		explicit IoServicePool();

		explicit IoServicePool(std::size_t pool_size);

		/// Run all io_service objects in the pool.
		void run();

		/// Stop all io_service objects in the pool.
		void stop();

		void setpoolsize(std::size_t pool_size);

		/// Get an io_service to use.
		boost::asio::io_service& get_io_service();

	private:
		typedef boost::shared_ptr<boost::asio::io_service> io_service_ptr;
		typedef boost::shared_ptr<boost::asio::io_service::work> work_ptr;

		std::vector<boost::shared_ptr<boost::thread> > threads_;

		/// The pool of io_services.
		std::vector<io_service_ptr> io_services_;

		/// The work that keeps the io_services running.
		std::vector<work_ptr> work_;

		/// The next io_service to use for a connection.
		std::size_t next_io_service_;

		std::size_t pool_size_;

		bool running_;
	};

	class IoServiceThreadPool
	{
	public:
		explicit IoServiceThreadPool(std::size_t thread_size)
			: thread_size_(thread_size)
			, work_(io_service_)
		{		
		}
		boost::asio::io_service& get_io_service()
		{
			return  io_service_;
		}
		void run()
		{
			for (std::size_t i = 0; i < thread_size_; ++i)
			{
				boost::shared_ptr<boost::thread> thread(new boost::thread(
					boost::bind(&boost::asio::io_service::run, &io_service_)));
				threads_.push_back(thread);
			}
		}

		void stop()
		{
			io_service_.stop();

			// Wait for all threads in the pool to exit.
			for (std::size_t i = 0; i < threads_.size(); ++i)
				threads_[i]->join();

			threads_.clear();
		}
		
	private:
		std::size_t thread_size_;
		boost::asio::io_service io_service_;
		boost::asio::io_service::work work_;
		std::vector<boost::shared_ptr<boost::thread> > threads_;
	};

} // namespace AsioModel

#endif // _IoServicePool_HPP
