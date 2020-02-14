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
#include <vector>
#include "UdpRecv.h"

static const int MAX_RECV_MSG_BODY_LEN = 1024;
static const int MAX_QUEUE_MSG_NUM = 5000;

static const unsigned int MAX_CONN_CLIENT_NUM = 20; //1.5.0.0
static const unsigned int BODY_READ_BUFFER_SIZE = 1024 * 8;
static const unsigned int MAX_ERROR_COUNT = 3;
static const unsigned int MAX_BODY_BUFFER_SIZE = 8 * 1024 * 1024; //1.5.0.0
static const unsigned int CONN_MSG_TIMEOUT = 180; //1.5.0.0 //s
static const unsigned int CONN_MSG_TIMEOUT_ENCODE = 0xabcdbcda; //1.5.0.0 //XOR
static const unsigned int CONN_CHECK_HEAD = 0x1234abcd; //1.5.0.0
static const unsigned int CONN_CONTEXT_Version = 0;
static const unsigned int CONN_CONTEXT_Encrypt = 1;
static const unsigned int CONN_CONTEXT_Compress = 2;
static const unsigned int CONN_CONTEXT_Format = 3;

enum e_read_type
{
	NET_HEAD,
	DATA_BODY,
	READ_UNDEFINED = 0xffffffff
};
#pragma pack(push,1)
struct net_head_t
{
	unsigned int headcheck; //如果错误则中断连接
							/*
							context:
							第0位: Version 当前为'1'，不做校验
							第1位: 加密方式 '0' 不加密 '1' AES256加密 ...
							第2位: 压缩方式 '0' 不压缩 '1' 压缩方式1 ...
							第3位: 数据体格式 '0' json  '1' 二进制 ...
							*/
	char context[4];
	unsigned int length; //数据体长度
	unsigned int raw_length; //未压缩的数据体长度,若压缩方式 0，则raw_length == length
	unsigned int function; //T 功能号
	unsigned int timekey; //1970年至今的秒数 //XOR CONN_MSG_TIMEOUT_ENCODE
	net_head_t() :headcheck(0), length(0), raw_length(0), function(0), timekey(0)
	{
		memset(context, 0, sizeof(context));
	}
};
#pragma pack(pop)

#pragma pack(push,1)
struct recv_task_t
{
	unsigned int get_length;
	unsigned int need_get_length;
	enum e_read_type get_type;
	net_head_t get_net_head;
};
#pragma pack(pop)

/**********************************************************************/
class MarketRecvApi;
class Connection : public boost::enable_shared_from_this<Connection>, private boost::noncopyable
{
public:
	Connection(boost::asio::io_service & io, const std::string & ip, const int port, const std::string & user_id, const std::string & user_passwd, MarketRecvApi * server);
	virtual ~Connection();

	void start();
	void stop();
	void subscribe(const std::vector <std::string >& codes);
	void unsubscribe(const std::vector <std::string >& codes);
	
protected:
	void reconnect();
	void close();
	void connect_server();
	void on_connect(const boost::system::error_code & err,boost::asio::ip::tcp::resolver::iterator it);
	void login();
	
	void startAyncHeadRead();
	void head_read_handle(const boost::system::error_code& e, std::size_t bytes_transferred);
	void read_handle(const boost::system::error_code& e);
	void onWrite(const boost::system::error_code& e, std::size_t bytesWrite);
	void asyncSend(const std::string& msg, int type);
	void startSend();
	void handlemsgs(const std::string& text);

	int getConnMessageTime() //获取时间戳
	{
		return (recv_task_.get_net_head.timekey ^ CONN_MSG_TIMEOUT_ENCODE);
	}
	void check_heartbeat_timeout(const boost::system::error_code& error);
protected:
	boost::asio::io_service & io_;
	boost::asio::ip::tcp::socket socket_;
	boost::asio::deadline_timer heartbeat_check_timer_;
	recv_task_t recv_task_;
	std::string remoteIp_;
	int remotePort_;
	bool isShut_;
	int errCount_;
	bool isConnected_;
	bool connecting;
	boost::array<char, BODY_READ_BUFFER_SIZE> buffer_;
	std::vector<boost::array<char, BODY_READ_BUFFER_SIZE> > recvBuf_;
	std::vector<int> recvBufLen_;
	bool is_login_;
	std::string login_user_id_;
	std::string login_user_passwd_;
	int recv_msg_count_;
	int recv_err_count_;

	std::deque<std::string> send_queue_; //在往队列里放和发送完毕回调都在同一个线程中时成立，否则要加锁；为了保证async_write在未完成前不会被再次调用 //2017-02-10 am
	std::vector<UdpRecvPtr> udp_recvs_;

	MarketRecvApi * server_;
	boost::mutex sub_list_mutex_;
	std::set < std::string > already_sub_list_;
};

typedef boost::shared_ptr<Connection> connection_ptr;


