#pragma once 
enum eMsgPort
{
	ID_MSG_PORT_NONE , // client to game server 
	ID_MSG_PORT_CLIENT,
	ID_MSG_PORT_GATE,
	ID_MSG_PORT_CENTER,
	ID_MSG_PORT_LOGIN,
	ID_MSG_PORT_VERIFY,
	ID_MSG_PORT_APNS,
	ID_MSG_PORT_LOG,
	ID_MSG_PORT_DATA,
	ID_MSG_PORT_TAXAS,
	ID_MSG_PORT_DB,
	ID_MSG_PORT_ALL_SERVER,
	ID_MSG_PORT_NIU_NIU,
	ID_MSG_PORT_GOLDEN,

	ID_MSG_C2S,
	ID_MSG_CENTER2GATE,
	ID_MSG_C2LOGIN,
	ID_MSG_C2GATE,
	ID_MSG_GATE2C,
	ID_MSG_C2GAME,
	ID_MSG_LOGIN2C,
	ID_MSG_LOGIN2DB,
	ID_MSG_DB2LOGIN,
	ID_MSG_GAME2C,
	ID_MSG_GM2GA,  // GM = game server , GA = gate server ;
	ID_MSG_GA2GM,
	ID_MSG_DB2GM,
	ID_MSG_GM2DB,
	ID_MSG_VERIFY,
	ID_MSG_TO_VERIFY_SERVER,
	ID_MSG_FROM_VERIFY_SERVER,
	ID_MSG_TO_APNS_SERVER,
	ID_MSG_TO_GM2LOG,
	ID_MSG_TRANSFER
};

enum eMsgType 
{
	MSG_NONE,
	//--new define begin---
	// the msg title used between servers 
	MSG_SERVERS_USE,
	MSG_VERIFY_BEGIN,
	MSG_VERIFY_GAME,  // verify that is game server ;
	MSG_VERIFY_LOGIN, // verify login server ;
	MSG_VERIFY_CLIENT, // verify that is client ;
	MSG_VERIFY_VERIYF, // verify buy transaction ok ;
	MSG_VERIFY_GATE, // verify that is gate server 
	MSG_VERIFY_DB,  // verify that is DBserver ;
	MSG_VERIFY_APNS, // apple push notification ;
	MSG_VERIFY_LOG, // LOG sever 
	MSG_VERIFY_TAXAS, // TAXAS POKER SERVER 
	MSG_VERIFY_DATA, // VIERIFY DATA SERVER ;
	MSG_VERIFY_NIU_NIU,
	MSG_VERIFY_END,
	MSG_CONTROL_FLAG,
	MSG_REQUEST_CLIENT_IP,
	MSG_VERIFY_GOLDEN,
	MSG_JSON_CONTENT,
	MSG_CLIENT_NET_STATE,
	MSG_TRANSER_DATA = 100, // tranfer data between servers ;
	MSG_DISCONNECT_SERVER, 
	MSG_DISCONNECT_CLIENT,
	MSG_CONNECT_NEW_CLIENT,
	MSG_VERIFY_TANSACTION,
	MSG_APNS_INFO,   // send push notification ;
	MSG_PUSH_APNS_TOKEN,  // used for apns ; APPLE remote push notification ;
	MSG_SAVE_LOG,
	MSG_LOGIN_INFORM_GATE_SAVE_LOG,
	// msg title used between server and client ;
	MSG_RECONNECT,   // client with gate 
	MSG_SAVE_PLAYER_MONEY, // send to DB ;
	MSG_SAVE_COMMON_LOGIC_DATA,
	MSG_SAVE_DB_LOG, // save log inter log db ;
	
