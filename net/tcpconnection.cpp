// ***********************************************************************
// Assembly         : libAsioModel
// Author           : lhmei
// Created          : 10-30-2014
//
// Last Modified By : lhmei
// Last Modified On : 01-06-2015
// ***********************************************************************
// <copyright file="tcpconnection.cpp" company="iflytek">
//     Copyright (c) iflytek. All rights reserved.
// </copyright>
// <summary></summary>
// ***********************************************************************

#include "tcpconnection.h"
#include <boost/bind.hpp>
#include "boost/date_time/posix_time/conversion.hpp"
#include "boost/date_time/microsec_time_clock.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/thread/locks.hpp>

using namespace std;
using namespace boost::gregorian;

namespace AsioModel{

	/// <summary>
	/// Initializes a new tcp connection of the <see cref="TcpConnection"/> class.
	/// </summary>
	/// <param name="io_service">The io_service.</param>
	/// <param name="cb">The cb.</param>
	TcpConnection::TcpConnection(boost::asio::io_service& io_service,
		MessageCallBack cb, 
		TimingWheel<TcpConnection>* tw)
		: socket_(io_service)
		, messageCallBack_(cb)
		, p_timing_wheel_(tw)
	{

	}

	/// <summary>
	/// Sockets of this connection.
	/// </summary>
	/// <returns>boost.asio.ip.tcp.socket &.</returns>
	boost::asio::ip::tcp::socket& TcpConnection::socket()
	{
		return socket_;
	}

	/// <summary>
	/// Starts wait receive message.
	/// </summary>
	void TcpConnection::Start()
	{
		socket_.async_read_some(boost::asio::buffer(readBuffer_),
			boost::bind(&TcpConnection::handle_read, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}

	/// <summary>
	/// Handle reads the data with error code.
	/// </summary>
	/// <param name="e">The error code.</param>
	/// <param name="bytes_transferred">The bytes_transferred.</param>
	void TcpConnection::handle_read(const boost::system::error_code& e,
		std::size_t bytes_transferred)
	{
		if (!e)
		{
			if(bytes_transferred > 0)
			{
				receiveMsgbuffer_.append(readBuffer_.data(),bytes_transferred);
				boost::posix_time::ptime  receiveTime = boost::posix_time::microsec_clock::universal_time();
				bool receAgain = true;

				if (messageCallBack_)
				{
					receAgain = messageCallBack_(shared_from_this(),boost::ref(receiveMsgbuffer_),receiveTime);
				}

				if ( receAgain && socket_.is_open())
				{
					socket_.async_read_some(boost::asio::buffer(readBuffer_),
						boost::bind(&TcpConnection::handle_read, shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
				}

				boost::weak_ptr<WheelEntry<TcpConnection> > weak_ptr(boost::any_cast<boost::weak_ptr<WheelEntry<TcpConnection> > >(any_));

				if(p_timing_wheel_)
				{
					p_timing_wheel_->Active(weak_ptr);
				}
			}
		}
		else
		{
			// TODO
			if (errorCallBack_)
			{
				errorCallBack_(shared_from_this(),e);
			}
			Stop();
		}
	}

	/// <summary>
	/// Handle writes the data with error code.
	/// </summary>
	/// <param name="e">The error code.</param>
	/// <param name="pSendMsg">The point of MSG sent.</param>
	void TcpConnection::handle_write(const boost::system::error_code& e, std::string* pSendMsg)
	{
		if (!e)
		{
			bool callback = false;
			{
				boost::lock_guard<boost::mutex> lock(sendMutex_);
				callback = sendCompleteCallBackList_.front();
			}
			if ( callback && writecompleteCallBack_ )
			{
				writecompleteCallBack_(shared_from_this());
			}
		}
		else
		{
			// TODO
			if (errorCallBack_)
			{
				errorCallBack_(shared_from_this(),e);
			}
		}

		boost::lock_guard<boost::mutex> lock(sendMutex_);
		sendList_.pop_front();
		sendCompleteCallBackList_.pop_front();
		bool bSendListEmpty = sendList_.empty();

		if ( !bSendListEmpty )
		{
			std::string* pNextSendMsg = sendList_.front();
			boost::asio::async_write(socket_,
				boost::asio::buffer(pNextSendMsg->c_str(),pNextSendMsg->length()),
				boost::bind(&TcpConnection::handle_write, shared_from_this(),
				boost::asio::placeholders::error,pNextSendMsg));
		}

		if (pSendMsg)
		{
			delete pSendMsg;
			pSendMsg = NULL;
		}
	}

	/// <summary>
	/// Sends the specified buf.
	/// </summary>
	/// <param name="buf">需要发送数据的指针.</param>
	/// <param name="nLength">发送消息的长度.</param>
	/// <param name="callback">本次发送是否需要设置发送完成的回调.</param>
	void TcpConnection::Send( char* buf, uint32_t nLength,bool callback)
	{
		string msg(buf,nLength);
		Send( msg ,callback);
	}

	/// <summary>
	/// Sends the specified message.
	/// </summary>
	/// <param name="message">需要发送的消息.</param>
	/// <param name="callback">本次发送是否需要设置发送完成的回调.</param>
	void TcpConnection::Send( std::string& message, bool callback)
	{
		std::string* pSendMsg = new std::string;
		pSendMsg->swap(message);
		//pSendMsg->append("\r\n\r\n");
		boost::lock_guard<boost::mutex> lock(sendMutex_);
		if (sendList_.empty())
		{
			sendList_.push_back(pSendMsg);
			sendCompleteCallBackList_.push_back(callback);
			boost::asio::async_write(socket_,
				boost::asio::buffer(pSendMsg->c_str(),pSendMsg->length()),
				boost::bind(&TcpConnection::handle_write, shared_from_this(),
				boost::asio::placeholders::error,pSendMsg));
		}
		else
		{
			sendList_.push_back(pSendMsg);
			sendCompleteCallBackList_.push_back(callback);
		}
	}

	/// <summary>
	/// Sends the specified buf.
	/// </summary>
	/// <param name="buf">待发送的常量字符串指针.</param>
	/// <param name="callback">本次发送是否需要设置发送完成的回调.</param>
	void TcpConnection::Send( const char* buf, bool callback /*= false*/ )
	{
		string msg(buf);
		Send( msg ,callback);
	}

	/// <summary>
	/// 关闭连接.
	/// </summary>
	void TcpConnection::Stop()
	{
		if(socket_.is_open())
		{
		//boost::system::error_code ec;
		//socket_.shutdown(boost::asio::socket_base::shutdown_receive,ec);
			try
			{
				socket_.close();
			}
			catch(...)
			{
				printf("socket_  close  throw error  \n");
			}
		}
	}

} // namespace AsioModel
