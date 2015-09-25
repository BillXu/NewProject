#pragma warning(disable:4800)
#include "DBManager.h"
#include "LogManager.h"
#include "DBRequest.h"
#include "ServerMessageDefine.h"
#include "DBApp.h"
#include "DataBaseThread.h"
#include "AutoBuffer.h"
#define PLAYER_BRIF_DATA "playerName,userUID,sex,vipLevel,photoID,coin,diamond"
#define PLAYER_BRIF_DATA_DETAIL_EXT ",signature,singleWinMost,mostCoinEver,vUploadedPic,winTimes,loseTimes,longitude,latitude,offlineTime,maxCard,vJoinedClubID"
CDBManager::CDBManager(CDBServerApp* theApp )
{
	m_vReserverArgData.clear();
	m_pTheApp = theApp ;
	nCurUserUID = 0 ;  // temp asign 
}

CDBManager::~CDBManager()
{
	LIST_ARG_DATA::iterator iter = m_vReserverArgData.begin() ;
	for ( ; iter != m_vReserverArgData.end() ; ++iter )
	{
		if ( *iter )
		{
			delete *iter ;
			*iter = NULL ;
		}
	}
	m_vReserverArgData.clear() ;
}

void CDBManager::Init()
{
	// register funcs here ;
	//stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
	//pRequest->cOrder = eReq_Order_High ;
	//pRequest->eType = eRequestType_Select ;
	//pRequest->nRequestUID = -1;
	//pRequest->pUserData = NULL;
	//pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"(select max(Account.UserUID) FROM Account)") ;
	//CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
}

void CDBManager::OnMessage(stMsg* pmsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	// construct sql
	stArgData* pdata = GetReserverArgData() ;
	if ( pdata == NULL )
	{
		pdata = new stArgData ;
	}

	pdata->eFromPort = eSenderPort ;
	pdata->nSessionID = nSessionID ;

	stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
	pRequest->cOrder = eReq_Order_Normal ;
	pRequest->nRequestUID = pmsg->usMsgType ;
	pRequest->pUserData = pdata;
	pRequest->eType = eRequestType_Max ;
	pRequest->nSqlBufferLen = 0 ;
	CLogMgr::SharedLogMgr()->PrintLog("recive db req = %d",pmsg->usMsgType);
	switch( pmsg->usMsgType )
	{
	case MSG_SELECT_DB_PLAYER_DATA:
		{
			stMsgSelectPlayerData* pRet = (stMsgSelectPlayerData*)pmsg ;
			pdata->nExtenArg1 = pRet->nReqPlayerSessionID ;
			pdata->nExtenArg2 = pRet->isDetail ;
			pRequest->eType = eRequestType_Select ;
			if ( pRet->isDetail )
			{
				pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
					"call selectPlayerDetailClient( '%u')",pRet->nTargetPlayerUID) ;
			}
			else
			{
				pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
					"SELECT * FROM playerbasedata WHERE userUID = '%d'",pRet->nTargetPlayerUID) ;
			}
		}
		break;
	case MSG_PLAYER_SAVE_MAIL:
		{
			stMsgSaveMail* pRet = (stMsgSaveMail*)pmsg ;
			pRequest->eType = eRequestType_Add;
			CAutoBuffer auBuffer(pRet->pMailToSave.nContentLen + 1 );
			auBuffer.addContent((char*)pmsg + sizeof(stMsgSaveMail),pRet->pMailToSave.nContentLen) ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO mail (userUID, postTime,mailType,mailContent) VALUES ('%u', '%u','%u','%s')",
				pRet->nUserUID,pRet->pMailToSave.nPostTime,pRet->pMailToSave.eType,auBuffer.getBufferPtr()) ;
			CLogMgr::SharedLogMgr()->PrintLog("save  SAVE_MAIL uid = %d",pRet->nUserUID);
			pdata->nExtenArg1 = pRet->nUserUID ;
		}
		break;
	case MSG_PLAYER_READ_MAIL_LIST:
		{
			stMsgReadMailList* pRet = (stMsgReadMailList*)pmsg ;
			pdata->nExtenArg1 = pRet->nUserUID ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"SELECT * FROM mail WHERE userUID = '%d' and state = '0' ",pRet->nUserUID) ;
		}
		break;
	case MSG_PLAYER_SET_MAIL_STATE:
		{
			stMsgResetMailsState* pRet = (stMsgResetMailsState*)pmsg ;
			pRequest->eType = eRequestType_Update ;

			if ( pRet->tMailType == eMail_Max )
			{
				pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
					"UPDATE mail SET state = '1' WHERE userUID = '%d' and state = '0' ",pRet->nUserUID) ;
			}
			else
			{
				pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
					"UPDATE mail SET state = '1' WHERE userUID = '%d' and mailType = '%d' and state = '0' ",pRet->nUserUID,pRet->tMailType) ;
			}
			
			CLogMgr::SharedLogMgr()->PrintLog("reset mail state for uid = %d",pRet->nUserUID);
		}
		break ;
	case MSG_SAVE_FRIEND_LIST:
		{
			stMsgSaveFirendList* pRet = (stMsgSaveFirendList*)pmsg ;
			pRequest->eType = eRequestType_Update ;
			CAutoBuffer str (pRet->nFriendCountLen + 1 );
			str.addContent((char*)pmsg + sizeof(stMsgSaveFirendList),pRet->nFriendCountLen) ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"UPDATE playerfriend SET friendUIDs = '%s' WHERE userUID = '%d'",str.getBufferPtr(),pRet->nUserUID) ;
			CLogMgr::SharedLogMgr()->PrintLog("save player FRIEND_LIST uid = %d",pRet->nUserUID);
		}
		break;
	case MSG_READ_FRIEND_LIST:
		{
			stMsgReadFriendList* pRet = (stMsgReadFriendList*)pmsg ;
			pdata->nExtenArg1 = pRet->nUserUID ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"SELECT * FROM playerfriend WHERE userUID = '%d'",pRet->nUserUID) ;
		}
		break;
	case MSG_REQUEST_CREATE_PLAYER_DATA:
		{
			stMsgRequestDBCreatePlayerData* pCreate = (stMsgRequestDBCreatePlayerData*)pmsg ;
			pdata->nExtenArg1 = pCreate->nUserUID ;

			uint16_t nRandID = rand() % 10000 ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,
				"call CreateNewRegisterPlayerData(%d,'guest%d','%d')",pCreate->nUserUID,nRandID,pCreate->isRegister) ;
		}
		break;
	case MSG_READ_PLAYER_BASE_DATA:
		{
			stMsgDataServerGetBaseData* pRet = (stMsgDataServerGetBaseData*)pmsg ;
			pdata->nExtenArg1 = pRet->nUserUID ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"SELECT * FROM playerbasedata WHERE userUID = '%d'",pRet->nUserUID) ;
		}
		break;
	case MSG_READ_PLAYER_TAXAS_DATA:
		{
			stMsgDataServerGetBaseData* pRet = (stMsgDataServerGetBaseData*)pmsg ;
			pdata->nExtenArg1 = pRet->nUserUID ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"SELECT * FROM playertaxasdata WHERE userUID = '%d'",pRet->nUserUID) ;
		}
		break;
	case MSG_PLAYER_SAVE_PLAYER_INFO:
		{
			stMsgSavePlayerInfo* pRet = (stMsgSavePlayerInfo*)pmsg ;
			pRequest->eType = eRequestType_Update ;
			std::string strUploadPic = stMysqlField::UnIntArraryToString(pRet->vUploadedPic,MAX_UPLOAD_PIC) ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"UPDATE playerbasedata SET playerName = '%s', signature = '%s',vUploadedPic = '%s',photoID = '%d' WHERE userUID = '%d'",pRet->vName,pRet->vSigure,strUploadPic.c_str(),pRet->nPhotoID,pRet->nUserUID) ;
		}
		break;
	case MSG_SAVE_PLAYER_MONEY:
		{
			stMsgSavePlayerMoney* pRet = (stMsgSavePlayerMoney*)pmsg ;
			pRequest->eType = eRequestType_Update ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"UPDATE playerbasedata SET coin = '%I64d', diamond = '%d' WHERE userUID = '%d'",pRet->nCoin,pRet->nDiamoned,pRet->nUserUID) ;
			CLogMgr::SharedLogMgr()->PrintLog("save player coin = %I64d uid = %d",pRet->nCoin,pRet->nUserUID);
		}
		break;
	case MSG_SAVE_PLAYER_TAXAS_DATA:
		{
			stMsgSavePlayerTaxaPokerData* pRet = (stMsgSavePlayerTaxaPokerData*)pmsg ;
			pRequest->eType = eRequestType_Update ;

			CAutoBuffer FollowedRooms(pRet->nFollowedRoomsStrLen + 1 );
			FollowedRooms.addContent(((char*)pRet)+ sizeof(stMsgSavePlayerTaxaPokerData),pRet->nFollowedRoomsStrLen );

			CAutoBuffer myOwnRooms(pRet->nMyOwnRoomsStrLen + 1 );
			myOwnRooms.addContent(((char*)pRet)+ sizeof(stMsgSavePlayerTaxaPokerData) + pRet->nFollowedRoomsStrLen,pRet->nMyOwnRoomsStrLen );

			std::string strMaxcard = stMysqlField::UnIntArraryToString(pRet->tData.vMaxCards,MAX_TAXAS_HOLD_CARD) ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"UPDATE playertaxasdata SET winTimes = '%d', playTimes = '%d', singleWinMost = '%I64d', maxCard = '%s',myOwnRooms = '%s',followedRooms = '%s' WHERE userUID = '%d'",pRet->tData.nWinTimes,pRet->tData.nPlayTimes,pRet->tData.nSingleWinMost,strMaxcard.c_str(),myOwnRooms.getBufferPtr(),FollowedRooms.getBufferPtr(),pRet->nUserUID) ;
		}
		break;
	case MSG_SAVE_COMMON_LOGIC_DATA:
		{
			stMsgSavePlayerCommonLoginData* pRet = (stMsgSavePlayerCommonLoginData*)pmsg ;
			pRequest->eType = eRequestType_Update ;
			std::string strJoinedClub = stMysqlField::UnIntArraryToString(pRet->vJoinedClubID,MAX_JOINED_CLUB_CNT) ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"UPDATE playerbasedata SET mostCoinEver = '%I64d', vipLevel = '%d', nYesterdayCoinOffset = '%I64d', \
				nTodayCoinOffset = '%I64d',offlineTime = '%d',continueLoginDays = '%d',lastLoginTime = '%d',lastTakeCharityCoinTime = '%d', \
				longitude = '%f',latitude = '%f',vJoinedClubID = '%s' WHERE userUID = '%d' ",
				pRet->nMostCoinEver,pRet->nVipLevel,pRet->nYesterdayCoinOffset,pRet->nTodayCoinOffset,pRet->tOfflineTime,pRet->nContinueDays,pRet->tLastLoginTime,pRet->tLastTakeCharityCoinTime,pRet->dfLongitude,pRet->dfLatidue,
				strJoinedClub.c_str(),pRet->nUserUID) ;
		}
		break;
	case MSG_SAVE_CREATE_TAXAS_ROOM_INFO:
		{
			stMsgSaveCreateTaxasRoomInfo* pRet = (stMsgSaveCreateTaxasRoomInfo*)pmsg ;
			pRequest->eType = eRequestType_Add;
			
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO taxasroom (roomID, configID,ownerUID,createTime,chatRoomID) VALUES ('%d', '%u','%u','%u','%I64d')",
			 						pRet->nRoomID,pRet->nConfigID,pRet->nRoomOwnerUID,pRet->nCreateTime,pRet->nChatRoomID) ;
			CLogMgr::SharedLogMgr()->PrintLog("save create taxas room room id = %d",pRet->nRoomID);
		}
		break;
	case MSG_SAVE_UPDATE_TAXAS_ROOM_INFO:
		{
			stMsgSaveUpdateTaxasRoomInfo* pRet = (stMsgSaveUpdateTaxasRoomInfo*)pmsg ;
			pRequest->eType = eRequestType_Update ;
			pRet->vRoomDesc[MAX_LEN_ROOM_DESC-1] = 0 ;
			pRet->vRoomName[MAX_LEN_ROOM_NAME-1] = 0 ;
			CAutoBuffer autoBuffer(pRet->nInformLen + 1 );
			autoBuffer.addContent(((char*)&pRet) + sizeof(stMsgSaveUpdateTaxasRoomInfo),pRet->nInformLen);
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"UPDATE taxasroom SET deadTime = '%d', avataID = '%d', profit = '%I64d', roomName = '%s', roomDesc = '%s', roomInform = '%s', informSerial = '%d' WHERE roomID = '%d'"
				,pRet->nDeadTime,pRet->nAvataID,pRet->nRoomProfit,pRet->vRoomName,pRet->vRoomDesc,autoBuffer.getBufferPtr(),pRet->nInformSerial,pRet->nRoomID) ;
			CLogMgr::SharedLogMgr()->PrintLog("save taxas room update info room id = %d",pRet->nRoomID);
		}
		break;
	case MSG_READ_TAXAS_ROOM_INFO:
		{
			pRequest->eType = eRequestType_Select;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"SELECT * FROM taxasroom WHERE roomID != '0'") ;
			CLogMgr::SharedLogMgr()->PrintLog("read all room rooms");
		}
		break;
	case MSG_SAVE_TAXAS_ROOM_PLAYER:
		{
			stMsgSaveTaxasRoomPlayer* pRet = (stMsgSaveTaxasRoomPlayer*)pmsg ;
			int64_t nOffset = pRet->nFinalLeftInThisRoom - pRet->nTotalBuyInThisRoom ;
			if ( pRet->isUpdate )
			{
				pRequest->eType = eRequestType_Update;
				pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
					"UPDATE taxasroomplayers SET readInformSerial = '%u', totalBuyin = '%I64d', finalLeft = '%I64d', playTimes = '%u', winTimes = '%u', offset = '%lld' WHERE roomID = '%u' and playerUID = '%d'and flag = '0' "
					,pRet->m_nReadedInformSerial,pRet->nTotalBuyInThisRoom,pRet->nFinalLeftInThisRoom,pRet->nPlayeTimesInThisRoom,pRet->nWinTimesInThisRoom,nOffset,pRet->nRoomID,pRet->nPlayerUID) ;
				CLogMgr::SharedLogMgr()->PrintLog("updata taxas room player data room id = %u , uid = %u",pRet->nRoomID,pRet->nPlayerUID);
			}
			else
			{
				pRequest->eType = eRequestType_Add;
				pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO taxasroomplayers (roomID, playerUID,readInformSerial,totalBuyin,finalLeft,playTimes,winTimes,offset) VALUES ('%u', '%u','%u','%I64d','%I64d','%u','%u','%I64d')",
					pRet->nRoomID,pRet->nPlayerUID,pRet->m_nReadedInformSerial,pRet->nTotalBuyInThisRoom,pRet->nFinalLeftInThisRoom,pRet->nPlayeTimesInThisRoom,pRet->nWinTimesInThisRoom,nOffset) ;
				CLogMgr::SharedLogMgr()->PrintLog("add taxas room player data room id = %u , uid = %u",pRet->nRoomID,pRet->nPlayerUID);
			}
		}
		break;
	case MSG_READ_TAXAS_ROOM_PLAYERS:
		{
			stMsgReadTaxasRoomPlayers* pRet = (stMsgReadTaxasRoomPlayers*)pmsg ;
			pRequest->eType = eRequestType_Select;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"SELECT * FROM taxasroomplayers WHERE roomID = '%d' and flag = '0' order by offset desc limit 50 ",pRet->nRoomID) ;
			CLogMgr::SharedLogMgr()->PrintLog("read taxas room players room id = %d",pRet->nRoomID);
			pdata->nExtenArg1 = pRet->nRoomID;
		}
		break;
	case MSG_SAVE_REMOVE_TAXAS_ROOM_PLAYERS:
		{
			stMsgSaveRemoveTaxasRoomPlayers* pRet = (stMsgSaveRemoveTaxasRoomPlayers*)pmsg ;
			pRequest->eType = eRequestType_Update;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,sizeof(pRequest->pSqlBuffer),
				"UPDATE taxasroomplayers SET flag = '1' WHERE roomID = '%u' and flag = '0' ",pRet->nRoomID) ;
			CLogMgr::SharedLogMgr()->PrintLog("remove taxas room player data room id = %u ",pRet->nRoomID);
		}
		break;