	MSG_GATESERVER_INFO,
	// login 
	MSG_PLAYER_REGISTER,     // register an account ;
	MSG_REQUEST_CREATE_PLAYER_DATA,  // INFORM DB prepare data for player
	MSG_PLAYER_LOGIN,  // check an account is valid ;
	MSG_PLAYER_BIND_ACCOUNT, //  a quick enter player need to bind a real account and password ;
	MSG_MODIFY_PASSWORD,
	MSG_PLAYER_OTHER_LOGIN,  // more than one place login , prelogin need disconnect ; client recived must disconnect from server
	MSG_CROSS_SERVER_REQUEST,
	MSG_CROSS_SERVER_REQUEST_RET,
	MSG_READ_PLAYER_TAXAS_DATA,
	MSG_SAVE_CREATE_TAXAS_ROOM_INFO_unUse,
	MSG_SAVE_UPDATE_TAXAS_ROOM_INFO_unUse,
	MSG_READ_TAXAS_ROOM_INFO_unUse,
	MSG_SAVE_TAXAS_ROOM_PLAYER,
	MSG_SAVE_REMOVE_TAXAS_ROOM_PLAYERS,
	MSG_READ_TAXAS_ROOM_PLAYERS,
	MSG_REQUEST_MY_OWN_ROOMS,
	MSG_REQUEST_MY_FOLLOW_ROOMS,
	//MSG_REQUEST_MY_OWN_ROOM_DETAIL,
	MSG_REQUEST_ROOM_REWARD_INFO,
	MSG_REQUEST_PLAYER_DATA,
	MSG_SELECT_DB_PLAYER_DATA,
	MSG_PLAYER_BASE_DATA_TAXAS,
	MSG_ON_PLAYER_BIND_ACCOUNT,
	// modify name and sigure
	MSG_PLAYER_MODIFY_NAME,
	MSG_PLAYER_MODIFY_SIGURE,
	MSG_PLAYER_MODIFY_PHOTO,
	MSG_PLAYER_UPDATE_MONEY,  // USE WHEN OTHER MAIL A GITF  ;
	MSG_PLAYER_MODIFY_SEX,
	MSG_RESET_PASSWORD,
	MSG_SAVE_CREATE_ROOM_INFO,
	MSG_SAVE_UPDATE_ROOM_INFO,
	MSG_READ_ROOM_INFO,
	MSG_PLAYER_BASE_DATA_NIUNIU,
	MSG_READ_PLAYER_GAME_DATA,
	MSG_SAVE_PLAYER_GAME_DATA,
	MSG_DB_CHECK_INVITER,
	MSG_PLAYER_CHECK_INVITER,
	MSG_DLG_NOTICE,
	MSG_REQUEST_EXCHANGE_LIST,
	MSG_REQUEST_EXCHANGE_DETAIL,
	MSG_PLAYER_EXCHANGE,
	MSG_GET_VIP_CARD_GIFT,
	MSG_READ_NOTICE_PLAYER,
	MSG_PUSH_NOTICE,
	MSG_SAVE_NOTICE_PLAYER,
	MSG_READ_EXCHANGE,
	MSG_SAVE_EXCHANGE,
	MSG_DB_PLAYER_MODIFY_NAME,
	MSG_ASYNC_REQUEST,
	MSG_ASYNC_REQUEST_RESULT,
	MSG_PLAYER_BASE_DATA = 250,
	MSG_READ_PLAYER_BASE_DATA,
	MSG_PLAYER_SAVE_PLAYER_INFO,
	MSG_PLAYER_LOGOUT,
	// friend module
	MSG_READ_FRIEND_LIST = 300,  //;
	MSG_SAVE_FRIEND_LIST,  // send to db 
	MSG_REQUEST_FRIEND_LIST,
	MSG_PLAYER_ADD_FRIEND,  // want to add other player 
	MSG_PLAYER_ADD_FRIEND_REPLAY,  // other player replay my request ;
	MSG_PLAYER_BE_ADDED_FRIEND,   // other player want to add me 
	MSG_PLAYER_BE_ADDED_FRIEND_REPLY,
	MSG_PLAYER_REPLAY_BE_ADD_FRIEND,  // I replay to other player who want to add me ;
	MSG_PLAYER_SERACH_PEERS, // mo hu search , to add other o be firend ;
	MSG_PLAYER_DELETE_FRIEND, // remove friend ;

