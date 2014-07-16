#pragma warning(disable:4800)
#include "DBManager.h"
#include "LogManager.h"
#include "DBRequest.h"
#include "ServerMessageDefine.h"
#include "DBApp.h"
#include "DataBaseThread.h"
#define PLAYER_BRIF_DATA "playerName,userUID,sex,vipLevel,defaultPhotoID,isUploadPhoto,exp,coin,diamond"
#define PLAYER_DETAIL_DATA "playerName,userUID,sex,vipLevel,defaultPhotoID,isUploadPhoto,exp,coin,diamond,signature,singleWinMost,winTimes,loseTimes,yesterdayPlayTimes,todayPlayTimes,longitude,latitude,offlineTime"
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
	stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
	pRequest->cOrder = eReq_Order_High ;
	pRequest->eType = eRequestType_Select ;
	pRequest->nRequestUID = -1;
	pRequest->pUserData = NULL;
	pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"(select max(Account.UserUID) FROM Account)") ;
	CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
}

void CDBManager::OnMessage(RakNet::Packet* packet)
{
	// construct sql
	stArgData* pdata = GetReserverArgData() ;
	if ( pdata == NULL )
	{
		pdata = new stArgData ;
	}

	pdata->m_nReqrestFromAdd = packet->guid ;
	stMsg* pmsg = (stMsg*)packet->data ;
	switch( pmsg->usMsgType )
	{
	case MSG_PLAYER_CHECK_ACCOUNT:
		{
			stMsgLoginAccountCheck* pLoginCheck = (stMsgLoginAccountCheck*)pmsg ;
			pdata->nSessionID = pLoginCheck->nSessionID ;
			pdata->pUserData = new char[MAX_LEN_PASSWORD] ;
			memset(pdata->pUserData,0,MAX_LEN_PASSWORD);
			memcpy(pdata->pUserData,pLoginCheck->cPassword,MAX_LEN_PASSWORD);

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_High ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata ;
			// format sql String ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"SELECT password,userUID,state,registerType FROM account WHERE account = '%s'",pLoginCheck->cAccount ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case MSG_PLAYER_REGISTER:
		{
			stMsgLoginRegister* pLoginRegister = (stMsgLoginRegister*)pmsg ;
			pdata->nSessionID = pLoginRegister->nSessionID ;
			pdata->nExtenArg1 = GenerateUserUID();

			if ( pLoginRegister->cRegisterType == 0  )
			{
				memset(pLoginRegister->cAccount,0,sizeof(pLoginRegister->cAccount)) ;
				memset(pLoginRegister->cPassword,0,sizeof(pLoginRegister->cPassword)) ;
				sprintf_s(pLoginRegister->cAccount," %d s",GenerateUserUID());
				sprintf_s(pLoginRegister->cPassword,"hello");
			}

			pdata->pUserData = new char[sizeof(stMsgLoginRegister)];
			memcpy(pdata->pUserData,pLoginRegister,sizeof(stMsgLoginRegister));

			/*		char* paccount = NULL , *ppassword = NULL , *pcharaName = NULL;
			char* pBuffer = (char*)pmsg ;
			pBuffer += sizeof(stMsgLoginRegister);
			paccount = pBuffer ;
			pBuffer += pLoginRegister->pAcoundLen ;
			ppassword = pBuffer ;
			pBuffer += pLoginRegister->ppPasswordLen ;
			pcharaName = pBuffer ;*/

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Super ;
			pRequest->eType = eRequestType_Add ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata ;
			// format sql String ;
			unsigned int nNow = (unsigned int)time(NULL);
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO`account` (`account`, `password`, `registerDate`, `registerType`, `email`, `userUID`, `channel`) \
				VALUES ('%s', '%s', '%u', '%u', '%s','%u','%u');",pLoginRegister->cAccount,pLoginRegister->cPassword,nNow,pLoginRegister->cRegisterType ,pLoginRegister->cEmail,pdata->nExtenArg1,pLoginRegister->nChannel) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case MSG_PLAYER_BIND_ACCOUNT:
		{
			stMsgLoginBindAccount* pMsgRet = (stMsgLoginBindAccount*)pmsg ;
			pdata->nSessionID = pMsgRet->nSessionID ;
			pdata->nExtenArg1 = pMsgRet->nCurUserUID ;
			pdata->pUserData = new char[sizeof(stMsgLoginBindAccount)] ;
			memcpy(pdata->pUserData,pMsgRet,sizeof(stMsgLoginBindAccount));

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Super ;
			pRequest->eType = eRequestType_Update ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE account SET account = '%s', password = '%s',email = '%s',registerType = '%d' WHERE userUID = '%u'",pMsgRet->cAccount,pMsgRet->cPassword,pMsgRet->cEmail,2,pMsgRet->nCurUserUID ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case MSG_MODIFY_PASSWORD:
		{
			stMsgLoginModifyPassword* pMsgRet = (stMsgLoginModifyPassword*)pmsg ;
			pdata->nSessionID = pMsgRet->nSessionID ;
			pdata->pUserData = new char[MAX_LEN_PASSWORD] ;
			memcpy(pdata->pUserData,pMsgRet->cNewPassword,MAX_LEN_PASSWORD);

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Super ;
			pRequest->eType = eRequestType_Update ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE account SET password = '%s' WHERE userUID = '%u' && password = '%s'",pMsgRet->cNewPassword,pMsgRet->nUserUID,pMsgRet->cOldPassword ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case MSG_CREATE_ROLE:
		{
			stMsgGameServerCreateRole* pCreate = (stMsgGameServerCreateRole*)pmsg ;
			pdata->nSessionID = pCreate->nSessionID ;
			pdata->nExtenArg1 = pCreate->nUserUID ;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Super ;
			pRequest->eType = eRequestType_Add ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,
				"INSERT INTO playerbasedata (userUID, defaultPhotoID, sex, playerName) VALUES ('%u', '%u', '%u', '%s');   \
				INSERT INTO masterstudent (userUID) VALUES ('%u');  \
				INSERT INTO playerfriend (userUID) VALUES ('%u');  \
				INSERT INTO playeritems (userUID) VALUES ('%u');  \
				INSERT INTO playermission (userUID) VALUES ('%u');  \
				",pCreate->nUserUID,pCreate->nDefaultPhotoID,pCreate->nSex,pCreate->cName,
				pCreate->nUserUID,
				pCreate->nUserUID,
				pCreate->nUserUID,
				pCreate->nUserUID ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case MSG_PLAYER_BASE_DATA:
		{
			stMsgGameServerGetBaseData* pbasedata = (stMsgGameServerGetBaseData*)pmsg ;
			pdata->nSessionID = pbasedata->nSessionID ;
			pdata->nExtenArg1 = pbasedata->nUserUID ;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_High ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"SELECT * FROM playerbasedata WHERE userUID = '%d'",pbasedata->nUserUID) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case MSG_PLAYER_SAVE_BASE_DATA:
		{
			stMsgGameServerSaveBaseData* pSaveBaseData = (stMsgGameServerSaveBaseData*)pmsg ;
			pdata->nSessionID = pSaveBaseData->nSessionID ;
			pdata->nExtenArg1 = pSaveBaseData->stBaseData.nUserUID ;

			char pMaxCards[sizeof(pSaveBaseData->stBaseData.vMaxCards)*2 + 1 ] = {0} ;
			m_pTheApp->GetDBThread()->EscapeString(pMaxCards,(char*)pSaveBaseData->stBaseData.vMaxCards,sizeof(pSaveBaseData->stBaseData.vMaxCards)) ;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Low ;
			pRequest->eType = eRequestType_Update ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer, 
			"UPDATE playerbasedata SET playerName = '%s', defaultPhotoID = '%u', isUploadPhoto = '%u', signature = '%s', sex = '%u', vipLevel = '%u',   \
			coin = '%I64u', diamond = '%u', winTimes = '%u', loseTimes = '%u', singleWinMost = '%I64u', maxCard = '%s',   \
			longitude = '%f', latitude = '%f', exp = '%u', offlineTime = '%u',noticeID = '%u',vipEndTime = '%u',\
			continueLoginDays = '%u',lastLoginTime = '%u',lastTakeCharityCoinTime = '%u',todayPlayTimes = '%u' , \
			yesterdayPlayTimes = '%u',todayWinCoin = '%I64d',yesterdayWinCoin = '%I64d', \
			takeMasterStudentRewardTime = '%u',rechargeTimes = '%u',curOnlineBoxID = '%u',onlineBoxPassedTime = '%u' WHERE userUID = '%u'", 
			pSaveBaseData->stBaseData.cName,pSaveBaseData->stBaseData.nDefaulPhotoID,pSaveBaseData->stBaseData.bIsUploadPhoto,pSaveBaseData->stBaseData.cSignature,pSaveBaseData->stBaseData.nSex,pSaveBaseData->stBaseData.nVipLevel,
			pSaveBaseData->stBaseData.nCoin,pSaveBaseData->stBaseData.nDiamoned,pSaveBaseData->stBaseData.nWinTimes,pSaveBaseData->stBaseData.nLoseTimes,pSaveBaseData->stBaseData.nSingleWinMost,pMaxCards,
			pSaveBaseData->stBaseData.dfLongitude,pSaveBaseData->stBaseData.dfLatidue,pSaveBaseData->stBaseData.nExp,(unsigned int)pSaveBaseData->stBaseData.tOfflineTime,pSaveBaseData->stBaseData.nNoticeID,pSaveBaseData->stBaseData.nVipEndTime,
			pSaveBaseData->stBaseData.nContinueDays,(unsigned int)pSaveBaseData->stBaseData.tLastLoginTime,(unsigned int)pSaveBaseData->stBaseData.tLastTakeCharityCoinTime,pSaveBaseData->stBaseData.nTodayPlayTimes,
			pSaveBaseData->stBaseData.nYesterdayPlayTimes,pSaveBaseData->stBaseData.nTodayWinCoin,pSaveBaseData->stBaseData.nYesterdayWinCoin,
			(unsigned int)pSaveBaseData->stBaseData.tTakeMasterStudentRewardTime,pSaveBaseData->stBaseData.nRechargeTimes,pSaveBaseData->stBaseData.nCurOnlineBoxID,pSaveBaseData->stBaseData.nOnlineBoxPassedTime,pSaveBaseData->stBaseData.nUserUID) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break ;
	case MSG_SAVE_PLAYER_COIN:
		{
			stMsgGameServerSavePlayerCoin* pSaveCoin = (stMsgGameServerSavePlayerCoin*)pmsg ;
			pdata->nSessionID = pSaveCoin->nSessionID ;
			pdata->nExtenArg1 = pSaveCoin->nUserUID ;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Low ;
			pRequest->eType = eRequestType_Update ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE gamedb.account SET nCoin = '%I64d', nDiamoned = '%d' WHERE UserUID = '%d'",pSaveCoin->nCoin,pSaveCoin->nDiamoned,pSaveCoin->nUserUID) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;  
	case MSG_SAVE_FRIEND_LIST:
		{
			stMsgGameServerSaveFirendList* pSaveFriend = (stMsgGameServerSaveFirendList*)pmsg ;
			pdata->nSessionID = pSaveFriend->nSessionID ;
			pdata->nExtenArg1 = pSaveFriend->nUserUID ;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Low ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			pRequest->eType = eRequestType_Update ;

			char* pBuffer = (char*)pmsg ;
			pBuffer += sizeof(stMsgGameServerSaveFirendList);

			char *pFriendListBuffer = new char[sizeof(stServerSaveFrienItem)*pSaveFriend->nFriendCount*2+1] ;
			memset(pFriendListBuffer,0,sizeof(sizeof(stServerSaveFrienItem)*pSaveFriend->nFriendCount*2+1));
			m_pTheApp->GetDBThread()->EscapeString(pFriendListBuffer,pBuffer,pSaveFriend->nFriendCount * sizeof(stServerSaveFrienItem) ) ;
			unsigned int nSaveTime = (unsigned int)time(NULL) ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE playerfriend SET friendCount = '%u',contentData = '%s',saveTime = '%u' WHERE userUID = '%d'",pSaveFriend->nFriendCount,pFriendListBuffer,nSaveTime,pSaveFriend->nUserUID) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
			delete[]pFriendListBuffer ;
		}
		break;
	case MSG_REQUEST_FRIEND_LIST:
		{
			stMsgGameServerRequestFirendList* pRequestFriend = (stMsgGameServerRequestFirendList*)pmsg ;
			pdata->nSessionID = pRequestFriend->nSessionID ;
			pdata->nExtenArg1 = 0 ;  // more than one time select ; 

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_High ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"SELECT * FROM playerfriend WHERE userUID = '%u'",pRequestFriend->nUserUID ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case MSG_REQUEST_FRIEND_BRIFDATA_LIST:
		{
			stMsgGameServerRequestFriendBrifDataList* pRet = (stMsgGameServerRequestFriendBrifDataList*)pmsg ;
			pdata->nSessionID = pRet->nSessionID ;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_High ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			char* pBffer = (char*)pRet ;
			pBffer += sizeof(stMsgGameServerRequestFriendBrifDataList);
			unsigned int * pUserUID = (unsigned int*)pBffer ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"SELECT %s FROM playerfriend WHERE userUID = '%u'",PLAYER_BRIF_DATA,*pUserUID ) ;
			--pRet->nFriendCount ;
			while ( pRet->nFriendCount--)
			{
				pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"%s || userUID = '%u'",pRequest->pSqlBuffer,*pUserUID ) ;
				++pUserUID ;
			}
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case MSG_PLAYER_SERACH_PEERS:
		{
			stMsgGameServerGetSearchFriendResult* pMsgRet = (stMsgGameServerGetSearchFriendResult*)pmsg ;
			pdata->nSessionID = pMsgRet->nSessionID ;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_High ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			char* pBuffer = new char[pMsgRet->nLen * 2 + 1 ] ;
			memset(pBuffer,0 ,pMsgRet->nLen * 2 + 1  ) ;
			m_pTheApp->GetDBThread()->EscapeString(pBuffer, (char*)pMsgRet + sizeof(stMsgGameServerGetSearchFriendResult),pMsgRet->nLen) ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"SELECT %s FROM playerbasedata where userUID regexp( '%s' ) or playerName regexp('%s') limit 15;", PLAYER_BRIF_DATA,pBuffer, pBuffer ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
			delete[] pBuffer ;
		}
		break;
	case MSG_PLAYER_REQUEST_SEARCH_PEER_DETAIL:
		{
			stMsgGameServerGetSearchedPeerDetail* pMsgRet = (stMsgGameServerGetSearchedPeerDetail*)pmsg ;
			pdata->nSessionID = pMsgRet->nSessionID ;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_High ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"SELECT %s FROM playerbasedata where userUID = %u", PLAYER_DETAIL_DATA,pMsgRet->nPeerUserUID) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case MSG_PLAYER_REQUEST_FRIEND_DETAIL:
		{
			stMsgGameServerGetFriendDetail* pMsgRet = (stMsgGameServerGetFriendDetail*)pmsg ;
			pdata->nSessionID = pMsgRet->nSessionID ;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_High ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"SELECT %s FROM playerbasedata where userUID = %d", PLAYER_DETAIL_DATA,pMsgRet->nFriendUID) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case MSG_PLAYER_GET_MAIL_LIST:
		{
			stMsgGameServerGetMailList* pMsgRet = (stMsgGameServerGetMailList*)pmsg;
			pdata->nSessionID = pMsgRet->nSessionID ;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_High ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"select * from mail where userUID = %u order by postTime desc limit %d",pMsgRet->nUserUID, MAX_KEEP_MAIL_COUNT ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break; 
	case MSG_PLAYER_SAVE_MAIL:
		{
			stMsgGameServerSaveMail* pMsgRet = (stMsgGameServerSaveMail*)pmsg  ;
			pdata->nSessionID = pMsgRet->nSessionID ;
			pdata->nExtenArg1 = pMsgRet->nUserUID ;
			char* pmsgData = (char*)pmsg ;
			pmsgData += sizeof(stMsgGameServerSaveMail);
			stMail* pMailToSave = (stMail*)pmsgData ;
			pdata->nExtenArg2 = (unsigned int)pMailToSave->nMailUID ;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Low ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			switch ( pMsgRet->nOperateType ) 
			{
			case eDBAct_Update:
				{
					pRequest->eType = eRequestType_Update ;
					pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE mail SET processAct = '%d' WHERE mailUID = '%I64d'",pMailToSave->eProcessAct,pMailToSave->nMailUID ) ;
				}
				break;
			case eDBAct_Delete:
				{
					pRequest->eType = eRequestType_Delete ;
					pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"DELETE FROM mail WHERE mailUID ='%I64d' limit 1",pMailToSave->nMailUID ) ;
				}
				break;
			case eDBAct_Add:
				{
					pRequest->eType = eRequestType_Add ;
					char* pContent = new char[pMailToSave->nContentLen * 2 + 1 ] ;
					memset(pContent,0,pMailToSave->nContentLen * 2 + 1 );
					
					char* pBuffer = (char*)pMsgRet;
					pBuffer += sizeof(stMsgGameServerSaveMail);
					pBuffer += sizeof(stMail);
					m_pTheApp->GetDBThread()->EscapeString(pContent,pBuffer,pMailToSave->nContentLen ) ;
					CLogMgr::SharedLogMgr()->PrintLog("mail title content Len = %d",pMailToSave->nContentLen) ;
					pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO mail (mailUID, userUID,postTime,mailType,mailContentLen,mailContent,processAct) VALUES ('%I64d', '%u','%u','%u','%u','%s','%u')",
						pMailToSave->nMailUID,pMsgRet->nUserUID,pMailToSave->nPostTime,pMailToSave->eType,pMailToSave->nContentLen,pContent,pMailToSave->eProcessAct) ;
					delete[] pContent ;
				}
				break;
			default:
				CLogMgr::SharedLogMgr()->ErrorLog("unknown save mail operation type !") ;
				pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"") ;
				break;
			}
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break; 
	case MSG_GAME_SERVER_GET_MAX_MAIL_UID:
		{
			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Normal ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = MSG_GAME_SERVER_GET_MAX_MAIL_UID;
			pRequest->pUserData = pdata;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"(select max(mail.mailUID) FROM mail)") ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case MSG_REQUEST_ITEM_LIST:
		{
			stMsgGameServerRequestItemList* pMsgRet = (stMsgGameServerRequestItemList*)pmsg ;
			pdata->nSessionID = pMsgRet->nSessionID ;
			pdata->nExtenArg1 = pMsgRet->nUserUID ;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_High ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"select * from playeritems where userUID = %u",pMsgRet->nUserUID ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case MSG_SAVE_ITEM_LIST:
		{
			stMsgGameServerSaveItemList* pMsgRet = (stMsgGameServerSaveItemList*)pmsg ;
			pdata->nSessionID = pMsgRet->nSessionID ;
			pdata->nExtenArg1 = pMsgRet->nUserUID ;
			char* pBuffer = new char[pMsgRet->nOwnItemKindCount * sizeof(stPlayerItem) * 2 + 1] ;
			memset(pBuffer,0,pMsgRet->nOwnItemKindCount * sizeof(stPlayerItem) * 2 + 1);
			m_pTheApp->GetDBThread()->EscapeString(pBuffer, (((char*)pmsg) + sizeof(stMsgGameServerSaveItemList)),pMsgRet->nOwnItemKindCount * sizeof(stPlayerItem) ) ;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Low ;
			pRequest->pUserData = pdata;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->eType = eRequestType_Update ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE playeritems SET itemsData = '%s',ownItemsKindCount = '%u' WHERE userUID = '%d'",pBuffer,pMsgRet->nOwnItemKindCount,pMsgRet->nUserUID ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
			delete[] pBuffer ;
		}
		break; 
	case MSG_REQUEST_RANK:
		{
			stMsgGameServerRequestRank* pMsgRet = (stMsgGameServerRequestRank*)pmsg ;
			pdata->nExtenArg1 = pMsgRet->eType ;
			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Normal ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			switch ( pMsgRet->eType)
			{
			case eRank_AllCoin:
				{
					pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"select yesterdayWinCoin ,%s from playerbasedata where userUID !=0 order by coin desc limit %d",PLAYER_DETAIL_DATA,RANK_SHOW_PEER_COUNT) ;
				}
				break;
			case eRank_YesterDayWin:
				{
					unsigned int tYesterDay = (unsigned int)time(NULL) - 24 * 3600 ;
					//struct tm tmNow = *localtime(&tNow) ;
					//tmNow.tm_hour = 0 ;
					//tmNow.tm_min = 0 ;
					//tmNow.tm_sec = 0 ;
					//time_t tZero = mktime(&tmNow) ;
					pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"select yesterdayWinCoin ,%s from playerbasedata where (offlineTime >= '%u' || lastLoginTime >= '%u') order by todayWinCoin desc limit %d",PLAYER_DETAIL_DATA,tYesterDay,tYesterDay,RANK_SHOW_PEER_COUNT) ;
				}
				break;
			case eRank_SingleWinMost:
				{
					pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"select yesterdayWinCoin ,%s from playerbasedata where userUID !=0 order by singleWinMost desc limit %d",PLAYER_DETAIL_DATA,RANK_SHOW_PEER_COUNT) ;
				}
				break;
			default:
				CLogMgr::SharedLogMgr()->ErrorLog("unknown rank type to select type = %d",pMsgRet->eType) ;
				break;
			}
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	//case MSG_GET_SHOP_BUY_RECORD:
	//	{
	//		stMsgGameServerGetShopBuyRecord* pMsgRet = (stMsgGameServerGetShopBuyRecord*)pmsg ;
	//		pdata->nSessionID = pMsgRet->nSessionID ;
	//		pdata->nExtenArg1 = pMsgRet->nUserUID ;

	//		stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
	//		pRequest->cOrder = eReq_Order_High ;
	//		pRequest->eType = eRequestType_Select ;
	//		pRequest->nRequestUID = pmsg->usMsgType ;
	//		pRequest->pUserData = pdata;
	//		pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"select * from playershopbuyrecord where nUserUID = %d",pMsgRet->nUserUID ) ;
	//		CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
	//	}
	//	break;
	//case MSG_SAVE_SHOP_BUY_RECORD:
	//	{
	//		stMsgGameServerSaveShopBuyRecord* pMsgRet = (stMsgGameServerSaveShopBuyRecord*)pmsg;
	//		pdata->nSessionID = pMsgRet->nSessionID ;
	//		pdata->nExtenArg1 = pMsgRet->nUserUID ;

	//		stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
	//		pRequest->cOrder = eReq_Order_Low ;
	//		pRequest->nRequestUID = pmsg->usMsgType ;
	//		pRequest->pUserData = pdata;

	//		char* pSaveBuffer = new char[pMsgRet->nBufferLen * 2 + 1 ] ;
	//		memset(pSaveBuffer,0,pMsgRet->nBufferLen * 2 + 1) ;
	//		char* pBuffer = (char*)pmsg ;
	//		pBuffer += sizeof(stMsgGameServerSaveShopBuyRecord);
	//		m_pTheApp->GetDBThread()->EscapeString(pSaveBuffer,pBuffer,pMsgRet->nBufferLen ) ;
	//		if ( pMsgRet->bAdd )
	//		{
	//			pRequest->eType = eRequestType_Add ;
	//			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO `gamedb`.`playershopbuyrecord` (`nUserUID`, `pBuffer`) VALUES ('%d', '%s');",pMsgRet->nUserUID,pSaveBuffer) ;
	//		}
	//		else
	//		{
	//			pRequest->eType = eRequestType_Update ;
	//			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE gamedb.playershopbuyrecord SET pBuffer = '%s' WHERE nUserUID = '%d'",pSaveBuffer,pMsgRet->nUserUID) ;
	//		}
	//		CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
	//		delete[] pSaveBuffer ;
	//	}
	//	break;
	case MSG_GAME_SERVER_SAVE_MISSION_DATA:
		{
			stMsgGameServerSaveMissionData* pMissionData = (stMsgGameServerSaveMissionData*)pmsg ;
			pdata->nSessionID = pMissionData->nSessionID ;
			pdata->nExtenArg1 = pMissionData->nUserUID ;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Low ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;

			char* pSaveBuffer = new char[pMissionData->nMissonCount * sizeof(stMissionSate) * 2 + 1 ] ;
			memset(pSaveBuffer,0,pMissionData->nMissonCount * sizeof(stMissionSate) * 2 + 1) ;
			char* pBuffer = (char*)pmsg ;
			pBuffer += sizeof(stMsgGameServerSaveMissionData);
			m_pTheApp->GetDBThread()->EscapeString(pSaveBuffer,pBuffer,pMissionData->nMissonCount * sizeof(stMissionSate)) ;
			pRequest->eType = eRequestType_Update ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE playermission SET missionData = '%s',saveTime = '%u',missionCount = '%u' WHERE userUID = '%d'",pSaveBuffer,pMissionData->nSavetime,pMissionData->nMissonCount,pMissionData->nUserUID) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
			delete[] pSaveBuffer ;
		}
		break;
	case MSG_GAME_SERVER_GET_MISSION_DATA:
		{
			stMsgGameServerGetMissionData* pMsgRet = (stMsgGameServerGetMissionData*)pmsg ;
			pdata->nSessionID = pMsgRet->nSessionID ;
			pdata->nExtenArg1 = pMsgRet->nUserUID ;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_High ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"select * from playermission where userUID = %d",pMsgRet->nUserUID ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	default:
		{
			m_vReserverArgData.push_back(pdata) ;
			CLogMgr::SharedLogMgr()->ErrorLog("unknown msg type = %d",pmsg->usMsgType ) ;
		}
	}
}

void CDBManager::OnDBResult(stDBResult* pResult)
{
	// process result 
	if ( pResult->nRequestUID == (unsigned int )-1 )   // get current max curUID ;
	{
		if ( pResult->nAffectRow > 0 )
		{
			CMysqlRow& pRow = *pResult->vResultRows.front(); 
			unsigned int nMaxID = pRow["max(Account.UserUID)"]->IntValue();
			if ( nMaxID >= nCurUserUID )
			{
				nCurUserUID = ++nMaxID ;
				CLogMgr::SharedLogMgr()->SystemLog("curMaxUID is %d",nMaxID ) ;
			}
		}
		return;
	}

	stArgData*pdata = (stArgData*)pResult->pUserData ;
	switch ( pResult->nRequestUID )
	{
	case  MSG_PLAYER_REGISTER:
		{
			stMsgLoginRegister* pRegMsg = (stMsgLoginRegister*)pdata->pUserData ;
			stMsgLoginRegisterRet msgRet ;
			msgRet.nRet = pResult->nAffectRow > 0 ? 0 : 1 ;
			msgRet.nSessionID = pdata->nSessionID;
			msgRet.nUserID = pdata->nExtenArg1;
			memcpy(msgRet.cAccount,pRegMsg->cAccount,sizeof(msgRet.cAccount));
			memcpy(msgRet.cPassword,pRegMsg->cPassword,sizeof(msgRet.cPassword));
			msgRet.cRegisterType = pRegMsg->cRegisterType ;
			m_pTheApp->SendMsg((char*)&msgRet,sizeof(msgRet),pdata->m_nReqrestFromAdd) ;
			delete[] pdata->pUserData ;
			pdata->pUserData = NULL ;
		}
		break;
	case MSG_PLAYER_CHECK_ACCOUNT:
		{
			stMsgLoginAccountCheckRet msgRet ;
			msgRet.nSessionID = pdata->nSessionID ;
			if ( pResult->nAffectRow > 0 )
			{
				CMysqlRow& pRow = *pResult->vResultRows.front() ;
				// check password 
				if ( strcmp((char*)pdata->pUserData,pRow["password"]->BufferData()) == 0 )
				{
					msgRet.cRegisterType = pRow["registerType"]->IntValue();
					if ( pRow["state"]->IntValue() != 0 )
					{
						msgRet.nRet = 3 ;
						CLogMgr::SharedLogMgr()->PrintLog("account sate error") ;
					}
					else
					{
						msgRet.nRet = 0 ;
						msgRet.nUserID = pRow["userUID"]->IntValue() ;
						CLogMgr::SharedLogMgr()->PrintLog("check account success") ;
					}
				}
				else
				{
					msgRet.nRet = 2 ;  // password error ;
					msgRet.nUserID = 0 ;
					CLogMgr::SharedLogMgr()->PrintLog("check account password error") ;
				}
			}
			else
			{
				msgRet.nRet = 1 ;  // account error ;   
				msgRet.nUserID = 0 ;
				CLogMgr::SharedLogMgr()->PrintLog("check account  account error") ;
			}
			m_pTheApp->SendMsg((char*)&msgRet,sizeof(msgRet),pdata->m_nReqrestFromAdd) ;
			delete [] pdata->pUserData ;
			pdata->pUserData = NULL ;
		}
		break;
	case MSG_PLAYER_BIND_ACCOUNT:
		{
			stMsgLoginBindAccount*pBind = (stMsgLoginBindAccount*)pdata->pUserData ;
			stMsgLoginBindAccountRet msgBack ;
			msgBack.nCurUserUID = pBind->nCurUserUID;
			msgBack.nSessionID = pdata->nSessionID ;
			msgBack.nRet = 0 ;
			memcpy(msgBack.cAccount,pBind->cAccount,sizeof(msgBack.cAccount));
			memcpy(msgBack.cPassword,pBind->cPassword,sizeof(msgBack.cPassword));
			memcpy(msgBack.cEmail,pBind->cEmail,sizeof(msgBack.cEmail));
			if ( pResult->nAffectRow <= 0 )
			{
				msgBack.nRet = 1 ;
			}
			m_pTheApp->SendMsg((char*)&msgBack,sizeof(msgBack) ,pdata->m_nReqrestFromAdd); 
			delete[] pdata->pUserData;
			pdata->pUserData = NULL ;
		}
		break;
	case MSG_MODIFY_PASSWORD:
		{
			char* pNewPassword = (char*)pdata->pUserData;
			stMsgLoginModifyPasswordRet msgBack ;
			msgBack.nSessionID = pdata->nSessionID ;
			msgBack.nRet = pResult->nAffectRow > 0 ? 0 : 1 ;
			memcpy(msgBack.cNewPassword,pNewPassword,MAX_LEN_PASSWORD);
			delete[] pdata->pUserData ;
			m_pTheApp->SendMsg((char*)&msgBack,sizeof(msgBack) ,pdata->m_nReqrestFromAdd); 
		}
		break;
	case MSG_CREATE_ROLE:
		{
			stMsgGameServerCreateRoleRet msgBack ;
			msgBack.nSessionID = pdata->nSessionID ;
			msgBack.nRet = pResult->nAffectRow > 0 ? 0 : 1 ;
			msgBack.nUserUID = pdata->nExtenArg1;
			m_pTheApp->SendMsg((char*)&msgBack,sizeof(msgBack) ,pdata->m_nReqrestFromAdd); 
		}
		break;
	case MSG_PLAYER_BASE_DATA:
		{
			stArgData* pdata = (stArgData*)pResult->pUserData ;
			stMsgGameServerGetBaseDataRet msg ;
			msg.nSessionID = pdata->nSessionID ;
			memset(&msg.stBaseData,0,sizeof(msg.stBaseData)) ;
			msg.nRet = 0 ;
			if ( pResult->nAffectRow <= 0 )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("can not find base data with userUID = %d , session id = %d " , pdata->nExtenArg1,msg.nSessionID ) ;
				msg.nRet = 1 ;
				m_pTheApp->SendMsg((char*)&msg,sizeof(msg),pdata->m_nReqrestFromAdd) ;
			}
			else
			{
				CMysqlRow& pRow = *pResult->vResultRows.front(); 
				if ( pRow["playerName"]->nBufferLen > 0 )
				{
					memcpy(msg.stBaseData.cName,pRow["playerName"]->BufferData(),pRow["playerName"]->nBufferLen);
				}
				msg.stBaseData.nDefaulPhotoID = pRow["defaultPhotoID"]->IntValue();
				msg.stBaseData.bIsUploadPhoto = pRow["isUploadPhoto"]->IntValue();
				if ( pRow["signature"]->nBufferLen > 0 )
				{
					memcpy(msg.stBaseData.cSignature,pRow["signature"]->BufferData(),pRow["signature"]->nBufferLen);
				}
				msg.stBaseData.nSex = pRow["sex"]->IntValue();
				msg.stBaseData.nVipLevel = pRow["vipLevel"]->IntValue();
				msg.stBaseData.nCoin = pRow["coin"]->IntValue64();
				msg.stBaseData.nDiamoned = pRow["diamond"]->IntValue();
				msg.stBaseData.nWinTimes = pRow["winTimes"]->IntValue();
				msg.stBaseData.nLoseTimes = pRow["loseTimes"]->IntValue();
				msg.stBaseData.nSingleWinMost = pRow["singleWinMost"]->IntValue64();
				if ( pRow["maxCard"]->nBufferLen > 0 )
				{
					memcpy(msg.stBaseData.vMaxCards,pRow["maxCard"]->BufferData(),pRow["maxCard"]->nBufferLen);
				}
				msg.stBaseData.dfLongitude = pRow["longitude"]->FloatValue();
				msg.stBaseData.dfLatidue = pRow["latitude"]->FloatValue();
				msg.stBaseData.nExp = pRow["exp"]->IntValue();
				msg.stBaseData.tOfflineTime = pRow["offlineTime"]->IntValue();
				msg.stBaseData.nNoticeID = pRow["noticeID"]->IntValue();
				msg.stBaseData.nVipEndTime = pRow["vipEndTime"]->IntValue();
				msg.stBaseData.nContinueDays = pRow["continueLoginDays"]->IntValue();
				msg.stBaseData.tLastLoginTime = pRow["lastLoginTime"]->IntValue();
				msg.stBaseData.tLastTakeCharityCoinTime = pRow["lastTakeCharityCoinTime"]->IntValue();
				msg.stBaseData.nTodayPlayTimes = pRow["todayPlayTimes"]->IntValue();
				msg.stBaseData.nYesterdayPlayTimes = pRow["yesterdayPlayTimes"]->IntValue();
				msg.stBaseData.tLastTakeCharityCoinTime = pRow["takeMasterStudentRewardTime"]->IntValue();
				msg.stBaseData.nYesterdayWinCoin = pRow["yesterdayWinCoin"]->IntValue64();
				msg.stBaseData.nTodayWinCoin = pRow["todayWinCoin"]->IntValue64();
				msg.stBaseData.nCurOnlineBoxID = pRow["curOnlineBoxID"]->IntValue();
				msg.stBaseData.nOnlineBoxPassedTime = pRow["onlineBoxPassedTime"]->IntValue();
				msg.stBaseData.nUserUID = pRow["userUID"]->IntValue();
				// process today and yesterday things 
				if ( msg.stBaseData.tOfflineTime != 0 )
				{
					time_t  tCur = time(NULL) ;
					struct tm tmNow = *localtime(&tCur);
					struct tm tmLastLogin = *localtime((time_t*)&msg.stBaseData.tOfflineTime);
					if ( tmNow.tm_yday == tmLastLogin.tm_yday )
					{
						// the same day , do nothing ;
					}
					else if( tmNow.tm_yday == tmLastLogin.tm_yday + 1 )
					{
						msg.stBaseData.nYesterdayWinCoin = msg.stBaseData.nTodayWinCoin;
						msg.stBaseData.nTodayWinCoin = 0 ;

						msg.stBaseData.nYesterdayPlayTimes = msg.stBaseData.nTodayPlayTimes;
						msg.stBaseData.nTodayPlayTimes = 0 ;
					}
					else
					{
						msg.stBaseData.nYesterdayWinCoin = 0;
						msg.stBaseData.nTodayWinCoin = 0 ;

						msg.stBaseData.nYesterdayPlayTimes = 0;
						msg.stBaseData.nTodayPlayTimes = 0 ;
					}
				}
				m_pTheApp->SendMsg((char*)&msg,sizeof(msg),pdata->m_nReqrestFromAdd) ;
			}
		}
		break;
	case MSG_PLAYER_SAVE_BASE_DATA:
		{
			if ( pResult->nAffectRow > 0 )
			{
				CLogMgr::SharedLogMgr()->PrintLog("save player base data ok UID = %d",pdata->nExtenArg1) ;
			}
			else
			{
				CLogMgr::SharedLogMgr()->PrintLog("save player base data Error UID = %d",pdata->nExtenArg1) ;
			}
		}
		break;
	case MSG_SAVE_PLAYER_COIN:
		{
			stMsgGameServerSavePlayerCoinRet ret ;
			ret.nSessionID = pdata->nSessionID ;
			if ( pResult->nAffectRow > 0 )
			{
				ret.nRet = 0 ;
			}
			else
			{
				ret.nRet = 1 ;
				CLogMgr::SharedLogMgr()->PrintLog("Save player COIN Error ! UID = %d",pdata->nExtenArg1) ;
			}
			m_pTheApp->SendMsg((char*)&ret,sizeof(ret),pdata->m_nReqrestFromAdd); 
		}
		break;
	case MSG_SAVE_FRIEND_LIST:
		{
			stMsgGameServerSaveFriendListRet ret ;
			ret.nSessionID = pdata->nSessionID ;
			if ( pResult->nAffectRow > 0 )
			{
				ret.nRet = 0 ;
			}
			else
			{
				ret.nRet = 1 ;
				CLogMgr::SharedLogMgr()->PrintLog("Save player MSG_SAVE_FRIEND_LIST ! UID = %d",pdata->nExtenArg1) ;
			}
			m_pTheApp->SendMsg((char*)&ret,sizeof(ret),pdata->m_nReqrestFromAdd); 
		}
		break; 
	case MSG_REQUEST_FRIEND_LIST:
		{
			stMsgGameServerRequestFirendListRet msgBack ;
			msgBack.nSessionID = pdata->nSessionID ; 
			if ( pResult->nAffectRow > 0 )
			{
				CMysqlRow& pRow = *pResult->vResultRows[0] ;
				msgBack.nFriendCount = pRow["friendCount"]->IntValue();
				if ( msgBack.nFriendCount == 0 )
				{
					m_pTheApp->SendMsg((char*)&msgBack,sizeof(msgBack),pdata->m_nReqrestFromAdd); 
					break;
				}

				// parse friends ;
				char* pBuffer = new char[msgBack.nFriendCount * sizeof(stServerSaveFrienItem) + sizeof(msgBack)] ;
				unsigned char nOffset = 0 ;
				memcpy(pBuffer,&msgBack,sizeof(msgBack));
				nOffset += sizeof(msgBack);
				memcpy(pBuffer + nOffset , pRow["contentData"]->BufferData(),pRow["contentData"]->nBufferLen);
				nOffset += pRow["contentData"]->nBufferLen ;
#ifdef DEBUG
				if ( pRow["contentData"]->nBufferLen != msgBack.nFriendCount * sizeof(stServerSaveFrienItem) )
				{
					CLogMgr::SharedLogMgr()->ErrorLog("why save buffer and read buffer is not equal len , read friend list ?") ;
				}
#endif
				// update present times info ;
				time_t tNow = time(NULL) ;
				time_t nLastSave = pRow["saveTime"]->IntValue();
				struct tm tmNow = *localtime(&tNow) ;
				struct tm tmLastSave = *localtime(&nLastSave) ;
				if ( tmNow.tm_yday != tmLastSave.tm_yday )
				{
					// reset present times ;
					char* pDBuffer = pBuffer + sizeof(msgBack);
					stServerSaveFrienItem* pSave = (stServerSaveFrienItem*)pDBuffer;
					int nCount = msgBack.nFriendCount ;
					while(nCount--)
					{
						pSave->nPresentTimes = 0 ;
						++pSave ;
					}
				}
				m_pTheApp->SendMsg(pBuffer,nOffset,pdata->m_nReqrestFromAdd); 
				delete[] pBuffer ;
			}
			else
			{
				msgBack.nFriendCount = 0 ;
				m_pTheApp->SendMsg((char*)&msgBack,sizeof(msgBack),pdata->m_nReqrestFromAdd); 
				break;
			}
		}
		break;
	case MSG_REQUEST_FRIEND_BRIFDATA_LIST:
		{
			stMsgGameServerRequestFriendBrifDataListRet msgRet ;
			msgRet.nCount = pResult->nAffectRow ;
			if ( msgRet.nCount <= 0 )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("How can fried brif info list is NULL ?") ;
				m_pTheApp->SendMsg((char*)&msgRet,sizeof(msgRet),pdata->m_nReqrestFromAdd); 
				break;
			}

			char* pBuffer = new char[sizeof(msgRet) + sizeof(stPlayerBrifData) * msgRet.nCount] ;
			memcpy(pBuffer,&msgRet,sizeof(msgRet));
			stPlayerBrifData* pInfo = (stPlayerBrifData*)(pBuffer + sizeof(msgRet));
			for ( int i = 0 ; i < pResult->nAffectRow ; ++i )
			{
				CMysqlRow& pRow = *pResult->vResultRows[i];
				GetPlayerBrifData(pInfo,pRow) ;
				++pInfo ;
			}
			m_pTheApp->SendMsg(pBuffer,sizeof(msgRet) + sizeof(stPlayerBrifData) * msgRet.nCount ,pdata->m_nReqrestFromAdd); 
			delete[] pBuffer ;
		}
		break;
	case MSG_PLAYER_SERACH_PEERS:
		{
			stMsgGameServerGetSearchFriendResultRet msgBack ;
			msgBack.nSessionID = pdata->nSessionID ;
			msgBack.nResultCount = pResult->nAffectRow ;
			if ( msgBack.nResultCount <= 0 )
			{
				m_pTheApp->SendMsg((char*)&msgBack,sizeof(msgBack) ,pdata->m_nReqrestFromAdd); 
				break;
			}

			char* pBuffer = new char[sizeof(msgBack) + sizeof(stPlayerBrifData) * msgBack.nResultCount] ;
			memcpy(pBuffer,&msgBack,sizeof(msgBack));
			stPlayerBrifData* pInfo = (stPlayerBrifData*)(pBuffer + sizeof(msgBack));
			for ( int i = 0 ; i < pResult->nAffectRow ; ++i )
			{
				CMysqlRow& pRow = *pResult->vResultRows[i];
				GetPlayerBrifData(pInfo,pRow) ;
				++pInfo ;
			}
			m_pTheApp->SendMsg(pBuffer,sizeof(msgBack) + sizeof(stPlayerBrifData) * msgBack.nResultCount ,pdata->m_nReqrestFromAdd); 
			delete[] pBuffer ;
		}
		break;
	case MSG_PLAYER_REQUEST_FRIEND_DETAIL:
		{
			stMsgGameServerGetFriendDetailRet msg ;
			msg.nSessionID = pdata->nSessionID ;
			if ( pResult->nAffectRow <= 0 )
			{
				msg.nRet = 1 ;
			}
			else
			{
				msg.nRet = 0 ;
				CMysqlRow& pRow = *pResult->vResultRows[0];
				GetPlayerDetailData(&msg.stPeerInfo,pRow) ;
			}
			m_pTheApp->SendMsg((char*)&msg,sizeof(msg),pdata->m_nReqrestFromAdd); 
		}
		break;
	case MSG_PLAYER_REQUEST_SEARCH_PEER_DETAIL:
		{
			stMsgGameServerGetSearchedPeerDetailRet msg ;
			msg.nSessionID = pdata->nSessionID ;
			if ( pResult->nAffectRow <= 0 )
			{
				msg.nRet = 1 ;
			}
			else
			{
				msg.nRet = 0 ;
				CMysqlRow& pRow = *pResult->vResultRows[0];
				GetPlayerDetailData(&msg.stPeerInfo,pRow) ;
			}
			m_pTheApp->SendMsg((char*)&msg,sizeof(msg),pdata->m_nReqrestFromAdd); 
		}
		break;
	case MSG_PLAYER_SAVE_MAIL:
		{
			if ( pResult->nAffectRow > 0 )
			{
				CLogMgr::SharedLogMgr()->PrintLog("Save Mail Success") ;
			}
			else
			{
				CLogMgr::SharedLogMgr()->ErrorLog("Save Mail Failed, UserID = %d,MainID = %d",pdata->nExtenArg1,pdata->nExtenArg2) ;
			}

		}
		break;
	case MSG_PLAYER_GET_MAIL_LIST:
		{
			stMsgGameServerGetMailListRet pSendMsg ;
			pSendMsg.nSessionID = pdata->nSessionID ;
			pSendMsg.nMailCount = pResult->nAffectRow ;
			std::vector<stMail*> VecMail ;
			unsigned short nToTalLen = sizeof(pSendMsg);
			for ( unsigned int i = 0 ; i < pResult->nAffectRow ; ++i )
			{
				CMysqlRow& pRow = *pResult->vResultRows[i];
				stMail* mail = new stMail;
				mail->eProcessAct = (bool)pRow["processAct"]->IntValue();
				mail->eType = (eMailType)pRow["mailType"]->IntValue();
				mail->nContentLen = pRow["mailContent"]->nBufferLen;
				mail->nMailUID = pRow["mailUID"]->IntValue64();
				mail->nPostTime = pRow["postTime"]->IntValue() ;

				if ( 0 == mail->nContentLen )
				{
					mail->pContent = NULL;
				}
				else
				{
					mail->pContent = new char[mail->nContentLen]; 
					memcpy(mail->pContent,pRow["mailContent"]->BufferData(),mail->nContentLen);
				}

				VecMail.push_back(mail) ;
				nToTalLen += sizeof(stMail);
				nToTalLen += mail->nContentLen ;
			}

			char* pBuffer = new char[nToTalLen] ;
			unsigned short nOffset = 0 ;
			memcpy(pBuffer,&pSendMsg,sizeof(pSendMsg));
			nOffset += sizeof(pSendMsg);
			for ( unsigned int i = 0 ; i < VecMail.size() ; ++i )
			{
				stMail* sMail = VecMail[i] ;
				memcpy(pBuffer + nOffset , sMail,sizeof(stMail));
				nOffset += sizeof(stMail);

				if ( sMail->nContentLen > 0 )
				{
					memcpy(pBuffer + nOffset , sMail->pContent,sMail->nContentLen);
					nOffset += sMail->nContentLen ;
					delete[] sMail->pContent ;
				}

				delete sMail ;
			}
			m_pTheApp->SendMsg(pBuffer,nOffset,pdata->m_nReqrestFromAdd); 
			delete[] pBuffer ;
		}
		break;
	case MSG_GAME_SERVER_GET_MAX_MAIL_UID:
		{
			stMsgGameServerGetMaxMailUIDRet msg ;
			if ( pResult->nAffectRow == 0 )
			{
				msg.nMaxMailUID = 10 ;
			}
			else
			{
				msg.nMaxMailUID = pResult->vResultRows[0]->GetFiledByName("max(mail.mailUID)")->IntValue();
			}
			m_pTheApp->SendMsg((char*)&msg,sizeof(msg),pdata->m_nReqrestFromAdd); 
			CLogMgr::SharedLogMgr()->SystemLog("Cur Max MailUID = %I64d",msg.nMaxMailUID) ;
		}
		break;
	case MSG_REQUEST_ITEM_LIST:
		{
			stMsgGameServerRequestItemListRet msg;
			msg.nSessionID = pdata->nSessionID;
			if ( pResult->nAffectRow <= 0 )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("why have no item recorder ? must inster when create player ") ;
				msg.nOwnItemKindCount = 0 ;
				m_pTheApp->SendMsg((char*)&msg,sizeof(msg),pdata->m_nReqrestFromAdd); 
				break;
			}
			else
			{
				CMysqlRow& pRow = *pResult->vResultRows[0] ;
				msg.nOwnItemKindCount = pRow["ownItemsKindCount"]->IntValue();
				char* pBuffer = new char[sizeof(msg) + pRow["itemsData"]->nBufferLen] ;
				memcpy(pBuffer,&msg,sizeof(msg));
				unsigned short nOffset = sizeof(msg);
				memcpy(pBuffer + nOffset ,pRow["itemsData"]->BufferData(), pRow["itemsData"]->nBufferLen );
				nOffset += pRow["itemsData"]->nBufferLen;
				m_pTheApp->SendMsg(pBuffer,nOffset,pdata->m_nReqrestFromAdd); 
				delete[] pBuffer;
			}
		}
		break; 
	case MSG_SAVE_ITEM_LIST:
		{
			if ( pResult->nAffectRow > 0 )
			{
				CLogMgr::SharedLogMgr()->PrintLog("Save Item List successed UID = %d",pdata->nExtenArg1) ;
			}
			else
			{
				CLogMgr::SharedLogMgr()->PrintLog("Save Item List Failed UID = %d",pdata->nExtenArg1) ;
			}
		}
		break; 
	case MSG_REQUEST_RANK:
		{
			stMsgGameServerRequestRankRet msg ;
			msg.eType = pdata->nExtenArg1 ;
			msg.nPeerCount = pResult->vResultRows.size();
			if ( msg.nPeerCount <= 0 )
			{
				m_pTheApp->SendMsg((char*)&msg,sizeof(msg),pdata->m_nReqrestFromAdd); 
				break;
			}

			char* pBuffer = new char[sizeof(msg) + msg.nPeerCount * sizeof(stServerGetRankPeerInfo)];
			char* pBufferTemp = NULL ;
			memcpy(pBuffer,&msg,sizeof(msg));
			pBufferTemp = pBuffer + sizeof(msg);
			stServerGetRankPeerInfo* pInfo = (stServerGetRankPeerInfo*)pBufferTemp ;
			for ( int i = 0 ; i < msg.nPeerCount ; ++i )
			{
				CMysqlRow& pRow = *pResult->vResultRows[i] ;
				GetPlayerDetailData(&pInfo->tDetailData,pRow);
				pInfo->nYesterDayWin = pRow["yesterdayWinCoin"]->IntValue64() ;
				++pInfo;
			}
			m_pTheApp->SendMsg(pBuffer,sizeof(msg) + msg.nPeerCount * sizeof(stServerGetRankPeerInfo) ,pdata->m_nReqrestFromAdd); 
			delete[] pBuffer ;
		}
		break ;
	//case MSG_GET_SHOP_BUY_RECORD:
	//	{
	//		stMsgGameServerGetShopBuyRecordRet msg ;
	//		msg.nSessionID = pdata->nSessionID ;
	//		if ( pResult->nAffectRow == 0 )
	//		{
	//			msg.nBufferLen = 0 ;
	//			m_pTheApp->SendMsg((char*)&msg,sizeof(msg) ,pdata->m_nReqrestFromAdd); 
	//		}
	//		else
	//		{
	//			msg.nBufferLen = pResult->vResultRows[0]->GetFiledByName("pBuffer")->nBufferLen ;
	//			char* pBuffer = new char[sizeof(msg) + msg.nBufferLen ] ;
	//			memcpy(pBuffer,&msg,sizeof(msg));
	//			memcpy(pBuffer + sizeof(msg),pResult->vResultRows[0]->GetFiledByName("pBuffer")->BufferData(),msg.nBufferLen );
	//			m_pTheApp->SendMsg(pBuffer,sizeof(msg) + msg.nBufferLen ,pdata->m_nReqrestFromAdd); 
	//			delete[] pBuffer ;
	//		}
	//	}
	//	break;
	//case MSG_SAVE_SHOP_BUY_RECORD:
	//	{
	//		if ( pResult->nAffectRow > 0 )
	//		{
	//			CLogMgr::SharedLogMgr()->PrintLog("Save SHOP_BUY_RECORD successed UID = %d",pdata->nExtenArg1) ;
	//		}
	//		else
	//		{
	//			CLogMgr::SharedLogMgr()->PrintLog("Save SHOP_BUY_RECORD Failed UID = %d",pdata->nExtenArg1) ;
	//		}
	//	}
	//	break;
	case MSG_GAME_SERVER_SAVE_MISSION_DATA:
		{
			if ( pResult->nAffectRow > 0 )
			{
				CLogMgr::SharedLogMgr()->PrintLog("Save MSG_GAME_SERVER_SAVE_MISSION_DATA successed UID = %d",pdata->nExtenArg1) ;
			}
			else
			{
				CLogMgr::SharedLogMgr()->PrintLog("Save MSG_GAME_SERVER_SAVE_MISSION_DATA Failed UID = %d",pdata->nExtenArg1) ;
			}
		}
		break;
	case MSG_GAME_SERVER_GET_MISSION_DATA:
		{
			stMsgGameServerGetMissionDataRet msg ;
			msg.nSessionID = pdata->nSessionID ;
			if ( pResult->nAffectRow <= 0 )
			{
				msg.nMissonCount = 0 ;
				msg.nLastSaveTime = 0 ;
				m_pTheApp->SendMsg((char*)&msg,sizeof(msg) ,pdata->m_nReqrestFromAdd); 
				CLogMgr::SharedLogMgr()->ErrorLog("Mission recorder can not be null , inster one when create player ") ;
			}
			else
			{
				CMysqlRow& pRow = *pResult->vResultRows[0];
				msg.nMissonCount = pRow["missionCount"]->IntValue() ;
				msg.nLastSaveTime = pRow["saveTime"]->IntValue(); 
				char* pBuffer = new char[sizeof(msg) + pRow["missionData"]->nBufferLen ] ;
				memcpy(pBuffer,&msg,sizeof(msg));
				memcpy(pBuffer + sizeof(msg),pRow["missionData"]->BufferData(),pRow["missionData"]->nBufferLen );
				m_pTheApp->SendMsg(pBuffer,sizeof(msg) + pRow["missionData"]->nBufferLen ,pdata->m_nReqrestFromAdd); 
				delete[] pBuffer ;
			}
		}
		break;
	default:
		{
			CLogMgr::SharedLogMgr()->ErrorLog("unprocessed db result msg id = %d ", pResult->nRequestUID );
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
	memcpy(pData->cSignature,prow["signature"]->BufferData(),prow["signature"]->nBufferLen);
	pData->dfLatidue = prow["latitude"]->FloatValue();
	pData->dfLongitude = prow["longitude"]->FloatValue();
	pData->nLoseTimes = prow["loseTimes"]->IntValue();
	pData->nSingleWinMost = prow["singleWinMost"]->IntValue64();
	pData->nWinTimes = prow["winTimes"]->IntValue();
	pData->nYesterDayPlayTimes = prow["yesterdayPlayTimes"]->IntValue();
	time_t tLastOffline = prow["offlineTime"]->IntValue();
	time_t tNow = time(NULL) ;
	struct tm tLast = *localtime(&tLastOffline) ;
	struct tm tNowt = *localtime(&tNow) ;
	if ( tLast.tm_yday == tNowt.tm_yday - 1 )  // yesterday offline ;
	{
		pData->nYesterDayPlayTimes = prow["todayPlayTimes"]->IntValue();
	}
	else if ( tLast.tm_yday < tNowt.tm_yday -1 )
	{
		pData->nYesterDayPlayTimes = 0 ;
	}
}

void CDBManager::GetPlayerBrifData(stPlayerBrifData*pData,CMysqlRow&prow)
{
	pData->bIsOnLine = false ;
	pData->bIsUploadPhoto = prow["isUploadPhoto"]->IntValue();
	memset(pData->cName,0,sizeof(pData->cName)) ;
	memcpy(pData->cName,prow["playerName"]->BufferData(),prow["playerName"]->nBufferLen);
	pData->nCoin = prow["coin"]->IntValue64();
	pData->nDefaultPhotoID = prow["defaultPhotoID"]->IntValue();
	pData->nDiamoned = prow["diamond"]->IntValue();
	pData->nExp = prow["exp"]->IntValue();
	pData->nSex = prow["sex"]->IntValue();
	pData->nUserUID = prow["userUID"]->IntValue();
	pData->nVipLevel = prow["vipLevel"]->IntValue();
}