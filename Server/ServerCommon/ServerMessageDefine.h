#pragma once
#pragma pack(push)
#pragma pack(1)
// Define message , used between Servers ;mainly DBServer and GameServer 
#include "MessageDefine.h"
#include "BaseData.h"
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
	bool bIsForClientReconnect ; 
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

//----above is new , below is ok---------

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
// game and db 
struct stMsgGameServerGetBaseData
	:public stMsgGM2DB
{
	stMsgGameServerGetBaseData(){cSysIdentifer = ID_MSG_GM2DB ; usMsgType = MSG_PLAYER_BASE_DATA ; }
	unsigned int nUserUID ;
};

struct stMsgGameServerGetBaseDataRet
	:public stMsgDB2GM
{
	stMsgGameServerGetBaseDataRet(){cSysIdentifer = ID_MSG_DB2GM ; usMsgType = MSG_PLAYER_BASE_DATA ; }
	unsigned char nRet ; // 0 success , 1 not exsit  ;
	stServerBaseData stBaseData ;
};

struct stMsgGameServerSaveBaseData
	:public stMsgGM2DB
{
	stMsgGameServerSaveBaseData(){ cSysIdentifer = ID_MSG_GM2DB; usMsgType = MSG_PLAYER_SAVE_BASE_DATA ; }
	stServerBaseData stBaseData ;
};

struct stMsgGameServerSavePlayerCoin
	:public stMsgGM2DB
{
	stMsgGameServerSavePlayerCoin(){cSysIdentifer = ID_MSG_GM2DB; usMsgType = MSG_SAVE_PLAYER_COIN ; }
	unsigned int nUserUID ;
	uint64_t nCoin ;
	unsigned int nDiamoned ;
};

struct stMsgGameServerSavePlayerCoinRet
	:public stMsgDB2GM
{
	stMsgGameServerSavePlayerCoinRet(){ ID_MSG_DB2GM ; usMsgType = MSG_SAVE_PLAYER_COIN ; }
	unsigned char nRet ; // 0 success , 1 failed ;
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

struct stMsgGameServerRequestFriendBrifDataList
	:public stMsgGM2DB
{
	stMsgGameServerRequestFriendBrifDataList(){ cSysIdentifer = ID_MSG_GM2DB; usMsgType = MSG_REQUEST_FRIEND_BRIFDATA_LIST ; }
	unsigned int nFriendCount ;
	unsigned int* pFriendUserUIDs ;
};

struct stMsgGameServerRequestFriendBrifDataListRet
	:public stMsgDB2GM
{
	stMsgGameServerRequestFriendBrifDataListRet(){ cSysIdentifer = ID_MSG_DB2GM ; usMsgType = MSG_REQUEST_FRIEND_BRIFDATA_LIST ; }
	unsigned int nCount ;
	stPlayerBrifData* pFriendBrifData ;
};

struct stMsgGameServerSaveFirendList
	:public stMsgGM2DB
{
	stMsgGameServerSaveFirendList(){ cSysIdentifer = ID_MSG_GM2DB ;usMsgType = MSG_SAVE_FRIEND_LIST ; }
	unsigned int nUserUID ;
	unsigned short nFriendCount ;
	stServerSaveFrienItem* pFriends;
};

struct stMsgGameServerSaveFriendListRet
	:public stMsgDB2GM
{
	stMsgGameServerSaveFriendListRet(){ cSysIdentifer = ID_MSG_DB2GM ; usMsgType = MSG_SAVE_FRIEND_LIST ; }
	unsigned char nRet ; // 0 means success ; other value failed ;
};

struct stMsgGameServerGetFriendDetail
	:public stMsgGM2DB
{
	stMsgGameServerGetFriendDetail(){ cSysIdentifer = ID_MSG_GM2DB; usMsgType = MSG_PLAYER_REQUEST_FRIEND_DETAIL ; }
	unsigned int nFriendUID ;
};

struct stMsgGameServerGetFriendDetailRet
	:public stMsgDB2GM
{
	stMsgGameServerGetFriendDetailRet(){ usMsgType = MSG_PLAYER_REQUEST_FRIEND_DETAIL ; }
	unsigned char nRet ; // 0 success ; 1 failed ;
	stPlayerDetailData stPeerInfo ;
};

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

struct stMsgGameServerGetSearchedPeerDetail
	:public stMsgGM2DB
{
	stMsgGameServerGetSearchedPeerDetail(){ usMsgType = MSG_PLAYER_REQUEST_SEARCH_PEER_DETAIL ; }
	unsigned int nPeerUserUID ;
};

struct stMsgGameServerGetSearchedPeerDetailRet
	:public stMsgDB2GM 
{
	stMsgGameServerGetSearchedPeerDetailRet(){ usMsgType = MSG_PLAYER_REQUEST_SEARCH_PEER_DETAIL ; }
	unsigned char nRet ; // 0 success , 1 can not find ;
	stPlayerDetailData stPeerInfo ;
};
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

// other with Verify Server ;
struct stMsgToVerifyServer
	:public stMsg
{
	stMsgToVerifyServer(){ cSysIdentifer = ID_MSG_TO_VERIFY_SERVER ; usMsgType = MSG_VERIFY_TANSACTION ; }
	unsigned int nBuyerPlayerUserUID ; 
	unsigned int nBuyForPlayerUserUID ;
	unsigned short nShopItemID ; // for mutilp need to verify ;
	unsigned short nTranscationIDLen ;
	char* ptransactionID ;   // base64 ed ;
};

struct stMsgFromVerifyServer
	:public stMsg
{
	stMsgFromVerifyServer(){ cSysIdentifer = ID_MSG_FROM_VERIFY_SERVER ; usMsgType = MSG_VERIFY_TANSACTION ; }
	unsigned int nBuyerPlayerUserUID ; 
	unsigned int nBuyForPlayerUserUID ;
	unsigned short nShopItemID ; // for mutilp need to verify ;
	unsigned char nRet ; // 0 apple check error iap free crack ,2 duplicate tansactionid  , 4 Success ;
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
