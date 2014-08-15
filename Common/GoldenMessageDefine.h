#pragma once
#pragma pack(push)
#pragma pack(1)
#include "MessageDefine.h"
#include "CommonData.h"

struct stMsgGoldenRoomEnter
	:public stMsg
{
	stMsgGoldenRoomEnter(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_GOLDEN_ROOM_ENTER ; }
	stGoldenPeerData tPeerEnterPeerData ;
};

struct stMsgGoldenRoomInfo
	:public stMsg
{
	stMsgGoldenRoomInfo(){cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_GOLDEN_ROOM_INFO ; }
	stRoomGoldenDataOnly tData ;
	unsigned char nSelfIdx ;
	unsigned char nPeerCnt ;
	stGoldenPeerData* pPeers ;
};

struct stMsgGoldenRoomPlayerReady
	:public stMsg
{
	stMsgGoldenRoomPlayerReady(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_GOLDEN_ROOM_PLAYER_READY ; }
};

struct stMsgGoldenRoomPlayerReadyRet
	:public stMsg
{
	stMsgGoldenRoomPlayerReadyRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_GOLDEN_ROOM_PLAYER_READY ; }
	unsigned char nRet ; // 0 success , 1 room state not fit , 2 coin too few ;
};

struct stMsgGoldenRoomReady
	:public stMsgGoldenRoomPlayerReady
{
	stMsgGoldenRoomReady(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_GOLDEN_ROOM_READY ; }
	unsigned char nReadyPlayerIdx ;
};

struct stMsgGoldenRoomDistributy
	:public stMsg
{
	stMsgGoldenRoomDistributy(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_GOLDEN_ROOM_DISTRIBUTY ; }
	unsigned char cBankIdx ;
	unsigned char nCnt ;
	stGoldenHoldPeerCard* peerCards ;
};

struct stMsgGoldenRoomInformAct
	:public stMsg
{
	stMsgGoldenRoomInformAct(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_GOLDEN_ROOM_INFORM_ACT ;}
	unsigned char nActIdx ;
	unsigned short nRound ;
	uint64_t nCurMaxBet ;
	uint64_t nTotalBetCoin ;
};

struct stMsgGoldenRoomPlayerLook
	:public stMsg
{
	stMsgGoldenRoomPlayerLook(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_GOLDEN_ROOM_PLAYER_LOOK ; }
};

struct stMsgGoldenRoomPlayerLookRet
	:public stMsg
{
	stMsgGoldenRoomPlayerLookRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_GOLDEN_ROOM_PLAYER_LOOK; }
	unsigned char nRet ; // 0 success , other value error ;
};

struct stMsgGoldenRoomLook
	:public stMsgGoldenRoomPlayerLookRet
{
	stMsgGoldenRoomLook(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_GOLDEN_ROOM_LOOK; }
	unsigned char cLookPlayerIdx ;
	unsigned char vCard[GOLDEN_PEER_CARD];
};

struct stMsgGoldenRoomPlayerGiveUp
	:public stMsg
{
	stMsgGoldenRoomPlayerGiveUp(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_GOLDEN_ROOM_PLAYER_GIVEUP ; }
};

struct stMsgGoldenRoomPlayerGiveUpRet
	:public stMsg
{
	stMsgGoldenRoomPlayerGiveUpRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_GOLDEN_ROOM_PLAYER_GIVEUP ; }
	unsigned char nRet ; // 0 success , 1 room state error , 2 self state error ;
};

struct stMsgGoldenRoomGiveUp
	:public stMsg
{
	stMsgGoldenRoomGiveUp(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_GOLDEN_ROOM_GIVEUP ; }
	unsigned char nGiveUpIdx ;
};

struct stMsgGoldenRoomPlayerFollow
	:public stMsg
{
	stMsgGoldenRoomPlayerFollow(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_GOLDEN_ROOM_PLAYER_FOLLOW ; }
	uint64_t nFollowCoin ;
};

struct stMsgGoldenRoomPlayerFollowRet
	:public stMsg
{
	stMsgGoldenRoomPlayerFollowRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_GOLDEN_ROOM_PLAYER_FOLLOW ; }
	unsigned char nRet ; // 0 success , 1 not your turn, 2  coin not enough 
};

struct stMsgGoldenRoomFollow
	:public stMsgGoldenRoomPlayerFollow
{
	stMsgGoldenRoomFollow(){ ID_MSG_GAME2C ; usMsgType = MSG_GOLDEN_ROOM_FOLLOW ; }
	unsigned char nFollowIdx ;
};

struct stMsgGoldenRoomPlayerAdd 
	:public stMsg
{
	stMsgGoldenRoomPlayerAdd(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_GOLDEN_ROOM_PLAYER_ADD ; }
	uint64_t nAddCoin ;
};

struct stMsgGoldenRoomPlayerAddRet
	:public stMsg
{
	stMsgGoldenRoomPlayerAddRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_GOLDEN_ROOM_PLAYER_ADD ; }
	unsigned char nRet ; // 0 success , 1 money is not enough , 2 not your turn ;
};

struct stMsgGoldenRoomAdd
	:public stMsgGoldenRoomPlayerAdd
{
	stMsgGoldenRoomAdd(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_GOLDEN_ROOM_ADD ; }
	unsigned char nActIdx ;
};

struct stMsgGoldenRoomPlayerPK
	:public stMsg
{
	stMsgGoldenRoomPlayerPK(){ cSysIdentifer = ID_MSG_C2GAME ;usMsgType = MSG_GOLDEN_ROOM_PLAYER_PK ; }
	unsigned char nPkTargetIdx ;
};

struct stMsgGoldenRoomPlayerPKRet
	:public stMsg
{
	stMsgGoldenRoomPlayerPKRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_GOLDEN_ROOM_PLAYER_PK ; }
	unsigned char cRet ; // 0 success , 1 not youre turn , 2 target not invalid ;
};

struct stMsgGoldenRoomPK
	:public stMsg
{
	stMsgGoldenRoomPK(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_GOLDEN_ROOM_PK ; }
	unsigned char nActPlayerIdx ;
	unsigned char nTargetIdx ;
	bool bWin ; 
};

struct stMsgGoldenRoomResult
	:public stMsg
{
	stMsgGoldenRoomResult(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_GOLDEN_ROOM_RESULT ; }
	unsigned char cWinnerIdx ;
	uint64_t nWinCoin ;
	unsigned char vCard[GOLDEN_PEER_CARD] ;
};

#pragma pack(pop)