	// friend invite ;
	MSG_PlAYER_INVITED_FRIEND_TO_JOIN_ROOM,  // invite a friend to join
	MSG_PLAYER_BE_INVITED,   // i were invited by my friend ;
	MSG_PLAYER_REPLAY_BE_INVITED,   // when i were invited by friend, i make a choice , reply;
	MSG_PLAYER_RECIEVED_INVITED_REPLAY, // the player I invited ,replayed me ;
	MSG_REQUEST_ROOM_RANK,
	MSG_REQUEST_LAST_TERM_ROOM_RANK,

	// msg request math list ;
	MSG_REQUEST_MATCH_ROOM_LIST,
	// message id for taxas poker


	// new room msg are here ;
	MSG_PLAYER_ENTER_ROOM,
	MSG_PLAYER_LEAVE_ROOM,
	MSG_PLAYER_SITDOWN,
	MSG_PLAYER_STANDUP,
	MSG_ROOM_SITDOWN,
	MSG_ROOM_STANDUP,
	MSG_SVR_ENTER_ROOM,
	MSG_SVR_DO_LEAVE_ROOM,
	MSG_SVR_DELAYED_LEAVE_ROOM,
	MSG_GET_MAX_ROOM_ID,
	MSG_PRIVATE_ROOM_PLAYER_REBUY,
	MSG_SYNC_PRIVATE_ROOM_RESULT,
	MSG_REQUEST_PRIVATE_ROOM_RECORDER,
	MSG_PLAYER_REQUEST_GAME_RECORDER,
	MSG_SAVE_GAME_RESULT,
	MSG_READ_GAME_RESULT,
	MSG_SAVE_PLAYER_GAME_RECORDER,
	MSG_READ_PLAYER_GAME_RECORDER,
	MSG_TP_BEGIN = 450,
	MSG_TP_CREATE_ROOM,

	MSG_TP_ROOM_OWNER_BEGIN ,
	MSG_TP_MODIFY_ROOM_NAME,
	MSG_TP_MODIFY_ROOM_DESC,
	MSG_ADD_RENT_TIME,
	MSG_TP_ADD_RENT_TIME = MSG_ADD_RENT_TIME,
	MSG_TP_MODIFY_ROOM_INFORM,
	MSG_CACULATE_ROOM_PROFILE,
	MSG_TP_CACULATE_ROOM_PROFILE = MSG_CACULATE_ROOM_PROFILE,
	MSG_TP_REMIND_NEW_ROOM_INFORM,
	MSG_REMIND_NEW_ROOM_INFORM = MSG_TP_REMIND_NEW_ROOM_INFORM,
	MSG_TP_REQUEST_ROOM_LIST,
	MSG_REQUEST_ROOM_LIST = MSG_TP_REQUEST_ROOM_LIST,
	MSG_TP_ROOM_OWNER_END,

	MSG_TP_QUICK_ENTER,
	MSG_TP_REQUEST_PLAYER_DATA,
	unUsed_MSG_TP_ROOM_BASE_INFO,
	MSG_TP_ROOM_VICE_POOL,
	MSG_TP_ROOM_PLAYER_DATA,

	MSG_TP_WITHDRAWING_MONEY,

	MSG_TP_SYNC_PLAYER_DATA,
	// in room msg 
	MSG_TP_ENTER_STATE,
	MSG_TP_START_ROUND,
	MSG_TP_PRIVATE_CARD,
	MSG_TP_WAIT_PLAYER_ACT,
	MSG_TP_PLAYER_ACT,
	MSG_TP_ROOM_ACT,
	MSG_TP_ONE_BET_ROUND_RESULT,
	MSG_TP_PUBLIC_CARD,
	MSG_TP_GAME_RESULT,
	MSG_TP_UPDATE_PLAYER_STATE,

