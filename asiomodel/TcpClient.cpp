#include "TcpClient.h"
#include "boost/asio/ip/address.hpp"
#include "boost/bind.hpp"
#include "boost/thread/locks.hpp"
#include "boost/ref.hpp"

using namespace boost;
namespace AsioModel{

	/// <summary>
	/// Initializes a new instance of the <see cref="TcpClient"/> class.
	/// </summary>
	/// <param name="ioservice">The boost::io_service obj.</param>
	/// <param name="messageCallBack">The receive message call back.</param>
	/// <param name="connectCallBack">The socket connected call back.</param>
	/// <param name="writecompleteCallBack">The write complete call back.</param>
	/// <param name="errorCallBack">The error call back.</param>
	TcpClient::TcpClient(asio::io_service& ioservice,
		MessageCallBack messageCallBack , 
		ConnectedCallBack connectCallBack,
		WriteCompleteCallBack writecompleteCallBack /*= NULL*/, 
		ErrorCallBack errorCallBack /*= NULL*/ )
		: ioservice_(ioservice)
		, messageCallBack_(messageCallBack)
		, connectCallBack_(connectCallBack)
		, errorCallBack_(errorCallBack)
	{
	}

	/// <summary>
	/// Finalizes an instance of the <see cref="TcpClient"/> class.
	/// </summary>
	TcpClient::~TcpClient(void)
	{
	}

	/// <summary>
	/// 异步TCP连接 ip:port 服务器
	/// </summary>
	/// <param name="ip">The ip.</param>
	/// <param name="port">The port.</param>
	void TcpClient::AsyncConnect( std::string ip, int port )
	{
		conn_.reset(new TcpConnection(ioservice_, messageCallBack_));
		asio::ip::address addr;
		system::error_code error;
		addr = addr.from_string(ip,error);
		if (error)
		{
			errorCallBack_(conn_,error);
			return;
		}
		asio::ip::tcp::endpoint endpoint(addr,port);
		conn_->socket().async_connect(endpoint,
			boost::bind(&TcpClient::handle_connect, this,
			asio::placeholders::error));
	}

	/// <summary>
	/// 同步TCP连接 ip:port 服务器 成功返回true
	/// </summary>
	/// <param name="ip">The ip.</param>
	/// <param name="port">The port.</param>
	/// <returns>bool.</returns>
	bool TcpClient::SynchConnect( std::string ip, int port )
	{
		conn_.reset(new TcpConnection(ioservice_, messageCallBack_));
		asio::ip::address addr;
		system::error_code error;
		addr = addr.from_string(ip,error);
		if (error)
		{
			return false;
		}
		asio::ip::tcp::endpoint endpoint(addr,port);
		conn_->socket().connect(endpoint,error);
		if (error)
		{
			return false;
		}
		conn_->Start();
		return true;
	}

	/// <summary>
	/// 断开TCP连接
	/// </summary>
	///
	void TcpClient::Disconnect()
	{
		conn_->Stop();
	}

	/// <summary>
	/// Handle_connects the specified error.
	/// </summary>
	/// <param name="error">The error.</param>
	void TcpClient::handle_connect(const system::error_code& error)
	{
		if (!error)
		{
			boost::lock_guard<mutex> lock(mutex_);

			conn_->SetErrorCallBack(errorCallBack_);
			conn_->SetMessageCallBack(messageCallBack_);
			conn_->SetWriteCompleteCallBack(writecompleteCallBack_);
			conn_->Start();

			if(connectCallBack_)
			{
				connectCallBack_(conn_);
			}
		}
		else
		{
			if (errorCallBack_)
			{
				errorCallBack_(conn_,error);
			}
		}
	}

	/// <summary>
	/// 获取TCP连接对象引用，可执行发送操作
	/// </summary>
	/// <returns>TcpConnectionPtr &.</returns>
	TcpConnectionPtr& TcpClient::GetTcpConnection()
	{
		return conn_;
	}

}