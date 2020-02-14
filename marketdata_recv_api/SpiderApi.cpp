#include "SpiderApi.h"
#include "MarketRecvApi.h"

SpiderApi* SpiderApi::createSpiderApi(const std::string ip, const int port, const std::string user_id, const std::string user_passwd)
{
	//std::cerr << ip << ","<< port << "," << user_id << "," << user_passwd << std::endl;
	return new MarketRecvApi(ip, port, user_id, user_passwd);
}

void SpiderApi::destroySpiderApi()
{
	delete this;
}