	MSG_TP_READ_MY_OWN_ROOMS,
	MSG_READ_MY_OWN_ROOMS = MSG_TP_READ_MY_OWN_ROOMS,
	MSG_TP_CHANGE_ROOM,
	MSG_REQUEST_ROOM_INFO,
	MSG_TP_END = 700,
	// mail module
	MSG_PLAYER_SAVE_MAIL ,
	MSG_PLAYER_SET_MAIL_STATE,
	MSG_PLAYER_READ_MAIL_LIST,
	MSG_PLAYER_REQUEST_MAIL_LIST,
	MSG_PLAYER_NEW_MAIL_ARRIVED,
	MSG_PLAYER_INFORM_NEW_MAIL,
	MSG_PLAYER_LOOK_MAIL,
	MSG_GAME_SERVER_GET_MAX_MAIL_UID,
	MSG_PLAYER_PROCESSED_MAIL,
	MSG_SAVE_PLAYER_ADVICE,
	MSG_PLAYER_ADVICE,
	MSG_PLAYER_MAIL_MODULE = 750,
	MSG_PLAYER_REQUEST_CHARITY_STATE,
	MSG_PLAYER_GET_CHARITY,
	MSG_BUY_SHOP_ITEM,

	MSG_SAVE_ROOM_PLAYER,
	MSG_READ_ROOM_PLAYER,
	MSG_REMOVE_ROOM_PLAYER,

	MSG_SHOP_BUY_ITEM_ORDER,
	MSG_VERIFY_ITEM_ORDER,
	MSG_SAVE_PRIVATE_ROOM_PLAYER,
	MSG_READ_PRIVATE_ROOM_PLAYER,
	MSG_ROOM_ENTER_NEW_STATE = 800,
	// niuniu room 
	MSG_NN_PLAYER_CACULATE_CARD_OK,
	MSG_NN_CACULATE_CARD_OK,
	MSG_NN_ROOM_PLAYERS,
	unUsd_MSG_NN_REQUEST_ROOM_INFO,
	MSG_NN_DISTRIBUTE_4_CARD,
	MSG_NN_PLAYER_TRY_BANKER,
	MSG_NN_TRY_BANKER,
	MSG_NN_PRODUCED_BANKER,
	MSG_NN_RAND_BANKER,
	MSG_NN_PLAYER_BET,
	MSG_NN_BET,
	MSG_NN_DISTRIBUTE_FINAL_CARD,
	MSG_NN_GAME_RESULT,
	MSG_NN_CREATE_ROOM,
	MSG_NN_MODIFY_ROOM_NAME,

	// golden msg 
	MSG_GOLDEN_WAIT_PLAYER_ACT,
	MSG_GOLDEN_PLAYER_ACT,
	MSG_GOLDEN_ROOM_ACT,
	MSG_GOLDEN_ROOM_DISTRIBUTE,
	MSG_GOLDEN_PLAYER_PK,
	MSG_GOLDEN_ROOM_PK,
	MSG_GOLDEN_ROOM_PLAYERS,
	// poker circle 
	MSG_CIRCLE_BEGIN = 1000,
	MSG_CIRCLE_READ_TOPICS,
	MSG_CIRCLE_SAVE_ADD_TOPIC,
	MSG_CIRCLE_SAVE_DELETE_TOPIC,
	MSG_CIRCLE_PUBLISH_TOPIC,
	MSG_CIRCLE_DELETE_TOPIC,
	MSG_CIRCLE_REQUEST_TOPIC_DETAIL,
	MSG_CIRCLE_REQUEST_TOPIC_LIST,

