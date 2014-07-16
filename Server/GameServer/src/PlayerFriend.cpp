#include "PlayerFriend.h"
#include "MessageDefine.h"
#include "LogManager.h"
#include "GameServerApp.h"
#include "PlayerManager.h"
#include "Player.h"
#include "PlayerBaseData.h"
#include <time.h>
#include "ServerMessageDefine.h"
#include "PlayerEvent.h"
#include "PlayerMail.h"
#include "EventCenter.h"
void stFriendInfo::OnFriendOffline(CPlayer* pPlayerOffline)
{
	if (! pPlayer  || pPlayer->GetSessionID() > pPlayerOffline->GetSessionID() )
	{
		return ;
	}

	if ( pDetail == NULL )
	{
		pDetail = new stPlayerDetailData ;
	}
	pPlayer->GetBaseData()->GetPlayerBrifData(&tBrifData) ;
	pPlayer->GetBaseData()->GetPlayerDetailData(pDetail) ;
	pDetail->bIsOnLine = false ;
	tBrifData.bIsOnLine = false ;
	pPlayer = NULL ;
}

void stFriendInfo::OnFriendOnLine(CPlayer* pOnLinePlayer)
{
	if ( pOnLinePlayer == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("friend player online can not be null") ;
		return ;
	}
	pPlayer = pOnLinePlayer ;
	pPlayer->GetBaseData()->GetPlayerBrifData(&tBrifData) ;
	bHaveBrifeData = true ;
}

bool stFriendInfo::GetDetailData(stPlayerDetailData* pData )
{
	if ( pPlayer )
	{
		pPlayer->GetBaseData()->GetPlayerDetailData(pData) ;
		return true;
	}

	if ( pDetail )
	{
		memcpy(pData,pDetail,sizeof(stPlayerDetailData));
		pData->bIsOnLine = false ;
		return true ;
	}
	return false ;
}

void stFriendInfo::SetDetail(stPlayerDetailData* pDetailData )
{
	if ( pDetail == NULL )
	{
		pDetail = new stPlayerDetailData ;
	}
	memcpy(pDetail,pDetailData,sizeof(stPlayerDetailData));
	memcpy(&tBrifData,pDetailData,sizeof(stPlayerBrifData));
	bHaveBrifeData = true ;
}

CPlayerFriend::CPlayerFriend(CPlayer* pPlayer):IPlayerComponent(pPlayer)
{
	m_bDirty = false;
}

CPlayerFriend::~CPlayerFriend()
{
	ClearFriendInfo();
}

