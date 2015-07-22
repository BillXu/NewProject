#pragma once
#pragma pack(push)
#pragma pack(1)
#include "MessageDefine.h"
#include "CommonData.h"

struct stMsgToRoom
	:public stMsg
{
	stMsgToRoom(){ cSysIdentifer = ID_MSG_PORT_TAXAS; }
	uint32_t nRoomID ;
};

struct stMsgTaxasEnterRoom
	:public stMsg
{
	stMsgTaxasEnterRoom(){ cSysIdentifer = ID_MSG_PORT_TAXAS; usMsgType = MSG_TP_ENTER_ROOM ; }
	uint8_t nType ;
	uint8_t nLevel ;
	uint32_t nRoomID ;
};

struct stMsgTaxasEnterRoomRet 
	:public stMsg
{
	stMsgTaxasEnterRoomRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_TP_ENTER_ROOM ; }
	uint8_t nRet ; // 0 success , 1 invalid session id ;2 already in room ; 3 room id error ;
};

struct stMsgTaxasRoomInfoBase
	:public stMsg
{
	stMsgTaxasRoomInfoBase(){ cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_TP_ROOM_BASE_INFO; }
	uint32_t nRoomID ;
	uint8_t nMaxSeat;
	uint32_t nLittleBlind;
	uint32_t nMiniTakeIn ;
	uint64_t nMaxTakeIn ;
	// running members ;
	uint32_t eCurRoomState ; // eeRoomState ;
	uint8_t nBankerIdx ;
	uint8_t nLittleBlindIdx ;
	uint8_t nBigBlindIdx ;
	int8_t nCurWaitPlayerActionIdx ;
	uint64_t  nCurMainBetPool ;
	uint64_t  nMostBetCoinThisRound;
	uint8_t vPublicCardNums[TAXAS_PUBLIC_CARD] ; 
};

struct stMsgTaxasRoomInfoVicePool
	:public stMsg
{
	stMsgTaxasRoomInfoVicePool(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_TP_ROOM_VICE_POOL ; }
	uint64_t vVicePool[MAX_PEERS_IN_TAXAS_ROOM] ;
};

struct stMsgTaxasRoomInfoPlayerData
	:public stMsg
{
	stMsgTaxasRoomInfoPlayerData(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_TP_ROOM_PLAYER_DATA ; }
	bool bIsLast ;
	stTaxasPeerBaseData tPlayerData ;
};

// player stand up 
struct stMsgTaxasPlayerSitDown
	:public stMsgToRoom
{
	stMsgTaxasPlayerSitDown(){ cSysIdentifer = ID_MSG_PORT_TAXAS; usMsgType = MSG_TP_PLAYER_SIT_DOWN ; }
	uint8_t nSeatIdx ;
	uint64_t nTakeInMoney ;
};

struct stMsgTaxasPlayerSitDownRet
	:public stMsg
{
	stMsgTaxasPlayerSitDownRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_TP_PLAYER_SIT_DOWN ; }
	uint8_t nRet ; // 0 succsss , 1 pos have peer , 2 invalid session id, not in room   ; 3 invalid take in coin , 4 ,withrawing error, money not engouth  , 6. withdrawing momey please wait 
	uint8_t nSeatIdx ;
};

struct stMsgWithdrawingMoneyRet
	:public stMsg
{
	stMsgWithdrawingMoneyRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_TP_WITHDRAWING_MONEY ; }
	uint8_t nRet ; // 0 success , 1 coin not enough , 3 you are not sit down you pos ;
};

struct stMsgTaxasRoomUpdatePlayerState
	:public stMsg
{
	stMsgTaxasRoomUpdatePlayerState(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_TP_UPDATE_PLAYER_STATE; }
	uint8_t nSeatIdx ;
	uint32_t nStateFlag ;
	uint64_t nTakeInCoin ;   // if nTakeInCoin is 0 , means withdrawing coin is error , not enough coin ;stand up 
};

struct stMsgTaxasRoomSitDown
	:public stMsg
{
	stMsgTaxasRoomSitDown(){cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_TP_ROOM_SIT_DOWN ; }
	stTaxasPeerBaseData tPlayerData ;
};

struct stMsgTaxasPlayerStandUp
	:stMsgToRoom
{
	stMsgTaxasPlayerStandUp(){ cSysIdentifer = ID_MSG_PORT_TAXAS ; usMsgType = MSG_TP_PLAYER_STAND_UP ; }
};

struct stMsgTaxasRoomStandUp
	:public stMsg
{
	stMsgTaxasRoomStandUp(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_TP_ROOM_STAND_UP ; }
	uint32_t nSeatIdx ;
};

struct stMsgTaxasPlayerLeave
	:public stMsgToRoom
{
	stMsgTaxasPlayerLeave(){ cSysIdentifer = ID_MSG_PORT_TAXAS; usMsgType = MSG_TP_PLAYER_LEAVE; }
};

struct stMsgTaxasRoomLeave
	:public stMsg
{
	stMsgTaxasRoomLeave(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_TP_PLAYER_LEAVE ; }
	uint32_t nUserUID ;
};

