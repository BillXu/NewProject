#pragma once
#pragma pack(push)
#pragma pack(1)
#include "MessageDefine.h"
#include "CommonData.h"

struct stVicePoolItem 
{
	unsigned char nPoolIdx ;
	uint64_t nCoinInPool;
	unsigned char nPeerCnt ;
	unsigned char* cPlayerIdxInPool ;
};

struct stMsgTaxasRoomInfo
	:public stMsg
{
	stMsgTaxasRoomInfo(){ cSysIdentifer = ID_MSG_GAME2C; usMsgType = MSG_TAXAS_ROOM_INFO; }
	stTaxasRoomDataSimple tSimpleData ;
	unsigned char nPeerCnt ;
	stTaxasPeerData* pPlayerData ;
	unsigned char nVicePoolCnt ;
	stVicePoolItem* pPools ;
	unsigned char nKickCnt ;
	unsigned int* nKickSessionID ;
};

struct stMsgTaxasPlayerSitDown
	:public stMsg
{
	stMsgTaxasPlayerSitDown(){cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_TAXAS_PLAYER_SITDOWN ; }
	unsigned char nIdx ;
	uint64_t nTakeInCoin ;
};

struct stMsgTaxasPlayerSitDownRet
	:public stMsg
{
	stMsgTaxasPlayerSitDownRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_PLAYER_SITDOWN ; }
	unsigned char nRet ; // 0 success , 1 money is not engough , 2 room is full ,other value error ; 3  money is not reach the low limit to sit down , 4 takein coin must times bigBlind bet , 5 take in too much coin .
};

struct stMsgTaxasRoomSitDown
	:public stMsg
{
	stMsgTaxasRoomSitDown(){cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_ROOM_SITDOWN ; }
	stTaxasPeerData vPlayerSitDown ;
};

struct stMsgTaxasPlayerStandUp
	:public stMsg
{
	stMsgTaxasPlayerStandUp(){cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_TAXAS_PLAYER_STANDUP ; }
};

struct stMsgTaxasPlayerStandUpRet
	:public stMsg
{
	stMsgTaxasPlayerStandUpRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_PLAYER_STANDUP ; }
	unsigned char cRet ; // 0 success , other value means error ;
};

struct stMsgTaxasRoomStandUp
	:public stMsg
{
	stMsgTaxasRoomStandUp(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_ROOM_STANDUP ; }
	unsigned char nPlayerIdx ;
};

struct stMsgTaxasRoomNewState
	:public stMsg
{
	stMsgTaxasRoomNewState(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_ROOM_NEW_STATE; }
	unsigned char cNewState ;
};

struct stTaxasHoldCardItems
{
	unsigned char cPlayerIdx ;
	unsigned char vCards[TAXAS_PEER_CARD] ;
};

struct stMsgTaxasRoomCards
	:public stMsg
{
	stMsgTaxasRoomCards(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_ROOM_CARDS ; }
	unsigned char vPublicCard[TAXAS_PUBLIC_CARD];
	unsigned char nPeerCnt ;
	stTaxasHoldCardItems* pHoldItems ;
};

struct stMsgTaxasPlayerFollow
	:public stMsg
{
	stMsgTaxasPlayerFollow() { cSysIdentifer = ID_MSG_C2GAME; usMsgType = MSG_TAXAS_PLAYER_FOLLOW ; }
};

struct stMsgTaxasPlayerFollowRet
	:public stMsg
{
	stMsgTaxasPlayerFollowRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_PLAYER_FOLLOW ;}
	unsigned char nRet ; // 0 means success ;
};

struct stMsgTaxasRoomFollow
	:public stMsg
{
	stMsgTaxasRoomFollow(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_ROOM_FOLLOW ; }
	unsigned char nPlayerIdx ;
};

struct stMsgTaxasPlayerAdd
	:public stMsg
{
	stMsgTaxasPlayerAdd(){ cSysIdentifer = ID_MSG_C2GAME ;usMsgType = MSG_TAXAS_PLAYER_ADD; }
	uint64_t nAddCoin ; 
};

struct stMsgTaxasPlayerAddRet
	:public stMsg
{
	stMsgTaxasPlayerAddRet() { cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_PLAYER_ADD ; }
	unsigned char nRet ; // 0 success , other value means error ;
};

struct stMsgTaxasRoomAdd
	:public stMsgTaxasPlayerAdd
{
	stMsgTaxasRoomAdd(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_ROOM_ADD; }
	unsigned char nPlayerIdx ;
};

struct stMsgTaxasPlayerGiveUp
	:public stMsg
{
	stMsgTaxasPlayerGiveUp(){ cSysIdentifer = ID_MSG_C2GAME; usMsgType = MSG_TAXAS_PLAYER_GIVEUP; }
};

struct stMsgTaxasPlayerGiveUpRet
	:public stMsg
{
	stMsgTaxasPlayerGiveUpRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_PLAYER_GIVEUP; }
	unsigned char nRet ; // 0 success , other means error ;
};

struct stMsgTaxasRoomGiveUp
	:public stMsg
{
	stMsgTaxasRoomGiveUp(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_ROOM_GIVEUP ; }
	unsigned char nPlayerIdx ;
};

struct stMsgTaxasPlayerAllIn
	:public stMsg
{
	stMsgTaxasPlayerAllIn(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_TAXAS_PLAYER_ALLIN ; }
};

struct stMsgTaxasPlayerAllInRet
	:public stMsg
{
	stMsgTaxasPlayerAllInRet(){ cSysIdentifer = ID_MSG_GAME2C ;usMsgType = MSG_TAXAS_PLAYER_ALLIN; }
	unsigned char nRet ; // 0 success , other value means error ;
};

struct stMsgTaxasRoomAllIn
	:public stMsg
{
	stMsgTaxasRoomAllIn(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_ROOM_ALLIN ; }
	unsigned char nPlayerIdx ;
};

struct stMsgTaxasPlayerPass
	:public stMsg
{
	stMsgTaxasPlayerPass(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_TAXAS_PLAYER_PASS ; }
};

struct stMsgTaxasPlayerPassRet
	:public stMsg
{
	stMsgTaxasPlayerPassRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_PLAYER_PASS ; }
	unsigned char nRet ; // 0 success , other value means error ;
};

struct stMsgTaxasRoomPass
	:public stMsg
{
	stMsgTaxasRoomPass(){ cSysIdentifer = ID_MSG_GAME2C; usMsgType = MSG_TAXAS_ROOM_PASS ; }
	unsigned char nPlayerIdx ;
};

struct stMsgTaxasRoomWaitPlayerAct
	:public stMsg
{
	stMsgTaxasRoomWaitPlayerAct(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_TAXAS_ROOM_WAIT_PLAYER_ACT ;}
	unsigned char nActPlayerIdx ; 
};

struct stAutoTakeInItem
{
	unsigned char nIdx ;
	uint64_t nAutoTakeInCoin ; // if 0 , means , will auto stand up ;
};

struct stMsgTaxasRoomAutoTakeIn
	:public stMsg
{
	stMsgTaxasRoomAutoTakeIn(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_TAXAS_ROOM_AUTOTAKEIN ; }
	unsigned char nAutoTakeInPeerCnt ;
	stAutoTakeInItem* pItems;
};

#pragma pack(pop)