bool CPlayerFriend::OnMessage(stMsg* pMsg )
{
	switch ( pMsg->usMsgType )
	{
	case MSG_REQUEST_FRIEND_LIST:
		{
			if ( pMsg->cSysIdentifer == ID_MSG_DB2GM )
			{
				stMsgGameServerRequestFirendListRet* pMsgRet = (stMsgGameServerRequestFirendListRet*)pMsg ;
				char* pBuffer = (char*)pMsg ;
				pBuffer += sizeof(stMsgGameServerRequestFirendListRet) ;
				while ( pMsgRet->nFriendCount--)
				{
					stFriendInfo* pInfo = new stFriendInfo ;
					stServerSaveFrienItem* pItem = (stServerSaveFrienItem*)pBuffer ;
					pInfo->nPresentCoinTimes = pItem->nPresentTimes ;
					pInfo->tBrifData.nUserUID = pItem->nFriendUserUID ;
					if ( GetFriendByUID(pItem->nFriendUserUID) == NULL )
					{
						m_vAllFriends[pItem->nFriendUserUID] = pInfo;
					}
					
					++pItem;
				}
				UpdateFirendInfo();
				CLogMgr::SharedLogMgr()->PrintLog("read friend list from DB") ;
			}
			else
			{
				OnClientRequestFriendList();
			}
		}
		break;
	case MSG_REQUEST_FRIEND_BRIFDATA_LIST:
		{
			stMsgGameServerRequestFriendBrifDataListRet* pRet = (stMsgGameServerRequestFriendBrifDataListRet*)pMsg ;
			char* pBuffer = (char*)pRet ;
			pBuffer = pBuffer + sizeof(stMsgGameServerRequestFriendBrifDataListRet);
			stPlayerBrifData* pData = (stPlayerBrifData*)pBuffer;
			while ( pRet->nCount-- > 0 )
			{
				stFriendInfo* pInfo = GetFriendByUID(pData->nUserUID) ;
				if ( pInfo == NULL )
				{
					CLogMgr::SharedLogMgr()->ErrorLog("friend info is NULL ? how ?") ;
					continue;
				}
				memcpy(&pInfo->tBrifData,pData,sizeof(stPlayerBrifData));
				pInfo->tBrifData.bIsOnLine = false ;
				pInfo->bHaveBrifeData = true ;
				++pData ;
			}
			CLogMgr::SharedLogMgr()->PrintLog("read friend brife data list from DB ,send list to client") ;
			SendListToClient() ;
		}
		break;
	case MSG_SAVE_FRIEND_LIST:
		{
			stMsgGameServerSaveFriendListRet* pRet = (stMsgGameServerSaveFriendListRet*)pMsg ;
			if ( pRet->nRet == 0 )
			{
				CLogMgr::SharedLogMgr()->PrintLog("%s Save friend list success ",GetPlayer()->GetBaseData()->GetPlayerName()) ;
			}
			else
			{
				CLogMgr::SharedLogMgr()->ErrorLog("%s Save friend list Failed ",GetPlayer()->GetBaseData()->GetPlayerName()) ;
			}
		}
		break; 
	case MSG_PLAYER_ADD_FRIEND:
		{
			stMsgPlayerAddFriend* pMsgRet = (stMsgPlayerAddFriend*)pMsg ;
			CPlayer* pTargetPlayer = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetPlayerByUserUID(pMsgRet->nUID) ;
			stMsgPlayerAddFriendRet msg ;
			msg.nRet = 0 ;

			if ( IsFriendListFull() )
			{
				msg.nRet = 1 ;
				CLogMgr::SharedLogMgr()->PrintLog("friend list is full, can not add");
			}
			else if ( GetFriendByUID(pMsgRet->nUID) != NULL )
			{
				msg.nRet = 4 ;
				CLogMgr::SharedLogMgr()->PrintLog("already your friend , can not add");
			}
			else
			{
				CPlayerMailComponent* pMail = NULL ;
				if ( pTargetPlayer )
				{
					 pMail = (CPlayerMailComponent*)pTargetPlayer->GetComponent(ePlayerComponent_Mail) ;
					 AddFriend(pTargetPlayer);
					 CLogMgr::SharedLogMgr()->PrintLog("add friend target online , mail infom it ");
				}
				else
				{
					pMail = (CPlayerMailComponent*)GetPlayer()->GetComponent(ePlayerComponent_Mail) ;
					AddFriend(pMsgRet->nUID);
					CLogMgr::SharedLogMgr()->PrintLog("add friend target not online , mail infom it ");
				}
				pMail->PostBeAddedFriendMail(GetPlayer(),pMsgRet->nUID);
				
			}
			SendMsgToClient((char*)&msg,sizeof(msg)) ;
		}
		break;
	//case MSG_PLAYER_REPLAY_BE_ADD_FRIEND:
	//	{
	//		stMsgPlayerReplayBeAddedFriend* pMsgRet = (stMsgPlayerReplayBeAddedFriend*)pMsg ;
	//		CPlayer* pReplyToPlayer = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetPlayerByUserUID(pMsgRet->nReplayToPlayerUserUID) ;
	//		CPlayerFriend* pReplyToFriendComponed =  NULL;
	//		if ( pReplyToPlayer )
	//		{
	//			pReplyToFriendComponed = (CPlayerFriend*)pReplyToPlayer->GetComponent(ePlayerComponent_Friend) ;
	//		}
	//		
	//		stMsgPlayerReplayBeAddedFriendRet msgBack ;
	//		msgBack.nRet = 0 ;
	//		if ( pReplyToPlayer )
	//		{
	//			memcpy(msgBack.pTargetName,pReplyToPlayer->GetBaseData()->GetPlayerName(),MAX_LEN_CHARACTER_NAME);
	//		}
	//		if ( IsFriendListFull() )
	//		{
	//			msgBack.nRet = 2 ;
	//		}
	//		else if ( pMsgRet->bAgree && GetFriendByUID(pMsgRet->nReplayToPlayerUserUID) != NULL )
	//		{
	//			msgBack.nRet = 4 ;
	//		}
	//		else if ( pReplyToPlayer == NULL )
	//		{
	//			msgBack.nRet = 1 ;
	//			CPlayerMailComponent* pMail = (CPlayerMailComponent*)GetPlayer()->GetComponent(ePlayerComponent_Mail) ;
	//			pMail->PostReplyToAddFriendMail(GetPlayer(),pMsgRet->bAgree,pMsgRet->nReplayToPlayerUserUID);
	//		}
	//		else if ( pReplyToFriendComponed->IsFriendListFull() )
	//		{
	//			msgBack.nRet = 3 ;
	//		}
	//		else
	//		{
	//			pReplyToFriendComponed->OnOtherReplayMeAboutAddItbeFriend(pMsgRet->bAgree,GetPlayer()) ;
	//		}

	//		if ( pMsgRet->bAgree )
	//		{
	//			SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
	//		}

	//		if ( pMsgRet->bAgree )
	//		{
	//			if ( pReplyToPlayer )
	//			{
	//				AddFriend(pReplyToPlayer);
	//			}
	//			else
	//			{
	//				AddFriend(pMsgRet->nReplayToPlayerUserUID);
	//			}
	//		}
	//	}
	//	break;
	case MSG_PLAYER_SERACH_PEERS:
		{
			if ( pMsg->cSysIdentifer == ID_MSG_DB2GM ) // db result 
			{
				stMsgGameServerGetSearchFriendResultRet* pMsgRet = (stMsgGameServerGetSearchFriendResultRet*)pMsg ;
				stMsgPlayerSearchPeerRet msgBack ;
				msgBack.nRetCount = pMsgRet->nResultCount ;
				char* pBuffer = new char[ sizeof(msgBack) + pMsgRet->nResultCount * sizeof(stPlayerBrifData)] ;
				unsigned short nOffset = 0 ;
				memcpy(pBuffer,&msgBack,sizeof(msgBack));
				nOffset += sizeof(msgBack);
				memcpy(pBuffer + nOffset,(char*)pMsgRet + sizeof(stMsgGameServerGetSearchFriendResultRet),pMsgRet->nResultCount * sizeof(stPlayerBrifData));
				nOffset += pMsgRet->nResultCount * sizeof(stPlayerBrifData) ;

				// update online state 
				char* pTemp = pBuffer + sizeof(msgBack);
				stPlayerBrifData* pinfo = (stPlayerBrifData*)pTemp ;
				for ( int i = 0 ; i < pMsgRet->nResultCount ; ++i )
				{
					pinfo->bIsOnLine = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetPlayerByUserUID(pinfo->nUserUID) != NULL ;
				}
				SendMsgToClient(pBuffer,nOffset) ;
				delete[] pBuffer;
				CLogMgr::SharedLogMgr()->PrintLog("read search result from db");
			}
			else   // from client ;
			{
				stMsgPlayerSearchPeer* pMsgRet = (stMsgPlayerSearchPeer*)pMsg ;

				if ( pMsgRet->nSearchContentLen >= MAX_LEN_CHARACTER_NAME )  // serach content is too long 
				{
					stMsgPlayerSearchPeerRet msgBack ;
					msgBack.nRetCount = 0 ;
					SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
					CLogMgr::SharedLogMgr()->PrintLog("search content is too long , no result");
					break; ;
				}
				// sent to db , request the search ;
				stMsgGameServerGetSearchFriendResult msgToDB ;
				msgToDB.nLen = pMsgRet->nSearchContentLen ;
				msgToDB.nSessionID = GetPlayer()->GetSessionID() ;
				char* pBuffer = new char[sizeof(msgToDB) + msgToDB.nLen] ;
				unsigned short nOffset = 0 ;
				memcpy(pBuffer,&msgToDB,sizeof(msgToDB));
				nOffset += sizeof(msgToDB);
				memcpy(pBuffer + nOffset, (char*)pMsgRet + sizeof(stMsgPlayerSearchPeer),msgToDB.nLen);
				nOffset += msgToDB.nLen ;
				SendMsgToDB(pBuffer,nOffset) ;
				delete[] pBuffer ;
				CLogMgr::SharedLogMgr()->PrintLog("send request to DB to read search result");
			}
		}
		break;
	case MSG_PLAYER_REQUEST_FRIEND_DETAIL:
		{
			if ( pMsg->cSysIdentifer == ID_MSG_DB2GM )  // come from db search result ;
			{
				stMsgGameServerGetFriendDetailRet* pRetMsg = (stMsgGameServerGetFriendDetailRet*)pMsg ;
				stMsgPlayerRequestFriendDetailRet msgBack ;
				msgBack.nRet = pRetMsg->nRet ;
				if ( msgBack.nRet != 0 )
				{
					CLogMgr::SharedLogMgr()->ErrorLog("request friend info error , friend uid = %d ",pRetMsg->stPeerInfo.nUserUID);
				}
				else
				{
					stFriendInfo* pInfo = GetFriendByUID(pRetMsg->stPeerInfo.nUserUID) ;
					if ( pInfo == NULL )
					{
						CLogMgr::SharedLogMgr()->ErrorLog("NO this friend info , how ?") ;
					}
					else
					{
						pInfo->SetDetail(&pRetMsg->stPeerInfo) ;
					}
					memcpy(&msgBack.stPeerInfo,&pRetMsg->stPeerInfo,sizeof(msgBack.stPeerInfo));
				}
				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				CLogMgr::SharedLogMgr()->PrintLog("read friend detail from db, and send to client ");
			}
			else
			{
				stMsgPlayerRequestFriendDetail* pRequest = (stMsgPlayerRequestFriendDetail*)pMsg ;
				stFriendInfo* pInfo = GetFriendByUID(pRequest->nFriendUserUID) ;
				if ( pInfo->HaveDetailData() )
				{
					stMsgPlayerRequestFriendDetailRet msgBack ;
					msgBack.nRet = 0 ;
					pInfo->GetDetailData(&msgBack.stPeerInfo);
					SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
					CLogMgr::SharedLogMgr()->PrintLog("already have friend detail, send to client ");
				}
				else
				{
					// send to db 
					stMsgGameServerGetFriendDetail msgToDB; 
					msgToDB.nSessionID = GetPlayer()->GetSessionID() ;
					msgToDB.nFriendUID = pRequest->nFriendUserUID ;
					SendMsgToDB((char*)&msgToDB,sizeof(msgToDB)) ;
					CLogMgr::SharedLogMgr()->PrintLog("now do not have friend detail, request from db ");
				}
			}
		}
		break;
	case MSG_PLAYER_REQUEST_SEARCH_PEER_DETAIL:
		{
			if ( pMsg->cSysIdentifer == ID_MSG_DB2GM )
			{
				stMsgGameServerGetSearchedPeerDetailRet* pRetMsg = (stMsgGameServerGetSearchedPeerDetailRet*)pMsg ;
				stMsgPlayerRequestSearchedPeerDetailRet msgBack ;
				msgBack.nRet = pRetMsg->nRet ;
				if ( msgBack.nRet != 0 )
				{

				}
				else
				{
					memcpy(&msgBack.stPeerInfo,&pRetMsg->stPeerInfo,sizeof(msgBack.stPeerInfo));
				}
				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
			}
			else
			{
				stMsgPlayerRequestSearchedPeerDetail* pRetMsg = (stMsgPlayerRequestSearchedPeerDetail*)pMsg ;
				CPlayer* pPlayer = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetPlayerByUserUID(pRetMsg->nPeerUserUID) ;
				if ( pPlayer )
				{
					CPlayerBaseData* pbaseData = pPlayer->GetBaseData() ;
					stMsgPlayerRequestSearchedPeerDetailRet msgBack ;
					pbaseData->GetPlayerDetailData(&msgBack.stPeerInfo) ;
					SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
				}
				else
				{
					// send to db request ;
					stMsgGameServerGetSearchedPeerDetail msgToDB; 
					msgToDB.nSessionID = GetPlayer()->GetSessionID() ;
					msgToDB.nPeerUserUID = pRetMsg->nPeerUserUID ;
					SendMsgToDB((char*)&msgToDB,sizeof(msgToDB)) ;
				}
			}
		}
		break;
	case MSG_PLAYER_DELETE_FRIEND:
		{
			stMsgPlayerDelteFriendRet msgBack ;
			msgBack.nRet = 0 ;
			stMsgPlayerDelteFriend* pMsgRet = (stMsgPlayerDelteFriend*)pMsg ;
			msgBack.nDeleteUID = pMsgRet->nDelteFriendUserUID ;
			if ( !GetFriendByUID(pMsgRet->nDelteFriendUserUID) )
			{
				msgBack.nRet = 1 ;
			}
			else
			{
				RemoveFriendByUID(pMsgRet->nDelteFriendUserUID) ;
			}
			SendMsgToClient((char*)&msgBack,sizeof(msgBack));
		}
		break;
	default:
		return false ;
	}
	return true ;
}