	MSG_PLAYER_USE_ENCRYPT_NUMBER,
	MSG_VERIFY_ENCRYPT_NUMBER,
	// robot special msg 
	MSG_ADD_MONEY = 1300, 
	MSG_TELL_PLAYER_TYPE,
	MSG_TELL_ROBOT_IDLE,
	MSG_TELL_ROBOT_ENTER_ROOM,
	MSG_REQ_ROBOT_ENTER_ROOM,
	MSG_TELL_ROBOT_LEAVE_ROOM,
	MSG_REQ_CUR_GAME_OFFSET,
	MSG_REQ_TOTAL_GAME_OFFSET,
	MSG_ADD_TEMP_HALO,
	MSG_MODIFY_ROOM_RANK,
	MSG_ROBOT_GENERATE_ENCRYPT_NUMBER,
	MSG_SAVE_ENCRYPT_NUMBER,

	// msg js content type 
	MSG_CREATE_CLUB = 1511,  // ID_MSG_PORT_DATA ;
	// client : { newClubID : 2345 , cityCode : 23 }
	// svr : { ret : 0 ,newClubID : 234 }
	// ret : 0 , means ok , 1 means  can not create more clubs ;
	MSG_DISMISS_CLUB, // ID_MSG_PORT_DATA ;
	// client : { clubID : 235 }
	// svr : { ret : 0 }
	// ret : 0 success , 1 you do not have that club , 2 club have room keep running .

	MSG_REQ_CLUB_ROOM, // ID_MSG_PORT_DATA ;
	// client : { clubID : 234 }
	// svr : { ret : 0 , rooms : [21,23,45,23] } 
	// ret : 0 success , 1 club not exist 

	MSG_CREATE_ROOM, // ID_MSG_PORT_DATA ;
	// client : { name : "this is room name" ,roomType : eRoomType , baseBet : 23 , duringMin : 2345 , clubID : 23 , baseTakeIn : 235, isControlTakeIn : 0 , seatCnt : 2 , opts : { ... } }
	// roomType : means eRoomType . baseBet , for taxas , it represent small blind . duringMin : room keep running time , by minite . clubID : when equal 0 , means ,quick game , opts : depend on game type ;
	// NIU NIU  opts : { unbankerType : 0 }  // 0 no niu leave banker , 1 lose to all  leave banker , 2 manual leave banker;
	// Taxas Poker opts : { maxTakeIn : 2345 }
	// Golden opts : { maxSingleBet : 20 }
	// svr : { ret : 0 , roomID : 235 , clubID : 23 } ;
	// ret : 0 means success , 1 can not create more room , 2 you have not permission to creator room for club; 3 , room type error 
	MSG_DELETE_ROOM, // ID_MSG_PORT_DATA ;
	// client : { roomID : 2345 , clubID : 23  }
	// svr : { ret : 0 }
	// ret : 0 means success , 1 you are not creator , 2 room is running, try later , 3 club do not have room with that id ;
	MSG_REQUEST_ROOM_ITEM_DETAIL, //eMsgPort::ID_MSG_PORT_TAXAS , eMsgPort::ID_MSG_PORT_GOLDEN , eMsgPort::ID_MSG_PORT_NIU_NIU  , 
	// client : { roomID : 0 }
	// svr : { ret : 0 , name: "fsg" , creatorUID : 235, baseBet : 2, playerCnt : 23, roomID : 235 , roomType : eRoomType , initTime : 20, playedTime : 2345, seatCnt : 6 ,clubID : 0 }
	// ret : 0 success , 1 can not find room ;

	MSG_ROOM_INFO,
	// svr : { ownerUID : 234552 , roomID : 2345 , seatCnt : 4 , chatID : 23455 , curState : eRoomState , leftTimeSec : 235 , baseTakeIn : 2345 , selfCoin : 2345 , game : { ... } } 
	// goldn :  game : { "betRound" = 23, "bankIdx":3 ,"baseBet" : 20 ,"curBet" : 40 ,"mainPool" : 1000 ,curActIdx : 3 }
	// NiuNiu : game : { "bankIdx":3 ,"baseBet" : 20 , "bankerTimes" : 2, unbankerType : 0  }
	// taxas :   game : { "litBlind":20,"maxTakIn":300, "bankIdx":3 ,"litBlindIdx":2,"bigBlindIdx" : 0,"curActIdx" : 3,"curPool":4000,"mostBet":200,"pubCards":[0,1] };

