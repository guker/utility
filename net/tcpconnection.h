// ***********************************************************************
// Assembly         : libAsioModel
// Author           : lhmei
// Created          : 10-30-2014
//
// Last Modified By : lhmei
// Last Modified On : 01-06-2015
// ***********************************************************************
// <copyright file="tcpconnection.h" company="iflytek">
//     Copyright (c) iflytek. All rights reserved.
// </copyright>
// <summary></summary>
// ***********************************************************************

#ifndef tcpconnection_h__
#define tcpconnection_h__

#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/system/error_code.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/any.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <string>
#include <deque>
#include <list>
#include <vector>
#include "../buffer/Buffer.h"
#include "TimingWheel.h"

using namespace std;

namespace AsioModel{

	class TcpConnection;
	typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;

	typedef std::deque<TcpConnectionPtr> TcpConnectionList;

	typedef boost::function<bool (const TcpConnectionPtr&, 
		BaseLib::Buffer&, boost::posix_time::ptime )> MessageCallBack;

	typedef boost::function<void (const TcpConnectionPtr&)> WriteCompleteCallBack;

	typedef boost::function<void (const TcpConnectionPtr&, boost::system::error_code error)> ErrorCallBack;

	typedef boost::function<void (const TcpConnectionPtr&)> ConnectedCallBack;

	typedef boost::function<void (const TcpConnectionPtr&)> AcceptedCallBack;

	/// <summary>
	/// Class TcpConnection
	/// </summary>
	class TcpConnection
		: public boost::enable_shared_from_this<TcpConnection>,
		private boost::noncopyable
	{
	public:

		explicit TcpConnection(boost::asio::io_service& io_service,
			MessageCallBack cb, TimingWheel<TcpConnection>* tw = NULL);

		/// <summary>
		/// Finalizes an instance of the <see cref="TcpConnection" /> class.
		/// </summary>
		~TcpConnection()
		{
			Stop();
		}

		/// 获取关联的Socket对象的引用
		boost::asio::ip::tcp::socket& socket();

		/// 启动TCP连接，异步启动接收数据
		void Start();

		/// 关闭TCP连接
		void Stop();

		/// 发送内存数据 callback决定发送成功后是否回调通知WriteCompleteCallBack
		void Send(char* buf, uint32_t nLength, bool callback = false);

		void Send(std::string& message, bool callback = false);

		void Send( const char* buf, bool callback = false);

		/// <summary>
		/// Sets the message call back.
		/// </summary>
		/// <param name="cb">The cb.</param>
		void SetMessageCallBack(MessageCallBack cb)
		{	messageCallBack_ = cb;	}

		/// <summary>
		/// Sets the write complete call back.
		/// </summary>
		/// <param name="cb">The cb.</param>
		void SetWriteCompleteCallBack(WriteCompleteCallBack cb)
		{	writecompleteCallBack_ = cb;	}

		/// <summary>
		/// 设置错误事件回调.
		/// </summary>
		/// <param name="cb">The cb.</param>
		void SetErrorCallBack(ErrorCallBack cb)
		{	errorCallBack_ = cb;	}


		/// <summary>
		/// 设置与连接绑定的用户数据对象.
		/// </summary>
		/// <param name="any">Any.</param>
		void SetContent(boost::any any)
		{
			any_ = any;
		}

		/// <summary>
		/// 获取与连接绑定的用户数据对象.
		/// </summary>
		/// <returns>boost.any &.</returns>
		boost::any& GetContent()
		{
			return any_;
		}
		
	private:
		void handle_read(const boost::system::error_code& e,
			std::size_t bytes_transferred);

		/// Handle completion of a write operation.
		void handle_write(const boost::system::error_code& e,std::string* pSendMsg);

		/// <summary>
		/// The socket_
		/// </summary>
		/// Socket for the connection.
		boost::asio::ip::tcp::socket socket_;

		/// <summary>
		/// The read buffer_
		/// </summary>
		/// Buffer for incoming data.

		boost::array<char, 2048> readBuffer_;

		/// <summary>
		/// The receive msgbuffer_
		/// </summary>
		BaseLib::Buffer receiveMsgbuffer_;

		/// <summary>
		/// The send list_
		/// </summary>
		std::deque<std::string*>	sendList_;

		/// <summary>
		/// The send complete call back list_
		/// </summary>
		std::deque<bool>	sendCompleteCallBackList_;

		/// <summary>
		/// The mutex for send list
		/// </summary>
		boost::mutex sendMutex_;

		/// <summary>
		/// The message call back_
		/// </summary>
		MessageCallBack messageCallBack_;

		/// <summary>
		/// The write complete call back_
		/// </summary>
		WriteCompleteCallBack writecompleteCallBack_;

		/// <summary>
		/// The error call back_
		/// </summary>
		ErrorCallBack errorCallBack_;

		/// <summary>
		/// The any_
		/// </summary>
		boost::any any_;

		/// <summary>
		/// The timewheel_
		/// </summary>
		TimingWheel<TcpConnection> *p_timing_wheel_;
	};

} // namespace AsioModel
#endif // tcpconnection_h__