void CPlayerFriend::OnPlayerDisconnect()
{
	TimerSave();
	CEventCenter::SharedEventCenter()->RemoveEventListenner(eEvent_PlayerOnline,this,CPlayerFriend::EventFunc ) ;
	CEventCenter::SharedEventCenter()->RemoveEventListenner(eEvent_PlayerOffline,this,CPlayerFriend::EventFunc ) ;
}

void CPlayerFriend::Reset()
{
	m_bDirty = false;
	ClearFriendInfo();
	// send request ;
	stMsgGameServerRequestFirendList msgRequest ;
	msgRequest.nUserUID = GetPlayer()->GetUserUID() ;
	msgRequest.nSessionID = GetPlayer()->GetSessionID() ;
	SendMsgToDB((char*)&msgRequest,sizeof(msgRequest)) ;
	CEventCenter::SharedEventCenter()->RegisterEventListenner(eEvent_PlayerOnline,this,CPlayerFriend::EventFunc ) ;
	CEventCenter::SharedEventCenter()->RegisterEventListenner(eEvent_PlayerOffline,this,CPlayerFriend::EventFunc ) ;
}

void CPlayerFriend::Init()
{
	ClearFriendInfo();
	IPlayerComponent::Init() ;
	Reset();
	m_bDirty = false;
}

