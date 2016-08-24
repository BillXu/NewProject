#pragma once
#include "NativeTypes.h"
#define MAX_LEN_ACCOUNT 40   // can not big then unsigned char max = 255
#define  MAX_LEN_PASSWORD 25 // can not big then unsigned char max = 255
#define MAX_LEN_CHARACTER_NAME 25 // can not big then unsigned char  max = 255
#define MAX_LEN_SIGURE 60   // can not big then unsigned char  max = 255
#define MAX_LEN_ROOM_NAME 25
#define MAX_LEN_ROOM_DESC 60  // can not big then unsigned char max = 255
#define MAX_LEN_ROOM_INFORM 500  
#define MAX_LEN_EMAIL 50
#define MAX_LEN_SPEAK_WORDS 200  
#define MAX_MSG_BUFFER_LEN 2048*3

#define PEER_CARD_COUNT 3
#define GOLDEN_PEER_CARD 3
#define TAXAS_PEER_CARD 2
#define TAXAS_PUBLIC_CARD 5
#define MAX_ROOM_PEER 5
#define MAX_TAXAS_HOLD_CARD 5
#define MAX_UPLOAD_PIC 4
#define MAX_JOINED_CLUB_CNT 10

#define MATCH_MGR_UID 1349

#define NIUNIU_HOLD_CARD_COUNT 5


#define COIN_CONDITION_TO_GET_CHARITY 500
#define TIMES_GET_CHARITY_PER_DAY 3   // 2 HOURE
#define COIN_FOR_CHARITY 800
#define GOLDEN_ROOM_COIN_LEVEL_CNT 4
#define GOLDEN_PK_ROUND 2

#ifndef SERVER
#define PIEXL_TO_POINT(px) (px)/CC_CONTENT_SCALE_FACTOR()
#define FOINT_NAME "Helvetica"
#endif

#define MAX_IP_STRING_LEN 17

#define CIRCLE_TOPIC_CNT_PER_PAGE 7
#define MAX_CIRCLE_CONTENT_LEN 700
enum ePayChannel
{
	ePay_AppStore,
	ePay_WeChat,
	ePay_ZhiFuBao,
	ePay_XiaoMi,
	ePay_Max,
};

enum eRoomType
{
	eRoom_None,
	eRoom_TexasPoker = eRoom_None,
	eRoom_NiuNiu,
	eRoom_Golden,
	eRoom_Max ,
};

enum eVipCardType
{
	eCard_None,
	eCard_LV1,
	eCard_LV2,
	eCard_LV3,
	eCard_Max,
};

enum ePlayerType
{
	ePlayer_Normal,
	ePlayer_Robot,
	ePlayer_Mgr,
	ePlayer_Max,
};

enum eRoomState
{
	eRoomState_None,
	eRoomState_WaitOpen,
	eRoomState_Opening,
	eRoomState_Pasue,
	eRoomState_Dead,
	eRoomState_WillDead,
	eRoomState_TP_Dead = eRoomState_Dead,
	eRoomState_WaitJoin,
	eRoomState_NN_WaitJoin = eRoomState_WaitJoin ,
	eRoomState_TP_WaitJoin = eRoomState_WaitJoin,
	eRoomState_WillClose,
	eRoomState_Close,
	eRoomState_DidGameOver,
	eRoomState_StartGame,
	eRoomState_NN_Disribute4Card = eRoomState_StartGame,
	// state for texas poker
	eRoomState_TP_BetBlind = eRoomState_StartGame,
	eRoomState_TP_PrivateCard = eRoomState_StartGame,
	eRoomState_TP_Beting,
	eRoomState_TP_OneRoundBetEndResult,
	eRoomState_TP_PublicCard,
	eRoomState_TP_GameResult,
	eRoomState_TP_MAX,
	eRoomState_TP_Insurance = eRoomState_TP_MAX,  // in case of changing below enum value ; i want add a insurance value ;

