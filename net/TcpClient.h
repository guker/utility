// ***********************************************************************
// Assembly         : libAsioModel
// Author           : lhmei
// Created          : 10-30-2014
//
// Last Modified By : lhmei
// Last Modified On : 10-22-2014
// ***********************************************************************
// <copyright file="TcpClient.h" company="iflytek">
//     Copyright (c) iflytek. All rights reserved.
// </copyright>
// <summary></summary>
// ***********************************************************************
#pragma once
#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "tcpconnection.h"
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include "boost/system/error_code.hpp"
#include "boost/thread/mutex.hpp"
#include "ioservicepool.h"
#include "boost/noncopyable.hpp"
#include "boost/asio/io_service.hpp"
#include "boost/smart_ptr/shared_ptr.hpp"

namespace AsioModel{

	/// <summary>
	/// Class TcpClient
	/// </summary>
	class TcpClient
		: private boost::noncopyable
	{
	public:
		TcpClient(boost::asio::io_service& ioservice,
			MessageCallBack messageCallBack,
			ConnectedCallBack connectCallBack = NULL,
			WriteCompleteCallBack writecompleteCallBack = NULL,
			ErrorCallBack errorCallBack = NULL);

		~TcpClient(void);

		/// 异步TCP连接 ip:port 服务器
		void AsyncConnect(std::string ip, int port);

		/// 同步TCP连接 ip:port 服务器 成功返回true
		bool SynchConnect(std::string ip, int port);

		/// 断开TCP连接
		void Disconnect();

		/// 获取TCP连接对象引用，可执行发送操作
		TcpConnectionPtr& GetTcpConnection();

		/// <summary>
		/// Sets the message call back.
		/// </summary>
		/// <param name="cb">The cb.</param>
		/// 设置事件回调处理
		void SetMessageCallBack(MessageCallBack cb)
		{	messageCallBack_ = cb;	if (conn_)	conn_->SetMessageCallBack(cb);	}

		/// <summary>
		/// Sets the write complete call back.
		/// </summary>
		/// <param name="cb">The cb.</param>
		void SetWriteCompleteCallBack(WriteCompleteCallBack cb)
		{	writecompleteCallBack_ = cb; if (conn_) conn_->SetWriteCompleteCallBack( cb );	}

		/// <summary>
		/// Sets the error call back.
		/// </summary>
		/// <param name="cb">The cb.</param>
		void SetErrorCallBack(ErrorCallBack cb)
		{	errorCallBack_ = cb; if (conn_) conn_->SetErrorCallBack(cb);}

		/// <summary>
		/// Sets the connected call back.
		/// </summary>
		/// <param name="cb">The cb.</param>
		void SetConnectedCallBack(ConnectedCallBack cb)
		{	connectCallBack_ = cb;}

	protected:

		void TcpClient::handle_connect(const boost::system::error_code& error);

	private:

		/// <summary>
		/// The mutex_
		/// </summary>
		boost::mutex mutex_;

		/// <summary>
		/// The ioservice_
		/// </summary>
		boost::asio::io_service& ioservice_;

		/// <summary>
		/// The conn_
		/// </summary>
		TcpConnectionPtr conn_;

		/// <summary>
		/// The message call back_
		/// </summary>
		MessageCallBack messageCallBack_;

		/// <summary>
		/// The writecomplete call back_
		/// </summary>
		WriteCompleteCallBack writecompleteCallBack_;

		/// <summary>
		/// The connect call back_
		/// </summary>
		ConnectedCallBack connectCallBack_;

		/// <summary>
		/// The error call back_
		/// </summary>
		ErrorCallBack errorCallBack_;
	};
}

#endif //TCP_CLIENT_H