void CPlayerFriend::UpdateFirendInfo()
{

	if ( m_vAllFriends.empty() )
	{
		CLogMgr::SharedLogMgr()->PrintLog( "m_vAllFriends is null , can not update friend info !" ) ;
		return ; 
	}

 	CPlayerManager* pPlayerMgr = CGameServerApp::SharedGameServerApp()->GetPlayerMgr();
	
	MAP_FRIENDS::iterator iter = m_vAllFriends.begin() ;
	for ( ; iter != m_vAllFriends.end() ; ++iter )
	{
		stFriendInfo* pFriendInfo = iter->second ;
		if ( pFriendInfo == NULL )
		{
			CLogMgr::SharedLogMgr()->ErrorLog( "Why this friend info is null" ) ;
			continue; 
		}

		CPlayer* pPlayer = pPlayerMgr->GetPlayerByUserUID(pFriendInfo->tBrifData.nUserUID) ;
		if ( pPlayer )
		{
			pFriendInfo->OnFriendOnLine(pPlayer) ;
		}
	}
}

void CPlayerFriend::ClearFriendInfo()
{
	MAP_FRIENDS::iterator iter = m_vAllFriends.begin() ;
	for ( ; iter != m_vAllFriends.end() ; ++iter )
	{
		delete iter->second ;
	}
	m_vAllFriends.clear() ;
}