// 	case MSG_PLAYER_SAVE_BASE_DATA:
// 		{
// 			stMsgGameServerSaveBaseData* pSaveBaseData = (stMsgGameServerSaveBaseData*)pmsg ;
// 			pdata->nSessionID = pSaveBaseData->nSessionID ;
// 			pdata->nExtenArg1 = pSaveBaseData->stBaseData.nUserUID ;
// 
// 			char pMaxCards[sizeof(pSaveBaseData->stBaseData.vMaxCards)*2 + 1 ] = {0} ;
// 			m_pTheApp->GetDBThread()->EscapeString(pMaxCards,(char*)pSaveBaseData->stBaseData.vMaxCards,sizeof(pSaveBaseData->stBaseData.vMaxCards)) ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_Low ;
// 			pRequest->eType = eRequestType_Update ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer, 
// 			"UPDATE playerbasedata SET playerName = '%s', defaultPhotoID = '%u', isUploadPhoto = '%u', signature = '%s', sex = '%u', vipLevel = '%u',   \
// 			coin = '%I64u', diamond = '%u', winTimes = '%u', loseTimes = '%u', singleWinMost = '%I64u', maxCard = '%s',   \
// 			longitude = '%f', latitude = '%f', exp = '%u', offlineTime = '%u',noticeID = '%u',vipEndTime = '%u',\
// 			continueLoginDays = '%u',lastLoginTime = '%u',lastTakeCharityCoinTime = '%u',todayPlayTimes = '%u' , \
// 			yesterdayPlayTimes = '%u',todayWinCoin = '%I64d',yesterdayWinCoin = '%I64d', \
// 			takeMasterStudentRewardTime = '%u',rechargeTimes = '%u',curOnlineBoxID = '%u',onlineBoxPassedTime = '%u' WHERE userUID = '%u'", 
// 			pSaveBaseData->stBaseData.cName,pSaveBaseData->stBaseData.nDefaulPhotoID,pSaveBaseData->stBaseData.bIsUploadPhoto,pSaveBaseData->stBaseData.cSignature,pSaveBaseData->stBaseData.nSex,pSaveBaseData->stBaseData.nVipLevel,
// 			pSaveBaseData->stBaseData.nCoin,pSaveBaseData->stBaseData.nDiamoned,pSaveBaseData->stBaseData.nWinTimes,pSaveBaseData->stBaseData.nLoseTimes,pSaveBaseData->stBaseData.nSingleWinMost,pMaxCards,
// 			pSaveBaseData->stBaseData.dfLongitude,pSaveBaseData->stBaseData.dfLatidue,pSaveBaseData->stBaseData.nExp,(unsigned int)pSaveBaseData->stBaseData.tOfflineTime,pSaveBaseData->stBaseData.nNoticeID,pSaveBaseData->stBaseData.nVipEndTime,
// 			pSaveBaseData->stBaseData.nContinueDays,(unsigned int)pSaveBaseData->stBaseData.tLastLoginTime,(unsigned int)pSaveBaseData->stBaseData.tLastTakeCharityCoinTime,pSaveBaseData->stBaseData.nTodayPlayTimes,
// 			pSaveBaseData->stBaseData.nYesterdayPlayTimes,pSaveBaseData->stBaseData.nTodayWinCoin,pSaveBaseData->stBaseData.nYesterdayWinCoin,
// 			(unsigned int)pSaveBaseData->stBaseData.tTakeMasterStudentRewardTime,pSaveBaseData->stBaseData.nRechargeTimes,pSaveBaseData->stBaseData.nCurOnlineBoxID,pSaveBaseData->stBaseData.nOnlineBoxPassedTime,pSaveBaseData->stBaseData.nUserUID) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break ;
// 	case MSG_SAVE_PLAYER_COIN:
// 		{
// 			stMsgGameServerSavePlayerCoin* pSaveCoin = (stMsgGameServerSavePlayerCoin*)pmsg ;
// 			pdata->nSessionID = pSaveCoin->nSessionID ;
// 			pdata->nExtenArg1 = pSaveCoin->nUserUID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_Low ;
// 			pRequest->eType = eRequestType_Update ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE gamedb.account SET nCoin = '%I64d', nDiamoned = '%d' WHERE UserUID = '%d'",pSaveCoin->nCoin,pSaveCoin->nDiamoned,pSaveCoin->nUserUID) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break;  
// 	case MSG_SAVE_FRIEND_LIST:
// 		{
// 			stMsgGameServerSaveFirendList* pSaveFriend = (stMsgGameServerSaveFirendList*)pmsg ;
// 			pdata->nSessionID = pSaveFriend->nSessionID ;
// 			pdata->nExtenArg1 = pSaveFriend->nUserUID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_Low ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			pRequest->eType = eRequestType_Update ;
// 
// 			char* pBuffer = (char*)pmsg ;
// 			pBuffer += sizeof(stMsgGameServerSaveFirendList);
// 
// 			char *pFriendListBuffer = new char[sizeof(stServerSaveFrienItem)*pSaveFriend->nFriendCount*2+1] ;
// 			memset(pFriendListBuffer,0,sizeof(sizeof(stServerSaveFrienItem)*pSaveFriend->nFriendCount*2+1));
// 			m_pTheApp->GetDBThread()->EscapeString(pFriendListBuffer,pBuffer,pSaveFriend->nFriendCount * sizeof(stServerSaveFrienItem) ) ;
// 			unsigned int nSaveTime = (unsigned int)time(NULL) ;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE playerfriend SET friendCount = '%u',contentData = '%s',saveTime = '%u' WHERE userUID = '%d'",pSaveFriend->nFriendCount,pFriendListBuffer,nSaveTime,pSaveFriend->nUserUID) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 			delete[]pFriendListBuffer ;
// 		}
// 		break;
// 	case MSG_REQUEST_FRIEND_LIST:
// 		{
// 			stMsgGameServerRequestFirendList* pRequestFriend = (stMsgGameServerRequestFirendList*)pmsg ;
// 			pdata->nSessionID = pRequestFriend->nSessionID ;
// 			pdata->nExtenArg1 = 0 ;  // more than one time select ; 
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_High ;
// 			pRequest->eType = eRequestType_Select ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"SELECT * FROM playerfriend WHERE userUID = '%u'",pRequestFriend->nUserUID ) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break;
// 	case MSG_REQUEST_FRIEND_BRIFDATA_LIST:
// 		{
// 			stMsgGameServerRequestFriendBrifDataList* pRet = (stMsgGameServerRequestFriendBrifDataList*)pmsg ;
// 			pdata->nSessionID = pRet->nSessionID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_High ;
// 			pRequest->eType = eRequestType_Select ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			char* pBffer = (char*)pRet ;
// 			pBffer += sizeof(stMsgGameServerRequestFriendBrifDataList);
// 			unsigned int * pUserUID = (unsigned int*)pBffer ;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"SELECT %s FROM playerfriend WHERE userUID = '%u'",PLAYER_BRIF_DATA,*pUserUID ) ;
// 			--pRet->nFriendCount ;
// 			while ( pRet->nFriendCount--)
// 			{
// 				pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"%s || userUID = '%u'",pRequest->pSqlBuffer,*pUserUID ) ;
// 				++pUserUID ;
// 			}
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break;
// 	case MSG_PLAYER_SERACH_PEERS:
// 		{
// 			stMsgGameServerGetSearchFriendResult* pMsgRet = (stMsgGameServerGetSearchFriendResult*)pmsg ;
// 			pdata->nSessionID = pMsgRet->nSessionID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_High ;
// 			pRequest->eType = eRequestType_Select ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			char* pBuffer = new char[pMsgRet->nLen * 2 + 1 ] ;
// 			memset(pBuffer,0 ,pMsgRet->nLen * 2 + 1  ) ;
// 			m_pTheApp->GetDBThread()->EscapeString(pBuffer, (char*)pMsgRet + sizeof(stMsgGameServerGetSearchFriendResult),pMsgRet->nLen) ;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"SELECT %s FROM playerbasedata where userUID regexp( '%s' ) or playerName regexp('%s') limit 15;", PLAYER_BRIF_DATA,pBuffer, pBuffer ) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 			delete[] pBuffer ;
// 		}
// 		break;
// 	case MSG_PLAYER_REQUEST_SEARCH_PEER_DETAIL:
// 		{
// 			stMsgGameServerGetSearchedPeerDetail* pMsgRet = (stMsgGameServerGetSearchedPeerDetail*)pmsg ;
// 			pdata->nSessionID = pMsgRet->nSessionID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_High ;
// 			pRequest->eType = eRequestType_Select ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"SELECT %s FROM playerbasedata where userUID = %u", PLAYER_DETAIL_DATA,pMsgRet->nPeerUserUID) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break;
// 	case MSG_PLAYER_REQUEST_FRIEND_DETAIL:
// 		{
// 			stMsgGameServerGetFriendDetail* pMsgRet = (stMsgGameServerGetFriendDetail*)pmsg ;
// 			pdata->nSessionID = pMsgRet->nSessionID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_High ;
// 			pRequest->eType = eRequestType_Select ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"SELECT %s FROM playerbasedata where userUID = %d", PLAYER_DETAIL_DATA,pMsgRet->nFriendUID) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break;
// 	case MSG_PLAYER_GET_MAIL_LIST:
// 		{
// 			stMsgGameServerGetMailList* pMsgRet = (stMsgGameServerGetMailList*)pmsg;
// 			pdata->nSessionID = pMsgRet->nSessionID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_High ;
// 			pRequest->eType = eRequestType_Select ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"select * from mail where userUID = %u order by postTime desc limit %d",pMsgRet->nUserUID, MAX_KEEP_MAIL_COUNT ) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break; 
// 	case MSG_PLAYER_SAVE_MAIL:
// 		{
// 			stMsgGameServerSaveMail* pMsgRet = (stMsgGameServerSaveMail*)pmsg  ;
// 			pdata->nSessionID = pMsgRet->nSessionID ;
// 			pdata->nExtenArg1 = pMsgRet->nUserUID ;
// 			char* pmsgData = (char*)pmsg ;
// 			pmsgData += sizeof(stMsgGameServerSaveMail);
// 			stMail* pMailToSave = (stMail*)pmsgData ;
// 			pdata->nExtenArg2 = (unsigned int)pMailToSave->nMailUID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_Low ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			switch ( pMsgRet->nOperateType ) 
// 			{
// 			case eDBAct_Update:
// 				{
// 					pRequest->eType = eRequestType_Update ;
// 					pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE mail SET processAct = '%d' WHERE mailUID = '%I64d'",pMailToSave->eProcessAct,pMailToSave->nMailUID ) ;
// 				}
// 				break;
// 			case eDBAct_Delete:
// 				{
// 					pRequest->eType = eRequestType_Delete ;
// 					pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"DELETE FROM mail WHERE mailUID ='%I64d' limit 1",pMailToSave->nMailUID ) ;
// 				}
// 				break;
// 			case eDBAct_Add:
// 				{
// 					pRequest->eType = eRequestType_Add ;
// 					char* pContent = new char[pMailToSave->nContentLen * 2 + 1 ] ;
// 					memset(pContent,0,pMailToSave->nContentLen * 2 + 1 );
// 					
// 					char* pBuffer = (char*)pMsgRet;
// 					pBuffer += sizeof(stMsgGameServerSaveMail);
// 					pBuffer += sizeof(stMail);
// 					m_pTheApp->GetDBThread()->EscapeString(pContent,pBuffer,pMailToSave->nContentLen ) ;
// 					CLogMgr::SharedLogMgr()->PrintLog("mail title content Len = %d",pMailToSave->nContentLen) ;
// 					pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO mail (mailUID, userUID,postTime,mailType,mailContentLen,mailContent,processAct) VALUES ('%I64d', '%u','%u','%u','%u','%s','%u')",
// 						pMailToSave->nMailUID,pMsgRet->nUserUID,pMailToSave->nPostTime,pMailToSave->eType,pMailToSave->nContentLen,pContent,pMailToSave->eProcessAct) ;
// 					delete[] pContent ;
// 				}
// 				break;
// 			default:
// 				CLogMgr::SharedLogMgr()->ErrorLog("unknown save mail operation type !") ;
// 				pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"") ;
// 				break;
// 			}
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break; 
// 	case MSG_GAME_SERVER_GET_MAX_MAIL_UID:
// 		{
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_Normal ;
// 			pRequest->eType = eRequestType_Select ;
// 			pRequest->nRequestUID = MSG_GAME_SERVER_GET_MAX_MAIL_UID;
// 			pRequest->pUserData = pdata;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"(select max(mail.mailUID) FROM mail)") ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break;
// 	case MSG_REQUEST_ITEM_LIST:
// 		{
// 			stMsgGameServerRequestItemList* pMsgRet = (stMsgGameServerRequestItemList*)pmsg ;
// 			pdata->nSessionID = pMsgRet->nSessionID ;
// 			pdata->nExtenArg1 = pMsgRet->nUserUID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_High ;
// 			pRequest->eType = eRequestType_Select ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"select * from playeritems where userUID = %u",pMsgRet->nUserUID ) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break;
// 	case MSG_SAVE_ITEM_LIST:
// 		{
// 			stMsgGameServerSaveItemList* pMsgRet = (stMsgGameServerSaveItemList*)pmsg ;
// 			pdata->nSessionID = pMsgRet->nSessionID ;
// 			pdata->nExtenArg1 = pMsgRet->nUserUID ;
// 			char* pBuffer = new char[pMsgRet->nOwnItemKindCount * sizeof(stPlayerItem) * 2 + 1] ;
// 			memset(pBuffer,0,pMsgRet->nOwnItemKindCount * sizeof(stPlayerItem) * 2 + 1);
// 			m_pTheApp->GetDBThread()->EscapeString(pBuffer, (((char*)pmsg) + sizeof(stMsgGameServerSaveItemList)),pMsgRet->nOwnItemKindCount * sizeof(stPlayerItem) ) ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_Low ;
// 			pRequest->pUserData = pdata;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->eType = eRequestType_Update ;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE playeritems SET itemsData = '%s',ownItemsKindCount = '%u' WHERE userUID = '%d'",pBuffer,pMsgRet->nOwnItemKindCount,pMsgRet->nUserUID ) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 			delete[] pBuffer ;
// 		}
// 		break; 
// 	case MSG_REQUEST_RANK:
// 		{
// 			stMsgGameServerRequestRank* pMsgRet = (stMsgGameServerRequestRank*)pmsg ;
// 			pdata->nExtenArg1 = pMsgRet->eType ;
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_Normal ;
// 			pRequest->eType = eRequestType_Select ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			switch ( pMsgRet->eType)
// 			{
// 			case eRank_AllCoin:
// 				{
// 					pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"select yesterdayWinCoin ,%s from playerbasedata where userUID !=0 order by coin desc limit %d",PLAYER_DETAIL_DATA,RANK_SHOW_PEER_COUNT) ;
// 				}
// 				break;
// 			case eRank_YesterDayWin:
// 				{
// 					unsigned int tYesterDay = (unsigned int)time(NULL) - 24 * 3600 ;
// 					//struct tm tmNow = *localtime(&tNow) ;
// 					//tmNow.tm_hour = 0 ;
// 					//tmNow.tm_min = 0 ;
// 					//tmNow.tm_sec = 0 ;
// 					//time_t tZero = mktime(&tmNow) ;
// 					pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"select yesterdayWinCoin ,%s from playerbasedata where (offlineTime >= '%u' || lastLoginTime >= '%u') order by todayWinCoin desc limit %d",PLAYER_DETAIL_DATA,tYesterDay,tYesterDay,RANK_SHOW_PEER_COUNT) ;
// 				}
// 				break;
// 			case eRank_SingleWinMost:
// 				{
// 					pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"select yesterdayWinCoin ,%s from playerbasedata where userUID !=0 order by singleWinMost desc limit %d",PLAYER_DETAIL_DATA,RANK_SHOW_PEER_COUNT) ;
// 				}
// 				break;
// 			default:
// 				CLogMgr::SharedLogMgr()->ErrorLog("unknown rank type to select type = %d",pMsgRet->eType) ;
// 				break;
// 			}
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break;
// 	//case MSG_GET_SHOP_BUY_RECORD:
// 	//	{
// 	//		stMsgGameServerGetShopBuyRecord* pMsgRet = (stMsgGameServerGetShopBuyRecord*)pmsg ;
// 	//		pdata->nSessionID = pMsgRet->nSessionID ;
// 	//		pdata->nExtenArg1 = pMsgRet->nUserUID ;
// 
// 	//		stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 	//		pRequest->cOrder = eReq_Order_High ;
// 	//		pRequest->eType = eRequestType_Select ;
// 	//		pRequest->nRequestUID = pmsg->usMsgType ;
// 	//		pRequest->pUserData = pdata;
// 	//		pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"select * from playershopbuyrecord where nUserUID = %d",pMsgRet->nUserUID ) ;
// 	//		CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 	//	}
// 	//	break;
// 	//case MSG_SAVE_SHOP_BUY_RECORD:
// 	//	{
// 	//		stMsgGameServerSaveShopBuyRecord* pMsgRet = (stMsgGameServerSaveShopBuyRecord*)pmsg;
// 	//		pdata->nSessionID = pMsgRet->nSessionID ;
// 	//		pdata->nExtenArg1 = pMsgRet->nUserUID ;
// 
// 	//		stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 	//		pRequest->cOrder = eReq_Order_Low ;
// 	//		pRequest->nRequestUID = pmsg->usMsgType ;
// 	//		pRequest->pUserData = pdata;
// 
// 	//		char* pSaveBuffer = new char[pMsgRet->nBufferLen * 2 + 1 ] ;
// 	//		memset(pSaveBuffer,0,pMsgRet->nBufferLen * 2 + 1) ;
// 	//		char* pBuffer = (char*)pmsg ;
// 	//		pBuffer += sizeof(stMsgGameServerSaveShopBuyRecord);
// 	//		m_pTheApp->GetDBThread()->EscapeString(pSaveBuffer,pBuffer,pMsgRet->nBufferLen ) ;
// 	//		if ( pMsgRet->bAdd )
// 	//		{
// 	//			pRequest->eType = eRequestType_Add ;
// 	//			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO `gamedb`.`playershopbuyrecord` (`nUserUID`, `pBuffer`) VALUES ('%d', '%s');",pMsgRet->nUserUID,pSaveBuffer) ;
// 	//		}
// 	//		else
// 	//		{
// 	//			pRequest->eType = eRequestType_Update ;
// 	//			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE gamedb.playershopbuyrecord SET pBuffer = '%s' WHERE nUserUID = '%d'",pSaveBuffer,pMsgRet->nUserUID) ;
// 	//		}
// 	//		CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 	//		delete[] pSaveBuffer ;
// 	//	}
// 	//	break;
// 	case MSG_GAME_SERVER_SAVE_MISSION_DATA:
// 		{
// 			stMsgGameServerSaveMissionData* pMissionData = (stMsgGameServerSaveMissionData*)pmsg ;
// 			pdata->nSessionID = pMissionData->nSessionID ;
// 			pdata->nExtenArg1 = pMissionData->nUserUID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_Low ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 
// 			char* pSaveBuffer = new char[pMissionData->nMissonCount * sizeof(stMissionSate) * 2 + 1 ] ;
// 			memset(pSaveBuffer,0,pMissionData->nMissonCount * sizeof(stMissionSate) * 2 + 1) ;
// 			char* pBuffer = (char*)pmsg ;
// 			pBuffer += sizeof(stMsgGameServerSaveMissionData);
// 			m_pTheApp->GetDBThread()->EscapeString(pSaveBuffer,pBuffer,pMissionData->nMissonCount * sizeof(stMissionSate)) ;
// 			pRequest->eType = eRequestType_Update ;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE playermission SET missionData = '%s',saveTime = '%u',missionCount = '%u' WHERE userUID = '%d'",pSaveBuffer,pMissionData->nSavetime,pMissionData->nMissonCount,pMissionData->nUserUID) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 			delete[] pSaveBuffer ;
// 		}
// 		break;
// 	case MSG_GAME_SERVER_GET_MISSION_DATA:
// 		{
// 			stMsgGameServerGetMissionData* pMsgRet = (stMsgGameServerGetMissionData*)pmsg ;
// 			pdata->nSessionID = pMsgRet->nSessionID ;
// 			pdata->nExtenArg1 = pMsgRet->nUserUID ;
// 
// 			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
// 			pRequest->cOrder = eReq_Order_High ;
// 			pRequest->eType = eRequestType_Select ;
// 			pRequest->nRequestUID = pmsg->usMsgType ;
// 			pRequest->pUserData = pdata;
// 			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"select * from playermission where userUID = %d",pMsgRet->nUserUID ) ;
// 			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
// 		}
// 		break;
	default:
		{
			m_vReserverArgData.push_back(pdata) ;
			CLogMgr::SharedLogMgr()->ErrorLog("unknown msg type = %d",pmsg->usMsgType ) ;
		}
	}

	if ( pRequest->nSqlBufferLen == 0 || pRequest->eType == eRequestType_Max )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("a request sql len = 0 , msg = %d" , pRequest->nRequestUID ) ;
		
		CDBRequestQueue::VEC_DBREQUEST v ;
		v.push_back(pRequest) ;
		CDBRequestQueue::SharedDBRequestQueue()->PushReserveRequest(v);
	}
	else
	{
		CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
	}
}

