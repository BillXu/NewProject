#pragma once
#pragma pack(push)
#pragma pack(1)
// Define message , used between Servers ;mainly DBServer and GameServer 
#pragma warning(disable:4200)
#include "MessageDefine.h"
#include "ServerCommon.h"
#include "TaxasMessageDefine.h"
#define DBServer_PORT 5001
// WARNNING:变长字符串，我们不包括终结符 \0 ;

struct stMsgGateServerInfo
	:public stMsg
{
	stMsgGateServerInfo(){ cSysIdentifer = ID_MSG_PORT_GATE; usMsgType = MSG_GATESERVER_INFO;  }
	bool	 bIsGateFull; // if gate is full can not set up , center svr say , can not set up more gate server , if want add more pls modify serverConfig.txt
	uint16_t  uIdx ;
	uint16_t  uMaxGateSvrCount ;
};

struct stMsgClientDisconnect
	:public stMsg
{
public:
	stMsgClientDisconnect(){ cSysIdentifer = ID_MSG_PORT_CENTER ;  usMsgType = MSG_DISCONNECT_CLIENT ; }
	uint32_t nSeesionID ;
	//bool bIsForClientReconnect ; 
};

struct stMsgNewClientConnected
	:public stMsg
{
	stMsgNewClientConnected(){ cSysIdentifer = ID_MSG_PORT_CENTER ; usMsgType = MSG_CONNECT_NEW_CLIENT; }
	uint32_t nNewSessionID ;
};

struct stMsgRequestDBCreatePlayerData
	:public stMsg
{
	stMsgRequestDBCreatePlayerData(){ cSysIdentifer = ID_MSG_PORT_DB; usMsgType = MSG_REQUEST_CREATE_PLAYER_DATA ; }
	uint32_t nUserUID ;
};

struct stMsgOnPlayerLogin
	:public stMsg
{
	stMsgOnPlayerLogin(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_PLAYER_LOGIN ; }
	uint32_t nUserUID ;
};

// game and db 
struct stMsgDataServerGetBaseData
	:public stMsg
{
	stMsgDataServerGetBaseData(){cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_READ_PLAYER_BASE_DATA ; }
	unsigned int nUserUID ;
};

struct stMsgDataServerGetBaseDataRet
	:public stMsg
{
	stMsgDataServerGetBaseDataRet(){cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_READ_PLAYER_BASE_DATA ; }
	unsigned char nRet ; // 0 success , 1 not exsit  ;
	stServerBaseData stBaseData ;
};

// save player data ;
struct stMsgSavePlayerInfo   
	:public stMsg
{
	stMsgSavePlayerInfo(){ cSysIdentifer = ID_MSG_PORT_DB; usMsgType = MSG_PLAYER_SAVE_PLAYER_INFO ; }
	uint32_t nUserUID ;
	char vName[MAX_LEN_CHARACTER_NAME] ;
	char vSigure[MAX_LEN_SIGURE] ;
	uint8_t vUploadedPic[MAX_UPLOAD_PIC] ;
	uint16_t nPhotoID ;
};

struct stMsgSavePlayerMoney
	:public stMsg
{
	stMsgSavePlayerMoney(){cSysIdentifer = ID_MSG_PORT_DB; usMsgType = MSG_SAVE_PLAYER_MONEY ; }
	uint32_t nUserUID ;
	uint64_t nCoin ;
	uint32_t nDiamoned ;
};

struct stMsgSavePlayerTaxaPokerData
	:public stMsg
{
	stMsgSavePlayerTaxaPokerData(){cSysIdentifer = ID_MSG_PORT_DB; usMsgType = MSG_SAVE_PLAYER_TAXAS_DATA ; }
	uint32_t nUserUID ;
	stPlayerTaxasData tData ;
	uint32_t nFollowedRoomsStrLen ;
	PLACE_HOLDER(char* pFollowedRoomsJsonStr);
	uint32_t nMyOwnRoomsStrLen ;
	PLACE_HOLDER(char* pMyOwnRooms);
};

struct stMsgReadPlayerTaxasData
	:public stMsg
{
	stMsgReadPlayerTaxasData(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_READ_PLAYER_TAXAS_DATA ; }
	uint32_t nUserUID ;
};