	MSG_SET_GAME_STATE, //eMsgPort::ID_MSG_PORT_TAXAS , eMsgPort::ID_MSG_PORT_GOLDEN , eMsgPort::ID_MSG_PORT_NIU_NIU  , 
	// client : { roomID : 0 , state : 2 , uid : 2345 }  
	// state : 2 start game , 3 pause game 
	// svr : { ret : 0 }
	// ret : 0 success , 1 can not find room , 2 you are not creator , 3 do not set the same state , 4 , you are not the creator , 5, invalid state value; 

	MSG_ROOM_GAME_STATE_CHANGED,
	// svr : { state : 2 }
	// state : 2 start game , 3 pause game 

	MSG_APPLY_TAKE_IN, //eMsgPort::ID_MSG_PORT_TAXAS , eMsgPort::ID_MSG_PORT_GOLDEN , eMsgPort::ID_MSG_PORT_NIU_NIU  , 
	// client : { roomID : 234 , takeIn : 2345 }
	// svr : { ret : 0 , isApply : 1 , inRoomCoin : 23455 }
	// ret : 0 success , 1 can not find room , 2 take in reached limit , 3 coin not enough , 4 already applying , do not do again;
	// isApply , 0 means direct takeIn , 1 waiting creator check ;

	MSG_REPLY_APPLY_TAKE_IN, //eMsgPort::ID_MSG_PORT_TAXAS , eMsgPort::ID_MSG_PORT_GOLDEN , eMsgPort::ID_MSG_PORT_NIU_NIU  , 
	// client : { roomID : 2445 , replyToUID : 2345, isAgree : 0 , coin : 2300 }
	// isAgree : 1 agree, 0 refuse ;
	// replyUID : the applyer uid ;

	MSG_REQUEST_ROOM_AUDIENTS, // audients ;
	// client : { roomID : 2345 }
	// svr : { audients : [234, 235 ,2456 ] }  , audients player uid array ;

	MSG_CLUB_ADD_MEMBER,
	// client : { groupID : 2345 , userAccount : 2345 }  // gotpe user account , not game player user account 
	// client { ret : 0 , groupID : 2345 , userAccount : 2345 } // ret : 1 can not find groupID ; 2 svr error , 3 reach member cnt limit;

	MSG_CLUB_DELETE_MEMBER,
	// client : { groupID : 2345 , userAccount : 2345 }  // gotpe user account , not game player user account 
	// client { ret : 0 , groupID : 2345 , userAccount : 2345 } // ret : 1 can not find groupID ; 2 svr error ;

	MSG_REQ_CLUB_INFO,
	//client : { groupID : 23455 }
	// svr: { ret : 0 , groupID : 23455,curCnt : 23 , capacity : 80, level : 0 , deadTime : 2345523 } , ret : 0 success , 1 can not find club ;

	MSG_REQ_RECORMED_CLUB,
	// client : null
	// svr { clubIDs : [234, 234 ,23452,2345] } 

	MSG_REQ_LEVEL_UP_CLUB,
	// client : { clubID : 234, level : 234 } 
	// svr : { ret : 0 , clubID : 2345 , level : 2345 , deadTime : 23456 }
	// ret : 0 , success , 1 can not find club , 2 diamond is not enough, 3 invalid argument  , 4 target level invalid;

	MSG_REQ_RESIGN_BANKER, // request leave banker ;
	// client : null 
	// svr : { ret : 0 } ;
	// ret : 0 success , 1 you are not banker now , 2 , current leave banker mode is not manual leave ;

	MSG_REQ_PLAYER_JOINED_CLUBS,  // request player joined clubs 
	// client : { uid : 2345 }
	// svr : { uid : 2345 , clubIDs : [2 , 3 ,6 ,3 ]  } 

	MSG_REQ_SELF_CREATE_ROOMS, 
	// client : { uid : 2345 }
	// svr : { roomIDs : [234,2345,2345,2345] } 































