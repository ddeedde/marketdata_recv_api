#include "Messages.h"
#ifndef WIN32
#include <string.h>
#endif


boost::shared_ptr<Message> readMessage( const std::string & text )
{
	CJsonParser parser;
	cJSON* root = parser.parse(text.c_str());
	if (root==NULL || root->type!=cJSON_Object)
		return boost::shared_ptr<Message>();
	cJSON *jsonObject = root->child;
	MessageType type = UNKNOWN_MESSAGE;
	for (;jsonObject!=NULL; jsonObject = jsonObject->next)
	{
		if ( strcmp(jsonObject->string,MESSAGE_TYPE_NAME)==0 && jsonObject->type==cJSON_Number){
			type = MessageType(jsonObject->valueint);
			break;
		}
	}
	if (type == UNKNOWN_MESSAGE)
	{
		return boost::shared_ptr<Message>();
	}
	cJSON *poi = root->child;
	poi=poi->next;

	boost::shared_ptr<Message> msg;

	switch (type)
	{
	case HEARTBEAT:
		msg=readSimpleMessage(poi);
		break;
	case MARKET_DATA_LOGIN:
		msg=readLoginMessage(poi);
		break;
	case MARKET_DATA_LOGOUT:
		msg=readLogoutMessage(poi);
		break;
	case MARKET_DATA_SUBSCRIBE:
		msg=readSubMessage(poi);
		break;
	case MARKET_DATA_SUBSCRIBE_BATCH:
	case MARKET_DATA_WSUBSCRIBE_BATCH:
		msg=readBatchSubMessage(poi);
		break;
	default:
		return boost::shared_ptr<Message>();
	}

	if (msg.get()!=NULL)
		msg->messageType = type;

	return msg;
}
boost::shared_ptr<Message> readSimpleMessage( cJSON *jsonObject )
{
	boost::shared_ptr<Message> msg( new Message());

	for (;jsonObject!=NULL; jsonObject = jsonObject->next)
	{
		if ( strcmp(jsonObject->string,SEQUENCE_NO_NAME)==0 && jsonObject->type==cJSON_Number)
			msg->requestSeq = jsonObject->valueint;
		else
		{
			msg.reset();
			return msg;
		}
	}
	return msg;
}
boost::shared_ptr<ClientLoginMessage> readLoginMessage( cJSON *jsonObject )
{
	boost::shared_ptr<ClientLoginMessage> msg( new ClientLoginMessage());

	for (;jsonObject!=NULL; jsonObject = jsonObject->next)
	{
		if ( strcmp(jsonObject->string,SEQUENCE_NO_NAME)==0 && jsonObject->type==cJSON_Number)
			msg->requestSeq = jsonObject->valueint;
		else if ( strcmp(jsonObject->string,"user_id")==0 && jsonObject->type==cJSON_String)
			msg->user_id = jsonObject->valuestring;
		else if ( strcmp(jsonObject->string,"password")==0 && jsonObject->type==cJSON_String)
			msg->password = jsonObject->valuestring;
		else
		{
			msg.reset();
			return msg;
		}
	}
	return msg;
}
boost::shared_ptr<ClientLogoutMessage> readLogoutMessage( cJSON *jsonObject )
{
	boost::shared_ptr<ClientLogoutMessage> msg( new ClientLogoutMessage());

	for (;jsonObject!=NULL; jsonObject = jsonObject->next)
	{
		if ( strcmp(jsonObject->string,SEQUENCE_NO_NAME)==0 && jsonObject->type==cJSON_Number)
			msg->requestSeq = jsonObject->valueint;
		else if ( strcmp(jsonObject->string,"user_id")==0 && jsonObject->type==cJSON_String)
			msg->user_id = jsonObject->valuestring;
		else
		{
			msg.reset();
			return msg;
		}
	}
	return msg;
}
boost::shared_ptr<ClientSubMessage> readSubMessage( cJSON *jsonObject )
{
	boost::shared_ptr<ClientSubMessage> msg( new ClientSubMessage());

	for (;jsonObject!=NULL; jsonObject = jsonObject->next)
	{
		if ( strcmp(jsonObject->string,SEQUENCE_NO_NAME)==0 && jsonObject->type==cJSON_Number)
			msg->requestSeq = jsonObject->valueint;
		else if ( strcmp(jsonObject->string,"market_type")==0 && jsonObject->type==cJSON_Number)
			msg->market_type = jsonObject->valueint;
		else if ( strcmp(jsonObject->string,"stock_id")==0 && jsonObject->type==cJSON_String)
			msg->stock_id = jsonObject->valuestring;
		else if ( strcmp(jsonObject->string,"ex_id")==0 && jsonObject->type==cJSON_String)
			msg->ex_id = jsonObject->valuestring;
		else
		{
			msg.reset();
			return msg;
		}
	}
	return msg;
}
boost::shared_ptr<ClientBatchSubMessage> readBatchSubMessage( cJSON *jsonObject )
{
	boost::shared_ptr<ClientBatchSubMessage> msg( new ClientBatchSubMessage());
	cJSON *jsonArray = NULL;
	for (;jsonObject!=NULL; jsonObject = jsonObject->next)
	{
		if ( strcmp(jsonObject->string,SEQUENCE_NO_NAME)==0 && jsonObject->type==cJSON_Number)
			msg->requestSeq = jsonObject->valueint;
		else if ( strcmp(jsonObject->string,"market_type")==0 && jsonObject->type==cJSON_Number)
			msg->market_type = jsonObject->valueint;
		else if ( strcmp(jsonObject->string,"stock_list")==0 && jsonObject->type==cJSON_Array )
			jsonArray = jsonObject;
		else
		{
			msg.reset();
			return msg;
		}
	}
	if (jsonArray == NULL)
	{
		msg.reset();
		return msg;
	}
	for (cJSON *aka = jsonArray->child; aka!=NULL; aka=aka->next)
	{
		std::string stock_id(""),ex_id("");
		for (cJSON *aka1=aka->child; aka1!=NULL; aka1=aka1->next)
		{
			if ( strcmp( aka1->string,"stock_id")==0 && aka1->type==cJSON_String ){
				stock_id= aka1->valuestring;
			} else if ( strcmp(aka1->string,"ex_id")==0 && aka1->type==cJSON_String ){
				ex_id = aka1->valuestring;
			}else{
				continue;
			}
		}
		msg->stock_list.push_back(std::make_pair(stock_id,ex_id));
	}
	return msg;
}