struct stMsgReadPlayerTaxasDataRet
	:public stMsg
{
	stMsgReadPlayerTaxasDataRet(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_READ_PLAYER_TAXAS_DATA ; }
	uint8_t nRet ;
	uint32_t nUserUID ;
	stPlayerTaxasData tData ;
	uint32_t nFollowedRoomsStrLen ;
	PLACE_HOLDER(char* pFollowedRoomsJsonStr);
	uint32_t nMyOwnRoomsStrLen ;
	PLACE_HOLDER(char* pMyOwnRooms);
};

struct stMsgSaveCreateTaxasRoomInfo
	:public stMsg
{
	stMsgSaveCreateTaxasRoomInfo(){ cSysIdentifer = ID_MSG_PORT_DB; usMsgType = MSG_SAVE_CREATE_TAXAS_ROOM_INFO; }
	uint32_t nRoomID ;
	uint16_t nConfigID ;
	uint32_t nRoomOwnerUID ;
	uint32_t nCreateTime ;
	uint64_t nChatRoomID ;
};

struct stMsgSaveUpdateTaxasRoomInfo
	:public stMsg
{
	stMsgSaveUpdateTaxasRoomInfo(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_SAVE_UPDATE_TAXAS_ROOM_INFO ;}
	uint32_t nRoomID ;
	uint32_t nDeadTime ;
	uint16_t nAvataID ;
	uint32_t nInformSerial;
	uint64_t nRoomProfit;
	char vRoomName[MAX_LEN_ROOM_NAME];
	char vRoomDesc[MAX_LEN_ROOM_DESC];
	uint16_t nInformLen ;
	PLACE_HOLDER(char* pRoomInfom);
};

struct stMsgReadTaxasRoomInfo
	:public stMsg
{
	stMsgReadTaxasRoomInfo(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_READ_TAXAS_ROOM_INFO ; }
};

struct stMsgReadTaxasRoomInfoRet
	:public stMsg
{
	stMsgReadTaxasRoomInfoRet(){ cSysIdentifer = ID_MSG_PORT_TAXAS; usMsgType = MSG_READ_TAXAS_ROOM_INFO ; }
	uint32_t nRoomID ;
	uint16_t nConfigID ;
	uint32_t nRoomOwnerUID ;
	uint32_t nCreateTime ;
	uint32_t nDeadTime ;
	uint16_t nAvataID ;
	uint32_t nInformSerial;
	uint64_t nRoomProfit;
	uint64_t nChatRoomID ;
	char vRoomName[MAX_LEN_ROOM_NAME];
	char vRoomDesc[MAX_LEN_ROOM_DESC];
	uint16_t nInformLen ;
	PLACE_HOLDER(char* pRoomInfom);
};

struct stMsgSaveTaxasRoomPlayer
	:public stMsg
{
	stMsgSaveTaxasRoomPlayer(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_SAVE_TAXAS_ROOM_PLAYER ; }
	bool isUpdate ; // update or add 
	uint32_t nRoomID ;
	uint32_t nPlayerUID ;
	uint32_t m_nReadedInformSerial; 
	uint64_t nTotalBuyInThisRoom ; 
	uint64_t nFinalLeftInThisRoom ;  
	uint32_t nWinTimesInThisRoom ;
	uint32_t nPlayeTimesInThisRoom ;
};

struct stMsgSaveRemoveTaxasRoomPlayers
	:public stMsg
{
	stMsgSaveRemoveTaxasRoomPlayers(){cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_SAVE_REMOVE_TAXAS_ROOM_PLAYERS ;}
	uint32_t nRoomID ;
};

struct stMsgReadTaxasRoomPlayers
	:public stMsg
{
	stMsgReadTaxasRoomPlayers(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_READ_TAXAS_ROOM_PLAYERS ; }
	uint32_t nRoomID ;
};

struct stMsgReadTaxasRoomPlayersRet
	:public stMsgToRoom
{
	stMsgReadTaxasRoomPlayersRet(){ cSysIdentifer = ID_MSG_PORT_TAXAS; usMsgType = MSG_READ_TAXAS_ROOM_PLAYERS ; }
	uint32_t nPlayerUID ;
	uint32_t m_nReadedInformSerial; 
	uint64_t nTotalBuyInThisRoom ; 
	uint64_t nFinalLeftInThisRoom ;  
	uint32_t nWinTimesInThisRoom ;
	uint32_t nPlayeTimesInThisRoom ;
};


