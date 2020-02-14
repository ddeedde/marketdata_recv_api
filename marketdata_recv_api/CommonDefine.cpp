#include "CommonDefine.h"



std::string getExIDByInsID(MarketDataType type, const std::string ins_id)
{
	if (ins_id.size() <= 0)
		return "";

	switch (type)
	{
	//case MARKET_DATA_TYPE_STOCK_INDEX:
	//case MARKET_DATA_TYPE_STOCK_INDEX_SNAP:
	//{
	//	if (ins_id[0] == '3')
	//		return "SZ";
	//	else
	//		return "SH";
	//	break;
	//}
	case MARKET_DATA_TYPE_STOCK_L2:
	case MARKET_DATA_TYPE_STOCK_L2_SNAP:
	{
		if (ins_id[0] == '6')
			return "SH";
		else
			return "SZ";
		break;
	}
	default:
	{
		return "";
	}

	}

}