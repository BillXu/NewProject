#include "PlayerManager.h"
#include "ServerMessageDefine.h"
#include "LogManager.h"
#include "Player.h"
#include "CommonDefine.h"
#include "GameServerApp.h"
#include <assert.h>
#include "EventCenter.h"
#include "PlayerBaseData.h"
#include "AutoBuffer.h"
CPlayerManager::CPlayerManager()
{
	m_vOfflinePlayers.clear() ;
	m_vAllActivePlayers.clear();
}

CPlayerManager::~CPlayerManager()
{
	MAP_SESSIONID_PLAYERS::iterator iter = m_vAllActivePlayers.begin();
	for ( ; iter != m_vAllActivePlayers.end() ; ++iter )
	{
		delete iter->second ;
		iter->second = NULL ;
	}
	m_vAllActivePlayers.clear() ;

	MAP_UID_PLAYERS::iterator iter_R = m_vOfflinePlayers.begin() ;
	for ( ; iter_R != m_vOfflinePlayers.end(); ++iter_R )
	{
		delete iter_R->second ;
		iter_R->second = NULL ;
	}
	m_vOfflinePlayers.clear() ;
}

bool CPlayerManager::OnMessage( stMsg* pMessage , eMsgPort eSenderPort , uint32_t nSessionID )
{
	if ( ProcessPublicMessage(pMessage,eSenderPort,nSessionID) )
	{
		return true ;
	}

	CPlayer* pTargetPlayer = GetPlayerBySessionID(nSessionID,true );
	if ( pTargetPlayer && pTargetPlayer->OnMessage(pMessage,eSenderPort ) )
	{
		if (pTargetPlayer->IsState(CPlayer::ePlayerState_Offline) )
		{
			pTargetPlayer->OnTimerSave(0,0);
		}
		return true  ;
	}
	else
	{
		if (pTargetPlayer == NULL )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("can not find session id = %d to process msg id = %d ,from = %d",nSessionID,pMessage->usMsgType,eSenderPort) ;
		}
		else
		{
			CLogMgr::SharedLogMgr()->ErrorLog( "unprocess msg for player uid = %d , msg = %d ,from %d ",pTargetPlayer->GetUserUID(),pMessage->usMsgType,eSenderPort ) ;
		}
	}
	return false ;
}