struct stMsgSavePlayerCommonLoginData
	:public stMsg
{
	stMsgSavePlayerCommonLoginData(){ cSysIdentifer = ID_MSG_PORT_DB ; usMsgType = MSG_SAVE_COMMON_LOGIC_DATA;  } 
	uint32_t nUserUID ;
	uint64_t nMostCoinEver;
	uint8_t nVipLevel ;
	int64_t nYesterdayCoinOffset ;
	int64_t nTodayCoinOffset ;
	uint32_t tOfflineTime ;  // last offline time ;
	uint32_t nContinueDays ;
	uint32_t tLastLoginTime;
	uint32_t tLastTakeCharityCoinTime ;
	double dfLongitude;
	double dfLatidue;
	uint32_t vJoinedClubID[MAX_JOINED_CLUB_CNT] ;
};

// data and taxas server 
struct stMsgRequestTaxasPlayerData
	:public stMsg
{
	uint32_t nRoomID ;
	stMsgRequestTaxasPlayerData(){ cSysIdentifer = ID_MSG_PORT_DATA; usMsgType = MSG_TP_REQUEST_PLAYER_DATA ; }
};

struct stMsgRequestTaxasPlayerDataRet
	:public stMsg
{
	stMsgRequestTaxasPlayerDataRet(){ cSysIdentifer = ID_MSG_PORT_TAXAS ; usMsgType = MSG_TP_REQUEST_PLAYER_DATA ; }
	uint8_t nRet ; // 0 succes , 1 not find player data , 2 already in another taxas room ;
	stTaxasInRoomPeerData tData ;
};



//struct stMsgPlayerRequestCoin
//	:public stMsg
//{
//	stMsgPlayerRequestCoin(){ cSysIdentifer = ID_MSG_PORT_DATA; usMsgType = MSG_REQUEST_MONEY ; nAtLeast = 0; }
//	uint8_t nReqType ;  // eReqMoneyType
//	uint32_t nUserUID ;  // if do not known , mark it 1 ;
//	uint32_t nSessionID ;
//	uint64_t nWantMoney;
//	uint64_t nAtLeast;
//	bool bIsDiamond ;
//	int32_t nBackArg[eReqMoneyArgCnt] ;
//};
//
//struct stMsgPlayerRequestCoinRet
//	:public stMsg
//{
//	stMsgPlayerRequestCoinRet(){ cSysIdentifer = ID_MSG_PORT_TAXAS; usMsgType = MSG_REQUEST_MONEY ; }
//	uint8_t nRet ; // 0 success , 1 not enough , 2 canot find player , 3 not in a taxas room 
//	uint8_t nReqType ; // eReqMoneyType
//	uint32_t nUserUID ;
//	uint64_t nAddedMoney;  // 0 means money not enough ;
//	bool bIsDiamond ;
//	int32_t nBackArg[eReqMoneyArgCnt] ;
//};
//
//struct stMsgTaxasPlayerRequestCoinComfirm
//	:public stMsg
//{
//	stMsgTaxasPlayerRequestCoinComfirm(){ cSysIdentifer = ID_MSG_PORT_DATA, usMsgType = MSG_REQUEST_MONEY_COMFIRM ; }
//	uint8_t nRet ; // 0 success , 1 failed;
//	bool bDiamond ;
//	uint64_t nWantedMoney ;
//	uint32_t nUserUID ;
//};

struct  stMsgSyncTaxasPlayerData
	:public stMsg
{
	stMsgSyncTaxasPlayerData(){ cSysIdentifer = ID_MSG_PORT_DATA,usMsgType = MSG_TP_SYNC_PLAYER_DATA ; }
	uint32_t nUserUID ;
	uint32_t nWinTimes ;
	uint32_t nPlayTimes ;
	uint64_t nSingleWinMost ;
};

struct stMsgInformTaxasPlayerLeave
	:public stMsg
{
	stMsgInformTaxasPlayerLeave(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_TP_INFORM_LEAVE; }
	uint32_t nUserUID ;
	//uint64_t nTakeInMoney ;
	//bool bIsDiamond ;
};

