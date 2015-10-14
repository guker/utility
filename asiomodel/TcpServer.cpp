﻿#include "TcpServer.h"
#include "boost/asio/ip/tcp.hpp"
#include "boost/bind.hpp"

using boost::asio::ip::tcp;

namespace AsioModel{

	/// <summary>
	/// Initializes a new instance of the <see cref="TcpServer"/> class.
	/// </summary>
	/// <param name="port">The port.</param>
	/// <param name="io_service_pool_size">The io_service_pool_size.</param>
	/// <param name="messageCallBack">The message call back.</param>
	/// <param name="writecompleteCallBack">The writecomplete call back.</param>
	/// <param name="acceptedCallBack">The accepted call back.</param>
	/// <param name="errorCallBack">The error call back.</param>
	TcpServer::TcpServer( short port, short io_service_pool_size , int timeout,
		MessageCallBack messageCallBack , 
		WriteCompleteCallBack writecompleteCallBack /*= NULL*/, 
		AcceptedCallBack acceptedCallBack /* NULL*/,
		ErrorCallBack errorCallBack /*= NULL*/ )
		: ioservicepool_(io_service_pool_size) 
		, ioservicethreadpool_(64)
		, acceptor_(ioservicethreadpool_.get_io_service())
		, timing_wheel_(ioservicethreadpool_.get_io_service(), timeout)
		, messageCallBack_(messageCallBack)
		, writecompleteCallBack_(writecompleteCallBack)
		, acceptedCallBack_(acceptedCallBack)
		, errorCallBack_(errorCallBack)
		, nPort_(port)
	{
	}


	/// <summary>
	/// Finalizes an instance of the <see cref="TcpServer"/> class.
	/// </summary>
	TcpServer::~TcpServer(void)
	{
	}

	/// <summary>
	/// Handle_accepts the specified error.
	/// </summary>
	/// <param name="error">The error.</param>
	/// <param name="conn">The conn.</param>
	void TcpServer::handle_accept( const boost::system::error_code& error, const TcpConnectionPtr conn)
	{
		if (!error)	/// 处理监听事件
		{
			conn->SetMessageCallBack(messageCallBack_);
			conn->SetErrorCallBack(errorCallBack_);
			conn->SetWriteCompleteCallBack(writecompleteCallBack_);
			if (acceptedCallBack_)
			{
				acceptedCallBack_(conn);
			}

			boost::weak_ptr<WheelEntry<TcpConnection> > weak_ptr = timing_wheel_.Register(conn, 
				TimeOutCallBackT<TcpConnection>(boost::bind(&TcpServer::handle_timeout, this, _1)));

			conn->SetContent(boost::any(weak_ptr));

			conn->Start();
			TcpConnectionPtr tcpconnPtr(new TcpConnection(ioservicepool_.get_io_service(), messageCallBack_, &timing_wheel_));
			acceptor_.async_accept(tcpconnPtr->socket(),
				boost::bind(&TcpServer::handle_accept, this,
				boost::asio::placeholders::error,tcpconnPtr));
		}
		else
		{
			if (errorCallBack_)
			{
				errorCallBack_(conn, error);
			}
		}

	}
	void TcpServer::handle_timeout(const TcpConnectionPtr &conn)
	{
		conn->Stop();
	}
	/// <summary>
	/// 启动服务器.
	/// </summary>
	bool TcpServer::Start()
	{
		tcp::endpoint endpoint(tcp::v4(), nPort_);
		acceptor_.open(endpoint.protocol());
		boost::system::error_code error;
		acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
		acceptor_.bind(endpoint,error);
		if(error)
		{
			printf("监听端口失败，端口号：%d  错误信息：%s\n", nPort_,error.message().data() );
			return false;
		}
		acceptor_.listen();
		ioservicethreadpool_.run();
		ioservicepool_.run();
		TcpConnectionPtr tcpconnPtr_(new TcpConnection(ioservicepool_.get_io_service(),messageCallBack_, &timing_wheel_));
		acceptor_.async_accept(tcpconnPtr_->socket(),
			boost::bind(&TcpServer::handle_accept, this,
			boost::asio::placeholders::error, tcpconnPtr_));
		return true;
	}

	/// <summary>
	/// 关闭服务器.
	/// </summary>
	void TcpServer::Stop()
	{
		ioservicethreadpool_.stop();
		ioservicepool_.stop();
	}

}