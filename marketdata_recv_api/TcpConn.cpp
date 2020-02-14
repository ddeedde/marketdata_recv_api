#include "TcpConn.h"
#include "MarketData.h"
#include "cJSONWrapper.h"
#include "MarketRecvApi.h"
#include "Messages.h"
#include "LogWrapper.h"
#include "CommonDefine.h"

Connection::Connection(boost::asio::io_service & io, const std::string & ip, const int port, const std::string & user_id, const std::string & user_passwd, MarketRecvApi * server)
	: io_(io)
	, socket_(io)
	, heartbeat_check_timer_(io)
	, errCount_(0)
	, isShut_(false)
	, isConnected_(false)
	, connecting(false)
	, remoteIp_(ip)
	, remotePort_(port)
	, is_login_(false)
	, login_user_id_(user_id)
	, login_user_passwd_(user_passwd)
	, recv_msg_count_(0)
	, recv_err_count_(0)
	, server_(server)
{
	heartbeat_check_timer_.expires_from_now(boost::posix_time::seconds(15));
	heartbeat_check_timer_.async_wait(boost::bind(&Connection::check_heartbeat_timeout, this, boost::asio::placeholders::error));
}

Connection::~Connection()
{
	heartbeat_check_timer_.cancel();
	stop();
}

void Connection::start()
{
	isShut_ = false;
	connect_server();
}

void Connection::connect_server()
{
	if (!isConnected_ && !connecting)
	{
		connecting = true;
		boost::asio::ip::tcp::resolver resolver(io_);
		boost::asio::ip::tcp::resolver::query query(remoteIp_, std::to_string(remotePort_));
		boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
		boost::asio::ip::tcp::endpoint endPoint = *iterator;
		LOGD( "Connecting to :" << remoteIp_ << ":" << remotePort_ );
		socket_.async_connect(endPoint, boost::bind(&Connection::on_connect,shared_from_this(),boost::asio::placeholders::error, ++iterator));
	}
}
void Connection::on_connect(const boost::system::error_code & err, boost::asio::ip::tcp::resolver::iterator it)
{
	if (!err)
	{
		isConnected_ = true;
		LOGD( "Connecting succ :" << remoteIp_ << ":" << remotePort_ );

		recv_task_.get_type = NET_HEAD;
		boost::asio::async_read(socket_, boost::asio::buffer(buffer_, sizeof(net_head_t)),
			boost::bind(&Connection::head_read_handle, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));

		//开始登陆
		login();
	}
	else {
		is_login_ = false;
		isConnected_ = false;
		LOGE( "Connecting fail :" << err.message() );
	}
	connecting = false;
}

void Connection::login()
{
	std::stringstream ss;
	ss << "{\"T\":101,\"user_id\":\"" << login_user_id_<<"\",\"password\":\""<<login_user_passwd_<<"\"}";

	asyncSend(ss.str(),101);
}
void Connection::stop()
{
	isShut_ = true;
	if (isConnected_ || connecting)
	{
		is_login_ = false;
		isConnected_ = false;
		connecting = false;
		close();
		for (size_t i = 0; i < udp_recvs_.size(); i++)
		{
			udp_recvs_[i]->stop();
		}
		//udp_recvs_.clear();
		server_->callMarketDisconnect();
	}
}
void Connection::reconnect()
{
	if (!isShut_ && isConnected_)
	{
		stop();
		connect_server();
		isShut_ = false;
	}
}
void Connection::close() 
{
	boost::system::error_code ignored_ec;
	try {
		socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
		socket_.close();
	}
	catch (std::exception& e) {
		LOGE( "Close connect error:" << e.what() );
	}
}

void Connection::read_handle(const boost::system::error_code& e)
{
	if (!e)
	{
		std::string line("");
		//unsigned int len = 0;
		for (unsigned int i = 0; i<recvBuf_.size(); ++i)
		{
			//len += recvBufLen_[i];
			line.append(recvBuf_[i].data(), recvBufLen_[i]);
		}
		LOGD("TCP Recv: "<<line);

		handlemsgs(line);

		if (isConnected_)
		{
			startAyncHeadRead();
		}
	}
	else {
		LOGE( "Connect read error,disconnected" << e.message() << ", reconnect soon");
		reconnect();
	}
}

