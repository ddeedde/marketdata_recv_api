#include "UdpRecv.h"
#include "LogWrapper.h"

UdpRecv::UdpRecv(boost::asio::io_service& io, const std::string& ip, int port, const std::string& token)
	: udp_io_service_(io)
	, socket_(io)
	, tmp_endpoint_(boost::asio::ip::address::from_string(ip), port)
	, heartbeat_check_timer_(udp_io_service_)
	, address_(ip)
	, port_(port)
	, token_(token)
	, recv_count_(0)
	, marketdata_count_(0)
{
	//初始启动定时器
	heartbeat_check_timer_.expires_from_now(boost::posix_time::seconds(15));
	heartbeat_check_timer_.async_wait(boost::bind(&UdpRecv::check_heartbeat_timeout, this, boost::asio::placeholders::error));
}

UdpRecv::~UdpRecv()
{

}

void UdpRecv::stop()
{
	heartbeat_check_timer_.cancel();
	//if (!udp_io_service_.stopped())
	//{
	//	udp_io_service_.stop();
	//}
	boost::system::error_code ignored_ec;
	try {
		socket_.close();
	}
	catch (std::exception& e) {
		LOGE("关闭UDP socket错误" << e.what());
	}
	//udp_run_thread_->interrupt();
}


bool UdpRecv::start(onDataArrive c)
{
	try
	{
		if (!socket_.is_open()) {
			socket_.open(tmp_endpoint_.protocol());
		}
		boost::asio::socket_base::receive_buffer_size option_recv(80 * 1024 * 1024);
		socket_.set_option(option_recv);
		//socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
		//socket_.bind(endPoint);

		add_token();
	}
	catch (std::exception& e)
	{
		LOGE( "UDP Sender 启动失败: " << e.what() );
		return false;
	}
	LOGD( "UDP Sender 启动成功，端口号: " << port_ << ", 地址: " << address_ );
	callDataArrive = c;
	receive(); //开始接收
	return true;
}

void UdpRecv::receive()
{
	memset(data_, 0, sizeof(data_));
	socket_.async_receive_from(
		boost::asio::buffer(data_, sizeof(data_)), tmp_endpoint_,
		boost::bind(&UdpRecv::handle_receive_from, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void UdpRecv::handle_receive_from(const boost::system::error_code& error, size_t bytes_transferred)
{
	if (!error)
	{
		//LOGA(bytes_transferred);
		++recv_count_;
		if (bytes_transferred == sizeof(MarketData))
		{
			++marketdata_count_;
			MarketData *md = new MarketData();
			memcpy(md,data_,sizeof(MarketData));	

			callDataArrive(md);
		}
		//else if (bytes_transferred == sizeof(MarketDataIndex))
		//{
		//	MarketDataIndex md;
		//	memcpy(&md, data_, sizeof(MarketDataIndex));
		//	LOGD( "udp recv MarketDataIndex: " << md.SecID << ",at " << tmp_endpoint_.address().to_string() << ":" << tmp_endpoint_.port() );
		//}
		if (recv_count_ % 50000 == 0)
		{
			LOGD("udp total recv: " << recv_count_ << ", marketdata: "<< marketdata_count_ << ",from " << tmp_endpoint_.address().to_string() << ":" << tmp_endpoint_.port());
		}
	}
	else {
		//if (error != boost::asio::error::connection_refused) //10061这个报错太多了不打印
		//{
		//	LOGE( "udp_server接收异常:" << error.value() << " ,信息:" << error.message().c_str() );
		//}
	}
	receive();
}

void UdpRecv::send(const std::string & msg)
{
	LOGD("UDP Send: "<< msg);
	boost::shared_ptr<std::string> send_msg(new std::string(msg));
	socket_.async_send_to(
		boost::asio::buffer(send_msg->data(), send_msg->length()),
		tmp_endpoint_,
		boost::bind(&UdpRecv::handle_send_to, this,
			send_msg,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)
	);
}

void UdpRecv::handle_send_to(boost::shared_ptr<std::string> send_msg, const boost::system::error_code& error, size_t bytes_transferred)
{
	if (!error)
	{
	}
	else {
		LOGE("udp_server发送异常:" << error.value() << " ,信息:" << error.message().c_str());
	}
}

void UdpRecv::add_token()
{
	std::stringstream ss;
	ss << "{\"T\":200,\"token\":\""<<token_<<"\"}";
	send(ss.str());
}

void UdpRecv::check_heartbeat_timeout(const boost::system::error_code& error) 
{
	if (!error)
	{
		send("{\"T\":100}"); //心跳

		//重启定时器
		heartbeat_check_timer_.expires_from_now(boost::posix_time::seconds(15));
		heartbeat_check_timer_.async_wait(boost::bind(&UdpRecv::check_heartbeat_timeout, this, boost::asio::placeholders::error));

	}
}