struct stMsgTaxasRoomEnterState
	:public stMsg
{
	stMsgTaxasRoomEnterState(){ cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_TP_ENTER_STATE ; }
	uint8_t nNewState ;
	float fDuringTime ;
};

struct stMsgTaxasRoomStartRound
	:public stMsg
{
	stMsgTaxasRoomStartRound(){ cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_TP_START_ROUND ;} 
	uint8_t nBankerIdx ;
	uint8_t nBigBlindIdx ;
	uint8_t nLittleBlindIdx ;
};

struct stTaxasHoldCardItems
{
	unsigned char cPlayerIdx ;
	unsigned char vCards[TAXAS_PEER_CARD] ;
};

struct stMsgTaxasRoomPrivateCard
	:public stMsg
{
	stMsgTaxasRoomPrivateCard(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_TP_PRIVATE_CARD ; }
	uint8_t nPlayerCnt ;
	stTaxasHoldCardItems vHoldCards[0] ;
};

struct stMsgTaxasRoomWaitPlayerAct
	:public stMsg
{
	stMsgTaxasRoomWaitPlayerAct(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_TP_WAIT_PLAYER_ACT ;}
	uint8_t nActPlayerIdx ; 
};

struct stMsgTaxasPlayerAct
	:public stMsgToRoom
{
	stMsgTaxasPlayerAct(){ cSysIdentifer = ID_MSG_PORT_TAXAS; usMsgType = MSG_TP_PLAYER_ACT ; }
	uint8_t nPlayerAct ; // eRoomPeerAction ;   add, give up , follow , allin , pass 
	uint64_t nValue ;    // used when add act 
};

struct stMsgTaxasPlayerActRet
	:public stMsg
{
	stMsgTaxasPlayerActRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_TP_PLAYER_ACT ; }
	uint8_t nRet ; // 0 success ; 1 not your turn ; 2 . you are not in this game , 3 state error , you can not act , 4 unknown act type 5 . can not do this act , 6 coin not engough
};

struct stMsgTaxasRoomAct
	:public stMsg
{
	stMsgTaxasRoomAct(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_TP_ROOM_ACT; }
	uint8_t nPlayerIdx ;
	uint8_t nPlayerAct ; // eRoomPeerAction ;   add, give up , follow , allin , pass 
	uint64_t nValue ;    // used when add act 
};

struct stMsgTaxasRoomOneBetRoundResult
	:public stMsg
{
	stMsgTaxasRoomOneBetRoundResult(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_TP_ONE_BET_ROUND_RESULT ; }
	uint64_t nCurMainPool ;
	uint8_t nNewVicePoolCnt ; 
	uint64_t vNewVicePool[MAX_PEERS_IN_TAXAS_ROOM] ; // zero means no vice pool produced ;
};

struct stMsgTaxasRoomPublicCard
	:public stMsg
{
	stMsgTaxasRoomPublicCard(){ cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_TP_PUBLIC_CARD; }
	uint8_t nCardSeri; // 0 , 1 ,2 
	uint8_t vCard[3] ;  // 0->3 , 1 -> 1 , 2 -> 1 ;
};

struct stMsgTaxasRoomGameResult
	:public stMsg
{
	stMsgTaxasRoomGameResult(){ cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_TP_GAME_RESULT ;  }
	bool bIsLastOne ;
	uint8_t nPoolIdx ;
	uint64_t nCoinPerWinner ;
	uint8_t nWinnerCnt ;
	uint8_t vWinnerIdx[MAX_PEERS_IN_TAXAS_ROOM] ; 
};


//------------------------------------------------------beforear e new --


//struct stMsgTaxasPlayerSitDown
//	:public stMsg
//{
//	stMsgTaxasPlayerSitDown(){cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_TAXAS_PLAYER_SITDOWN ; }
//	unsigned char nIdx ;
//	uint64_t nTakeInCoin ;
//};
//
//struct stMsgTaxasPlayerSitDownRet
//	:public stMsg
//{
//	stMsgTaxasPlayerSitDownRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_PLAYER_SITDOWN ; }
//	unsigned char nRet ; // 0 success , 1 money is not engough , 2 room is full ,other value error ; 3  money is not reach the low limit to sit down , 4 takein coin must times bigBlind bet , 5 take in too much coin .
//};
//
//struct stMsgTaxasRoomSitDown
//	:public stMsg
//{
//	stMsgTaxasRoomSitDown(){cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_ROOM_SITDOWN ; }
//	//stTaxasPeerData vPlayerSitDown ;
//};
//
//struct stMsgTaxasPlayerStandUp
//	:public stMsg
//{
//	stMsgTaxasPlayerStandUp(){cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_TAXAS_PLAYER_STANDUP ; }
//};
//
//struct stMsgTaxasPlayerStandUpRet
//	:public stMsg
//{
//	stMsgTaxasPlayerStandUpRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_PLAYER_STANDUP ; }
//	unsigned char cRet ; // 0 success , other value means error ;
//};
//
//struct stMsgTaxasRoomStandUp
//	:public stMsg
//{
//	stMsgTaxasRoomStandUp(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_ROOM_STANDUP ; }
//	unsigned char nPlayerIdx ;
//};
//
//struct stMsgTaxasRoomNewState
//	:public stMsg
//{
//	stMsgTaxasRoomNewState(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_ROOM_NEW_STATE; }
//	unsigned char cNewState ;
//};
//
//
//
//struct stMsgTaxasRoomGameStart
//	:public stMsg
//{
//	stMsgTaxasRoomGameStart(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_ROOM_GAME_START ; }
//	unsigned char vPublicCard[TAXAS_PUBLIC_CARD];
//	unsigned char nPeerCnt ;
//	stTaxasHoldCardItems* pHoldItems ;
//};

