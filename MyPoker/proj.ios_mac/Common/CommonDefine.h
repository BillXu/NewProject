#pragma once
#include "NativeTypes.h"
#define MAX_LEN_ACCOUNT 50   // can not big then unsigned char max = 255
#define  MAX_LEN_PASSWORD 50 // can not big then unsigned char max = 255
#define MAX_LEN_CHARACTER_NAME 50 // can not big then unsigned char  max = 255
#define MAX_LEN_SIGURE 100   // can not big then unsigned char  max = 255
#define MAX_LEN_EMAIL 50
#define MAX_LEN_SPEAK_WORDS 200  
#define MAX_MSG_BUFFER_LEN 2048*3
#define MAX_LEN_ROOM_NAME 25
#define RESEVER_GAME_SERVER_PLAYERS 100 
#define PEER_CARD_COUNT 3
#define GOLDEN_PEER_CARD 3
#define TAXAS_PEER_CARD 2
#define TAXAS_PUBLIC_CARD 5
#define MAX_ROOM_PEER 5
#define MAX_TAXAS_HOLD_CARD 5

#define COIN_CONDITION_TO_GET_CHARITY 1000
#define TIME_GET_CHARITY_ELAPS 60*60*2   // 2 HOURE
#define COIN_FOR_CHARITY 20000
#define GOLDEN_ROOM_COIN_LEVEL_CNT 4
#define GOLDEN_PK_ROUND 2

#ifndef GAME_SERVER
#define PIEXL_TO_POINT(px) (px)/CC_CONTENT_SCALE_FACTOR()
#define FOINT_NAME "Helvetica"
#endif
enum eRoomType
{
	eRoom_None,
	eRoom_PaiJiu,
	eRoom_TexasPoker,
	eRoom_TexasPoker_Diamoned,
	eRoom_TexasPoker_Private,
	eRoom_Gold,
	eRoom_Baccarat,
	eRoom_Max ,
};

enum eRoomState
{
	eRoomState_None,
	// state for golden 
	eRoomState_Golden_WaitPeerToJoin = eRoomState_None,
	eRoomState_Golden_WaitPeerReady,
	eRoomState_Golden_DistributeCard,
	eRoomState_Golden_WaitPeerAction,
	eRoomState_Golden_PKing,
	eRoomState_Golden_ShowingResult,
	// state for pai_jiu
	eRoomState_PJ_WaitBanker,
	eRoomState_PJWaitNewBankerChoseShuffle,
	eRoomState_PJ_Shuffle,   // xi pai
	eRoomState_PJ_WaitForBet,
	eRoomState_PJ_Dice, // shuai zi 
	eRoomState_PJ_Distribute,
	eRoomState_PJ_ShowCard,   // fan kai xian jia de pai 
	eRoomState_PJ_Settlement, // jie suan ;
	eRoomState_PJ_BankerSelectGoOn,
	// state for texas poker
	eRoomState_TP_WaitJoin = eRoomState_None,
	eRoomState_TP_Player_Distr,
	eRoomState_TP_Distr_Public,
	eRoomState_TP_Wait_Bet,
	eRoomState_TP_Caculate_Round,
	eRoomState_TP_Caculate_GameResult,

	// state for Baccarat ;
	eRoomState_BC_Shuffle,
	eRoomState_BC_WaitBet,
	eRoomState_BC_Distribute,
	eRoomState_BC_AddIdleCard,
	eRoomState_BC_AddBankerCard,
	eRoomState_BC_Caculate,
	eRoomState_Max,
};
// ROOM TIME BY SECOND 
#define TIME_ROOM_WAIT_READY 5
#define TIME_ROOM_DISTRIBUTE 5
#define TIME_ROOM_WAIT_PEER_ACTION 30
#define TIME_ROOM_PK_DURATION 5
#define TIME_ROOM_SHOW_RESULT 5

// Golden room time 
#define TIME_GOLDEN_ROOM_WAIT_READY 10
#define TIME_GOLDEN_ROOM_DISTRIBUTY 3
#define TIME_GOLDEN_ROOM_WAIT_ACT 10
#define TIME_GOLDEN_ROOM_PK 4
#define TIME_GOLDEN_ROOM_RESULT 5

static unsigned char s_vChangeCardDimonedNeed[GOLDEN_PEER_CARD] = {0,4,8} ;

enum eDBAct
{
	eDBAct_Add,
	eDBAct_Delete,
	eDBAct_Update,
	eDBAct_Select,
	eDBAct_Max,
};

enum eSpeed
{
	eSpeed_Normal,
	eSpeed_Quick,
	eSpeed_Max,
};

enum eRoomSeat
{
	eSeatCount_5,
	eSeatCount_9,
	eSeatCount_Max,
};
// player State 
enum eRoomPeerState
{
	eRoomPeer_None,
	// peer state for golden peer 
	eRoomPeer_Golden_Playing = 1 ,
	eRoomPeer_Golden_WaitNextPlay = 1 << 1 ,
	eRoomPeer_Golden_WaitToReady = 1 << 2 ,
	eRoomPeer_Golden_Ready  = (1 << 3) | eRoomPeer_Golden_Playing  ,
    eRoomPeer_Golden_Look = (1 << 4) | eRoomPeer_Golden_Playing ,
	eRoomPeer_Golden_GiveUp = 1 << 5,
	eRoomPeer_Golden_PK_Failed = 1 << 6 ,

