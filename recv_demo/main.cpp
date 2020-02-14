#include <iostream>
#include <string>
#include <vector>

#include "../marketdata_recv_api/SpiderApi.h"

#pragma comment(lib,"../x64/Release/marketdata_recv_api.lib")


std::string server_ip = "127.0.0.1";
int server_port = 30000;
std::string user_id = "test";
std::string user_passwd = "123";


void p1()
{
	std::cout << "connect" << std::endl;
}

void p2()
{
	std::cout << "disconnect" << std::endl;
}

void p3(MarketData* md)
{
	std::cout << "Data: " << md->SecID << ", " << md->Match << std::endl;
	delete md;
	md = NULL;
}

int main()
{
	SpiderApi * demo = SpiderApi::createSpiderApi(server_ip, server_port, user_id, user_passwd);
	demo->setMarketCallBack(p1, p2, p3);
	demo->start();
	while (true)
	{
		std::vector<std::string> sub_list;
		std::string sub_stock;
		std::cout << "请输入订阅的股票代码：" << std::endl;
		std::cin >> sub_stock;
		if (sub_stock == "quit")
		{
			break;
		}
		else {
			sub_list.push_back(sub_stock);
			demo->subscribe(sub_list);
		}
	}
	demo->stop();
	//system("pause");
	demo->destroySpiderApi();
	demo = NULL;
	return 0;
}