	// state for Baccarat ;
	eRoomState_BC_Shuffle,
	eRoomState_BC_WaitBet,
	eRoomState_BC_Distribute,
	eRoomState_BC_AddIdleCard,
	eRoomState_BC_AddBankerCard,
	eRoomState_BC_Caculate,

	// state for NiuNiu
	eRoomState_NN_TryBanker,
	eRoomState_NN_GrabBanker = eRoomState_NN_TryBanker,
	eRoomState_NN_RandBanker,
	eRoomState_NN_StartBet,
	eRoomState_NN_FinalCard,
	eRoomState_NN_CaculateCard,
	eRoomState_NN_GameResult,

	// state for golden
	eRoomState_Golden_Bet,
	eRoomState_Golden_PK,
	eRoomState_Golden_GameResult,
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
#define TIME_GOLDEN_ROOM_WAIT_ACT 15
#define TIME_GOLDEN_ROOM_PK 4
#define TIME_GOLDEN_ROOM_RESULT 2

static unsigned char s_vChangeCardDimonedNeed[GOLDEN_PEER_CARD] = {0,4,8} ;


#define JS_KEY_MSG_TYPE "msgID"

enum eSpeed
{
	eSpeed_Normal,
	eSpeed_Quick,
	eSpeed_Max,
};

enum eNoticeType
{
	eNotice_Text,
	eNotice_BeInvite, // { targetUID : 2345 , addCoin : 34556 }
	eNotice_InvitePrize, // { targetUID : 2345 addCoin : 3555 }
	eNotice_ApplyTakeIn, // { roomID : 235 ,roomName : "hello", applyUID : 234 , takeIn : 23423 }
	eNotice_ReplyTakeIn, // { roomID : 2345,roomName : "hello" , coin : 235 , isAgree : 0 }  // isAgree : 1 agree , 0 refuse ;
	eNotice_BeAddedToClub, // { clubID : 2356 }
	eNotice_BeRemoveFromClub, // { clubID : 2345 }
	eNotice_RecivedApplyToJoinClub, // { applicantUID : 2345 , clubID : 2345 , text : "join me" }
	eNotice_RecivedReplyForApplyForJoinClub, // { clubID : 23455 , isAgree : 1 , text : "text" }
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
	// peer state for taxas poker peer
	eRoomPeer_SitDown = 1,
	eRoomPeer_StandUp = 1 << 1,
	eRoomPeer_Ready =  (1<<12)|eRoomPeer_SitDown ,
	eRoomPeer_StayThisRound = ((1 << 2)|eRoomPeer_SitDown)| eRoomPeer_Ready ,
	eRoomPeer_WaitCaculate = ((1 << 7)|eRoomPeer_StayThisRound ),
	eRoomPeer_AllIn = ((1 << 3)|eRoomPeer_WaitCaculate) ,
	eRoomPeer_GiveUp = ((1 << 4)|eRoomPeer_StayThisRound),
	eRoomPeer_CanAct = ((1 << 5)|eRoomPeer_WaitCaculate),
	eRoomPeer_WaitNextGame = ((1 << 6)|eRoomPeer_SitDown ),
	eRoomPeer_WithdrawingCoin = (1 << 8),  // when invoke drawing coin , must be sitdown , but when staup up , maybe in drawingCoin state 
	eRoomPeer_LackOfCoin = (1<<9)|eRoomPeer_SitDown,
	eRoomPeer_WillLeave = (1<<10)|eRoomPeer_StandUp ,
	eRoomPeer_Looked =  (1<<13)|eRoomPeer_CanAct ,
	eRoomPeer_PK_Failed = (1<<14)|eRoomPeer_StayThisRound ,
	eRoomPeer_Max,
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



 


// mail Module 
#define MAX_KEEP_MAIL_COUNT 50
enum eMailType
{
	eMail_SysOfflineEvent,// { event: concret type , arg:{ arg0: 0 , arg 1 = 3 } }  // processed in svr , will not send to client ;
	eMail_DlgNotice, // content will be send by , stMsgDlgNotice 
	eMail_ReadTimeTag,  // use tell time for public mail ;
	eMail_Sys_End,

