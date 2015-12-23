#pragma once
#pragma pack(push)
#pragma pack(1)
#include "MessageDefine.h"
struct stMsgNNEnterRoom
	:public stMsg
{
	stMsgNNEnterRoom(){ cSysIdentifer = ID_MSG_PORT_DATA;usMsgType = MSG_NN_ENTER_ROOM ; }
	uint16_t nRoomID ;
};

struct stMsgNNEnterRoomRet
	:public stMsgToNNRoom
{
	stMsgNNEnterRoomRet(){ usMsgType = MSG_NN_ENTER_ROOM ; cSysIdentifer = ID_MSG_PORT_CLIENT;}
	uint8_t nRet ; // 0 success , 1 can not find room ; 2, already in room ;
};

struct stMsgNNLeaveRoom
	:public stMsgToNNRoom
{
	stMsgNNLeaveRoom(){ cSysIdentifer = ID_MSG_PORT_NIU_NIU ; usMsgType = MSG_NN_LEAVE_ROOM ; }
};

struct stMsgNNRequestRoomInfo
	:public stMsgToNNRoom
{
	stMsgNNRequestRoomInfo(){ usMsgType = MSG_NN_REQUEST_ROOM_INFO ;}
};

struct stNNRoomInfoPayerItem
{
	uint8_t nIdx ;
	uint32_t nUserUID ;
	uint64_t nCoin ;
	uint8_t nBetTimes ;
	uint32_t nStateFlag ;
	uint8_t vHoldChard[NIUNIU_HOLD_CARD_COUNT] ;
};

struct stMsgNNRoomInfo 
	:public stMsg
{
	stMsgNNRoomInfo(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_ROOM_INFO ; }
	uint32_t nRoomID ;
	uint8_t nBankerIdx ;
	uint32_t nBottomBet ;
	uint8_t nBankerBetTimes ;
	uint8_t nPlayerCnt ;
	uint8_t nRoomState ;
    uint32_t nChatRoomID;
    uint32_t nBlind;
	PLACE_HOLDER(stNNRoomInfoPayerItem*) ;
};

struct stMsgNNPlayerSitDown
	:public stMsgToNNRoom
{
	stMsgNNPlayerSitDown(){ usMsgType = MSG_NN_PLAYER_SITDOWN ; }
	uint8_t nSeatIdx ;
};

struct stMsgNNPlayerSitDownRet
	:public stMsg
{
	stMsgNNPlayerSitDownRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_PLAYER_SITDOWN ; }
	uint8_t nRet ; // 0 success , 1 pos already have player , 2 coin not enough , 3 you are not enter room 
};

struct stMsgNNSitDown 
	:public stMsg
{
	stMsgNNSitDown(){ cSysIdentifer = ID_MSG_PORT_CLIENT ;  usMsgType = MSG_NN_SITDOWN ; }
	stNNRoomInfoPayerItem tSitDownPlayer ;
};

struct stMsgNNPlayerStandUp
	:public stMsgToNNRoom
{
	stMsgNNPlayerStandUp(){ usMsgType = MSG_NN_PLAYER_STANDUP ; }
};

struct stMsgNNStandUp
	:public stMsg
{
	stMsgNNStandUp(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_STANDUP ; }
	uint8_t nPlayerIdx ;
};

struct stDistriuet4CardItem
{
	uint8_t nSeatIdx ;
	uint8_t vCardCompsitNum[4] ;
};

struct stMsgNNDistriute4Card
	:public stMsg
{
	stMsgNNDistriute4Card(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_DISTRIBUTE_4_CARD ; }
	uint8_t nPlayerCnt ;
	PLACE_HOLDER(stDistriuet4CardItem*);
};

struct stMsgNNPlayerTryBanker
	:public stMsgToNNRoom
{
	stMsgNNPlayerTryBanker(){ usMsgType = MSG_NN_PLAYER_TRY_BANKER ; }
	uint8_t nTryBankerBetTimes ;
};

struct stMsgNNPlayerTryBankerRet
	:public stMsg
{
	stMsgNNPlayerTryBankerRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_PLAYER_TRY_BANKER ;  }
	uint8_t nRet ; // 0 success , 1 state error , 2 coin not enough , 3 , you are not sit down ;
};


struct stMsgNNTryBanker
	:public stMsg
{
	stMsgNNTryBanker(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_TRY_BANKER ; }
	uint8_t nTryerIdx ;
	uint8_t nTryBankerBetTimes ;
};

struct stMsgNNProducedBanker
	:public stMsg
{
	stMsgNNProducedBanker(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_PRODUCED_BANKER ; }
	uint8_t nBankerIdx ;
	uint8_t nBankerBetTimes ;
};

struct stMsgNNRandBanker
	:public stMsg
{
	stMsgNNRandBanker(){ cSysIdentifer  = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_RAND_BANKER ; }
	uint8_t nBankerIdx ;
	uint8_t nBankerBetTimes ;
};

struct stMsgNNPlayerBet
	:public stMsgToNNRoom
{
	stMsgNNPlayerBet(){ usMsgType = MSG_NN_PLAYER_BET ;}
	uint8_t nBetTimes ;
};

struct stMsgNNPlayerBetRet
	:public stMsg
{
	stMsgNNPlayerBetRet(){ usMsgType = MSG_NN_PLAYER_BET ; cSysIdentifer = ID_MSG_PORT_CLIENT ; }
	uint8_t nRet ; // 0 success , 1 banker coin not enough , 2 self coin not enough , 3 state error;
};

struct stMsgNNBet
	:public stMsg
{
	stMsgNNBet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_BET ; }
	uint8_t nPlayerIdx ;
	uint8_t nBetTimes ;
};

struct stDistributeFinalCardItem  
{
	uint8_t nPlayerIdx ;
	uint8_t nCardCompsitNum ;
};

struct stMsgNNDistributeFinalCard
	:public stMsg
{
	stMsgNNDistributeFinalCard(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_DISTRIBUTE_FINAL_CARD ; }
	uint8_t nPlayerCnt ;
	PLACE_HOLDER(stDistributeFinalCardItem*) ;
};

struct stNNGameResultItem
{
	uint8_t nPlayerIdx ;
	int64_t nOffsetCoin ;
	uint64_t nFinalCoin ;
};
struct stMsgNNGameResult
	:public stMsg
{
	stMsgNNGameResult(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_GAME_RESULT ; }
	uint8_t nPlayerCnt ;
	PLACE_HOLDER(stNNGameResultItem*);
};

struct stMsgNNPlayerCaculateCardOk
	:public stMsgToNNRoom
{
public:
	stMsgNNPlayerCaculateCardOk(){ cSysIdentifer = ID_MSG_PORT_NIU_NIU ; usMsgType = MSG_NN_PLAYER_CACULATE_CARD_OK ; }
};

struct stMsgNNCaculateCardOk
	:public stMsg
{
public: 
	stMsgNNCaculateCardOk(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_NN_CACULATE_CARD_OK ; }
	uint8_t nPlayerIdx ;
};

struct stMsgNNRequestRoomRank
	:public stMsgToNNRoom
{
	stMsgNNRequestRoomRank(){ usMsgType = MSG_REQUEST_ROOM_RANK ;}
};

struct stMsgNNRequestRoomRankRet
	:public stMsgRequestRoomRankRet
{
    
};

#pragma pack(pop)

