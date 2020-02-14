#pragma once
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/unordered_map.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <set>
#include <deque>
#include <iostream>
#include <sstream>
#include "MarketData.h"

class UdpRecv
{
public:
	UdpRecv(boost::asio::io_service& io, const std::string& ip, int port, const std::string& token);
	~UdpRecv();

	//typedef void(*onArrive)(MarketData *);

	void receive();
	void send(const std::string & msg);
	void handle_receive_from(const boost::system::error_code& error, size_t bytes_transferred);
	void handle_send_to(boost::shared_ptr<std::string> send_msg, const boost::system::error_code& error, size_t bytes_transferred);

	bool start(onDataArrive c);
	void stop();
	void add_token();

	void check_heartbeat_timeout(const boost::system::error_code& error); 

	
	onDataArrive callDataArrive;
private:
	enum { max_length = 20480 };
	char data_[max_length];

	//boost::shared_ptr<boost::thread> udp_run_thread_;
	boost::asio::io_service& udp_io_service_;
	boost::asio::ip::udp::socket socket_;
	boost::asio::ip::udp::endpoint tmp_endpoint_;
	boost::asio::deadline_timer heartbeat_check_timer_;

	std::string address_;
	int port_;
	std::string token_;

	int recv_count_;
	int marketdata_count_;

};
typedef boost::shared_ptr<UdpRecv> UdpRecvPtr;