struct stMsgOrderTaxasPlayerLeave
	:public stMsgToRoom
{
	stMsgOrderTaxasPlayerLeave(){ cSysIdentifer = ID_MSG_PORT_TAXAS; usMsgType = MSG_TP_ORDER_LEAVE ; }
	uint32_t nSessionID ;
	uint32_t nUserUID ;
};

struct stMsgOrderTaxasPlayerLeaveRet
	:public stMsg
{
	stMsgOrderTaxasPlayerLeaveRet(){ cSysIdentifer = ID_MSG_PORT_DATA; usMsgType = MSG_TP_ORDER_LEAVE ; }
	uint32_t nUserUID ;
	uint8_t nRet ; // 0 success , 1 can not find room ;
};

// other with Verify Server ;
struct stMsgToVerifyServer
	:public stMsg
{
	stMsgToVerifyServer(){ cSysIdentifer = ID_MSG_PORT_VERIFY ; usMsgType = MSG_VERIFY_TANSACTION ; }
	unsigned int nBuyerPlayerUserUID ; 
	unsigned int nBuyForPlayerUserUID ;
	unsigned short nShopItemID ; // for mutilp need to verify ;
	unsigned short nTranscationIDLen ;
	char* ptransactionID ;   // base64 ed ;
};

struct stMsgFromVerifyServer
	:public stMsg
{
	stMsgFromVerifyServer(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_VERIFY_TANSACTION ; }
	unsigned int nBuyerPlayerUserUID ; 
	unsigned int nBuyForPlayerUserUID ;
	unsigned short nShopItemID ; // for mutilp need to verify ;
	unsigned char nRet ; // 0 apple check error iap free crack ,2 duplicate tansactionid  , 4 Success ;
};

// request ipAddress ;
struct stMsgLoginSvrInformGateSaveLog
	:stMsg
{
	stMsgLoginSvrInformGateSaveLog(){ cSysIdentifer = ID_MSG_PORT_GATE ; usMsgType = MSG_LOGIN_INFORM_GATE_SAVE_LOG ; }
	uint32_t nUserUID ;
	uint16_t nlogType ;
};

struct stMsgSaveLog
	:public stMsg
{
	stMsgSaveLog(){ cSysIdentifer = ID_MSG_PORT_LOG,usMsgType = MSG_SAVE_LOG; nJsonExtnerLen = 0 ; }
	uint16_t nLogType ;
	uint32_t nTargetID ;
	int64_t vArg[LOG_ARG_CNT];
	uint16_t nJsonExtnerLen ;
	PLACE_HOLDER(char* pJsonstring);
};

// cross server request 
struct stMsgCrossServerRequest
	:public stMsg
{
	stMsgCrossServerRequest(){ cSysIdentifer = ID_MSG_PORT_NONE; usMsgType = MSG_CROSS_SERVER_REQUEST ; nJsonsLen = 0 ;}
	uint32_t nTargetID ; // when represent for player , it is UID ;
	uint32_t nReqOrigID ; // when represent for player , it is UID ;
	uint16_t nRequestType ;
	uint16_t nRequestSubType ;
	int64_t vArg[CROSS_SVR_REQ_ARG];
	uint16_t nJsonsLen ;
	PLACE_HOLDER(char* pJsonString);
};

#define CON_REQ_MSG_JSON(msgCrossReq,jsonArg,autoBuf)  Json::StyledWriter jsWrite ;\
	std::string str = jsWrite.write(jsonArg) ; \
	msgCrossReq.nJsonsLen = strlen(str.c_str()); \
	CAutoBuffer autoBuf(sizeof(msgCrossReq) + msgCrossReq.nJsonsLen ); \
	autoBuf.addContent((char*)&msgCrossReq,sizeof(msgCrossReq)); \
	autoBuf.addContent(str.c_str(),msgCrossReq.nJsonsLen ) ;

struct stMsgCrossServerRequestRet
	:public stMsg
{
	stMsgCrossServerRequestRet(){ cSysIdentifer = ID_MSG_PORT_DATA; usMsgType = MSG_CROSS_SERVER_REQUEST_RET ; nJsonsLen = 0 ;}
	uint8_t nRet ;
	uint32_t nTargetID ; // when represent for player , it is UID ;
	uint32_t nReqOrigID ; // when represent for player , it is UID ;
	uint16_t nRequestType ;
	uint16_t nRequestSubType ;
	int64_t vArg[CROSS_SVR_REQ_ARG];
	uint16_t nJsonsLen ;
	PLACE_HOLDER(char* pJsonString);
};

