#ifndef TCP_SERVER_H
#define TCP_SERVER_H


#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "ioservicepool.h"
#include "tcpconnection.h"


using namespace std;

namespace AsioModel{

	class TcpServer
		: private boost::noncopyable
	{
	public:
		explicit TcpServer( short port,	short io_service_pool_size,int timeout,
			MessageCallBack messageCallBack,
			WriteCompleteCallBack writecompleteCallBack = NULL,
			AcceptedCallBack acceptedCallBack = NULL,
			ErrorCallBack errorCallBack = NULL);
		explicit TcpServer( short io_service_pool_size,int timeout,
			MessageCallBack messageCallBack,
			WriteCompleteCallBack writecompleteCallBack = NULL,
			AcceptedCallBack acceptedCallBack = NULL,
			ErrorCallBack errorCallBack = NULL);

		bool TcpStart(short port);

		virtual ~TcpServer(void);

		/// 启动服务器
		void Start();

		/// 停止服务器
		void Stop();

		/// <summary>
		/// Sets the message call back.
		/// </summary>
		/// <param name="cb">The cb.</param>
		/// 设置相关事件回调函数
		void SetMessageCallBack(MessageCallBack cb)
		{	messageCallBack_ = cb;	}

		/// <summary>
		/// Sets the write complete call back.
		/// </summary>
		/// <param name="cb">The cb.</param>
		void SetWriteCompleteCallBack(WriteCompleteCallBack cb)
		{	writecompleteCallBack_ = cb;	}

		/// <summary>
		/// Sets the error call back.
		/// </summary>
		/// <param name="cb">The cb.</param>
		void SetErrorCallBack(ErrorCallBack cb)
		{	errorCallBack_ = cb;	}

		/// <summary>
		/// Sets the accepted call back.
		/// </summary>
		/// <param name="cb">The cb.</param>
		void SetAcceptedCallBack(AcceptedCallBack cb)
		{	acceptedCallBack_ = cb;	}

		boost::asio::io_service& get_io_service()
		{
			return ioservicepool_.get_io_service();
		}

	private:
		/// Handle completion of an asynchronous accept operation.
		void handle_accept(const boost::system::error_code& e,  const TcpConnectionPtr conn);

		void handle_timeout(const TcpConnectionPtr &conn);

		IoServicePool ioservicepool_;

		TimingWheel<TcpConnection> timing_wheel_;
		/// Acceptor used to listen for incoming connections.
		boost::asio::ip::tcp::acceptor acceptor_;

		MessageCallBack messageCallBack_;

		WriteCompleteCallBack writecompleteCallBack_;

		AcceptedCallBack acceptedCallBack_;

		ErrorCallBack errorCallBack_;
	};

	typedef boost::shared_ptr<TcpServer> TcpServerPtr;

}


#endif