bool CPlayerManager::ProcessPublicMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID  )
{
	switch ( prealMsg->usMsgType )
	{
	case MSG_CROSS_SERVER_REQUEST:
		{
			stMsgCrossServerRequest* pRet = (stMsgCrossServerRequest*)prealMsg ;

			Json::Value* pJsValue = nullptr ;
			Json::Value rootValue ;
			if ( pRet->nJsonsLen )
			{
				Json::Reader reader;
				char* pstr = ((char*)&pRet->nJsonsLen) + sizeof(pRet->nJsonsLen) ;
				reader.parse(pstr,pstr + pRet->nJsonsLen,rootValue,false);
				pJsValue = &rootValue ;
			}

			if ( onCrossServerRequest(pRet,eSenderPort,pJsValue) == false )
			{
				CPlayer* pPlayer = GetPlayerByUserUID(pRet->nTargetID);
				assert(pPlayer&&"this request no one to process or target id error");
				if ( pPlayer->onCrossServerRequest(pRet,eSenderPort,pJsValue) )
				{
					return true ;
				}
				CLogMgr::SharedLogMgr()->ErrorLog("cross request type = %d , subType = %d ,unprocessed",pRet->nRequestType,pRet->nRequestSubType);
				return false;
			}

			return true ;
		}
		break;
	case MSG_CROSS_SERVER_REQUEST_RET:
		{
			stMsgCrossServerRequestRet* pRet = (stMsgCrossServerRequestRet*)prealMsg ;
			Json::Value* pJsValue = nullptr ;
			Json::Value rootValue ;
			if ( pRet->nJsonsLen )
			{
				Json::Reader reader;
				char* pstr = ((char*)&pRet->nJsonsLen) + sizeof(pRet->nJsonsLen) ;
				reader.parse(pstr,pstr + pRet->nJsonsLen,rootValue,false);
				pJsValue = &rootValue ;
			}

			if ( onCrossServerRequestRet(pRet,pJsValue) == false )
			{
				CPlayer* pPlayer = GetPlayerByUserUID(pRet->nTargetID);
				assert(pPlayer&&"this request no one to process or target id error");
				if ( pPlayer->onCrossServerRequestRet(pRet,pJsValue) )
				{
					return true ;
				}
				CLogMgr::SharedLogMgr()->ErrorLog("cross request result type = %d , subType = %d ,unprocessed",pRet->nRequestType,pRet->nRequestSubType);
				return false;
			}
			return true ;
		}
		break;
	case MSG_REQUEST_PLAYER_DATA:
		{
			stMsgRequestPlayerData* pRet = (stMsgRequestPlayerData*)prealMsg ;
			stMsgRequestPlayerDataRet msgBack ;
			msgBack.nRet = 0 ;
			msgBack.isDetail = pRet->isDetail ;
			CPlayer* pPlayer = GetPlayerByUserUID(pRet->nPlayerUID);

			stPlayerDetailData stData ;
			CAutoBuffer auB (sizeof(msgBack) + sizeof(stPlayerDetailData));
			if ( pPlayer )
			{
				if ( pRet->isDetail )
				{
					pPlayer->GetBaseData()->GetPlayerDetailData(&stData);
				}
				else
				{
					pPlayer->GetBaseData()->GetPlayerBrifData(&stData) ;
				}
				
				auB.addContent(&msgBack,sizeof(msgBack));
				auB.addContent(&stData,pRet->isDetail ? sizeof(stPlayerDetailData) : sizeof(stPlayerBrifData) ) ;
				CGameServerApp::SharedGameServerApp()->sendMsg(nSessionID,auB.getBufferPtr(),auB.getContentSize()) ;
				return true ;
			}
			CLogMgr::SharedLogMgr()->PrintLog("req detail player not online , req from db") ;
			stMsgSelectPlayerData msgReq ;
			msgReq.isDetail = pRet->isDetail ;
			msgReq.nReqPlayerSessionID = nSessionID  ;
			msgReq.nTargetPlayerUID = pRet->nPlayerUID ;
			CGameServerApp::SharedGameServerApp()->sendMsg(nSessionID,(char*)&msgReq,sizeof(msgReq)) ;
		}
		break;
	case MSG_SELECT_DB_PLAYER_DATA:
		{
			stMsgSelectPlayerDataRet* pRet = (stMsgSelectPlayerDataRet*)prealMsg ;
			stMsgRequestPlayerDataRet msgBack ;
			msgBack.nRet = pRet->nRet ;
			msgBack.isDetail = pRet->isDetail ;
			if ( pRet->nRet )
			{
				CGameServerApp::SharedGameServerApp()->sendMsg(pRet->nReqPlayerSessionID,(char*)&msgBack,sizeof(msgBack)) ;
			}
			else
			{
				CAutoBuffer auB (sizeof(msgBack) + sizeof(stPlayerDetailData));
				auB.addContent(&msgBack,sizeof(msgBack));
				uint16_t nLen = pRet->isDetail ? sizeof(stPlayerDetailData) : sizeof(stPlayerBrifData) ;
				auB.addContent((char*)prealMsg + sizeof(stMsgSelectPlayerDataRet),nLen );
				CGameServerApp::SharedGameServerApp()->sendMsg(pRet->nReqPlayerSessionID,auB.getBufferPtr(),auB.getContentSize()) ;
			}
			CLogMgr::SharedLogMgr()->PrintLog("session id = %d req play data ret = %d",pRet->nReqPlayerSessionID,pRet->nRet ) ;
		}
		break;
	case MSG_PLAYER_LOGIN:
		{
			stMsgOnPlayerLogin* pmsgenter = (stMsgOnPlayerLogin*)prealMsg ;
			CPlayer* pPlayer = GetPlayerBySessionID(nSessionID) ;
			if ( pPlayer != NULL && pPlayer->GetUserUID() == pmsgenter->nUserUID )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("double nSession, this nSessionID already have player, already login , do not login again  id = %d? ",nSessionID) ;
				return true ;
			}

			if ( pPlayer != NULL && pPlayer->GetUserUID() != pmsgenter->nUserUID ) // switch account in main scene 
			{
				// disconnect pre player 
				OnPlayerOffline(pPlayer);
			}

			if ( ProcessIsAlreadyLogin(pmsgenter->nUserUID,nSessionID) )
			{
				return true ;
			}

			// is offline peer 
			MAP_UID_PLAYERS::iterator iterOfflien = m_vOfflinePlayers.begin();
			for ( ; iterOfflien != m_vOfflinePlayers.end(); ++iterOfflien )
			{
				if ( iterOfflien->second && iterOfflien->first == pmsgenter->nUserUID )
				{
					iterOfflien->second->OnReactive(nSessionID) ;
					AddPlayer(iterOfflien->second) ;
					m_vOfflinePlayers.erase(iterOfflien) ;
					return true ;
				}
			}

			CPlayer* pNew = new CPlayer ;
			pNew->Init(pmsgenter->nUserUID,nSessionID ) ;
			AddPlayer(pNew) ;
		}
		break;
	case MSG_DISCONNECT_CLIENT:
		{
			CPlayer* pPlayer = GetPlayerBySessionID(nSessionID) ;
			if ( pPlayer )
			{
				// post online event ;
				CLogMgr::SharedLogMgr()->PrintLog("player disconnect session id = %d",nSessionID);
				OnPlayerOffline(pPlayer) ;
			}
			else
			{
				CLogMgr::SharedLogMgr()->ErrorLog("client disconnect ! client is NULL session id = %d",nSessionID) ;
			}
			//#ifdef _DEBUG
			LogState();
			//#endif
		}
		break;
	case MSG_TP_ORDER_LEAVE:
		{
			stMsgOrderTaxasPlayerLeaveRet* pRet = (stMsgOrderTaxasPlayerLeaveRet*)prealMsg ;
			CPlayer* pp = GetPlayerByUserUID(pRet->nUserUID) ;
			if (!pp)
			{
				CLogMgr::SharedLogMgr()->ErrorLog("uid = %d not find , so can not inform leave",pRet->nUserUID);
			}
			else
			{
				pp->OnMessage(prealMsg,eSenderPort);
			}
		}
		break;
	case MSG_TP_INFORM_LEAVE:
		{
			stMsgInformTaxasPlayerLeave* pRet = (stMsgInformTaxasPlayerLeave*)prealMsg ;
			CPlayer* pp = GetPlayerByUserUID(pRet->nUserUID) ;
			if (!pp)
			{
				CLogMgr::SharedLogMgr()->ErrorLog("uid = %d not find , so can not inform leave",pRet->nUserUID);
			}
			else
			{
				pp->OnMessage(prealMsg,eSenderPort);
			}
		}
		break;
	case MSG_TP_SYNC_PLAYER_DATA:
		{
			stMsgSyncTaxasPlayerData* pRet = (stMsgSyncTaxasPlayerData*)prealMsg ;
			CPlayer* pp = GetPlayerByUserUID(pRet->nUserUID) ;
			if (!pp)
			{
				CLogMgr::SharedLogMgr()->ErrorLog("uid = %d not find , so can not sys data",pRet->nUserUID);
			}
			else
			{
				pp->OnMessage(prealMsg,eSenderPort);
			}
		}
		break;
	default:
		return false;
	}

	return true ;
}