void CPlayerFriend::SendListToClient()
{
	// send msg to client ;
	stMsgPlayerRequestFriendListRet msg ;
	msg.nFriendCount = m_vAllFriends.size() ;
	char* pBuffer = new char[sizeof(msg) + msg.nFriendCount * sizeof(stFriendBrifData)] ;
	unsigned short nOffset = 0 ;
	memcpy(pBuffer,&msg,sizeof(msg));
	nOffset += sizeof(msg);
	MAP_FRIENDS::iterator iter = m_vAllFriends.begin() ;
	for ( ; iter != m_vAllFriends.end(); ++iter )
	{
		stFriendInfo* pInfo = iter->second ;
		memcpy(pBuffer + nOffset , pInfo,sizeof(stFriendBrifData));
		nOffset += sizeof(stFriendBrifData);
	}
	SendMsgToClient(pBuffer,nOffset) ;
	delete[] pBuffer ;
	pBuffer = NULL ;
}

//void CPlayerFriend::OnPlayerWantAddMe(CPlayer* pPlayerWantAddMe )
//{
//	stMsgPlayerBeAddedFriend msg ;
//	msg.nPlayerUserUID = pPlayerWantAddMe->GetUserUID() ;
//	memcpy(msg.pPlayerName,pPlayerWantAddMe->GetBaseData()->strName,sizeof(msg.pPlayerName));
//	SendMsgToClient((char*)&msg,sizeof(msg)) ;
//}
//
//void CPlayerFriend::OnOtherReplayMeAboutAddItbeFriend(bool bAgree,CPlayer* pWhoReplyMe)
//{
//	if ( bAgree )
//	{
//		AddFriend(pWhoReplyMe) ;
//		// success to add firend ;
//		stPlayerEventArgAddFriend eventArg ;
//		eventArg.eEventType = ePlayerEvent_AddFriend ;
//		eventArg.pNewFirend = pWhoReplyMe ;
//		GetPlayer()->PostPlayerEvent(&eventArg) ;
//	}
//
//	stMsgPlayerAddFriendReplay msg;
//	msg.bAgree = bAgree ;
//	memcpy(msg.pReplayerName,pWhoReplyMe->GetBaseData()->strName,sizeof(msg.pReplayerName));
//	SendMsgToClient((char*)&msg,sizeof(msg)) ;
//}

