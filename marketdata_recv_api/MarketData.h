#pragma once
#include <memory.h>
#include <string>
#include <sstream>
#include <vector>


enum MarketDataFormat
{
	MARKET_DATA_FORMAT_JSON = 0,
	MARKET_DATA_FORMAT_BINARY = 1,
	MARKET_DATA_FORMAT_CSV =2
};


enum MarketDataType
{
	MARKET_DATA_TYPE_STOCK_L2			= 300,	//10档行情，实时
	MARKET_DATA_TYPE_STOCK_L2_SNAP		= 301,	//10档行情，快照
	MARKET_DATA_TYPE_MAX,
};


//股票L2行情，共304字节
struct MarketData
{
	unsigned int Type;			// 类型代码，MARKET_DATA_TYPE_STOCK_L2 或 MARKET_DATA_TYPE_STOCK_L2_SNAP
	unsigned int Seq;			// 序号
	char ExID[8];				// 交易所代码
	char SecID[8];				// 证券代码
	char SecName[16];			// 证券名称GBK		
	int	SourceID;				// 行情来源		
	unsigned int Time;			// 交易所时间，精确到毫秒，Int格式 10:01:02 000 = 100102000
	unsigned int PreClose;		// 前收盘，以0.0001元为单位
	unsigned int Open;			// 当日开盘价，以0.0001元为单位
	unsigned int High;			// 当日最高价，以0.0001元为单位
	unsigned int Low;			// 当日最低价，以0.0001元为单位
	unsigned int Match;			// 最新价，以0.0001元为单位
	unsigned int HighLimited;	// 涨停板价格，以0.0001元为单位
	unsigned int LowLimited;	// 跌停板价格，以0.0001元为单位
	unsigned int NumTrades;		// 成交笔数
	long long Volume;			// 累计成交量
	long long Turnover;			// 累计成交金额，以元为单位
	long long AdjVolume;		// 段内成交量
	long long AdjTurnover;		// 段内成交金额，以元为单位
	unsigned int BidPrice[10];	// 最优十档委买价，以0.0001元为单位
	unsigned int BidVol[10];	// 最优十档委买量
	unsigned int AskPrice[10];	// 最优十档委卖价，以0.0001元为单位
	unsigned int AskVol[10];	// 最优十档委卖量
	unsigned int LocalTime;		// 本地时间，精确到毫秒，Int格式 10:01:02 000 = 100102000
	unsigned int IOPV;			// IOPV
	long long TotalBidVol;		// 委买总量
	long long TotalAskVol;		// 委卖总量
	unsigned int WeightedAvgBidPrice;	// 加权平均委买价格，以0.0001元为单位
	unsigned int WeightedAvgAskPrice;	// 加权平均委买价格，以0.0001元为单位

	MarketData(){ memset(this, 0, sizeof(MarketData)); }

	std::string toString()
	{
		std::stringstream resultss;

		auto md = this;
		resultss << md->Type << "," << md->SourceID << "," << md->Seq << "," << md->ExID << "," << md->SecID << "," << md->SecName << "," << md->PreClose << "," << md->Match << "," << md->Open << "," << md->High << "," << md->Low << "," << md->Volume << "," << md->Turnover << "," << md->HighLimited << "," << md->LowLimited << "," << md->Time << "," << md->BidPrice[0] << "," << md->BidVol[0] << "," << md->AskPrice[0] << "," << md->AskVol[0] << "," << md->BidPrice[1] << "," << md->BidVol[1] << "," << md->AskPrice[1] << "," << md->AskVol[1] << "," << md->BidPrice[2] << "," << md->BidVol[2] << "," << md->AskPrice[2] << "," << md->AskVol[2] << "," << md->BidPrice[3] << "," << md->BidVol[3] << "," << md->AskPrice[3] << "," << md->AskVol[3] << "," << md->BidPrice[4] << "," << md->BidVol[4] << "," << md->AskPrice[4] << "," << md->AskVol[4] << "," << md->BidPrice[5] << "," << md->BidVol[5] << "," << md->AskPrice[5] << "," << md->AskVol[5] << "," << md->BidPrice[6] << "," << md->BidVol[6] << "," << md->AskPrice[6] << "," << md->AskVol[6] << "," << md->BidPrice[7] << "," << md->BidVol[7] << "," << md->AskPrice[7] << "," << md->AskVol[7] << "," << md->BidPrice[8] << "," << md->BidVol[8] << "," << md->AskPrice[8] << "," << md->AskVol[8] << "," << md->BidPrice[9] << "," << md->BidVol[9] << "," << md->AskPrice[9] << "," << md->AskVol[9] << "," << md->NumTrades << "," << md->AdjVolume << "," << md->AdjTurnover << "," << md->TotalBidVol << "," << md->TotalAskVol << "," << md->WeightedAvgBidPrice << "," << md->WeightedAvgAskPrice << "," << 0 << "," << 0 << ",\r\n";
		return resultss.str();
	}
	
};


typedef void(*onConnect)();
typedef void(*onDisconnect)();
typedef void(*onDataArrive)(MarketData *);