CPlayer* CPlayerManager::GetPlayerBySessionID( unsigned int nSessionID , bool bInclueOffline )
{
	MAP_SESSIONID_PLAYERS::iterator iter = m_vAllActivePlayers.find(nSessionID) ;
	if ( iter != m_vAllActivePlayers.end())
	{
		return iter->second ;
	}

	if ( bInclueOffline )
	{
		MAP_UID_PLAYERS::iterator iterOffline = m_vOfflinePlayers.begin();
		for ( ; iterOffline != m_vOfflinePlayers.end(); ++iterOffline )
		{
			if ( iterOffline->second->GetSessionID() == nSessionID  )
			{
				return iterOffline->second ;
			}
		}
	}
	return NULL ;
}

void CPlayerManager::Update(float fDeta )
{
	 
}

CPlayer* CPlayerManager::GetFirstActivePlayer()
{
	MAP_SESSIONID_PLAYERS::iterator iter = m_vAllActivePlayers.begin() ;
	if ( iter == m_vAllActivePlayers.end() || iter->second == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("first actvie player is NULL") ;
		return NULL ;
	}
	return iter->second ;
}

CPlayer* CPlayerManager::GetPlayerByUserUID( unsigned int nUserUID, bool bInclueOffline )
{
	MAP_SESSIONID_PLAYERS::iterator iter = m_vAllActivePlayers.begin() ;
	for ( ; iter != m_vAllActivePlayers.end(); ++iter )
	{
		if ( iter->second )
		{
			if ( nUserUID == iter->second->GetUserUID() )
			{
				return iter->second ;
			}
		}
	}

	if ( bInclueOffline )
	{
		MAP_UID_PLAYERS::iterator iter_L = m_vOfflinePlayers.find(nUserUID) ;
		if ( iter_L != m_vOfflinePlayers.end() )
		{
			return iter_L->second ;
		}
	}
	return NULL ;
}

