#pragma once
#include "CommonDefine.h"
#define LOG_ARG_CNT 6
#define CROSS_SVR_REQ_ARG 3
#define RESEVER_GAME_SERVER_PLAYERS 100 
enum  eLogType
{
	eLog_Register, // externString, {ip:"ipdizhi"}
	eLog_Login,  // externString, {ip:"ipdizhi"}
	eLog_BindAccount, // externString, {ip:"ipdizhi"}
	eLog_Logout, 
	eLog_ModifyPwd, // externString, {ip:"ipdizhi"}
	eLog_Max,
};

enum eDBAct
{
	eDBAct_Add,
	eDBAct_Delete,
	eDBAct_Update,
	eDBAct_Select,
	eDBAct_Max,
};

enum eServerType
{
	eSvrType_Gate,
	eSvrType_Login,
	eSvrType_DB,
	eSvrType_Game,
	eSvrType_Verify,
	eSvrType_DataBase,
	eSvrType_APNS,
	eSvrType_Log,
	eSvrType_LogDataBase,
	eSvrType_Center,
	eSvrType_Data,
	eSvrType_Taxas,
	eSvrType_Max,
};

enum  eReqMoneyType
{
	eReqMoney_TaxasTakeIn,// backArg[0] = roomID , backArg[1] = seatIdx ;
	eReqMoney_CreateRoom,  // backArg[0] = sessionID backArg[1] = ConfigID;
	eReqMoney_Max,
	eReqMoneyArgCnt = 3 ,
};

enum  eCrossSvrReqType
{
	eCrossSvrReq_DeductionMoney, //  var[0] isCoin ,var[1] needMoney, var[2] at least money,; result:  var[0] isCoin ,var[1] final deductionMoney 
	eCrossSvrReq_AddMoney, //  var[0] isCoin ,var[1] addCoin
	eCrossSvrReq_CreateTaxasRoom, // var[0] room config id, json arg:"roonName", result: var[0] newCreateRoomID
	eCrossSvrReq_Max,
};

enum eCrossSvrReqSubType
{
	eCrossSvrReqSub_Default,
	eCrossSvrReqSub_TaxasSitDown, // ps: json arg: seatIdx , result: json arg just back 
	eCrossSvrReqSub_TaxasSitDownFailed,
	eCrossSvrReqSub_TaxasStandUp,
	eCrossSvrReqSub_TaxasRoomProfit,
	eCrossSvrReqSub_Max,
};

#define CHECK_MSG_SIZE(CHECK_MSG,nLen) \
{\
	if (sizeof(CHECK_MSG) > (nLen) ) \
{\
	CLogMgr::SharedLogMgr()->ErrorLog("Msg Size Unlegal msg") ;	\
	return false; \
	}\
	}

#define CHECK_MSG_SIZE_VOID(CHECK_MSG,nLen) \
{\
	if (sizeof(CHECK_MSG) > (nLen) ) \
{\
	CLogMgr::SharedLogMgr()->ErrorLog("Msg Size Unlegal msg") ;	\
	return; \
	}\
	}