	///------new define end---
	MSG_SERVER_AND_CLIENT_COMMON_BEGIN,  // server and client common msg , beyond specail game 
	MSG_PLAYER_ENTER_GAME,    // after check , enter game 
	
	//MSG_PLAYER_CONTINUE_LOGIN,  // contiune login prize ;
	
	MSG_CREATE_ROLE,
	// player base Data 
	
	MSG_SHOW_CONTINUE_LOGIN_DLG,
	MSG_GET_CONTINUE_LOGIN_REWARD,

	MSG_PLAYER_UPDATE_VIP_LEVEL,


	// slot machine 
	MSG_PLAYER_SLOT_MACHINE, // lao hu ji ;

	// item 
	MSG_REQUEST_ITEM_LIST ,
	MSG_SAVE_ITEM_LIST,
	MSG_PLAYER_PAWN_ASSERT, //  dian dang zi chan
	MSG_PLAYER_USE_GIFT,
	// rank
	MSG_REQUEST_RANK,
	MSG_REQUEST_RANK_PEER_DETAIL,
	// inform 
	MSG_INFORM_NEW_NOTICES ,
	MSG_PLAYER_REQUEST_NOTICE,
	MSG_GLOBAL_BROCAST,
	MSG_PLAYER_SAY_BROCAST,
	// shop 
	MSG_SAVE_SHOP_BUY_RECORD,
	MSG_GET_SHOP_BUY_RECORD,
	MSG_PLAYER_REQUEST_SHOP_LIST,
	
	MSG_PLAYER_RECIEVED_SHOP_ITEM_GIFT,
	// mission 
	MSG_GAME_SERVER_SAVE_MISSION_DATA,
	MSG_GAME_SERVER_GET_MISSION_DATA,
	MSG_PLAYER_REQUEST_MISSION_LIST,
	MSG_PLAYER_NEW_MISSION_FINISHED,
	MSG_PLAYER_REQUEST_MISSION_REWORD,

	// online box 
	MSG_PLAYER_REQUEST_ONLINE_BOX_REWARD,
	MSG_PLAYER_REQUEST_ONLINE_BOX_STATE,

	// room common msg ;
	MSG_ROOM_MSG_BEGIN,
	MSG_ROOM_RET,
	MSG_ROOM_SPEAK,
	MSG_ROOM_OTHER_SPEAK,  
	MSG_ROOM_REQUEST_PEER_DETAIL,
	MSG_ROOM_KICK_PEER,
	MSG_ROOM_OTHER_KICK_PEER,
	MSG_ROOM_EXE_BE_KICKED,
	MSG_ROOM_PROCESSE_KIKED_RESULT,

	MSG_ROOM_ENTER,
	MSG_ROOM_PLAYER_ENTER,  // MSG_ROOM_PLAYER_x means other player actions 
	MSG_PLAYER_FOLLOW_TO_ROOM, // zhui zong pai ju 

	MSG_ROOM_LEAVE,
	MSG_ROOM_PLAYER_LEAVE,
	// private room 
	MSG_PLAYER_CREATE_PRIVATE_ROOM,  // create private Room ;

	// message id for pai jiu 
	MSG_PJ_BEGIN ,
	MSG_PJ_ENTER_ROOM,
	MSG_PJ_ACTION_RET,
	MSG_PJ_ROOM_INFO,
	MSG_PJ_WAIT_BET,
	MSG_PJ_BET,
	MSG_PJ_OTHER_BET,
	MSG_PJ_APPLY_BANKER,
	MSG_PJ_OTHER_APPLY_BANKER,
	MSG_PJ_BANKER_CHANGED,
	MSG_PJ_DISTRIBUTE,
	MSG_PJ_BANKER_VIEWCARD,
	MSG_PJ_BANKER_SHOW_CARD,
	MSG_PJ_SHOW_BANKER_CARD,
	MSG_PJ_SETTLEMENT,
	MSG_PJ_SHUFFLE,
	MSG_PJ_APPLY_UNBANKER,
	MSG_PJ_STATE_CHANGED,
	MSG_PJ_BANKER_CHOSE_SHUFFLE,
	MSG_PJ_BANKER_CHOSE_CONTINUE_CANCEL,  // will go on ? 
	MSG_PJ_ROOM_APPLY_BANKER_LIST,
	MSG_PJ_ROOM_RECORD,

	

