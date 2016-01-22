#pragma once
#pragma pack(push)
#pragma pack(1)
#pragma warning(disable:4200)
#include "MessageDefine.h"
#include "CommonData.h"



struct stMsgCreateTaxasRoom
	:public stMsg
{
	stMsgCreateTaxasRoom(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_TP_CREATE_ROOM ; }
	uint16_t nConfigID ;
	uint16_t nDays ;
	char vRoomName[MAX_LEN_ROOM_NAME] ;
};

struct stMsgCreateTaxasRoomRet
	:public stMsg
{
	stMsgCreateTaxasRoomRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_TP_CREATE_ROOM ; }
	uint8_t nRet ; // 0 success , 1 config error , 2 no more chat room id , 3 can not connect to chat svr , 4 coin not enough , 5 reach your own room cnt up limit
	uint32_t nRoomID ; 
	uint64_t nFinalCoin ;
};

struct stMsgRequestMyOwnRooms
	:public stMsg
{
	stMsgRequestMyOwnRooms(){ cSysIdentifer = ID_MSG_PORT_DATA; usMsgType = MSG_REQUEST_MY_OWN_ROOMS; }
	uint8_t nRoomType ;
};

struct stMsgRequestMyOwnRoomsRet
	:public stMsg
{
	stMsgRequestMyOwnRoomsRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_REQUEST_MY_OWN_ROOMS ; }
	uint16_t nCnt ;
	uint8_t nRoomType ;
	PLACE_HOLDER(uint32_t* pRoomIDs);
};

struct stMsgRequestMyOwnRoomDetail
	:public stMsgToRoom
{
	stMsgRequestMyOwnRoomDetail(){ cSysIdentifer = ID_MSG_PORT_TAXAS ; usMsgType = MSG_REQUEST_MY_OWN_ROOM_DETAIL ; }
};

struct stMsgRequestMyOwnRoomDetailRet
	:public stMsg
{
	stMsgRequestMyOwnRoomDetailRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_REQUEST_MY_OWN_ROOM_DETAIL ; }
	uint8_t nRet ; // 0 success , 1 can not find room ;
	uint8_t nRoomType ; 
	char vRoomName[MAX_LEN_ROOM_NAME] ;
	uint32_t nRoomID ;
	uint16_t nConfigID ;
	uint64_t nTotalProfit;
	uint32_t nCanWithdrawProfit;
	uint32_t nDeadTime;
	uint32_t nFollows ; 
};


struct stMsgRequestTaxasRoomInfo
	:public stMsgToRoom
{
	stMsgRequestTaxasRoomInfo(){ cSysIdentifer = ID_MSG_PORT_TAXAS; usMsgType = MSG_TP_REQUEST_ROOM_INFO ; }
};

struct stMsgModifyTaxasRoomName
	:public stMsgToRoom
{
	stMsgModifyTaxasRoomName(){ cSysIdentifer = ID_MSG_PORT_TAXAS; usMsgType = MSG_TP_MODIFY_ROOM_NAME ; }
	char vNewRoomName[MAX_LEN_ROOM_NAME] ;
};

struct stMsgModifyTaxasRoomNameRet
	:public stMsg
{
	stMsgModifyTaxasRoomNameRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_TP_MODIFY_ROOM_NAME ; }
	uint8_t nRet ; // 0 sucess , 1 you are not creator , 2 room is dead , please pay rent fee 3 , you are not in room;
};


struct  stMsgModifyTaxasRoomDesc
	:public stMsgToRoom
{
	stMsgModifyTaxasRoomDesc(){ cSysIdentifer = ID_MSG_PORT_TAXAS ; usMsgType = MSG_TP_MODIFY_ROOM_DESC ; }
	uint8_t nLen ;
	PLACE_HOLDER(char* pNewName);
};

struct stMsgModifyTaxasRoomDescRet
	:public stMsg
{
	stMsgModifyTaxasRoomDescRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_TP_MODIFY_ROOM_DESC ; }
	uint8_t nRet ; // 0 sucess , 1 you are not creator , 2 room is dead , please pay rent fee ; 3 , you are not in room, 4 desc is too long;
};

struct  stMsgModifyTaxasInform
	:public stMsgToRoom
{
	stMsgModifyTaxasInform(){ cSysIdentifer = ID_MSG_PORT_TAXAS ; usMsgType = MSG_TP_MODIFY_ROOM_INFORM ; }
	uint8_t nLen ;
	PLACE_HOLDER(char* pNewInfom);
};

struct stMsgModifyTaxasInformRet
	:public stMsg
{
	stMsgModifyTaxasInformRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_TP_MODIFY_ROOM_INFORM ; }
	uint8_t nRet ; // 0 sucess , 1 you are not creator , 2 room is dead , please pay rent fee ; 3 , you are not in room;
};

typedef stMsgCaculateRoomProfit stMsgCaculateTaxasRoomProfit;
typedef stMsgCaculateRoomProfitRet stMsgCaculateTaxasRoomProfitRet ;

struct stMsgRemindTaxasRoomNewInform
	:public stMsg
{
	stMsgRemindTaxasRoomNewInform(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_TP_REMIND_NEW_ROOM_INFORM ; }
};

