#pragma once
#include "CommonDefine.h"
#define LOG_ARG_CNT 6
#define CROSS_SVR_REQ_ARG 4
#define RESEVER_GAME_SERVER_PLAYERS 100 
enum  eLogType
{
	eLog_Register, // externString, {ip:"ipdizhi"}
	eLog_Login,  // externString, {ip:"ipdizhi"}
	eLog_BindAccount, // externString, {ip:"ipdizhi"}
	eLog_Logout, 
	eLog_ModifyPwd, // externString, {ip:"ipdizhi"}
	eLog_TaxasGameResult, // nTargetID = roomid , vArg[0] = creator uid ,var[1] = public0 ---var[5] = public4, externString: {[ {uid:234,idx:2,betCoin:4456,card0:23,card1:23,offset:-32,state:GIVE_UP,coin:23334 },{ ... },{ ... }] } 
	eLog_AddMoney, // nTargetID = userUID , var[0] = isCoin , var[1] = addMoneyCnt, var[2] final coin, var[3] finalDiamond ,var[4] subType, var[5] subarg ;
	eLog_DeductionMoney,  // nTargetID = userUID , var[0] = isCoin , var[1] = DeductionCnt, var[2] final coin, var[3] finalDiamond, var[4] subType, var[5] subarg ;
	eLog_ResetPassword,
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
	eCrossSvrReq_CreateTaxasRoom, // var[0] room config id, var[1] rent days; json arg:"roonName", result: var[0] room config id, var[1] newCreateRoomID,
	eCrossSvrReq_TaxasRoomProfit, // result: var[0] isCoin , var[1] RecvMoney;
	eCrossSvrReq_AddRentTime, // var[0] , add days;  result var[0] add days ;
	eCrossSvrReq_SelectTakeIn, // var[0] select player uid,  result: var[0] select player uid, var[1] isCoin, var[2] money 
	eCrossSvrReq_Inform, // var[0] target player uid 
	eCrossSvrReq_Max,
};

enum eCrossSvrReqSubType
{
	eCrossSvrReqSub_Default,
	eCrossSvrReqSub_TaxasSitDown, // ps: json arg: seatIdx , result: json arg just back 
	eCrossSvrReqSub_TaxasSitDownFailed,
	eCrossSvrReqSub_TaxasStandUp,
	eCrossSvrReqSub_SelectPlayerData,  // ps: orgid = sessionid , not uid this situation; var[1] isDeail, result: var[3] isDetail  , json: playTimes,winTimes,singleMost;
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