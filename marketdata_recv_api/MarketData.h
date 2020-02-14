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
	MARKET_DATA_TYPE_STOCK_L2			= 300,	//10�����飬ʵʱ
	MARKET_DATA_TYPE_STOCK_L2_SNAP		= 301,	//10�����飬����
	MARKET_DATA_TYPE_MAX,
};


//��ƱL2���飬��304�ֽ�
struct MarketData
{
	unsigned int Type;			// ���ʹ��룬MARKET_DATA_TYPE_STOCK_L2 �� MARKET_DATA_TYPE_STOCK_L2_SNAP
	unsigned int Seq;			// ���
	char ExID[8];				// ����������
	char SecID[8];				// ֤ȯ����
	char SecName[16];			// ֤ȯ����GBK		
	int	SourceID;				// ������Դ		
	unsigned int Time;			// ������ʱ�䣬��ȷ�����룬Int��ʽ 10:01:02 000 = 100102000
	unsigned int PreClose;		// ǰ���̣���0.0001ԪΪ��λ
	unsigned int Open;			// ���տ��̼ۣ���0.0001ԪΪ��λ
	unsigned int High;			// ������߼ۣ���0.0001ԪΪ��λ
	unsigned int Low;			// ������ͼۣ���0.0001ԪΪ��λ
	unsigned int Match;			// ���¼ۣ���0.0001ԪΪ��λ
	unsigned int HighLimited;	// ��ͣ��۸���0.0001ԪΪ��λ
	unsigned int LowLimited;	// ��ͣ��۸���0.0001ԪΪ��λ
	unsigned int NumTrades;		// �ɽ�����
	long long Volume;			// �ۼƳɽ���
	long long Turnover;			// �ۼƳɽ�����ԪΪ��λ
	long long AdjVolume;		// ���ڳɽ���
	long long AdjTurnover;		// ���ڳɽ�����ԪΪ��λ
	unsigned int BidPrice[10];	// ����ʮ��ί��ۣ���0.0001ԪΪ��λ
	unsigned int BidVol[10];	// ����ʮ��ί����
	unsigned int AskPrice[10];	// ����ʮ��ί���ۣ���0.0001ԪΪ��λ
	unsigned int AskVol[10];	// ����ʮ��ί����
	unsigned int LocalTime;		// ����ʱ�䣬��ȷ�����룬Int��ʽ 10:01:02 000 = 100102000
	unsigned int IOPV;			// IOPV
	long long TotalBidVol;		// ί������
	long long TotalAskVol;		// ί������
	unsigned int WeightedAvgBidPrice;	// ��Ȩƽ��ί��۸���0.0001ԪΪ��λ
	unsigned int WeightedAvgAskPrice;	// ��Ȩƽ��ί��۸���0.0001ԪΪ��λ

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