void CDBManager::OnDBResult(stDBResult* pResult)
{
	// process result 
// 	if ( pResult->nRequestUID == (unsigned int )-1 )   // get current max curUID ;
// 	{
// 		if ( pResult->nAffectRow > 0 )
// 		{
// 			CMysqlRow& pRow = *pResult->vResultRows.front(); 
// 			unsigned int nMaxID = pRow["max(Account.UserUID)"]->IntValue();
// 			if ( nMaxID >= nCurUserUID )
// 			{
// 				nCurUserUID = ++nMaxID ;
// 				CLogMgr::SharedLogMgr()->SystemLog("curMaxUID is %d",nMaxID ) ;
// 			}
// 		}
// 		return;
// 	}

	stArgData*pdata = (stArgData*)pResult->pUserData ;
	CLogMgr::SharedLogMgr()->PrintLog("processed db ret = %d",pResult->nRequestUID);
	switch ( pResult->nRequestUID )
	{
	case MSG_SELECT_DB_PLAYER_DATA:
		{
			stMsgSelectPlayerDataRet msgBack;
			msgBack.nRet = pResult->nAffectRow > 0 ? 0 : 1 ;
			if ( msgBack.nRet )
			{
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
				return ;
			}
			msgBack.nReqPlayerSessionID = pdata->nExtenArg1 ;
			msgBack.isDetail = pdata->nExtenArg2 ;
			CAutoBuffer auB(sizeof(msgBack) + sizeof(stPlayerDetailDataClient) );
			stPlayerDetailDataClient tData;
			CMysqlRow& pRow = *pResult->vResultRows.front();
			if ( msgBack.isDetail )
			{
				tData.bIsOnLine = false ;
				memset(tData.cName,0,sizeof(tData.cName)) ;
				sprintf_s(tData.cName,sizeof(tData.cName),"%s",pRow["cName"]->CStringValue()) ;
				tData.nCoin = pRow["nCoin"]->IntValue64();
				tData.nDiamoned = pRow["nDiamoned"]->IntValue64();
				tData.nPhotoID = pRow["nPhotoID"]->IntValue();
				tData.nSex = pRow["nSex"]->IntValue();
				tData.nUserUID = pRow["nUserUID"]->IntValue();
				tData.nVipLevel = pRow["nVipLevel"]->IntValue();

				memset(tData.cSignature,0,sizeof(tData.cSignature)) ;
				sprintf_s(tData.cSignature,sizeof(tData.cSignature),"%s",pRow["cSignature"]->CStringValue()) ;
				tData.nMostCoinEver = pRow["nMostCoinEver"]->IntValue64();
				tData.dfLatidue = pRow["dfLatidue"]->FloatValue();
				tData.dfLongitude = pRow["dfLongitude"]->FloatValue();

				time_t tLastOffline = pRow["tOfflineTime"]->IntValue();
				tData.tOfflineTime = tLastOffline ;

				std::vector<int> vInt ;
				vInt.clear();
				//read upload pic 
				vInt.clear();
				pRow["vUploadedPic"]->VecInt(vInt);
				memset(tData.vUploadedPic,0,sizeof(tData.vUploadedPic)) ;
				CLogMgr::SharedLogMgr()->PrintLog("vUploadedPic size = %d uid = %d",vInt.size(),tData.nUserUID ) ;
				if ( vInt.size() == MAX_UPLOAD_PIC )
				{
					for ( uint8_t nIdx = 0 ; nIdx < MAX_UPLOAD_PIC ; ++nIdx )
					{
						tData.vUploadedPic[nIdx] = vInt[nIdx] ;
					}
				}

				// read joined club ids ;
				vInt.clear();
				pRow["vJoinedClubID"]->VecInt(vInt);
				memset(tData.vJoinedClubID,0,sizeof(tData.vJoinedClubID)) ;
				CLogMgr::SharedLogMgr()->PrintLog("vJoinedClubID size = %d uid = %d",vInt.size(),tData.nUserUID ) ;
				if ( vInt.size() == MAX_JOINED_CLUB_CNT )
				{
					for ( uint8_t nIdx = 0 ; nIdx < MAX_JOINED_CLUB_CNT ; ++nIdx )
					{
						tData.vJoinedClubID[nIdx] = vInt[nIdx] ;
					}
				}

				// read taxas data 
				tData.tTaxasData.nPlayTimes = pRow["nPlayTimes"]->IntValue();
				tData.tTaxasData.nWinTimes = pRow["nWinTimes"]->IntValue();
				tData.tTaxasData.nSingleWinMost = pRow["nSingleWinMost"]->IntValue64();

				vInt.clear();
				// read max card ;
				pRow["vMaxCards"]->VecInt(vInt);
				memset(tData.tTaxasData.vMaxCards,0,sizeof(tData.tTaxasData.vMaxCards)) ;
				if ( vInt.size() == MAX_TAXAS_HOLD_CARD )
				{
					for ( uint8_t nIdx = 0 ; nIdx < MAX_TAXAS_HOLD_CARD ; ++nIdx )
					{
						tData.tTaxasData.vMaxCards[nIdx] = vInt[nIdx] ;
					}
				}
				CLogMgr::SharedLogMgr()->PrintLog("read select player detail uid = %d",tData.nUserUID) ;
			}
			else
			{
				GetPlayerBrifData(&tData,pRow) ;
			}
			auB.addContent(&msgBack,sizeof(msgBack)) ;
			auB.addContent(&tData,msgBack.isDetail ? sizeof(stPlayerDetailDataClient) : sizeof(stPlayerDetailDataClient)) ;
			m_pTheApp->sendMsg(pdata->nSessionID,auB.getBufferPtr(),auB.getContentSize()) ;
		}
		break;
	case MSG_PLAYER_READ_MAIL_LIST:
		{
			CLogMgr::SharedLogMgr()->PrintLog("read mail list for uid = %d cnt = %d",pdata->nExtenArg1,pResult->nAffectRow) ;
			if ( pResult->nAffectRow < 1 )
			{
				return ;
			}

			CAutoBuffer buffer(256);
			for ( uint16_t nIdx = 0 ; nIdx < pResult->nAffectRow ; ++nIdx )
			{
				CMysqlRow& pRow = *pResult->vResultRows[nIdx] ;
				stMsgReadMailListRet msgRet ;
				msgRet.bFinal = (nIdx + 1) == pResult->nAffectRow ;
				msgRet.pMails.eType = pRow["mailType"]->IntValue();
				msgRet.pMails.nPostTime = pRow["postTime"]->IntValue();
				msgRet.pMails.nContentLen = pRow["mailContent"]->nBufferLen ;
				if ( msgRet.pMails.nContentLen == 0 )
				{
					CLogMgr::SharedLogMgr()->ErrorLog("why this mail len is null uid = %d , post time = %u",pRow["userUID"]->IntValue(),msgRet.pMails.nPostTime);
				}
 
				if ( msgRet.pMails.nContentLen > 0 )
				{
					buffer.clearBuffer();
					buffer.addContent((char*)&msgRet,sizeof(msgRet)) ;
					buffer.addContent(pRow["mailContent"]->BufferData(),msgRet.pMails.nContentLen);
					m_pTheApp->sendMsg(pdata->nSessionID,buffer.getBufferPtr(),buffer.getContentSize()) ;
				}
				else
				{
					m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgRet,sizeof(msgRet)) ;
				}
			}
		}
		break;
	case MSG_READ_FRIEND_LIST:
		{
			if ( pResult->nAffectRow < 1 )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("read friend list error uid = %d",pdata->nExtenArg1) ;
				return ;
			}

			CMysqlRow& pRow = *pResult->vResultRows.front();
			stMsgReadFriendListRet msgRet ;
			msgRet.nFriendCountLen = pRow["friendUIDs"]->nBufferLen ;
			if ( msgRet.nFriendCountLen == 0 )
			{
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgRet,sizeof(msgRet)) ;
				return ;
			}

			CAutoBuffer auB(sizeof(msgRet) + msgRet.nFriendCountLen);
			auB.addContent(&msgRet,sizeof(msgRet)) ;
			auB.addContent(pRow["friendUIDs"]->BufferData(),pRow["friendUIDs"]->nBufferLen);
			m_pTheApp->sendMsg(pdata->nSessionID,auB.getBufferPtr(),auB.getContentSize()) ;
			CLogMgr::SharedLogMgr()->PrintLog("player uid = %d read friend list ok",pdata->nExtenArg1) ;
		}
		break;
	case MSG_REQUEST_CREATE_PLAYER_DATA:
		{
			if ( pResult->nAffectRow != 1 )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("create player data error uid = %d",pdata->nExtenArg1) ;
			}
			else
			{
				CMysqlRow& pRow = *pResult->vResultRows.front();
				if ( pRow["nOutRet"]->IntValue() != 0 )
				{
					CLogMgr::SharedLogMgr()->ErrorLog("pp create player data error uid = %d ret = %d",pdata->nExtenArg1,pRow["nOutRet"]->IntValue() ) ;
				}
				else
				{
					CLogMgr::SharedLogMgr()->PrintLog("create player data success uid = %d",pdata->nExtenArg1 ) ;
				}
			}
		}
		break;
	case MSG_READ_PLAYER_BASE_DATA:
		{
			stArgData* pdata = (stArgData*)pResult->pUserData ;
			stMsgDataServerGetBaseDataRet msg ;
			memset(&msg.stBaseData,0,sizeof(msg.stBaseData)) ;
			msg.nRet = 0 ;
			if ( pResult->nAffectRow <= 0 )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("can not find base data with userUID = %d , session id = %d " , pdata->nExtenArg1,pdata->nSessionID ) ;
				msg.nRet = 1 ;
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msg,sizeof(msg)) ;
			}
			else
			{
				CMysqlRow& pRow = *pResult->vResultRows[0] ;
				GetPlayerDetailData(&msg.stBaseData,pRow);
				msg.stBaseData.nYesterdayCoinOffset = pRow["nYesterdayCoinOffset"]->IntValue() ;
				msg.stBaseData.nTodayCoinOffset = pRow["nTodayCoinOffset"]->IntValue() ;
				msg.stBaseData.tLastLoginTime = pRow["lastLoginTime"]->IntValue() ;
				msg.stBaseData.tLastTakeCharityCoinTime = pRow["lastTakeCharityCoinTime"]->IntValue() ;
				msg.stBaseData.nContinueDays = pRow["continueLoginDays"]->IntValue() ;
				msg.stBaseData.isRegister = pRow["isRegister"]->IntValue() ;
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msg,sizeof(msg)) ;
			}
		}
		break;
	case MSG_READ_PLAYER_TAXAS_DATA:
		{
			stArgData* pdata = (stArgData*)pResult->pUserData ;
			stMsgReadPlayerTaxasDataRet msg ;
			msg.nRet = 0 ;
			msg.nUserUID = pdata->nExtenArg1 ;
			if ( pResult->nAffectRow <= 0 )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("can not find TAXAS_DATA with userUID = %d , session id = %d " , pdata->nExtenArg1,pdata->nSessionID ) ;
				msg.nRet = 1 ;
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msg,sizeof(msg)) ;
			}
			else
			{
				CMysqlRow& pRow = *pResult->vResultRows[0] ;
				msg.tData.nPlayTimes = pRow["playTimes"]->IntValue();
				msg.tData.nWinTimes = pRow["winTimes"]->IntValue();
				msg.tData.nSingleWinMost = pRow["singleWinMost"]->IntValue64();
				msg.nUserUID = pdata->nExtenArg1 ;

				std::vector<int> vInt ;
				vInt.clear();
				// read max card ;
				pRow["maxCard"]->VecInt(vInt);
				memset(msg.tData.vMaxCards,0,sizeof(msg.tData.vMaxCards)) ;
				if ( vInt.size() == MAX_TAXAS_HOLD_CARD )
				{
					for ( uint8_t nIdx = 0 ; nIdx < MAX_TAXAS_HOLD_CARD ; ++nIdx )
					{
						msg.tData.vMaxCards[nIdx] = vInt[nIdx] ;
					}
				}

				msg.nFollowedRoomsStrLen = pRow["followedRooms"]->nBufferLen ;
				msg.nMyOwnRoomsStrLen = pRow["myOwnRooms"]->nBufferLen;
				if ( msg.nFollowedRoomsStrLen <= 0 && msg.nMyOwnRoomsStrLen <= 0 )
				{
					m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msg,sizeof(msg)) ;
				}
				else
				{
					CAutoBuffer sendBuffer(sizeof(msg) + msg.nFollowedRoomsStrLen + msg.nMyOwnRoomsStrLen );
					sendBuffer.addContent((char*)&msg,sizeof(msg)) ;
					if ( msg.nFollowedRoomsStrLen > 0 )
					{
						sendBuffer.addContent(pRow["followedRooms"]->BufferData(),msg.nFollowedRoomsStrLen);
					}

					if ( msg.nMyOwnRoomsStrLen > 0 )
					{
						sendBuffer.addContent(pRow["myOwnRooms"]->BufferData(),msg.nMyOwnRoomsStrLen);
					}

					m_pTheApp->sendMsg(pdata->nSessionID,sendBuffer.getBufferPtr(),sendBuffer.getContentSize()) ;
				}
				
			}
		}
		break;
	case MSG_READ_TAXAS_ROOM_INFO:
		{
			stArgData* pdata = (stArgData*)pResult->pUserData ;
			if ( pResult->nAffectRow <= 0 )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("can not read taxas rooms ") ;
			}
			else
			{
				CAutoBuffer buffer(256);
				for ( uint16_t nIdx = 0 ; nIdx < pResult->nAffectRow ; ++nIdx )
				{
					CMysqlRow& pRow = *pResult->vResultRows[nIdx] ;
					stMsgReadTaxasRoomInfoRet msgRet ;
					msgRet.nAvataID = pRow["avataID"]->IntValue();
					msgRet.nConfigID = pRow["configID"]->IntValue();
					msgRet.nCreateTime = pRow["createTime"]->IntValue();
					msgRet.nDeadTime = pRow["deadTime"]->IntValue();
					msgRet.nInformSerial = pRow["informSerial"]->IntValue();
					msgRet.nRoomID = pRow["roomID"]->IntValue();
					msgRet.nRoomOwnerUID = pRow["ownerUID"]->IntValue();
					msgRet.nRoomProfit = pRow["profit"]->IntValue();
					msgRet.nChatRoomID = pRow["chatRoomID"]->IntValue64();
					memset(msgRet.vRoomName,0,sizeof(msgRet.vRoomName));
					memset(msgRet.vRoomDesc,0,sizeof(msgRet.vRoomDesc)) ;
					sprintf_s(msgRet.vRoomName,MAX_LEN_ROOM_NAME,"%s",pRow["roomName"]->CStringValue());
					sprintf_s(msgRet.vRoomDesc,MAX_LEN_ROOM_DESC,"%s",pRow["roomDesc"]->CStringValue());
					msgRet.nInformLen = pRow["roomInform"]->nBufferLen ; 
					if ( msgRet.nInformLen > 0 )
					{
						buffer.clearBuffer();
						buffer.addContent((char*)&msgRet,sizeof(msgRet)) ;
						buffer.addContent(pRow["roomInform"]->BufferData(),msgRet.nInformLen);
						m_pTheApp->sendMsg(pdata->nSessionID,buffer.getBufferPtr(),buffer.getContentSize()) ;
					}
					else
					{
						m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgRet,sizeof(msgRet)) ;
					}
					CLogMgr::SharedLogMgr()->PrintLog("read taxas room id = %d",msgRet.nRoomID);
				}
			}
		}
		break;
	case MSG_READ_TAXAS_ROOM_PLAYERS:
		{
			stArgData* pdata = (stArgData*)pResult->pUserData ;
			if ( pResult->nAffectRow == 0 )
			{
				CLogMgr::SharedLogMgr()->PrintLog("room id = %d have no history players",pdata->nExtenArg1) ;
			}
			else
			{
				for ( uint16_t nIdx = 0 ; nIdx < pResult->nAffectRow ; ++nIdx )
				{
					CMysqlRow& pRow = *pResult->vResultRows[nIdx] ;
					stMsgReadTaxasRoomPlayersRet msgRet ;
					msgRet.nRoomID = pRow["roomID"]->IntValue();
					msgRet.nPlayerUID = pRow["playerUID"]->IntValue();
					msgRet.m_nReadedInformSerial = pRow["readInformSerial"]->IntValue();
					msgRet.nTotalBuyInThisRoom = pRow["totalBuyin"]->IntValue64();
					msgRet.nFinalLeftInThisRoom = pRow["finalLeft"]->IntValue64();
					msgRet.nWinTimesInThisRoom = pRow["playTimes"]->IntValue();
					msgRet.nPlayeTimesInThisRoom = pRow["winTimes"]->IntValue();
					m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgRet,sizeof(msgRet)) ;
					CLogMgr::SharedLogMgr()->PrintLog("read taxas room players room id = %d",msgRet.nRoomID);
				}
			}
		}
		break;
	case MSG_SAVE_CREATE_TAXAS_ROOM_INFO:
	case MSG_SAVE_UPDATE_TAXAS_ROOM_INFO:
	case MSG_SAVE_REMOVE_TAXAS_ROOM_PLAYERS:
	case MSG_SAVE_TAXAS_ROOM_PLAYER:
	case MSG_SAVE_PLAYER_TAXAS_DATA:
	case MSG_SAVE_FRIEND_LIST:
	case MSG_PLAYER_SAVE_MAIL:
	case MSG_PLAYER_SET_MAIL_STATE:
		{
			if ( pResult->nAffectRow <= 0 )
			{
				CLogMgr::SharedLogMgr()->ErrorLog(" db result msg id = %d , row cnt = %d, failed  ", pResult->nRequestUID,pResult->nAffectRow );
			}
		}
		break;