	// message id for baccarat 
	MSG_BC_BEGIN = 20000,
	MSG_BC_ROOM_INFO,	
	MSG_BC_BET,
	MSG_BC_OTHER_BET,
	MSG_BC_DISTRIBUTE,
	MSG_BC_ADD_CARD,
	MSG_BC_CACULATE,
	MSG_BC_START_BET,
	MSG_BC_START_SHUFFLE,

	// message for robot 
	MSG_ROBOT_ORDER_TO_ENTER_ROOM = 25000,
	MSG_ROBOT_APPLY_TO_LEAVE,
	MSG_ROBOT_CHECK_BIGGIEST,
	MSG_ROBOT_INFORM_IDLE,
	
	// all room msg above ,

	// golden room 
	MSG_GOLDEN_ROOM_ENTER,
	MSG_GOLDEN_ROOM_LEAVE,
	MSG_GOLDEN_ROOM_INFO,
	MSG_GOLDEN_ROOM_STATE,

	MSG_GOLDEN_ROOM_PLAYER_SHOW_CARD,
	MSG_GOLDEN_ROOM_SHOW_CARD,

	MSG_GOLDEN_ROOM_PLAYER_CHANGE_CARD,
	MSG_GOLDEN_ROOM_CHANGE_CARD,

	MSG_GOLDEN_ROOM_PLAYER_PK_TIMES,
	MSG_GOLDEN_ROOM_PK_TIMES,

	MSG_GOLDEN_ROOM_PLAYER_READY,
	MSG_GOLDEN_ROOM_READY,

	
	MSG_GOLDEN_ROOM_INFORM_ACT,
	
	MSG_GOLDEN_ROOM_PLAYER_LOOK,
	MSG_GOLDEN_ROOM_LOOK,
	
	MSG_GOLDEN_ROOM_PLAYER_GIVEUP,
	MSG_GOLDEN_ROOM_GIVEUP,

	MSG_GOLDEN_ROOM_PLAYER_FOLLOW,
	MSG_GOLDEN_ROOM_FOLLOW,

	MSG_GOLDEN_ROOM_PLAYER_ADD,
	MSG_GOLDEN_ROOM_ADD,

	MSG_GOLDEN_ROOM_PLAYER_PK,

	MSG_GOLDEN_ROOM_RESULT,

	// new taxas poker msg id 
	MSG_TAXAS_ROOM_INFO,

	MSG_TAXAS_PLAYER_SITDOWN,
	MSG_TAXAS_ROOM_SITDOWN,

	MSG_TAXAS_PLAYER_STANDUP,
	MSG_TAXAS_ROOM_STANDUP,

	MSG_TAXAS_ROOM_NEW_STATE,
	MSG_TAXAS_ROOM_GAME_START,
	
	MSG_TAXAS_ROOM_WAIT_PLAYER_ACT,

	MSG_TAXAS_PLAYER_FOLLOW,
	MSG_TAXAS_ROOM_FOLLOW,

	MSG_TAXAS_PLAYER_ADD,
	MSG_TAXAS_ROOM_ADD,

	MSG_TAXAS_PLAYER_GIVEUP,
	MSG_TAXAS_ROOM_GIVEUP,

	MSG_TAXAS_PLAYER_ALLIN,
	MSG_TAXAS_ROOM_ALLIN,

	MSG_TAXAS_PLAYER_PASS,
	MSG_TAXAS_ROOM_PASS,
	
	MSG_TAXAS_ROOM_AUTOTAKEIN,
};