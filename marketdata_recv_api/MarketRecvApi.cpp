#include "MarketRecvApi.h"
#include "LogWrapper.h"

MarketRecvApi::MarketRecvApi(const std::string & ip, const int port, const std::string & user_id, const std::string & user_passwd)
	: myWork(myIO)
	//, callMarketConnect(NULL)
	//, callMarketDisconnect(NULL)
	//, callMarketDataArrive(NULL)
{
	InitGlobalLog();
	myConnect.reset(new Connection(myIO,ip,port,user_id,user_passwd,this));
	runThread.reset(new boost::thread(boost::bind(&boost::asio::io_service::run, &myIO)));
}
MarketRecvApi::~MarketRecvApi()
{
	if (myConnect.get() != NULL)
	{
		myConnect->stop();
	}
	myIO.stop();
	if (runThread.get() != NULL)
	{
		runThread->join();
	}
	StopGlobalLog();
}

void MarketRecvApi::start()
{
	myConnect->start();
}
void MarketRecvApi::stop()
{
	myConnect->stop();
}

void MarketRecvApi::subscribe(const std::vector <std::string >& codes)
{
	myConnect->subscribe(codes);
}
void MarketRecvApi::unSubscribe(const std::vector <std::string >& codes)
{
	myConnect->unsubscribe(codes);
}

void MarketRecvApi::setMarketCallBack(onConnect c1, onDisconnect c2, onDataArrive c3)
{
	callMarketConnect = c1;
	callMarketDisconnect = c2;
	callMarketDataArrive = c3;
}