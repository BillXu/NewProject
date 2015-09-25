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
	ID_MSG_TPORT_ALL_SERVER,

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
	MSG_VERIFY_END,
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
	MSG_SAVE_PLAYER_TAXAS_DATA,
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
	MSG_SAVE_CREATE_TAXAS_ROOM_INFO,
	MSG_SAVE_UPDATE_TAXAS_ROOM_INFO,
	MSG_READ_TAXAS_ROOM_INFO,
	MSG_SAVE_TAXAS_ROOM_PLAYER,
	MSG_SAVE_REMOVE_TAXAS_ROOM_PLAYERS,
	MSG_READ_TAXAS_ROOM_PLAYERS,
	MSG_REQUEST_MY_OWN_ROOMS,
	MSG_REQUEST_MY_FOLLOW_ROOMS,
	MSG_REQUEST_MY_OWN_ROOM_DETAIL,
	MSG_REQUEST_ROOM_DETAIL,
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
	// message id for taxas poker
	MSG_TP_BEGIN = 450,
	MSG_TP_CREATE_ROOM,

	MSG_TP_ROOM_OWNER_BEGIN ,
	MSG_TP_MODIFY_ROOM_NAME,
	MSG_TP_MODIFY_ROOM_DESC,
	MSG_TP_ADD_RENT_TIME,
	MSG_TP_MODIFY_ROOM_INFORM,
	MSG_TP_CACULATE_ROOM_PROFILE,
	MSG_TP_REMIND_NEW_ROOM_INFORM,
	MSG_TP_REQUEST_ROOM_INFORM,
	MSG_TP_REQUEST_ROOM_LIST,
	MSG_TP_ROOM_OWNER_END,

	MSG_TP_ENTER_ROOM,
	MSG_TP_QUICK_ENTER,
	MSG_TP_REQUEST_PLAYER_DATA,
	MSG_TP_ROOM_BASE_INFO,
	MSG_TP_ROOM_VICE_POOL,
	MSG_TP_ROOM_PLAYER_DATA,

	MSG_TP_ROOM_SIT_DOWN,
	MSG_TP_PLAYER_SIT_DOWN,
	MSG_TP_WITHDRAWING_MONEY,

	MSG_TP_ROOM_STAND_UP,
	MSG_TP_PLAYER_STAND_UP,
	MSG_TP_PLAYER_LEAVE,
	MSG_TP_ROOM_LEAVE,
	MSG_TP_INFORM_LEAVE,
	MSG_TP_ORDER_LEAVE,   // DATA SERVER ORDER PLAYER TO LEAVE
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
	MSG_TP_CHANGE_ROOM,
	MSG_TP_REQUEST_ROOM_RANK,
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
	MSG_PLAYER_MAIL_MODULE = 750,
	MSG_PLAYER_REQUEST_CHARITY_STATE,
	MSG_PLAYER_GET_CHARITY,

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
	MSG_BUY_SHOP_ITEM,
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
	MSG_ROBOT_ADD_MONEY,
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

	MSG_GOLDEN_ROOM_DISTRIBUTY,
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
	MSG_GOLDEN_ROOM_PK,

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