//	case MSG_PLAYER_SAVE_BASE_DATA:
//		{
//			if ( pResult->nAffectRow > 0 )
//			{
//				CLogMgr::SharedLogMgr()->PrintLog("save player base data ok UID = %d",pdata->nExtenArg1) ;
//			}
//			else
//			{
//				CLogMgr::SharedLogMgr()->PrintLog("save player base data Error UID = %d",pdata->nExtenArg1) ;
//			}
//		}
//		break;
//	case MSG_SAVE_PLAYER_COIN:
//		{
//			stMsgGameServerSavePlayerCoinRet ret ;
//			ret.nSessionID = pdata->nSessionID ;
//			if ( pResult->nAffectRow > 0 )
//			{
//				ret.nRet = 0 ;
//			}
//			else
//			{
//				ret.nRet = 1 ;
//				CLogMgr::SharedLogMgr()->PrintLog("Save player COIN Error ! UID = %d",pdata->nExtenArg1) ;
//			}
//			m_pTheApp->SendMsg((char*)&ret,sizeof(ret),pdata->m_nReqrestFromAdd); 
//		}
//		break;
//	case MSG_SAVE_FRIEND_LIST:
//		{
//			stMsgGameServerSaveFriendListRet ret ;
//			ret.nSessionID = pdata->nSessionID ;
//			if ( pResult->nAffectRow > 0 )
//			{
//				ret.nRet = 0 ;
//			}
//			else
//			{
//				ret.nRet = 1 ;
//				CLogMgr::SharedLogMgr()->PrintLog("Save player MSG_SAVE_FRIEND_LIST ! UID = %d",pdata->nExtenArg1) ;
//			}
//			m_pTheApp->SendMsg((char*)&ret,sizeof(ret),pdata->m_nReqrestFromAdd); 
//		}
//		break; 
//	case MSG_REQUEST_FRIEND_LIST:
//		{
//			stMsgGameServerRequestFirendListRet msgBack ;
//			msgBack.nSessionID = pdata->nSessionID ; 
//			if ( pResult->nAffectRow > 0 )
//			{
//				CMysqlRow& pRow = *pResult->vResultRows[0] ;
//				msgBack.nFriendCount = pRow["friendCount"]->IntValue();
//				if ( msgBack.nFriendCount == 0 )
//				{
//					m_pTheApp->SendMsg((char*)&msgBack,sizeof(msgBack),pdata->m_nReqrestFromAdd); 
//					break;
//				}
//
//				// parse friends ;
//				char* pBuffer = new char[msgBack.nFriendCount * sizeof(stServerSaveFrienItem) + sizeof(msgBack)] ;
//				unsigned char nOffset = 0 ;
//				memcpy(pBuffer,&msgBack,sizeof(msgBack));
//				nOffset += sizeof(msgBack);
//				memcpy(pBuffer + nOffset , pRow["contentData"]->BufferData(),pRow["contentData"]->nBufferLen);
//				nOffset += pRow["contentData"]->nBufferLen ;
//#ifdef DEBUG
//				if ( pRow["contentData"]->nBufferLen != msgBack.nFriendCount * sizeof(stServerSaveFrienItem) )
//				{
//					CLogMgr::SharedLogMgr()->ErrorLog("why save buffer and read buffer is not equal len , read friend list ?") ;
//				}
//#endif
//				// update present times info ;
//				time_t tNow = time(NULL) ;
//				time_t nLastSave = pRow["saveTime"]->IntValue();
//				struct tm tmNow = *localtime(&tNow) ;
//				struct tm tmLastSave = *localtime(&nLastSave) ;
//				if ( tmNow.tm_yday != tmLastSave.tm_yday )
//				{
//					// reset present times ;
//					char* pDBuffer = pBuffer + sizeof(msgBack);
//					stServerSaveFrienItem* pSave = (stServerSaveFrienItem*)pDBuffer;
//					int nCount = msgBack.nFriendCount ;
//					while(nCount--)
//					{
//						pSave->nPresentTimes = 0 ;
//						++pSave ;
//					}
//				}
//				m_pTheApp->SendMsg(pBuffer,nOffset,pdata->m_nReqrestFromAdd); 
//				delete[] pBuffer ;
//			}
//			else
//			{
//				msgBack.nFriendCount = 0 ;
//				m_pTheApp->SendMsg((char*)&msgBack,sizeof(msgBack),pdata->m_nReqrestFromAdd); 
//				break;
//			}
//		}
//		break;
//	case MSG_REQUEST_FRIEND_BRIFDATA_LIST:
//		{
//			stMsgGameServerRequestFriendBrifDataListRet msgRet ;
//			msgRet.nCount = pResult->nAffectRow ;
//			if ( msgRet.nCount <= 0 )
//			{
//				CLogMgr::SharedLogMgr()->ErrorLog("How can fried brif info list is NULL ?") ;
//				m_pTheApp->SendMsg((char*)&msgRet,sizeof(msgRet),pdata->m_nReqrestFromAdd); 
//				break;
//			}
//
//			char* pBuffer = new char[sizeof(msgRet) + sizeof(stPlayerBrifData) * msgRet.nCount] ;
//			memcpy(pBuffer,&msgRet,sizeof(msgRet));
//			stPlayerBrifData* pInfo = (stPlayerBrifData*)(pBuffer + sizeof(msgRet));
//			for ( int i = 0 ; i < pResult->nAffectRow ; ++i )
//			{
//				CMysqlRow& pRow = *pResult->vResultRows[i];
//				GetPlayerBrifData(pInfo,pRow) ;
//				++pInfo ;
//			}
//			m_pTheApp->SendMsg(pBuffer,sizeof(msgRet) + sizeof(stPlayerBrifData) * msgRet.nCount ,pdata->m_nReqrestFromAdd); 
//			delete[] pBuffer ;
//		}
//		break;
//	case MSG_PLAYER_SERACH_PEERS:
//		{
//			stMsgGameServerGetSearchFriendResultRet msgBack ;
//			msgBack.nSessionID = pdata->nSessionID ;
//			msgBack.nResultCount = pResult->nAffectRow ;
//			if ( msgBack.nResultCount <= 0 )
//			{
//				m_pTheApp->SendMsg((char*)&msgBack,sizeof(msgBack) ,pdata->m_nReqrestFromAdd); 
//				break;
//			}
//
//			char* pBuffer = new char[sizeof(msgBack) + sizeof(stPlayerBrifData) * msgBack.nResultCount] ;
//			memcpy(pBuffer,&msgBack,sizeof(msgBack));
//			stPlayerBrifData* pInfo = (stPlayerBrifData*)(pBuffer + sizeof(msgBack));
//			for ( int i = 0 ; i < pResult->nAffectRow ; ++i )
//			{
//				CMysqlRow& pRow = *pResult->vResultRows[i];
//				GetPlayerBrifData(pInfo,pRow) ;
//				++pInfo ;
//			}
//			m_pTheApp->SendMsg(pBuffer,sizeof(msgBack) + sizeof(stPlayerBrifData) * msgBack.nResultCount ,pdata->m_nReqrestFromAdd); 
//			delete[] pBuffer ;
//		}
//		break;
//	case MSG_PLAYER_REQUEST_FRIEND_DETAIL:
//		{
//			stMsgGameServerGetFriendDetailRet msg ;
//			msg.nSessionID = pdata->nSessionID ;
//			if ( pResult->nAffectRow <= 0 )
//			{
//				msg.nRet = 1 ;
//			}
//			else
//			{
//				msg.nRet = 0 ;
//				CMysqlRow& pRow = *pResult->vResultRows[0];
//				GetPlayerDetailData(&msg.stPeerInfo,pRow) ;
//			}
//			m_pTheApp->SendMsg((char*)&msg,sizeof(msg),pdata->m_nReqrestFromAdd); 
//		}
//		break;
//	case MSG_PLAYER_REQUEST_SEARCH_PEER_DETAIL:
//		{
//			stMsgGameServerGetSearchedPeerDetailRet msg ;
//			msg.nSessionID = pdata->nSessionID ;
//			if ( pResult->nAffectRow <= 0 )
//			{
//				msg.nRet = 1 ;
//			}
//			else
//			{
//				msg.nRet = 0 ;
//				CMysqlRow& pRow = *pResult->vResultRows[0];
//				GetPlayerDetailData(&msg.stPeerInfo,pRow) ;
//			}
//			m_pTheApp->SendMsg((char*)&msg,sizeof(msg),pdata->m_nReqrestFromAdd); 
//		}
//		break;
//	case MSG_PLAYER_SAVE_MAIL:
//		{
//			if ( pResult->nAffectRow > 0 )
//			{
//				CLogMgr::SharedLogMgr()->PrintLog("Save Mail Success") ;
//			}
//			else
//			{
//				CLogMgr::SharedLogMgr()->ErrorLog("Save Mail Failed, UserID = %d,MainID = %d",pdata->nExtenArg1,pdata->nExtenArg2) ;
//			}
//
//		}
//		break;
//	case MSG_PLAYER_GET_MAIL_LIST:
//		{
//			stMsgGameServerGetMailListRet pSendMsg ;
//			pSendMsg.nSessionID = pdata->nSessionID ;
//			pSendMsg.nMailCount = pResult->nAffectRow ;
//			std::vector<stMail*> VecMail ;
//			unsigned short nToTalLen = sizeof(pSendMsg);
//			for ( unsigned int i = 0 ; i < pResult->nAffectRow ; ++i )
//			{
//				CMysqlRow& pRow = *pResult->vResultRows[i];
//				stMail* mail = new stMail;
//				mail->eProcessAct = (bool)pRow["processAct"]->IntValue();
//				mail->eType = (eMailType)pRow["mailType"]->IntValue();
//				mail->nContentLen = pRow["mailContent"]->nBufferLen;
//				mail->nMailUID = pRow["mailUID"]->IntValue64();
//				mail->nPostTime = pRow["postTime"]->IntValue() ;
//
//				if ( 0 == mail->nContentLen )
//				{
//					mail->pContent = NULL;
//				}
//				else
//				{
//					mail->pContent = new char[mail->nContentLen]; 
//					memcpy(mail->pContent,pRow["mailContent"]->BufferData(),mail->nContentLen);
//				}
//
//				VecMail.push_back(mail) ;
//				nToTalLen += sizeof(stMail);
//				nToTalLen += mail->nContentLen ;
//			}
//
//			char* pBuffer = new char[nToTalLen] ;
//			unsigned short nOffset = 0 ;
//			memcpy(pBuffer,&pSendMsg,sizeof(pSendMsg));
//			nOffset += sizeof(pSendMsg);
//			for ( unsigned int i = 0 ; i < VecMail.size() ; ++i )
//			{
//				stMail* sMail = VecMail[i] ;
//				memcpy(pBuffer + nOffset , sMail,sizeof(stMail));
//				nOffset += sizeof(stMail);
//
//				if ( sMail->nContentLen > 0 )
//				{
//					memcpy(pBuffer + nOffset , sMail->pContent,sMail->nContentLen);
//					nOffset += sMail->nContentLen ;
//					delete[] sMail->pContent ;
//				}
//
//				delete sMail ;
//			}
//			m_pTheApp->SendMsg(pBuffer,nOffset,pdata->m_nReqrestFromAdd); 
//			delete[] pBuffer ;
//		}
//		break;
//	case MSG_GAME_SERVER_GET_MAX_MAIL_UID:
//		{
//			stMsgGameServerGetMaxMailUIDRet msg ;
//			if ( pResult->nAffectRow == 0 )
//			{
//				msg.nMaxMailUID = 10 ;
//			}
//			else
//			{
//				msg.nMaxMailUID = pResult->vResultRows[0]->GetFiledByName("max(mail.mailUID)")->IntValue();
//			}
//			m_pTheApp->SendMsg((char*)&msg,sizeof(msg),pdata->m_nReqrestFromAdd); 
//			CLogMgr::SharedLogMgr()->SystemLog("Cur Max MailUID = %I64d",msg.nMaxMailUID) ;
//		}
//		break;
//	case MSG_REQUEST_ITEM_LIST:
//		{
//			stMsgGameServerRequestItemListRet msg;
//			msg.nSessionID = pdata->nSessionID;
//			if ( pResult->nAffectRow <= 0 )
//			{
//				CLogMgr::SharedLogMgr()->ErrorLog("why have no item recorder ? must inster when create player ") ;
//				msg.nOwnItemKindCount = 0 ;
//				m_pTheApp->SendMsg((char*)&msg,sizeof(msg),pdata->m_nReqrestFromAdd); 
//				break;
//			}
//			else
//			{
//				CMysqlRow& pRow = *pResult->vResultRows[0] ;
//				msg.nOwnItemKindCount = pRow["ownItemsKindCount"]->IntValue();
//				char* pBuffer = new char[sizeof(msg) + pRow["itemsData"]->nBufferLen] ;
//				memcpy(pBuffer,&msg,sizeof(msg));
//				unsigned short nOffset = sizeof(msg);
//				memcpy(pBuffer + nOffset ,pRow["itemsData"]->BufferData(), pRow["itemsData"]->nBufferLen );
//				nOffset += pRow["itemsData"]->nBufferLen;
//				m_pTheApp->SendMsg(pBuffer,nOffset,pdata->m_nReqrestFromAdd); 
//				delete[] pBuffer;
//			}
//		}
//		break; 
//	case MSG_SAVE_ITEM_LIST:
//		{
//			if ( pResult->nAffectRow > 0 )
//			{
//				CLogMgr::SharedLogMgr()->PrintLog("Save Item List successed UID = %d",pdata->nExtenArg1) ;
//			}
//			else
//			{
//				CLogMgr::SharedLogMgr()->PrintLog("Save Item List Failed UID = %d",pdata->nExtenArg1) ;
//			}
//		}
//		break; 
//	case MSG_REQUEST_RANK:
//		{
//			stMsgGameServerRequestRankRet msg ;
//			msg.eType = pdata->nExtenArg1 ;
//			msg.nPeerCount = pResult->vResultRows.size();
//			if ( msg.nPeerCount <= 0 )
//			{
//				m_pTheApp->SendMsg((char*)&msg,sizeof(msg),pdata->m_nReqrestFromAdd); 
//				break;
//			}
//
//			char* pBuffer = new char[sizeof(msg) + msg.nPeerCount * sizeof(stServerGetRankPeerInfo)];
//			char* pBufferTemp = NULL ;
//			memcpy(pBuffer,&msg,sizeof(msg));
//			pBufferTemp = pBuffer + sizeof(msg);
//			stServerGetRankPeerInfo* pInfo = (stServerGetRankPeerInfo*)pBufferTemp ;
//			for ( int i = 0 ; i < msg.nPeerCount ; ++i )
//			{
//				CMysqlRow& pRow = *pResult->vResultRows[i] ;
//				GetPlayerDetailData(&pInfo->tDetailData,pRow);
//				pInfo->nYesterDayWin = pRow["yesterdayWinCoin"]->IntValue64() ;
//				++pInfo;
//			}
//			m_pTheApp->SendMsg(pBuffer,sizeof(msg) + msg.nPeerCount * sizeof(stServerGetRankPeerInfo) ,pdata->m_nReqrestFromAdd); 
//			delete[] pBuffer ;
//		}
//		break ;
//	//case MSG_GET_SHOP_BUY_RECORD:
//	//	{
//	//		stMsgGameServerGetShopBuyRecordRet msg ;
//	//		msg.nSessionID = pdata->nSessionID ;
//	//		if ( pResult->nAffectRow == 0 )
//	//		{
//	//			msg.nBufferLen = 0 ;
//	//			m_pTheApp->SendMsg((char*)&msg,sizeof(msg) ,pdata->m_nReqrestFromAdd); 
//	//		}
//	//		else
//	//		{
//	//			msg.nBufferLen = pResult->vResultRows[0]->GetFiledByName("pBuffer")->nBufferLen ;
//	//			char* pBuffer = new char[sizeof(msg) + msg.nBufferLen ] ;
//	//			memcpy(pBuffer,&msg,sizeof(msg));
//	//			memcpy(pBuffer + sizeof(msg),pResult->vResultRows[0]->GetFiledByName("pBuffer")->BufferData(),msg.nBufferLen );
//	//			m_pTheApp->SendMsg(pBuffer,sizeof(msg) + msg.nBufferLen ,pdata->m_nReqrestFromAdd); 
//	//			delete[] pBuffer ;
//	//		}
//	//	}
//	//	break;
//	//case MSG_SAVE_SHOP_BUY_RECORD:
//	//	{
//	//		if ( pResult->nAffectRow > 0 )
//	//		{
//	//			CLogMgr::SharedLogMgr()->PrintLog("Save SHOP_BUY_RECORD successed UID = %d",pdata->nExtenArg1) ;
//	//		}
//	//		else
//	//		{
//	//			CLogMgr::SharedLogMgr()->PrintLog("Save SHOP_BUY_RECORD Failed UID = %d",pdata->nExtenArg1) ;
//	//		}
//	//	}
//	//	break;
//	case MSG_GAME_SERVER_SAVE_MISSION_DATA:
//		{
//			if ( pResult->nAffectRow > 0 )
//			{
//				CLogMgr::SharedLogMgr()->PrintLog("Save MSG_GAME_SERVER_SAVE_MISSION_DATA successed UID = %d",pdata->nExtenArg1) ;
//			}
//			else
//			{
//				CLogMgr::SharedLogMgr()->PrintLog("Save MSG_GAME_SERVER_SAVE_MISSION_DATA Failed UID = %d",pdata->nExtenArg1) ;
//			}
//		}
//		break;
//	case MSG_GAME_SERVER_GET_MISSION_DATA:
//		{
//			stMsgGameServerGetMissionDataRet msg ;
//			msg.nSessionID = pdata->nSessionID ;
//			if ( pResult->nAffectRow <= 0 )
//			{
//				msg.nMissonCount = 0 ;
//				msg.nLastSaveTime = 0 ;
//				m_pTheApp->SendMsg((char*)&msg,sizeof(msg) ,pdata->m_nReqrestFromAdd); 
//				CLogMgr::SharedLogMgr()->ErrorLog("Mission recorder can not be null , inster one when create player ") ;
//			}
//			else
//			{
//				CMysqlRow& pRow = *pResult->vResultRows[0];
//				msg.nMissonCount = pRow["missionCount"]->IntValue() ;
//				msg.nLastSaveTime = pRow["saveTime"]->IntValue(); 
//				char* pBuffer = new char[sizeof(msg) + pRow["missionData"]->nBufferLen ] ;
//				memcpy(pBuffer,&msg,sizeof(msg));
//				memcpy(pBuffer + sizeof(msg),pRow["missionData"]->BufferData(),pRow["missionData"]->nBufferLen );
//				m_pTheApp->SendMsg(pBuffer,sizeof(msg) + pRow["missionData"]->nBufferLen ,pdata->m_nReqrestFromAdd); 
//				delete[] pBuffer ;
//			}
//		}
//		break;
	default:
		{
			if ( pResult->nAffectRow <= 0 )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("unprocessed db result msg id = %d , row cnt = %d  ", pResult->nRequestUID,pResult->nAffectRow );
			}
			else
			{
				CLogMgr::SharedLogMgr()->SystemLog("unprocessed db result msg id = %d , row cnt = %d  ", pResult->nRequestUID,pResult->nAffectRow );
			}
		}
	}
	m_vReserverArgData.push_back(pdata) ;
}