void Connection::head_read_handle(const boost::system::error_code& e, std::size_t bytes_transferred)
{
	if (!e)
	{
		switch (recv_task_.get_type)
		{
		case NET_HEAD:
		{
			recv_task_.get_type = DATA_BODY;
			memcpy(&recv_task_.get_net_head, buffer_.data(), sizeof(net_head_t));
			if (recv_task_.get_net_head.headcheck == CONN_CHECK_HEAD)
			{
				int delt_msg_time = (recv_task_.get_net_head.timekey ^ CONN_MSG_TIMEOUT_ENCODE) - (unsigned int)time(NULL);
				if (abs(delt_msg_time) > CONN_MSG_TIMEOUT)
				{
					if (++errCount_ > MAX_ERROR_COUNT)
					{
						LOGE( "too much error, disconnect:" << remoteIp_);
						stop();
						return;
					}
					else {
						LOGW( "消息中时间超时:" << delt_msg_time << " 秒,当前时间:" << time(NULL) << ",Err次数:" << errCount_ << ", ip:" << remoteIp_ );
					}
				}
				if (recv_task_.get_net_head.length <= 0 || recv_task_.get_net_head.length > MAX_BODY_BUFFER_SIZE)
				{
					LOGE( "消息体长度大于最大值:" << recv_task_.get_net_head.length );
					stop();
					return;
				}
				recv_task_.need_get_length = recv_task_.get_net_head.length;
			}
			else {
				LOGE( "消息头读取错误:" << recv_task_.get_net_head.length );
				stop();
				return;
			}
			recv_task_.get_length = 0;
			recvBuf_.clear();
			recvBufLen_.clear();
			break;
		}
		case DATA_BODY:
		{
			recv_task_.get_length += (unsigned int)bytes_transferred;
			recvBuf_.push_back(buffer_);
			recvBufLen_.push_back(bytes_transferred);
			++recv_msg_count_;
			if (recv_task_.get_length == recv_task_.need_get_length)
			{
				recv_task_.get_type = NET_HEAD;
				recv_task_.need_get_length = sizeof(net_head_t);
				recv_task_.get_length = 0;
				read_handle(e);
				return;
			}
			break;
		}
		default:
			break;
		}
		if (isConnected_)
		{
			startAyncHeadRead();
		}	
	}
	else {
		LOGE( "Connect read head error,disconnected" << e.message()<<", reconnect soon" );
		reconnect();
	}
}

