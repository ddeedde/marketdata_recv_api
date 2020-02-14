#pragma once
#include "SpiderApi.h"
#include "TcpConn.h"

class MarketRecvApi : public SpiderApi
{

public:
	MarketRecvApi(const std::string & ip, const int port, const std::string & user_id, const std::string & user_passwd);
	~MarketRecvApi();

	virtual void start();
	virtual void stop();

	virtual void subscribe(const std::vector <std::string >& codes);
	virtual void unSubscribe(const std::vector <std::string >& codes);

	virtual void setMarketCallBack(onConnect c1, onDisconnect c2, onDataArrive c3);

public:
	onConnect callMarketConnect;
	onDisconnect callMarketDisconnect;
	onDataArrive callMarketDataArrive;

private:
	boost::asio::io_service myIO;
	boost::asio::io_service::work myWork;
	boost::shared_ptr<boost::thread> runThread;
	connection_ptr myConnect;

};