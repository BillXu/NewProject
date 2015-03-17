#pragma once
#include <list>
#include "ServerNetwork.h"
//#include "RakNetTypes.h"
#define MAX_VERIFY_STR_LEN 1024*4
enum eVerifiy_Result
{
	eVerify_Apple_Error,
	eVerify_Apple_Success,
	eVerify_DB_Error,
	eVerify_DB_Success,
	eVerify_Success,
	eVerify_Max,
};
struct stVerifyRequest
{
	CONNECT_ID nFromNetID ;
	unsigned int nFromPlayerUserUID ;
	unsigned int nBuyedForPlayerUserUID ;  
	char pBufferVerifyID[MAX_VERIFY_STR_LEN] ;  // base64 from cliend , or tranfaction_id from apple server ;
	unsigned char nRequestType ; // 0 apple store ;
	eVerifiy_Result eResult ;  // eVerifiy_Result
	unsigned short nShopItemID ;  // for mutilp need to verify ;
	void* pUserData ;
};

typedef std::list<stVerifyRequest*> LIST_VERIFY_REQUEST ;