// friend 
struct stMsgSaveFirendList
	:public stMsg
{
	stMsgSaveFirendList(){ cSysIdentifer = ID_MSG_PORT_DB ;usMsgType = MSG_SAVE_FRIEND_LIST ; }
	uint32_t nUserUID ;
	uint16_t nFriendCountLen ;
	PLACE_HOLDER(char* vFriendUIDS);
};

struct stMsgReadFriendList
	:public stMsg
{
	stMsgReadFriendList(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_READ_FRIEND_LIST ; }
	uint32_t nUserUID ;
};

struct stMsgReadFriendListRet
	:public stMsg
{
	stMsgReadFriendListRet(){ cSysIdentifer = ID_MSG_PORT_DATA ; usMsgType = MSG_READ_FRIEND_LIST ; }
	uint16_t nFriendCountLen ;
	PLACE_HOLDER(char* vFriendUIDS);
};

//----above is new , below is old---------

//--------------------------------------------
// message between DB and Gamesever ;
struct stMsgGM2DB
	:public stMsg
{
	stMsgGM2DB(){ cSysIdentifer = ID_MSG_GM2DB, usMsgType = MSG_NONE ; }
public:
	unsigned int nSessionID;
};

struct stMsgDB2GM
	:public stMsg
{
	stMsgDB2GM(){ cSysIdentifer = ID_MSG_DB2GM, usMsgType = MSG_NONE ; }
	unsigned int nSessionID;
};
//
//struct stMsgPushBaseDataToGameServer
//	:stMsgDB2GM
//{
//public:
//	stMsgPushBaseDataToGameServer(){usMsgType = MSG_PUSH_BASE_DATA ;}
//	stBaseData stData ;
//};

//-----------------------------------------------
// message between GateServer and Gamesever ;
//struct stMsgPeerDisconnect
//	:public stMsg
//{
//	stMsgPeerDisconnect()
//	{
//		cSysIdentifer = ID_MSG_GA2GM ;
//		usMsgType = MSG_DISCONNECT;
//	}
//	unsigned int nSessionID ;
//	
//};


struct stMsgGameServerCreateRole
	:public stMsgCreateRole
{
	stMsgGameServerCreateRole(){ cSysIdentifer = ID_MSG_GM2DB; usMsgType = MSG_CREATE_ROLE ; }
	unsigned int nSessionID ;
};

struct stMsgGameServerCreateRoleRet
	:public stMsgCreateRoleRet
{
	stMsgGameServerCreateRoleRet(){ cSysIdentifer = ID_MSG_DB2GM ; usMsgType = MSG_CREATE_ROLE ; }
	unsigned int nSessionID ;
};

// mail module 
struct stMsgGameServerSaveMail
	:public stMsgGM2DB
{
	stMsgGameServerSaveMail(){ usMsgType = MSG_PLAYER_SAVE_MAIL ; }
	unsigned int nUserUID ;
	unsigned char nOperateType ; // eDBAct ;
	stMail* pMailToSave ;
};

struct stMsgGameServerGetMailList
	:public stMsgGM2DB
{
	stMsgGameServerGetMailList(){ usMsgType = MSG_PLAYER_GET_MAIL_LIST ; }
	unsigned int nUserUID ;
};

struct stMsgGameServerGetMailListRet
	:public stMsgDB2GM
{
	stMsgGameServerGetMailListRet(){ usMsgType = MSG_PLAYER_GET_MAIL_LIST ; }
	unsigned short nMailCount ;
	stMail* pMails ;
};

struct stMsgGameServerGetMaxMailUID
	: public stMsgGM2DB 
{
	stMsgGameServerGetMaxMailUID(){ usMsgType = MSG_GAME_SERVER_GET_MAX_MAIL_UID ; }
};

struct stMsgGameServerGetMaxMailUIDRet
	:public stMsgDB2GM
{
	stMsgGameServerGetMaxMailUIDRet(){ usMsgType = MSG_GAME_SERVER_GET_MAX_MAIL_UID ; }
	uint64_t nMaxMailUID ;
};