	eMail_RealMail_Begin, // will mail will show in golden server windown ;
	eMail_PlainText,  // need not parse , just display the content ;
	eMail_InvitePrize, // { targetUID : 2345 , addCoin : 300 } // you invite player to join game ,and give prize to you 
	eMail_WinMatch, // { gameType:234,roomName:234,rankIdx:2,addCoin:345,cup : 2 , diamomd : 34 }
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
#define TIME_TAXAS_FILP_CARD 0.2f
#define TIME_PLAYER_BET_COIN_ANI 0.3f
#define TIME_BLIND_BET_STATE (TIME_PLAYER_BET_COIN_ANI + 1) 
#define TIME_TAXAS_BET 15
#define TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL 0.6f
#define TIME_TAXAS_MAKE_VICE_POOLS 0.8f
#define TIME_TAXAS_DISTRIBUTE_ONE_HOLD_CARD (TIME_TAXAS_FILP_CARD + 0.2f)
#define TIME_TAXAS_DISTRIBUTE_HOLD_CARD_DELAY ( 0.65f * TIME_TAXAS_DISTRIBUTE_ONE_HOLD_CARD )
#define TIME_DISTRIBUTE_ONE_PUBLIC_CARD 0.5f
#define TIME_TAXAS_WIN_COIN_GOTO_PLAYER 1.2f
#define TIME_TAXAS_CACULATE_PER_BET_POOL (TIME_TAXAS_WIN_COIN_GOTO_PLAYER+1.0f)
#define TIME_TAXAS_SHOW_BEST_CARD 0.7f
#define TIME_TAXAS_WAIT_BUY_INSURANCE 20
#define TIME_TAXAS_WAIT_CALCULATE_INSURANCE 4

#define MIN_PEERS_IN_ROOM_ROBOT 6
#define MAX_PEERS_IN_TAXAS_ROOM 9
#define TIME_LOW_LIMIT_FOR_NORMAL_ROOM 10

// time for niuniu 
#define TIME_NIUNIU_DISTRIBUTE_4_CARD_PER_PLAYER 1.25f 
#define TIME_NIUNIU_TRY_BANKER 15.0f
#define TIME_NIUNIU_RAND_BANKER_PER_WILL_BANKER 0.7f
#define TIME_NIUNIU_PLAYER_BET 8.0f
#define TIME_NIUNIU_DISTRIBUTE_FINAL_CARD_PER_PLAYER 0.3f
#define TIME_NIUNIU_PLAYER_CACULATE_CARD 8.0f
#define TIME_NIUNIU_GAME_RESULT_PER_PLAYER 1.0f  //0.8f
#define TIME_NIUNIU_GAME_RESULT_EXT 8.0f

// time for golden
#define TIME_GOLDEN_DISTRIBUTE_CARD_PER_PLAYER 1.0f 

#define SETTING_MUSIC_ON "MusicOn"
#define SETTING_SOUND_ON "SoundOn"

#define LOCAL_ACCOUNT "account"
#define LOCAL_PASSWORD "password"
#define IS_AUTO_REGISTER "IsAutoLogin"
#define IS_CREATE_ROLE "IsCreateRole"

#define TEMP_NAME "tempName"
#define TEMP_ACCOUNT "tempAccount"
#define TEMP_PASSWORD "tempPassword"

//#define MUSIC_NAME "mp3/bgm2.mp3"
//#define SOUND_BET "mp3/bet.mp3"
//#define SOUND_PASS "mp3/check.mp3"
//#define SOUND_CLOCK "mp3/clock.mp3"
//#define SOUND_SHOW_PUBLIC "mp3/flop.mp3"
//#define SOUND_GIVE_UP "mp3/fold.mp3"
//#define SOUND_WIN_CHIP "mp3/movechips.mp3"
//#define SOUND_MY_TURN "mp3/myturn.mp3"
//#define SOUND_SHOW_SHOP "mp3/shopBell.mp3"