	// peer state for taxas poker peer
	eRoomPeer_SitDown = 1,
	eRoomPeer_StandUp = 1 << 1,
	eRoomPeer_StayThisRound = ((1 << 2)|eRoomPeer_SitDown) ,
	eRoomPeer_WaitCaculate = ((1 << 7)|eRoomPeer_StayThisRound ),
	eRoomPeer_AllIn = ((1 << 3)|eRoomPeer_WaitCaculate) ,
	eRoomPeer_GiveUp = ((1 << 4)|eRoomPeer_StayThisRound),
	eRoomPeer_CanAct = ((1 << 5)|eRoomPeer_WaitCaculate),
	eRoomPeer_WaitNextGame = ((1 << 6)|eRoomPeer_SitDown ),
	eRoomPeer_Max,
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
	eSvrType_Max,
};

enum eSex
{
	eSex_Unknown,
	eSex_Male,
	eSex_Female,
	eSex_Max,
};

enum eRoomPeerAction
{
	eRoomPeerAction_None,
	eRoomPeerAction_EnterRoom,
	eRoomPeerAction_Ready,
	eRoomPeerAction_Follow,
	eRoomPeerAction_Add,
	eRoomPeerAction_PK,
	eRoomPeerAction_GiveUp,
	eRoomPeerAction_ShowCard,
	eRoomPeerAction_ViewCard,
	eRoomPeerAction_TimesMoneyPk,
	eRoomPeerAction_LeaveRoom,
	eRoomPeerAction_Speak_Default,
	eRoomPeerAction_Speak_Text,
	// action for 
	eRoomPeerAction_Pass,
	eRoomPeerAction_AllIn,
	eRoomPeerAction_SitDown,
	eRoomPeerAction_StandUp,
	eRoomPeerAction_Max
};

enum eRoomFlag
{
	eRoomFlag_None ,
	eRoomFlag_ShowCard  ,
	eRoomFlag_TimesPK ,
	eRoomFlag_ChangeCard,
	eRoomFlag_Max,
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
// mail Module 
#define MAX_KEEP_MAIL_COUNT 50
enum eMailType
{
	eMail_Message,
	eMail_Friend,
	eMail_System,
	eMail_PresentGift,
	eMail_UnProcessedPurchaseVerify,
	eMail_BeAddedFriend,
	eMail_Max,
};

enum eProcessMailAct
{
	ePro_Mail_None,
	ePro_Mail_Delete,
	ePro_Mail_DoYes,
	ePro_Mail_DoNo,
	ePro_Mail_Look,
	ePor_Mail_Max,
};

// item id , here type = id ;
enum eItemType
{
	eItem_None,
	eItem_Car = eItem_None,
	eItem_Boat,
	eItem_Airplane,
	eItem_House,
	eItem_Asset, // uplow are assets ;
	// below are can be used item ;
	eItem_Props , // can be used item ;
	eItem_Gift,
	eItem_Max,
};

#define ITEM_ID_INTERACTIVE 10
#define ITEM_ID_LA_BA 12
#define ITEM_ID_KICK_CARD 11
#define ITEM_ID_CREATE_ROOM 13
// game ranker
enum eRankType
{
	eRank_AllCoin,
	eRank_SingleWinMost,
	eRank_YesterDayWin,
	eRank_Max,
};
#define RANK_SHOW_PEER_COUNT 50


#define MAX_PAIJIU_HISTROY_RECORDER 20

enum eRoomLevel
{
	eRoomLevel_None,
	eRoomLevel_Junior = eRoomLevel_None ,
	eRoomLevel_Middle,
	eRoomLevel_Advanced,
	eRoomLevel_Super,
	eRoomLevel_Max,
};

// texas poker timer measus by second
#define TIME_TAXAS_FILP_CARD 0.6
#define TIME_TAXAS_BET 12
#define TIME_TAXAS_CAL_ROUND_STAY 1.0
#define TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL 0.7f
#define TIME_TAXAS_MAKE_VICE_POOLS 0.15f
#define TIME_TAXAS_DISTRIBUTE_PER_PEER (TIME_TAXAS_FILP_CARD + 1.0)
#define TIME_TAXAS_ONE_CARD_DISTRIBUTE (TIME_TAXAS_FILP_CARD + 0.5)
#define TIME_TAXAS_CACULATE_PER_BET_POOL 1.5f
#define TIME_TAXAS_CACULATE_FINAL 2
#define TIME_TAXAS_RESULT_STAY 1.0
#define MIN_PEERS_IN_ROOM_ROBOT 6
#define MAX_PEERS_IN_ROOM 9
#define TIME_LOW_LIMIT_FOR_NORMAL_ROOM 10
// baccarat define 
enum eBaccaratBetPort
{
	eBC_BetPort_One,
	eBC_BetPort_BankerWin = eBC_BetPort_One,
	eBC_BetPort_IdleWin,
	eBC_BetPort_TheSame,
	eBC_BetPort_BankerPair,
	eBC_BetPort_IdlePair,
	eBC_BetPort_Max,
};

#define SETTING_MUSIC_ON "MusicOn"
#define SETTING_SOUND_ON "SoundOn"

#define LOCAL_ACCOUNT "account"
#define LOCAL_PASSWORD "password"
#define IS_AUTO_REGISTER "IsAutoLogin"
#define IS_CREATE_ROLE "IsCreateRole"

#define TEMP_NAME "tempName"
#define TEMP_ACCOUNT "tempAccount"
#define TEMP_PASSWORD "tempPassword"

#define MUSIC_NAME "mp3/bgm2.mp3"
#define SOUND_BET "mp3/bet.mp3"
#define SOUND_PASS "mp3/check.mp3"
#define SOUND_CLOCK "mp3/clock.mp3"
#define SOUND_SHOW_PUBLIC "mp3/flop.mp3"
#define SOUND_GIVE_UP "mp3/fold.mp3"
#define SOUND_WIN_CHIP "mp3/movechips.mp3"
#define SOUND_MY_TURN "mp3/myturn.mp3"
#define SOUND_SHOW_SHOP "mp3/shopBell.mp3"