// friend 
struct stServerSaveFrienItem
{
	unsigned int nFriendUserUID ;
	unsigned short nPresentTimes ;
};

struct stMsgGameServerRequestFirendList
	:public stMsgGM2DB
{
	stMsgGameServerRequestFirendList(){ cSysIdentifer = ID_MSG_GM2DB; usMsgType = MSG_REQUEST_FRIEND_LIST ; }
	unsigned int nUserUID ;
};

struct stMsgGameServerRequestFirendListRet
	:public stMsgDB2GM
{
	stMsgGameServerRequestFirendListRet(){ cSysIdentifer = ID_MSG_DB2GM; usMsgType = MSG_REQUEST_FRIEND_LIST ; }
	unsigned short nFriendCount ;
	stServerSaveFrienItem* pFirendsInfo;
};

////struct stMsgGameServerRequestFriendBrifDataList
////	:public stMsgGM2DB
////{
////	stMsgGameServerRequestFriendBrifDataList(){ cSysIdentifer = ID_MSG_GM2DB; usMsgType = MSG_REQUEST_FRIEND_BRIFDATA_LIST ; }
////	unsigned int nFriendCount ;
////	unsigned int* pFriendUserUIDs ;
////};

//struct stMsgGameServerRequestFriendBrifDataListRet
//	:public stMsgDB2GM
//{
//	stMsgGameServerRequestFriendBrifDataListRet(){ cSysIdentifer = ID_MSG_DB2GM ; usMsgType = MSG_REQUEST_FRIEND_BRIFDATA_LIST ; }
//	unsigned int nCount ;
//	stPlayerBrifData* pFriendBrifData ;
//};

////struct stMsgGameServerGetFriendDetail
////	:public stMsgGM2DB
////{
////	stMsgGameServerGetFriendDetail(){ cSysIdentifer = ID_MSG_GM2DB; usMsgType = MSG_PLAYER_REQUEST_FRIEND_DETAIL ; }
////	unsigned int nFriendUID ;
////};
////
////struct stMsgGameServerGetFriendDetailRet
////	:public stMsgDB2GM
////{
////	stMsgGameServerGetFriendDetailRet(){ usMsgType = MSG_PLAYER_REQUEST_FRIEND_DETAIL ; }
////	unsigned char nRet ; // 0 success ; 1 failed ;
////	stPlayerDetailData stPeerInfo ;
////};

struct stMsgGameServerGetSearchFriendResult
	:public stMsgGM2DB
{
	stMsgGameServerGetSearchFriendResult(){ cSysIdentifer = ID_MSG_GM2DB ;  usMsgType = MSG_PLAYER_SERACH_PEERS ; }
	char* pSearchContent ;
	unsigned char nLen ;
};

struct stMsgGameServerGetSearchFriendResultRet
	:public stMsgDB2GM
{
	stMsgGameServerGetSearchFriendResultRet(){ cSysIdentifer = ID_MSG_DB2GM ; usMsgType = MSG_PLAYER_SERACH_PEERS ; }
	unsigned char nResultCount ;
	stPlayerBrifData* pPeersInfo ;
};

//struct stMsgGameServerGetSearchedPeerDetail
//	:public stMsgGM2DB
//{
//	stMsgGameServerGetSearchedPeerDetail(){ usMsgType = MSG_PLAYER_REQUEST_SEARCH_PEER_DETAIL ; }
//	unsigned int nPeerUserUID ;
//};
//
//struct stMsgGameServerGetSearchedPeerDetailRet
//	:public stMsgDB2GM 
//{
//	stMsgGameServerGetSearchedPeerDetailRet(){ usMsgType = MSG_PLAYER_REQUEST_SEARCH_PEER_DETAIL ; }
//	unsigned char nRet ; // 0 success , 1 can not find ;
//	stPlayerDetailData stPeerInfo ;
//};
// item 
struct stMsgGameServerRequestItemList
	:public stMsgGM2DB
{
	stMsgGameServerRequestItemList(){usMsgType = MSG_REQUEST_ITEM_LIST ; }
	unsigned int nUserUID ;
};

