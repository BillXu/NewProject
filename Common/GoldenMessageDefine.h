#pragma once
#pragma pack(push)
#pragma pack(1)
#include "MessageDefine.h"
struct stRoomPeerBrifInfo
{
	unsigned  int nSessionID ;
	unsigned char nIdx ; // index in the room ;
    unsigned char ePeerState ; // peer state ;  eRoomPeerState
	char pName[MAX_LEN_CHARACTER_NAME] ;
	unsigned char nTitle ;
	unsigned int nCoin ;
	unsigned int nBetCoin ;
	char nDefaulPhotoID ;     
	unsigned int nUserDefinePhotoID ;
};

struct stMsgRoomCurInfo
	:public stMsg 
{
	 stMsgRoomCurInfo(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_ROOM_CURRENT_INFO ; }
public:
     char nSelfIdx ; // server roomidx ;
    
     unsigned int nRoomID ;  // used it to get curRoomInfo ;
	 int nSingleBetCoin;
	 int nTotalBetCoin; 
	 unsigned short nRound ;
	 unsigned char eRoomSate ;  // eRoomState 
	 unsigned char nPlayerCount ;
	 stRoomPeerBrifInfo* pInfos ;
};

struct stMsgRoomPlayerEnter
	:public stMsg
{
public:
	stMsgRoomPlayerEnter(){cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_ROOM_PLAYER_ENTER ; }
	stRoomPeerBrifInfo nEnterPlayerInfo ;
};

struct stMsgRoomReady
	:public stMsg
{
public:
	stMsgRoomReady(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_ROOM_READY ; }
};

struct stMsgRoomRet
	:public stMsg 
{
	stMsgRoomRet(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_ROOM_RET ; }
	unsigned char nRet ; // 0 means success , 1 room state not fitable , 2 add money should greate than crrent ; 3 , unlegal pk target ;
};

struct stMsgRoomPlayerReady
	:public stMsg
{
	stMsgRoomPlayerReady(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_ROOM_PLAYER_READY  ; }
	unsigned nReadyPlayerSessionID ; 
};

struct stMsgDistributeCard
:public stMsg
{
    stMsgDistributeCard(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_ROOM_DISTRIBUTE_CARD ; }
	char nCurMainIdx ;
} ;

struct stMsgRoomWaitPlayerAction
:public stMsg
{
    stMsgRoomWaitPlayerAction(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_ROOM_WAIT_PLAYER_ACTION ; }
    unsigned int nSessionID ;
    unsigned short nRound ;
};

struct stMsgRoomFollow
:public stMsg
{
    stMsgRoomFollow(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_ROOM_FOLLOW ; }
} ;

struct stMsgRoomPlayerFollow
:public stMsg
{
    stMsgRoomPlayerFollow(){cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_ROOM_PLAYER_FOLLOW ;}
    unsigned int nSessionID ;
} ;

struct stMsgRoomAdd
:public stMsg
{
    stMsgRoomAdd(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_ROOM_ADD ; }
    int nAddMoney ; //  0 means double ;
};


struct stMsgRoomPlayerAdd
:public stMsg
{
    stMsgRoomPlayerAdd(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_ROOM_PLAYER_ADD ;}
    unsigned int nSessionID ;
    int nNewSingle ;
    int nBetCoin ;
};

struct stMsgRoomLook
:public stMsg
{
    stMsgRoomLook(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_ROOM_LOOK ; }
} ;

struct stMsgRoomPlayerLook
:public stMsg
{
    stMsgRoomPlayerLook(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_ROOM_PLAYER_LOOK ; }
    unsigned int nSessionID ;
	char vCard[3] ;
};

struct stMsgRoomPK
:public stMsg
{
    stMsgRoomPK(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_ROOM_PK ; }
    unsigned int nPKWithSessionID ;
};

struct stMsgRoomPlayerPK
:public stMsg
{
    stMsgRoomPlayerPK(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_ROOM_PLAYER_PK ; }
    unsigned int nPKInvokeSessionID ;
    unsigned int nPKWithSessionID ;
    int nConsumCoin ;// pk will comsum Invoker's coin ;
    bool bWin ;
};

struct stMsgRoomGiveUp
:public stMsg
{
    stMsgRoomGiveUp(){ cSysIdentifer = ID_MSG_C2GAME ; usMsgType = MSG_ROOM_GIVEUP ; }
};

struct stMsgRoomPlayerGiveUp
:public stMsg
{
    stMsgRoomPlayerGiveUp(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_ROOM_PLAYER_GIVEUP ; }
    char nIdx ;
} ;

struct stResultData
{
    char idx ;
    char vCard[3] ;
    int nResultCoin ; // 0 < loss , > 0 win ;
};

struct stMsgRoomResult
:public stMsg
{
    stMsgRoomResult(){ cSysIdentifer = ID_MSG_GAME2C ; usMsgType = MSG_ROOM_RESULT ;}
    char nCount ;
    stResultData* pResultData ;
};
#pragma pack(pop)