//struct stMsgTaxasPlayerFollow
//	:public stMsg
//{
//	stMsgTaxasPlayerFollow() { cSysIdentifer = ID_MSG_C2GAME; usMsgType = MSG_TAXAS_PLAYER_FOLLOW ; }
//};
//
//struct stMsgTaxasPlayerFollowRet
//	:public stMsg
//{
//	stMsgTaxasPlayerFollowRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_PLAYER_FOLLOW ;}
//	unsigned char nRet ; // 0 means success ;
//};
//
//struct stMsgTaxasRoomFollow
//	:public stMsg
//{
//	stMsgTaxasRoomFollow(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_ROOM_FOLLOW ; }
//	unsigned char nPlayerIdx ;
//};
//
//struct stMsgTaxasPlayerAdd
//	:public stMsg
//{
//	stMsgTaxasPlayerAdd(){ cSysIdentifer = ID_MSG_C2GAME ;usMsgType = MSG_TAXAS_PLAYER_ADD; }
//	uint64_t nAddCoin ; 
//};
//
//struct stMsgTaxasPlayerAddRet
//	:public stMsg
//{
//	stMsgTaxasPlayerAddRet() { cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_PLAYER_ADD ; }
//	unsigned char nRet ; // 0 success , other value means error ;
//};
//
//struct stMsgTaxasRoomAdd
//	:public stMsgTaxasPlayerAdd
//{
//	stMsgTaxasRoomAdd(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_ROOM_ADD; }
//	unsigned char nPlayerIdx ;
//};
//
//struct stMsgTaxasPlayerGiveUp
//	:public stMsg
//{
//	stMsgTaxasPlayerGiveUp(){ cSysIdentifer = ID_MSG_C2GAME; usMsgType = MSG_TAXAS_PLAYER_GIVEUP; }
//};
//
//struct stMsgTaxasPlayerGiveUpRet
//	:public stMsg
//{
//	stMsgTaxasPlayerGiveUpRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_PLAYER_GIVEUP; }
//	unsigned char nRet ; // 0 success , other means error ;
//};
//
//struct stMsgTaxasRoomGiveUp
//	:public stMsg
//{
//	stMsgTaxasRoomGiveUp(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_ROOM_GIVEUP ; }
//	unsigned char nPlayerIdx ;
//};
//
//struct stMsgTaxasPlayerAllIn
//	:public stMsg
//{
//	stMsgTaxasPlayerAllIn(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_TAXAS_PLAYER_ALLIN ; }
//};
//
//struct stMsgTaxasPlayerAllInRet
//	:public stMsg
//{
//	stMsgTaxasPlayerAllInRet(){ cSysIdentifer = ID_MSG_GAME2C ;usMsgType = MSG_TAXAS_PLAYER_ALLIN; }
//	unsigned char nRet ; // 0 success , other value means error ;
//};
//
//struct stMsgTaxasRoomAllIn
//	:public stMsg
//{
//	stMsgTaxasRoomAllIn(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_ROOM_ALLIN ; }
//	unsigned char nPlayerIdx ;
//};
//
//struct stMsgTaxasPlayerPass
//	:public stMsg
//{
//	stMsgTaxasPlayerPass(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_TAXAS_PLAYER_PASS ; }
//};
//
//struct stMsgTaxasPlayerPassRet
//	:public stMsg
//{
//	stMsgTaxasPlayerPassRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_PLAYER_PASS ; }
//	unsigned char nRet ; // 0 success , other value means error ;
//};
//
//struct stMsgTaxasRoomPass
//	:public stMsg
//{
//	stMsgTaxasRoomPass(){ cSysIdentifer = ID_MSG_GAME2C; usMsgType = MSG_TAXAS_ROOM_PASS ; }
//	unsigned char nPlayerIdx ;
//};
//
//
//
//struct stAutoTakeInItem
//{
//	unsigned char nIdx ;
//	uint64_t nAutoTakeInCoin ; // if 0 , means , will auto stand up ;
//};
//
//struct stMsgTaxasRoomAutoTakeIn
//	:public stMsg
//{
//	stMsgTaxasRoomAutoTakeIn(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_ROOM_AUTOTAKEIN ; }
//	unsigned char nAutoTakeInPeerCnt ;
//	stAutoTakeInItem* pItems;
//};

#pragma pack(pop)