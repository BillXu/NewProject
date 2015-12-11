#pragma once
#include <list>
#include "NativeTypes.h"
//#include "RakNetTypes.h"
#define MAX_VERIFY_STR_LEN 1024*4
#define MAX_MI_UID_LEN 30
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
	unsigned int nFromPlayerUserUID ;
	unsigned int nBuyedForPlayerUserUID ;  
	char pBufferVerifyID[MAX_VERIFY_STR_LEN] ;  // base64 from cliend , or tranfaction_id from apple server ;
	unsigned char nRequestType ; // 0 apple store ;
	eVerifiy_Result eResult ;  // eVerifiy_Result
	unsigned short nShopItemID ;  // for mutilp need to verify ;
	uint32_t nSessionID ;
	uint32_t nMiUserUID;
	void* pUserData ;
};

typedef std::list<stVerifyRequest*> LIST_VERIFY_REQUEST ;