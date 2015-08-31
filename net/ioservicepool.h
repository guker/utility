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


} // namespace AsioModel

#endif // _IoServicePool_HPP