void CPlayerManager::OnPlayerOffline( CPlayer*pPlayer )
{
	if ( !pPlayer )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Can not Remove NULL player !") ;
		return ;
	}

	CEventCenter::SharedEventCenter()->PostEvent(eEvent_PlayerOffline,pPlayer) ;
	pPlayer->OnPlayerDisconnect() ;

	MAP_SESSIONID_PLAYERS::iterator iter = m_vAllActivePlayers.find(pPlayer->GetSessionID() ) ;
	if ( iter == m_vAllActivePlayers.end() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why pPlayer uid = %d , not active ? shuold active " , pPlayer->GetUserUID() ) ;
		delete pPlayer ;
		pPlayer = NULL ;
		return ;
	}
	m_vAllActivePlayers.erase(iter) ;

	MAP_UID_PLAYERS::iterator iterOffline = m_vOfflinePlayers.find(pPlayer->GetUserUID()) ;
	if ( iterOffline != m_vOfflinePlayers.end() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why player uid = %d already in offline list ? " , iterOffline->first ) ;
		
		delete pPlayer ;
		pPlayer = NULL ;
		return ;
	}
	m_vOfflinePlayers[pPlayer->GetUserUID()] = pPlayer ;
}

void CPlayerManager::AddPlayer(CPlayer*pPlayer)
{
	if ( !pPlayer )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Can not Add NULL player !") ;
		return ;
	}
	MAP_SESSIONID_PLAYERS::iterator iter = m_vAllActivePlayers.find(pPlayer->GetSessionID() ) ;
	if ( iter != m_vAllActivePlayers.end() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Player to add had existed in active map ! , player UID = %d",pPlayer->GetUserUID() ) ;
		delete pPlayer ;
		pPlayer = NULL ;
	}
	else
	{
		m_vAllActivePlayers[pPlayer->GetSessionID()]= pPlayer ;
	}
}

void CPlayerManager::LogState()
{
	CLogMgr::SharedLogMgr()->SystemLog( "Active Player: %d   Offline Player: %d " ,m_vAllActivePlayers.size(),m_vOfflinePlayers.size() ) ;
}

bool CPlayerManager::ProcessIsAlreadyLogin(unsigned int nUserID ,unsigned nSessionID )
{
	// is active player
	CPlayer* pPlayer = NULL ;
	MAP_SESSIONID_PLAYERS::iterator  iter_ma = m_vAllActivePlayers.begin() ;
	for ( ; iter_ma != m_vAllActivePlayers.end(); ++iter_ma )
	{
		pPlayer = iter_ma->second ;
		if ( !pPlayer )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("Why empty active player here ?") ;
			continue; 
		}

		if ( pPlayer->GetUserUID() == nUserID )
		{
			pPlayer->OnAnotherClientLoginThisPeer(nSessionID) ;
			m_vAllActivePlayers.erase(iter_ma) ;
			AddPlayer(pPlayer) ;
			CLogMgr::SharedLogMgr()->PrintLog("other decivec login");
			return true ;
		}
	}
	return false ;
}

bool CPlayerManager::onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue)
{
	return false ;
}

bool CPlayerManager::onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue)
{
	return false ;
}