struct stMsgGameServerRequestItemListRet 
	:public stMsgDB2GM
{
	stMsgGameServerRequestItemListRet(){ usMsgType = MSG_REQUEST_ITEM_LIST ; }
	unsigned short nOwnItemKindCount ;
	stPlayerItem* pItemsBuffer ;
};

struct stMsgGameServerSaveItemList
	:public stMsgGM2DB
{
	stMsgGameServerSaveItemList(){ usMsgType = MSG_SAVE_ITEM_LIST ; }
	unsigned int nUserUID ;
	unsigned short nOwnItemKindCount ;
	stPlayerItem* pItemsBuffer ;
};

// rank 
struct stMsgGameServerRequestRank
	:public stMsg
{
	stMsgGameServerRequestRank(){ usMsgType = MSG_REQUEST_RANK ; cSysIdentifer = ID_MSG_GM2DB ; }
	unsigned char eType ; // eRankType ;
};

//struct stRankPeerInfo
//	:public stRankPeerBrifInfo
//{
//	uint64_t nAllCoin ;
//	uint64_t nSingleWinMost ;
//	uint64_t nCurDayWin ;
//	unsigned int nDiamoned ;
//	char strSigure[MAX_LEN_SIGURE] ;
//	int nWinTimes ;
//	int nLostTimes ;
//	unsigned char eSex ;
//	unsigned int vAssetCount[eItem_Asset] ; 
//};

struct stServerGetRankPeerInfo
{
	stPlayerDetailData tDetailData ;
	uint64_t nYesterDayWin ;
};

struct stMsgGameServerRequestRankRet
	:public stMsg
{
	stMsgGameServerRequestRankRet(){ usMsgType = MSG_REQUEST_RANK ;cSysIdentifer = ID_MSG_DB2GM ; }
	unsigned char eType ; // eRankType ;
	unsigned char nPeerCount ;
	stServerGetRankPeerInfo* peers ;
};

// shop 
struct stMsgGameServerSaveShopBuyRecord
	:public stMsgGM2DB
{
public:
	stMsgGameServerSaveShopBuyRecord(){ usMsgType = MSG_SAVE_SHOP_BUY_RECORD ;}
	bool bAdd ; // or just update ;
	unsigned int nUserUID ;
	unsigned short nBufferLen ;
	char* pBuffer ;
};

struct stMsgGameServerGetShopBuyRecord
	:public stMsgGM2DB
{
	stMsgGameServerGetShopBuyRecord(){ usMsgType = MSG_GET_SHOP_BUY_RECORD ; }
	unsigned int nUserUID ;
};

struct stMsgGameServerGetShopBuyRecordRet
	:public stMsgDB2GM
{
	stMsgGameServerGetShopBuyRecordRet(){ usMsgType = MSG_GET_SHOP_BUY_RECORD ; }
	unsigned short nBufferLen ;
	char* pBuffer ;
};

// mission 
struct stMsgGameServerSaveMissionData
	:public stMsgGM2DB
{
	stMsgGameServerSaveMissionData(){ usMsgType = MSG_GAME_SERVER_SAVE_MISSION_DATA ; }
	unsigned int nUserUID ;
	unsigned int nSavetime ;
	unsigned short nMissonCount ;
	stMissionSate* pMissonData ;
};

struct stMsgGameServerGetMissionData
	:public stMsgGM2DB
{
	stMsgGameServerGetMissionData(){ usMsgType = MSG_GAME_SERVER_GET_MISSION_DATA ; }
	unsigned int nUserUID ;
};

struct stMsgGameServerGetMissionDataRet
	:public stMsgDB2GM
{
	stMsgGameServerGetMissionDataRet(){ usMsgType = MSG_GAME_SERVER_GET_MISSION_DATA ; }
	unsigned int nLastSaveTime ;
	unsigned short nMissonCount ;
	stMissionSate* pMissonData ;
};

// other game msg;


struct stMsgTransferData
	:public stMsg
{
	stMsgTransferData()
	{
		cSysIdentifer = ID_MSG_TRANSFER ;
		usMsgType = MSG_TRANSER_DATA;
		bBroadCast = false ;
		nSessionID = 0 ;
	}
	uint16_t nSenderPort ; // who send this msg ;  eMsgPort
	uint32_t nSessionID ;
	bool bBroadCast ;
	char pData[0] ;
};