void CPlayerFriend::AddFriend( CPlayer* pPlayerToAdd )
{
#ifdef DEBUG
	if ( GetFriendByUID(pPlayerToAdd->GetUserUID()) )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("error already friend");
		return ;
	}
#endif
	stFriendInfo* pInfo = new stFriendInfo ;
	pInfo->OnFriendOnLine(pPlayerToAdd) ;
	m_vAllFriends[pInfo->tBrifData.nUserUID] = pInfo ;
	m_bDirty = true;
}

stFriendInfo* CPlayerFriend::GetFriendByUID(unsigned int nPlayerUID )
{
	MAP_FRIENDS::iterator iter = m_vAllFriends.find(nPlayerUID) ;
	if ( iter != m_vAllFriends.end() )
	{
		return iter->second ;
	}
	return NULL ;
}

void CPlayerFriend::TimerSave()
{
	if ( !m_bDirty )
	{
		return ;
	}
	m_bDirty = false ;
	// save current firend info ;
	stMsgGameServerSaveFirendList msgSaveList ;
	msgSaveList.nSessionID = GetPlayer()->GetSessionID() ;
	msgSaveList.nFriendCount = m_vAllFriends.size() ;
	msgSaveList.nUserUID = GetPlayer()->GetUserUID();
	char* pBuffer = new char [ sizeof(msgSaveList) + sizeof(stServerSaveFrienItem) * msgSaveList.nFriendCount ] ;
	unsigned short nOffset = 0 ;
	memcpy(pBuffer,&msgSaveList,sizeof(msgSaveList));
	nOffset += sizeof(msgSaveList);
	MAP_FRIENDS::iterator iter = m_vAllFriends.begin() ;
	stServerSaveFrienItem saveItem ;
	for ( ; iter != m_vAllFriends.end(); ++iter) 
	{
		stFriendInfo* pInfo = iter->second ;
		if ( pInfo == NULL )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("how can friend info is NULL") ;
			continue;
		}
		saveItem.nFriendUserUID = pInfo->tBrifData.nUserUID;
		saveItem.nPresentTimes = pInfo->nPresentCoinTimes;
		memcpy(pBuffer + nOffset , &saveItem, sizeof(saveItem));
		//CLogMgr::SharedLogMgr()->PrintLog("save friend id = %d",pInfo->nUID) ;
		nOffset += sizeof(saveItem);
	}
	SendMsgToDB(pBuffer,nOffset) ;
	delete[] pBuffer ;
}