void Connection::startAyncHeadRead()
{
	memset(&buffer_, '\0', BODY_READ_BUFFER_SIZE);
	boost::asio::async_read(socket_, boost::asio::buffer(buffer_, ((BODY_READ_BUFFER_SIZE>(recv_task_.need_get_length - recv_task_.get_length)) ? (recv_task_.need_get_length - recv_task_.get_length) : BODY_READ_BUFFER_SIZE)),
		boost::bind(&Connection::head_read_handle, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void Connection::onWrite(const boost::system::error_code& e, std::size_t bytesWrite) {
	if (!e)
	{
		send_queue_.pop_front();
		if (!send_queue_.empty())
		{
			startSend();
		}
	}
	else
	{
		LOGE( "Connect write error,disconnected" << e.message() << ", reconnect soon");
		reconnect();
	}
}

void Connection::asyncSend(const std::string& msg, int type)
{
	if (!isConnected_ || msg == "")
	{
		return;
	}
	LOGD("TCP Send: " << msg);

	net_head_t write_task_head;
	write_task_head.headcheck = CONN_CHECK_HEAD;
	memcpy(write_task_head.context, recv_task_.get_net_head.context, sizeof(write_task_head.context));
	write_task_head.function = type; //需要支持
	write_task_head.timekey = ((unsigned int)time(NULL)) ^ CONN_MSG_TIMEOUT_ENCODE;
	std::string send_msg("");
	write_task_head.length = (unsigned int)msg.length();
	write_task_head.raw_length = (unsigned int)msg.length();
	send_msg.append((char*)&write_task_head, sizeof(net_head_t)); //2017-02-10 am
	send_msg.append(msg);

	bool writing = !send_queue_.empty(); //这种用法，只有当放消息进队列的线程和发送完成触发回调的线程是同一个时才成立，否则要加队列锁
	send_queue_.push_back(send_msg);
	if (!writing)
	{
		startSend();
	}
}

void Connection::startSend()
{
	boost::asio::async_write(socket_, boost::asio::buffer(send_queue_.front().data(), send_queue_.front().length()),
		boost::bind(&Connection::onWrite, shared_from_this(),
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void Connection::handlemsgs(const std::string& text)
{
	boost::shared_ptr<ClientReplyMessage> msg = readReplyMessage(text);
	if (msg.get() == NULL)
	{
		LOGW("不支持的消息："<<text);
		return;
	}

	switch (msg->messageType)
	{
	case HEARTBEAT:
	{
		break;
	}
	case UNKNOWN_MESSAGE:
	{
		break;
	}
	case MARKET_DATA_LOGIN:
	{	
		if (msg->succflag == "Y" || msg->succflag == "y")
		{
			LOGI("登录成功");
			server_->callMarketConnect();
			if (!already_sub_list_.empty())
			{
				std::vector<std::string> tmp_list;
				{
					boost::mutex::scoped_lock l(sub_list_mutex_);
					for (auto it = already_sub_list_.begin(); it != already_sub_list_.end(); ++it)
					{
						tmp_list.push_back(*it);
					}
				}
				subscribe(tmp_list);
			}
		}
		else {
			LOGI("登录失败");
		}
		break;
	}
	case MARKET_DATA_LOGOUT:
	{
		if (msg->succflag == "Y" || msg->succflag == "y")
		{
			LOGI("登出成功");
			stop();
		}
		else {
			LOGI("登出失败");
		}
		break;
	}
	case MARKET_DATA_SUBSCRIBE:
	{
		if (msg->succflag == "Y" || msg->succflag == "y")
		{
			LOGI("订阅成功");
		}
		else
		{
			LOGI("订阅失败");
		}
		break;
	}
	case MARKET_DATA_SUBSCRIBE_BATCH:
	{
		if (msg->succflag == "Y" || msg->succflag == "y")
		{
			LOGI("批量订阅成功");
		}
		else
		{
			LOGI("批量订阅失败");
		}
		break;
	}
	case MARKET_DATA_WSUBSCRIBE_BATCH:
	{
		if (msg->succflag == "Y" || msg->succflag == "y")
		{
			LOGI("批量退订成功");
		}
		else
		{
			LOGI("批量退订失败");
		}
		break;
	}
	case MARKET_DATA_PUSH_UDP_LIST:
	{
		boost::shared_ptr<UdpAddrListPushMessage> pushMsg = boost::static_pointer_cast<UdpAddrListPushMessage,ClientReplyMessage>(msg);
		for (size_t i = 0; i < pushMsg->addr_list.size(); i++)
		{
			if (pushMsg->addr_list[i].market_type != MARKET_DATA_TYPE_STOCK_L2) //暂时只支持股票行情
			{
				continue;
			}
			UdpRecvPtr ptr(new UdpRecv(io_, pushMsg->addr_list[i].bind_addr, pushMsg->addr_list[i].port, pushMsg->token));
			if (ptr->start(server_->callMarketDataArrive))
			{
				udp_recvs_.push_back(ptr);
			}
			else {
				stop();
			}
		}
		break;
	}
	default:
		break;
	}
}

void Connection::subscribe(const std::vector <std::string >& codes)
{
	ClientBatchSubMessage rmsg;
	rmsg.messageType = MARKET_DATA_SUBSCRIBE_BATCH;
	rmsg.market_type = MARKET_DATA_TYPE_STOCK_L2;
	for (unsigned int i = 0; i < codes.size(); ++i)
	{
		std::string stock_id = codes[i];
		std::string ex_id = getExIDByInsID(MARKET_DATA_TYPE_STOCK_L2, stock_id);
		if (stock_id.empty() || ex_id.empty())
		{
			LOGW("无法订阅错误的证券代码：" << stock_id);
			continue;
		}
		std::pair<std::string, std::string> tmp_pair;
		tmp_pair = std::make_pair(stock_id, ex_id);
		rmsg.stock_list.push_back(tmp_pair);

		boost::mutex::scoped_lock l(sub_list_mutex_);
		already_sub_list_.insert(stock_id);
	}
	std::string rtxt;
	printMessage(rtxt,rmsg);
	asyncSend(rtxt, MARKET_DATA_SUBSCRIBE_BATCH);
}

void Connection::unsubscribe(const std::vector <std::string >& codes)
{
	ClientBatchSubMessage rmsg;
	rmsg.messageType = MARKET_DATA_WSUBSCRIBE_BATCH;
	rmsg.market_type = MARKET_DATA_TYPE_STOCK_L2;
	for (unsigned int i = 0; i < codes.size(); ++i)
	{
		std::string stock_id = codes[i];
		std::string ex_id = getExIDByInsID(MARKET_DATA_TYPE_STOCK_L2, stock_id);
		if (stock_id.empty() || ex_id.empty())
		{
			LOGW("无法退订错误的证券代码：" << stock_id);
			continue;
		}
		std::pair<std::string, std::string> tmp_pair;
		tmp_pair = std::make_pair(stock_id, ex_id);
		rmsg.stock_list.push_back(tmp_pair);

		boost::mutex::scoped_lock l(sub_list_mutex_);
		already_sub_list_.erase(stock_id);
	}
	std::string rtxt;
	printMessage(rtxt, rmsg);
	asyncSend(rtxt, MARKET_DATA_WSUBSCRIBE_BATCH);
}

void Connection::check_heartbeat_timeout(const boost::system::error_code& error)
{
	if (!error)
	{
		if (isConnected_ && is_login_)
		{
			asyncSend("{\"T\":99}", HEARTBEAT); //心跳
		}
		
		//重启定时器
		heartbeat_check_timer_.expires_from_now(boost::posix_time::seconds(15));
		heartbeat_check_timer_.async_wait(boost::bind(&Connection::check_heartbeat_timeout, this, boost::asio::placeholders::error));
	}
}