#if !defined(SPIDER_COMMONAPI_H)
#define SPIDER_COMMONAPI_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#if defined(_WIN32)
#ifdef LIB_SPIDER_API_EXPORT
#define SPIDER_API_EXPORT __declspec(dllexport)
#else
#define SPIDER_API_EXPORT __declspec(dllimport)
#endif
#else
#define SPIDER_API_EXPORT 
#endif

#include "MarketData.h"


class SPIDER_API_EXPORT SpiderApi
{
protected:
	SpiderApi() {}
	virtual ~SpiderApi() {}

public:

	static SpiderApi* createSpiderApi(const std::string ip, const int port, const std::string user_id, const std::string user_passwd);
	void destroySpiderApi();

	virtual void start() = 0;
	virtual void stop() = 0;

	virtual void subscribe(const std::vector <std::string >& codes) = 0;
	virtual void unSubscribe(const std::vector <std::string >& codes) = 0;

	virtual void setMarketCallBack(onConnect c1, onDisconnect c2, onDataArrive c3) = 0;

};



#endif