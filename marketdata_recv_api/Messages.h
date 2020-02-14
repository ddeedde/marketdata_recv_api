#pragma once
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "cJSONWrapper.h"
using namespace raptor;



//Json消息字段定义
const char * const MESSAGE_TYPE_NAME = "T";
const char * const SEQUENCE_NO_NAME = "Seq";
const char * const REPLY_TO_NAME = "Re";
const char * const ERROR_NO_NAME="error_no";
const char * const ERROR_MSG_NAME="error_msg";


enum MessageType
{
	HEARTBEAT=99,
	UNKNOWN_MESSAGE=100,	
	MARKET_DATA_LOGIN = 101, 
	MARKET_DATA_LOGOUT=102, 
	MARKET_DATA_SUBSCRIBE=103,
	//MARKET_DATA_WSUBSCRIBE = 104,为简单起见，约定退订只支持一种Type，即105，而把所谓的单条退订Type砍掉，反正批量退订也能支持单条退订的需要
	MARKET_DATA_SUBSCRIBE_BATCH=105,
	MARKET_DATA_WSUBSCRIBE_BATCH=106,
	MARKET_DATA_PUSH_UDP_LIST=107,
	MARKET_DATA_MANAGE_END=199,
	MARKET_DATA_SEND_TOKEN_TO_UDP=200,
	MARKET_DATA_SEND_HearBeat_TO_UDP=100, //1.8.0.2
	QUERY_DEAL_TICK=300
};


//消息结构体
struct Message
{
	Message():requestSeq(0),replyTo(0){}
	virtual ~Message(){}
	MessageType messageType;
	int requestSeq;
	int replyTo;
};

struct ClientReplyMessage : public Message //客户端回复消息基类
{
	ClientReplyMessage():succflag("Y"){}
	std::string succflag; // Y/N
};

struct ClientErrorReplyMessage : public ClientReplyMessage //通用失败回复消息
{
	ClientErrorReplyMessage():error_no(0),error_msg(""){}
	int error_no;
	std::string error_msg;
};

struct ClientLoginMessage : public Message //101 登陆消息
{
	ClientLoginMessage():password(""),user_id(""){}
	std::string password;
	std::string user_id;
};

struct ClientLogoutMessage : public Message //102 登出消息
{
	ClientLogoutMessage():user_id(""){}
	std::string user_id;
};

struct ClientSubMessage : public Message //103/104 订阅/退订消息
{
	ClientSubMessage():market_type(0),stock_id(""),ex_id(""){}
	int market_type;
	std::string stock_id;
	std::string ex_id;
};

struct ClientBatchSubMessage : public Message //105/106 批量订阅/退订消息
{
	ClientBatchSubMessage():market_type(0){}
	int market_type;
	std::vector<std::pair<std::string,std::string> > stock_list;
};

struct ClientBatchSubResponseMessage : public ClientReplyMessage //105/106 批量订阅/退订消息
{
	ClientBatchSubResponseMessage():succ_num(0){}
	int succ_num;
	std::vector<std::pair<std::string,std::string> > stock_list;
};

struct ip_info
{
	std::string bind_addr;
	int port;
	int market_type;
	int market_format;
	int compress;
	int encrypt;
	int need_sub;
	int trans_proto;
	ip_info():bind_addr(""), port(0), market_type(0), market_format(0), compress(0), encrypt(0), need_sub(0), trans_proto(0){}
};
struct UdpAddrListPushMessage:public ClientReplyMessage
{
	std::string token;
	std::vector<ip_info> addr_list;
	UdpAddrListPushMessage():token("")
	{
		addr_list.clear();
	}
};



/***************************************************************************************/
boost::shared_ptr<Message> readMessage( const std::string & text );
boost::shared_ptr<Message> readSimpleMessage( cJSON *jsonObject );
boost::shared_ptr<ClientLoginMessage> readLoginMessage( cJSON *jsonObject );
boost::shared_ptr<ClientLogoutMessage> readLogoutMessage( cJSON *jsonObject );
boost::shared_ptr<ClientSubMessage> readSubMessage( cJSON *jsonObject );
boost::shared_ptr<ClientBatchSubMessage> readBatchSubMessage( cJSON *jsonObject );
/***************************************************************************************/
void printMessage(std::string& result, const ClientReplyMessage & message);
void printMessage(std::string& result, const ClientErrorReplyMessage & message);
void printMessage(std::string& result, const ClientBatchSubResponseMessage & message);
/***************************************************************************************/
boost::shared_ptr<ClientReplyMessage> readReplyMessage( const std::string & text );
boost::shared_ptr<ClientReplyMessage> readCommonAckMessage( cJSON *jsonObject );
boost::shared_ptr<ClientErrorReplyMessage> readCommonNackMessage( cJSON *jsonObject );
boost::shared_ptr<ClientBatchSubResponseMessage> readBatchSubAckMessage( cJSON *jsonObject );
boost::shared_ptr<UdpAddrListPushMessage> reaUdpAddrListPushMessage(cJSON *jsonObject);
/***************************************************************************************/
void printMessage(std::string& result, const ClientLoginMessage & message);
void printMessage(std::string& result, const ClientLogoutMessage & message);
void printMessage(std::string& result, const ClientSubMessage & message);
void printMessage(std::string& result, const ClientBatchSubMessage & message);
void printMessage(std::string& result, const UdpAddrListPushMessage & message);