CDBManager::stArgData* CDBManager::GetReserverArgData()
{
	LIST_ARG_DATA::iterator iter = m_vReserverArgData.begin() ;
	if ( iter != m_vReserverArgData.end() )
	{
		stArgData* p = *iter ;
		m_vReserverArgData.erase(iter) ;
		p->Reset();
		return p ;
	}
	return NULL ;
}

void CDBManager::GetPlayerDetailData(stPlayerDetailData* pData, CMysqlRow&prow)
{
	GetPlayerBrifData(pData,prow) ;
	memset(pData->cSignature,0,sizeof(pData->cSignature)) ;
	sprintf_s(pData->cSignature,sizeof(pData->cSignature),"%s",prow["signature"]->CStringValue()) ;
	pData->nMostCoinEver = prow["mostCoinEver"]->IntValue64();
	pData->dfLatidue = prow["latitude"]->FloatValue();
	pData->dfLongitude = prow["longitude"]->FloatValue();

	time_t tLastOffline = prow["offlineTime"]->IntValue();
	pData->tOfflineTime = tLastOffline ;
	
	std::vector<int> vInt ;
	vInt.clear();
	//read upload pic 
	vInt.clear();
	prow["vUploadedPic"]->VecInt(vInt);
	memset(pData->vUploadedPic,0,sizeof(pData->vUploadedPic)) ;
	CLogMgr::SharedLogMgr()->PrintLog("vUploadedPic size = %d uid = %d",vInt.size(),pData->nUserUID ) ;
	if ( vInt.size() == MAX_UPLOAD_PIC )
	{
		for ( uint8_t nIdx = 0 ; nIdx < MAX_UPLOAD_PIC ; ++nIdx )
		{
			pData->vUploadedPic[nIdx] = vInt[nIdx] ;
		}
	}

	// read joined club ids ;
	vInt.clear();
	prow["vJoinedClubID"]->VecInt(vInt);
	memset(pData->vJoinedClubID,0,sizeof(pData->vJoinedClubID)) ;
	CLogMgr::SharedLogMgr()->PrintLog("vJoinedClubID size = %d uid = %d",vInt.size(),pData->nUserUID ) ;
	if ( vInt.size() == MAX_JOINED_CLUB_CNT )
	{
		for ( uint8_t nIdx = 0 ; nIdx < MAX_JOINED_CLUB_CNT ; ++nIdx )
		{
			pData->vJoinedClubID[nIdx] = vInt[nIdx] ;
		}
	}

// 	time_t tNow = time(NULL) ;
// 	struct tm tLast = *localtime(&tLastOffline) ;
// 	struct tm tNowt = *localtime(&tNow) ;
// 	if ( tLast.tm_yday == tNowt.tm_yday - 1 )  // yesterday offline ;
// 	{
// 		//pData->nYesterDayPlayTimes = prow["todayPlayTimes"]->IntValue();
// 	}
// 	else if ( tLast.tm_yday < tNowt.tm_yday -1 )
// 	{
// 		//pData->nYesterDayPlayTimes = 0 ;
// 	}
}

void CDBManager::GetPlayerBrifData(stPlayerBrifData*pData,CMysqlRow&prow)
{
	pData->bIsOnLine = false ;
	memset(pData->cName,0,sizeof(pData->cName)) ;
	sprintf_s(pData->cName,sizeof(pData->cName),"%s",prow["playerName"]->CStringValue()) ;
	pData->nCoin = prow["coin"]->IntValue64();
	pData->nDiamoned = prow["diamond"]->IntValue();
	pData->nPhotoID = prow["photoID"]->IntValue();
	pData->nSex = prow["sex"]->IntValue();
	pData->nUserUID = prow["userUID"]->IntValue();
	pData->nVipLevel = prow["vipLevel"]->IntValue();
}