/*****************************************************************************/
void printMessage(std::string& result, const ClientReplyMessage & message)
{
	CJsonPrinter printer;
	cJSON *root = printer.getRoot();

	cJSON_AddNumberToObject(root, MESSAGE_TYPE_NAME, message.messageType);
	cJSON_AddNumberToObject(root, SEQUENCE_NO_NAME, message.requestSeq );
	cJSON_AddNumberToObject(root, REPLY_TO_NAME, message.replyTo );
	cJSON_AddStringToObject(root, "flag", message.succflag.c_str());

	result = printer.print();
}
void printMessage(std::string& result, const ClientErrorReplyMessage & message)
{
	CJsonPrinter printer;
	cJSON *root = printer.getRoot();

	cJSON_AddNumberToObject(root, MESSAGE_TYPE_NAME, message.messageType);
	cJSON_AddNumberToObject(root, SEQUENCE_NO_NAME, message.requestSeq );
	cJSON_AddNumberToObject(root, REPLY_TO_NAME, message.replyTo );
	cJSON_AddStringToObject(root, "flag", message.succflag.c_str());
	cJSON_AddNumberToObject(root, ERROR_NO_NAME, message.error_no );
	cJSON_AddStringToObject(root, ERROR_MSG_NAME, message.error_msg.c_str());

	result = printer.print();
}
void printMessage(std::string& result, const ClientBatchSubResponseMessage & message)
{
	CJsonPrinter printer;
	cJSON *root = printer.getRoot();

	cJSON_AddNumberToObject(root, MESSAGE_TYPE_NAME, message.messageType);
	cJSON_AddNumberToObject(root, SEQUENCE_NO_NAME, message.requestSeq );
	cJSON_AddNumberToObject(root, REPLY_TO_NAME, message.replyTo );
	cJSON_AddStringToObject(root, "flag", message.succflag.c_str());
	cJSON_AddNumberToObject(root, "succ_num", message.succ_num);
	cJSON *parray;
	cJSON_AddItemToObject(root, "succ_stock_list", parray=cJSON_CreateArray());
	{
		for (size_t i=0; i< message.stock_list.size(); ++i)
		{
			cJSON *object;
			cJSON_AddItemToArray(parray, object=cJSON_CreateObject());
			cJSON_AddStringToObject(object, "stock_id", message.stock_list[i].first.c_str());
			cJSON_AddStringToObject(object, "ex_id", message.stock_list[i].second.c_str());
		}
	}
	result = printer.print();
}
/*************************************************************************/
boost::shared_ptr<ClientReplyMessage> readReplyMessage( const std::string & text )
{
	CJsonParser parser;
	cJSON* root = parser.parse(text.c_str());
	if (root==NULL || root->type!=cJSON_Object)
		return boost::shared_ptr<ClientReplyMessage>();

	cJSON *jsonObject = root->child;
	MessageType type = UNKNOWN_MESSAGE;
	std::string succflag = "";
	for (;jsonObject!=NULL; jsonObject = jsonObject->next)
	{
		if ( strcmp(jsonObject->string,MESSAGE_TYPE_NAME)==0 && jsonObject->type==cJSON_Number)
			type = MessageType(jsonObject->valueint);
		else if ( strcmp(jsonObject->string,"flag")==0 && jsonObject->type==cJSON_String )
			succflag = jsonObject->valuestring;
	}
	if (type == UNKNOWN_MESSAGE )
	{
		return boost::shared_ptr<ClientReplyMessage>();
	}
	cJSON *poi = root->child;
	poi=poi->next;

	boost::shared_ptr<ClientReplyMessage> msg;

	if (succflag == "Y" || succflag == "y")
	{
		switch (type)
		{
		case MARKET_DATA_LOGIN:
		case MARKET_DATA_LOGOUT:
		case MARKET_DATA_SUBSCRIBE:
			msg=readCommonAckMessage(poi);
			break;
		case MARKET_DATA_SUBSCRIBE_BATCH:
		case MARKET_DATA_WSUBSCRIBE_BATCH:
			msg=readBatchSubAckMessage(poi);
			break;
		default:
			return boost::shared_ptr<ClientReplyMessage>();
		}
	}else{
		if (type == MARKET_DATA_PUSH_UDP_LIST)
		{
			msg = reaUdpAddrListPushMessage(poi);
		}
		else {
			msg = readCommonNackMessage(poi);
		}		
	}

	if (msg.get()!=NULL)
		msg->messageType = type;

	return msg;
}
boost::shared_ptr<ClientReplyMessage> readCommonAckMessage( cJSON *jsonObject )
{
	boost::shared_ptr<ClientReplyMessage> msg( new ClientReplyMessage());
	cJSON *jsonArray = NULL;
	for (;jsonObject!=NULL; jsonObject = jsonObject->next)
	{
		if ( strcmp(jsonObject->string,SEQUENCE_NO_NAME)==0 && jsonObject->type==cJSON_Number)
			msg->requestSeq = jsonObject->valueint;
		else if ( strcmp(jsonObject->string,REPLY_TO_NAME)==0 && jsonObject->type==cJSON_Number)
			msg->replyTo = jsonObject->valueint;
		else if ( strcmp(jsonObject->string,"flag")==0 && jsonObject->type==cJSON_String )
			msg->succflag = jsonObject->valuestring;
		else
		{
			//msg.reset();
			//return msg;
		}
	}

	return msg;
}
boost::shared_ptr<ClientErrorReplyMessage> readCommonNackMessage( cJSON *jsonObject )
{
	boost::shared_ptr<ClientErrorReplyMessage> msg( new ClientErrorReplyMessage());
	cJSON *jsonArray = NULL;
	for (;jsonObject!=NULL; jsonObject = jsonObject->next)
	{
		if ( strcmp(jsonObject->string,SEQUENCE_NO_NAME)==0 && jsonObject->type==cJSON_Number)
			msg->requestSeq = jsonObject->valueint;
		else if ( strcmp(jsonObject->string,REPLY_TO_NAME)==0 && jsonObject->type==cJSON_Number)
			msg->replyTo = jsonObject->valueint;
		else if ( strcmp(jsonObject->string,"flag")==0 && jsonObject->type==cJSON_String )
			msg->succflag = jsonObject->valuestring;
		else if ( strcmp(jsonObject->string,ERROR_NO_NAME)==0 && jsonObject->type==cJSON_Number)
			msg->error_no = jsonObject->valueint;
		else if ( strcmp(jsonObject->string,ERROR_MSG_NAME)==0 && jsonObject->type==cJSON_String )
			msg->error_msg = jsonObject->valuestring;
		else
		{
			//msg.reset();
			//return msg;
		}
	}

	return msg;
}
boost::shared_ptr<ClientBatchSubResponseMessage> readBatchSubAckMessage( cJSON *jsonObject )
{
	boost::shared_ptr<ClientBatchSubResponseMessage> msg( new ClientBatchSubResponseMessage());
	cJSON *jsonArray = NULL;
	for (;jsonObject!=NULL; jsonObject = jsonObject->next)
	{
		if ( strcmp(jsonObject->string,SEQUENCE_NO_NAME)==0 && jsonObject->type==cJSON_Number)
			msg->requestSeq = jsonObject->valueint;
		else if ( strcmp(jsonObject->string,REPLY_TO_NAME)==0 && jsonObject->type==cJSON_Number)
			msg->replyTo = jsonObject->valueint;
		else if ( strcmp(jsonObject->string,"flag")==0 && jsonObject->type==cJSON_String )
			msg->succflag = jsonObject->valuestring;
		//else if ( strcmp(jsonObject->string,"succ_num")==0 && jsonObject->type==cJSON_Number)
		//	msg->succ_num = jsonObject->valueint;
		//else if ( strcmp(jsonObject->string,"market_type")==0 && jsonObject->type==cJSON_Number)
		//	msg->succ_num = jsonObject->valueint;
		else if ( strcmp(jsonObject->string,"stock_list")==0 && jsonObject->type==cJSON_Array )
			jsonArray = jsonObject;
		else
		{
			//msg.reset();
			//return msg;
		}
	}
	if (jsonArray == NULL)
	{
		msg.reset();
		return msg;
	}
	 for (cJSON *aka = jsonArray->child; aka != NULL; aka = aka->next)
	 {
		 std::string stock_id(""), ex_id("");
		 for (cJSON *aka1 = aka->child; aka1 != NULL; aka1 = aka1->next)
		 {
			 if (strcmp(aka1->string, "stock_id") == 0 && aka1->type == cJSON_String) {
				 stock_id = aka1->valuestring;
			 }
			 else if (strcmp(aka1->string, "ex_id") == 0 && aka1->type == cJSON_String) {
				 ex_id = aka1->valuestring;
			 }
			 else {
				 continue;
			 }
		 }
		 msg->stock_list.push_back(std::make_pair(stock_id, ex_id));
	 }

	return msg;
}
boost::shared_ptr<UdpAddrListPushMessage> reaUdpAddrListPushMessage(cJSON *jsonObject)
{
	boost::shared_ptr<UdpAddrListPushMessage> msg(new UdpAddrListPushMessage());
	cJSON *jsonArray = NULL;
	for (; jsonObject != NULL; jsonObject = jsonObject->next)
	{
		if (strcmp(jsonObject->string, SEQUENCE_NO_NAME) == 0 && jsonObject->type == cJSON_Number)
			msg->requestSeq = jsonObject->valueint;
		else if (strcmp(jsonObject->string, REPLY_TO_NAME) == 0 && jsonObject->type == cJSON_Number)
			msg->replyTo = jsonObject->valueint;
		else if (strcmp(jsonObject->string, "flag") == 0 && jsonObject->type == cJSON_String)
			msg->succflag = jsonObject->valuestring;
		else if (strcmp(jsonObject->string, "token") == 0 && jsonObject->type == cJSON_String)
			msg->token = jsonObject->valuestring;
		else if (strcmp(jsonObject->string, "addr_list") == 0 && jsonObject->type == cJSON_Array)
			jsonArray = jsonObject;
		else
		{
			//msg.reset();
			//return msg;
		}
	}
	if (jsonArray == NULL)
	{
		msg.reset();
		return msg;
	}
	for (cJSON *aka = jsonArray->child; aka != NULL; aka = aka->next)
	{
		ip_info ip_item;
		for (cJSON *aka1 = aka->child; aka1 != NULL; aka1 = aka1->next)
		{
			if (strcmp(aka1->string, "address") == 0 && aka1->type == cJSON_String)
				ip_item.bind_addr = aka1->valuestring;
			else if (strcmp(aka1->string, "port") == 0 && aka1->type == cJSON_Number)
				ip_item.port = aka1->valueint;
			else if (strcmp(aka1->string, "market_type") == 0 && aka1->type == cJSON_Number)
				ip_item.market_type = aka1->valueint;
			else if (strcmp(aka1->string, "market_format") == 0 && aka1->type == cJSON_Number)
				ip_item.market_format = aka1->valueint;
			else if (strcmp(aka1->string, "compress") == 0 && aka1->type == cJSON_Number)
				ip_item.compress = aka1->valueint;
			else if (strcmp(aka1->string, "encrypt") == 0 && aka1->type == cJSON_Number)
				ip_item.encrypt = aka1->valueint;
			else if (strcmp(aka1->string, "need_sub") == 0 && aka1->type == cJSON_Number)
				ip_item.need_sub = aka1->valueint;
			else if (strcmp(aka1->string, "trans_proto") == 0 && aka1->type == cJSON_Number)
				ip_item.trans_proto = aka1->valueint;
			else {
				continue;
			}
		}
		msg->addr_list.push_back(ip_item);
	}

	return msg;
}
/*************************************************************************/
void printMessage(std::string& result, const ClientLoginMessage & message)
{
	CJsonPrinter printer;
	cJSON *root = printer.getRoot();

	cJSON_AddNumberToObject(root, MESSAGE_TYPE_NAME, message.messageType);
	cJSON_AddNumberToObject(root, SEQUENCE_NO_NAME, message.requestSeq);
	cJSON_AddStringToObject(root, "user_id", message.user_id.c_str());
	cJSON_AddStringToObject(root, "password", message.password.c_str());

	result = printer.print();
}
void printMessage(std::string& result, const ClientLogoutMessage & message)
{
	CJsonPrinter printer;
	cJSON *root = printer.getRoot();

	cJSON_AddNumberToObject(root, MESSAGE_TYPE_NAME, message.messageType);
	cJSON_AddNumberToObject(root, SEQUENCE_NO_NAME, message.requestSeq );
	cJSON_AddStringToObject(root, "user_id", message.user_id.c_str() );

	result = printer.print();
}
void printMessage(std::string& result, const ClientSubMessage & message)
{
	CJsonPrinter printer;
	cJSON *root = printer.getRoot();

	cJSON_AddNumberToObject(root, MESSAGE_TYPE_NAME, message.messageType);
	cJSON_AddNumberToObject(root, SEQUENCE_NO_NAME, message.requestSeq );
	cJSON_AddNumberToObject(root, "market_type", message.market_type );
	cJSON_AddStringToObject(root, "stock_id", message.stock_id.c_str());
	cJSON_AddStringToObject(root, "ex_id", message.ex_id.c_str());

	result = printer.print();
}
void printMessage(std::string& result, const ClientBatchSubMessage & message)
{
	CJsonPrinter printer;
	cJSON *root = printer.getRoot();

	cJSON_AddNumberToObject(root, MESSAGE_TYPE_NAME, message.messageType);
	cJSON_AddNumberToObject(root, SEQUENCE_NO_NAME, message.requestSeq );
	cJSON_AddNumberToObject(root, "market_type", message.market_type);
	cJSON *parray;
	cJSON_AddItemToObject(root, "stock_list", parray=cJSON_CreateArray());
	{
		for (size_t i=0; i< message.stock_list.size(); ++i)
		{
			cJSON *object;
			cJSON_AddItemToArray(parray, object=cJSON_CreateObject());
			cJSON_AddStringToObject(object, "stock_id", message.stock_list[i].first.c_str());
			cJSON_AddStringToObject(object, "ex_id", message.stock_list[i].second.c_str());
		}
	}
	result = printer.print();
}

