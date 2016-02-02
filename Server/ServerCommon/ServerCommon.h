#pragma once
#include "CommonDefine.h"
#define LOG_ARG_CNT 6
#define CROSS_SVR_REQ_ARG 4
#define RESEVER_GAME_SERVER_PLAYERS 100 
#define TIME_MATCH_PAUSE 60*30
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
	eLog_NiuNiuGameResult, // nTargetID = room id , vArg[0] = bankerUID , vArg[1] = banker Times, vArg[2] = finalBottomBet, externString: {[ {uid:234,idx:2,betTimes:4456,card0:23,card1:23,card2:23,card3:23,card4:23,offset:-32,coin:23334 },{ ... },{ ... }] } 
	eLog_MatchResult, // nTargetID = room id , var[0] = room type , var[1] champion UID , var[2] = offsetCoin , var[3] room profit;
	eLog_PlayerSitDown, // nTargetID = playerUID , var[0] = room type , var[1] = roomID  , var[2] = coin;
	eLog_PlayerStandUp, // nTargetID = playerUID , var[0] = room type , var[1] = roomID  , var[2] = coin; 
	eLog_GetCharity,   // nTargetID = playerUID , var[0] = final coin ;
	eLog_PlayerLogOut, // nTargetID = playerUID , var[0] = final Coin ;
	eLog_Purchase, // nTargetID = playerUID , var[0] = final Coin ; var[1] = shop item id ;
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
	eSvrType_NiuNiu,
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
	eCrossSvrReq_CreateTaxasRoom, // var[0] room config id, var[1] rent days; json arg:"roonName", result: var[0] room config id, var[1] newCreateRoomID, var[2] rent days ;
	eCrossSvrReq_CreateRoom, // var[0] room config id, var[1] rent days; var[2] roomType{eRoomType}json arg:"roonName", result: var[0] room config id, var[1] newCreateRoomID , var[2] roomType{eRoomType} var[3] rent days,
	eCrossSvrReq_RoomProfit, // result: var[0] isCoin , var[1] RecvMoney, var[2] roomType{eRoomType};
	eCrossSvrReq_AddRentTime, // var[0] add days, var[1] nRoomType,var[2] comsume coin;  result var[0] add days, var[1] nRoomType,var[2] comsume coin ;
	eCrossSvrReq_SelectTakeIn, // var[0] select player uid,  result: var[0] select player uid, var[1] isCoin, var[2] money 
	eCrossSvrReq_Inform, // var[0] target player uid 
	eCrossSvrReq_EnterRoom, // var[0] playerSessionID, var[1] targetID ,var[2] coin, var[3] target id type{0 room id , 1 ConfigID }, retsult: var[0] playerSessionID , var[1] roomType , var[2] roomID, ret{ 0, success , 1 can not find room };
	eCrossSvrReq_SyncCoin, // var[0] coin var[1] room type 
	eCrossSvrReq_ApplyLeaveRoom, // var[0] nRoomID , var[1] session id ;
	eCrossSvrReq_LeaveRoomRet, // var[0] roomType {eRoomType} ; var[1] nRoomID ;
	eCrossSvrReq_DeleteRoom, // var[0] roomType {eRoomType} ; var[1] nRoomID ;
	eCrossSvrReq_SyncNiuNiuData, // var[0] player times , var[1] win times , var[2] SingleWinMoset ;
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

#define FILL_CROSSE_REQUEST_BACK(resultBack,pRequest,eSenderPort)  \
	resultBack.cSysIdentifer = eSenderPort ; \
	resultBack.nJsonsLen = 0 ; \
	resultBack.nReqOrigID = pRequest->nTargetID ; \
	resultBack.nRequestSubType = pRequest->nRequestSubType ; \
	resultBack.nRequestType = pRequest->nRequestType ; \
	resultBack.nRet = 0 ; \
	resultBack.nTargetID = pRequest->nReqOrigID ; \
	memcpy(resultBack.vArg,pRequest->vArg,sizeof(resultBack.vArg)) ;


#define CON_REQ_MSG_JSON(msgCrossReq,jsonArg,autoBuf)  Json::StyledWriter jsWrite ;\
	std::string str = jsWrite.write(jsonArg) ; \
	msgCrossReq.nJsonsLen = strlen(str.c_str()); \
	CAutoBuffer autoBuf(sizeof(msgCrossReq) + msgCrossReq.nJsonsLen ); \
	autoBuf.addContent((char*)&msgCrossReq,sizeof(msgCrossReq)); \
	autoBuf.addContent(str.c_str(),msgCrossReq.nJsonsLen ) ;

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