void CPlayerFriend::RemoveFriendByUID(unsigned int nPlayerUID )
{
	MAP_FRIENDS::iterator iter = m_vAllFriends.begin() ;
	for ( ; iter != m_vAllFriends.end(); ++iter )
	{
		if ( iter->second->tBrifData.nUserUID == nPlayerUID )
		{
			delete iter->second ;
			m_vAllFriends.erase(iter) ;
			m_bDirty = true;
			return;
		}
	}
}

void CPlayerFriend::AddFriend(unsigned int nFriendUserUID)
{
#ifdef DEBUG
	if ( GetFriendByUID(nFriendUserUID) )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("error already friend");
		return ;
	}
#endif
	stFriendInfo* pInfo = new stFriendInfo ;
	pInfo->tBrifData.nUserUID = nFriendUserUID ;
	pInfo->bHaveBrifeData = false ;
	pInfo->nPresentCoinTimes = 0 ;
	m_vAllFriends[nFriendUserUID] = pInfo;
	m_bDirty = true;
}
void CPlayerFriend::OnClientRequestFriendList()
{
	// is all friend have brife Data ;
	std::vector<unsigned int > vUserUIDS ;
	MAP_FRIENDS::iterator iter = m_vAllFriends.begin();
	for ( ; iter != m_vAllFriends.end(); ++iter )
	{
		stFriendInfo* pInfo = iter->second ;
		if (! pInfo->HaveBrifData() )
		{
			vUserUIDS.push_back(pInfo->tBrifData.nUserUID) ;
		}
	}

	if ( vUserUIDS.size() == 0 )
	{
		SendListToClient();
	}
	else
	{
		// request from db to get more brif data 
		stMsgGameServerRequestFriendBrifDataList msg ;
		msg.nFriendCount = vUserUIDS.size();
		msg.nSessionID = GetPlayer()->GetSessionID() ;
		char* pBuffer = new char[sizeof(msg) + msg.nFriendCount * sizeof(unsigned int)] ;
		memcpy(pBuffer,&msg,sizeof(msg));
		unsigned int* pIntBuf = (unsigned int*)(pBuffer + sizeof(msg));
		for ( unsigned int i = 0 ; i < vUserUIDS.size(); ++i )
		{
			*pIntBuf = vUserUIDS[i] ;
			++pIntBuf ;
		}
		SendMsgToDB(pBuffer,sizeof(msg) + msg.nFriendCount * sizeof(unsigned int));
		delete []pBuffer ;
	}
}

bool CPlayerFriend::EventFunc(void* pUserData,stEventArg* pArg)
{
	CPlayerFriend* pF = (CPlayerFriend*)pUserData ;
	pF->OnProcessEvent(pArg) ;
	return false ;
}

void CPlayerFriend::OnProcessEvent(stEventArg* pArg)
{
	if ( pArg == NULL || (pArg->cEvent != eEvent_PlayerOffline && eEvent_PlayerOnline != pArg->cEvent) )
	{
		return ;
	}

	CPlayer* p = (CPlayer*)pArg->pData ;
	stFriendInfo* pinfo = GetFriendByUID(p->GetUserUID()) ;
	if ( pinfo )
	{
		if ( pArg->cEvent == eEvent_PlayerOnline )
		{
			pinfo->OnFriendOnLine(p) ;
		}
		else if ( eEvent_PlayerOffline == pArg->cEvent ) 
		{
			pinfo->OnFriendOffline(p);
		}
	}
}