void  printMessage(std::string& result, const UdpAddrListPushMessage & message)
{
	CJsonPrinter printer;
	cJSON *root = printer.getRoot();
	cJSON_AddNumberToObject(root, MESSAGE_TYPE_NAME, message.messageType);
	cJSON_AddNumberToObject(root, SEQUENCE_NO_NAME, message.requestSeq );
	cJSON_AddStringToObject(root, "token", message.token.c_str());
	cJSON *parray,*fld;
	cJSON_AddItemToObject(root, "addr_list", parray=cJSON_CreateArray());
	{
		for (size_t i=0; i< message.addr_list.size(); ++i)
		{
			cJSON_AddItemToArray(parray, fld=cJSON_CreateObject());
			cJSON_AddStringToObject(fld,"address",message.addr_list[i].bind_addr.c_str());
			cJSON_AddNumberToObject(fld,"port",message.addr_list[i].port);
			cJSON_AddNumberToObject(fld,"market_type",message.addr_list[i].market_type);
			cJSON_AddNumberToObject(fld,"market_format",message.addr_list[i].market_format);
			cJSON_AddNumberToObject(fld,"compress",message.addr_list[i].compress);
			cJSON_AddNumberToObject(fld,"encrypt",message.addr_list[i].encrypt);
			cJSON_AddNumberToObject(fld,"need_sub",message.addr_list[i].need_sub);
			cJSON_AddNumberToObject(fld,"trans_proto",message.addr_list[i].trans_proto);
		}
	}
	result = printer.print();
}