struct stMsgRequestTaxasRoomInform
	:public stMsgToRoom
{
	stMsgRequestTaxasRoomInform(){ cSysIdentifer = ID_MSG_PORT_TAXAS ; usMsgType = MSG_TP_REQUEST_ROOM_INFORM ; }
};

struct  stMsgRequestTaxasRoomInformRet
	:public stMsg
{
	stMsgRequestTaxasRoomInformRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_TP_REQUEST_ROOM_INFORM ; }
	uint16_t nLen ;
	PLACE_HOLDER(char* pInform);
};

struct stMsgTaxasEnterRoom
	:public stMsg
{
	stMsgTaxasEnterRoom(){ cSysIdentifer = ID_MSG_PORT_TAXAS; usMsgType = MSG_TP_ENTER_ROOM ; }
	uint8_t nIDType ; // 0 roomID , 1 RoomConfigID ;
	uint32_t nTargetID ;
};

struct stMsgTaxasEnterRoomRet 
	:public stMsg
{
	stMsgTaxasEnterRoomRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_TP_ENTER_ROOM ; }
	uint8_t nRet ; // 0 success , 1 invalid session id ;2 already in room ; 3 room id error ;
};

struct  stMsgTaxasQuickEnterRoom
	:public stMsg
{
	stMsgTaxasQuickEnterRoom()
	{
		cSysIdentifer = ID_MSG_PORT_TAXAS ; usMsgType = MSG_TP_QUICK_ENTER ;
	}
	uint64_t nCurCoin ;
};

struct stMsgTaxasRoomInfoBase
	:public stMsg
{
	stMsgTaxasRoomInfoBase(){ cSysIdentifer = ID_MSG_PORT_CLIENT; usMsgType = MSG_TP_ROOM_BASE_INFO; }
	uint32_t nRoomID ;
	uint32_t nOwnerUID ;
	uint16_t nAvataID ;
	uint8_t vRoomName[MAX_LEN_ROOM_NAME];
	uint8_t nMaxSeat;
	uint32_t nLittleBlind;
	uint32_t nMiniTakeIn ;
	uint64_t nMaxTakeIn ;
	uint64_t nChatRoomID;
	// running members ;
	uint32_t eCurRoomState ; // eeRoomState ;
	uint8_t nBankerIdx ;
	uint8_t nLittleBlindIdx ;
	uint8_t nBigBlindIdx ;
	int8_t nCurWaitPlayerActionIdx ;
	uint64_t  nCurMainBetPool ;
	uint64_t  nMostBetCoinThisRound;
	uint32_t nDeskFee;
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

struct stMsgRequestRoomList
	:public stMsg
{
	stMsgRequestRoomList(){ cSysIdentifer = ID_MSG_PORT_TAXAS ; usMsgType = MSG_TP_REQUEST_ROOM_LIST; }
};

struct stRoomListItem
{
	uint32_t nCreatOwnerUID;
	uint16_t nRoomID ;
	char vRoomName[MAX_LEN_ROOM_NAME];
	uint8_t nCurrentCount ;
	uint32_t nSmiallBlind ;
	uint8_t nSeatCnt;
	uint32_t nDeadTime ;
	//char vDesc[MAX_LEN_ROOM_DESC];
};

struct stMsgRequestRoomListRet
	:public stMsg
{
	stMsgRequestRoomListRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_REQUEST_ROOM_LIST ; }
	uint8_t nRoomType ;
	bool bFinal;
	uint8_t nListCnt ;
	PLACE_HOLDER(stRoomListItem* pRoomItem );
};


struct stMsgRequestMyFollowRooms 
	:public stMsg
{
	stMsgRequestMyFollowRooms(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_REQUEST_MY_FOLLOW_ROOMS ; }
};

struct stMsgRequestMyFollowRoomsRet
	:public stMsg
{
	stMsgRequestMyFollowRoomsRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_REQUEST_MY_FOLLOW_ROOMS ; }
	uint16_t nCnt ;
	PLACE_HOLDER(uint32_t* vRoomIDs);
};

struct stMsgRequestRoomDetail
	:public stMsgToRoom
{
	stMsgRequestRoomDetail(){ cSysIdentifer = ID_MSG_PORT_TAXAS ; usMsgType = MSG_REQUEST_ROOM_DETAIL ;}
};

struct stMsgRequestRoomDetailRet
	:public stMsg
{
	stMsgRequestRoomDetailRet(){ cSysIdentifer = ID_MSG_PORT_CLIENT ; usMsgType = MSG_REQUEST_ROOM_DETAIL ; }
	stRoomListItem detailInfo;
};

struct stMsgRequestTaxasRoomRank
	:public stMsgToRoom
{
	stMsgRequestTaxasRoomRank(){ usMsgType = MSG_TP_REQUEST_ROOM_RANK ;}
};

struct stTaxasRoomRankItem
{
	uint32_t nUID ;
	int64_t nCoinOffset ;
	uint64_t nTotoalBuyIn;
};

struct stMsgRequestTaxasRoomRankRet
	:public stMsg
{
	stMsgRequestTaxasRoomRankRet(){ usMsgType = MSG_TP_REQUEST_ROOM_RANK ; cSysIdentifer = ID_MSG_PORT_CLIENT ; }
	bool bLast;
	uint8_t nCnt ;
	PLACE_HOLDER(stTaxasRoomRankItem*);
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