// push server message 
struct stMsgToAPNSServer
	:public stMsg
{
	stMsgToAPNSServer(){ cSysIdentifer = ID_MSG_TO_APNS_SERVER, usMsgType = MSG_APNS_INFO; }
	unsigned short nBadge ; // >= 1 ;
	char pDeveiceToken[32] ;  // must proccesed in client ; change to htonl();  // change to network big endain ;
	char* cSound;   // sound file name ; the file must exsit in client ;
	unsigned char nSoundLen ; // < 50 ;
	char* pAlert ;   // a json right value , Warnning: must include [" " ] if not { } value ;
	unsigned char nAlertLen ; // < 219 ;
};

// DB log ;
enum eDBLog
{
	eDBLog_None,
	eDBLog_Login,
	eDBLog_Impawn,
	eDBLog_MissionReward,
	eDBLog_Shop,
	eDBLog_OtherMoneyOffset,
	eDBLog_StayInRoom,
	eDBLog_PresentAsset,
	eDBLog_RobotCoin,
	eDBLog_Max,
};

struct stDBLog
{
	eDBLog eLogType ;
	unsigned int nUserUID ;
	char cPlayerName[MAX_LEN_CHARACTER_NAME] ;
	uint64_t nCurCoin ;
	unsigned int nCurDiamond ;
	stDBLog(){ eLogType = eDBLog_Max;}
};

struct stPlayerLoginDBLog
	:public stDBLog
{
	unsigned int nLoginTime ;
	stPlayerLoginDBLog(){eLogType = eDBLog_Login ;}
};

struct stPlayerImpawnDBLog
	:public stDBLog
{
	unsigned short nImpawnItemID ;
	unsigned int nItemCount ;
	uint64_t nImpawnCoin ;
	unsigned int nImpawnDiamoned ;
	stPlayerImpawnDBLog(){eLogType = eDBLog_Impawn ;}
};

struct stPlayerGetMissionRewardDBlog
	:public stDBLog
{
	unsigned short nMissionID ;
	unsigned int nGetCoin ;
	stPlayerGetMissionRewardDBlog(){eLogType = eDBLog_MissionReward ;}
};

struct stPlayShopDBLog
	:public stDBLog
{
	unsigned char nMoneyType ; //0 钻石， 1 金币，3  RMB
	unsigned int nSpendMoney;
	unsigned char cChannel ; //渠道标示，0. appstore  1. pp 助手，2.  91商店 3. 360商店 4.winphone store , 100 淘宝充值
	unsigned short nShopID ;
	unsigned int nShopCnt ;
	stPlayShopDBLog(){eLogType = eDBLog_Shop ;}
};

struct stPlayerOtherMoneyActDBLog
	:public stDBLog
{
	unsigned char cActType ; // 0 好友免费赠送，1 师傅关系获得 ， 2  徒弟关系获得
	int nCoinOffset ;
	int nDiamondOffset ;
	stPlayerOtherMoneyActDBLog(){eLogType = eDBLog_OtherMoneyOffset ;}
};

struct stPlayerInRoomDBLog
	:public stDBLog
{
	unsigned int nRoomID ;
	uint64_t nCoinBeforEnterRoom ;
	unsigned int nEnterRoomTime ;
	stPlayerInRoomDBLog(){eLogType = eDBLog_StayInRoom ;}
};

struct stPlayerPresentAssertDBLog
	:public stDBLog
{
	unsigned int nTargetPlayerUID ;
	unsigned short nAssertID ;
	unsigned short nAssetCount ;
	unsigned short nPresentReason ; // 0 普通赠送， 1 拜师赠送 
	stPlayerPresentAssertDBLog(){eLogType = eDBLog_PresentAsset ;}
};

struct stRobotCoinDBLog
	:public stDBLog
{
	int64_t vRoomLevelRobotOffset[eRoomLevel_Max] ;
	int64_t vAllRobotOffset ;
	stRobotCoinDBLog(){eLogType = eDBLog_RobotCoin ;}
};

struct stMsgToLogDBServer
	:public stMsg
{
	stMsgToLogDBServer(){ cSysIdentifer = ID_MSG_TO_GM2LOG; usMsgType = MSG_SAVE_DB_LOG; }
	stDBLog* pLogContent ;
};

#pragma pack(pop)//
