#include "PlayerManager.h"
#include "ServerMessageDefine.h"
#include "LogManager.h"
#include "Player.h"
#include "CommonDefine.h"
#include "GameServerApp.h"
#include <assert.h>
#include "EventCenter.h"
#include "PlayerBaseData.h"
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
	if ( prealMsg->usMsgType == MSG_PLAYER_LOGIN )
	{
		stMsgOnPlayerLogin* pmsgenter = (stMsgOnPlayerLogin*)prealMsg ;
		CPlayer* pPlayer = GetPlayerBySessionID(nSessionID) ;
		if ( pPlayer != NULL && pPlayer->GetUserUID() == pmsgenter->nUserUID )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("double nSession, this nSessionID already have player, already login , do not login again ? ") ;
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
		return true ;
	}

	// process player logout ;
	if ( MSG_DISCONNECT_CLIENT == prealMsg->usMsgType )
	{
		CPlayer* pPlayer = GetPlayerBySessionID(nSessionID) ;
		if ( pPlayer )
		{
			// post online event ;
			OnPlayerOffline(pPlayer) ;
		}
		else
		{
			CLogMgr::SharedLogMgr()->ErrorLog("client disconnect ! client is NULL session id = %d",nSessionID) ;
		}
//#ifdef _DEBUG
		LogState();
//#endif
		return true ;
	}

	if ( ProcessTaxasServerMsg(prealMsg,eSenderPort,nSessionID) )
	{
		return true ;
	}

	return false ;
}

bool CPlayerManager::ProcessTaxasServerMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nRoomID  )
{
	if ( eSenderPort != ID_MSG_PORT_TAXAS )
	{
		return false ;
	}

	switch ( prealMsg->usMsgType )
	{
	case MSG_TP_REQUEST_PLAYER_DATA:
		{
			stMsgRequestTaxasPlayerData* pData = (stMsgRequestTaxasPlayerData*)prealMsg;
			stMsgRequestTaxasPlayerDataRet msgBack ;
			msgBack.nRoomID = nRoomID ;
			msgBack.nRet = 0 ;
			CPlayer* pPlayer = GetPlayerBySessionID(pData->nSessionID) ;
			if ( pPlayer == NULL )
			{
				msgBack.nRet = 1 ;
				CGameServerApp::SharedGameServerApp()->SendMsg(pData->nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
				CLogMgr::SharedLogMgr()->ErrorLog("why can not find player session from taxas server session id = %d",pData->nSessionID ) ;
				return true ;
			}

			if ( pPlayer->GetTaxasRoomID() )
			{
				msgBack.nRet = 2 ;
				CGameServerApp::SharedGameServerApp()->SendMsg(pData->nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
				CLogMgr::SharedLogMgr()->ErrorLog("can not enter room already in other room id = %d  session id = %d",pPlayer->GetTaxasRoomID(),pData->nSessionID ) ;

				// may have error  order leave 
				stMsgOrderTaxasPlayerLeave msg ;
				msg.nRoomID = pPlayer->GetTaxasRoomID() ;
				CGameServerApp::SharedGameServerApp()->SendMsg(pPlayer->GetSessionID(),(char*)&msg,sizeof(msg)) ;
				return true ;
			}

			msgBack.tData.nPhotoID = pPlayer->GetBaseData()->GetPhotoID();
			memset(msgBack.tData.cName,0,sizeof(msgBack.tData.cName) );
			sprintf_s(msgBack.tData.cName,sizeof(msgBack.tData.cName),"%s",pPlayer->GetBaseData()->GetPlayerName()) ;
			msgBack.tData.nSessionID = pData->nSessionID ;
			msgBack.tData.nSex = pPlayer->GetBaseData()->GetSex();
			msgBack.tData.nUserUID = pPlayer->GetUserUID();
			msgBack.tData.nVipLevel = pPlayer->GetBaseData()->GetVipLevel() ;
			CGameServerApp::SharedGameServerApp()->SendMsg(pData->nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
			pPlayer->SetStayInTaxasRoomID(nRoomID) ;
		}
		break;
	case MSG_TP_REQUEST_TAKE_IN_MONEY:
		{
			stMsgRequestTaxasPlayerTakeInCoin* pRet = (stMsgRequestTaxasPlayerTakeInCoin*)prealMsg ;
			stMsgRequestTaxasPlayerTakeInCoinRet msgBack ;
			CPlayer* pPlayer = GetPlayerBySessionID(pRet->nPlayerSessionID ) ;
			if ( pPlayer == NULL )
			{
				msgBack.nRet = 2 ;
				CGameServerApp::SharedGameServerApp()->SendMsg(pRet->nPlayerSessionID,(char*)&msgBack,sizeof(msgBack)) ;
				CLogMgr::SharedLogMgr()->ErrorLog("MSG_TP_REQUEST_TAKE_IN_MONEY why can not find player session from taxas server session id = %d",pRet->nPlayerSessionID ) ;
				return true ;
			}

			if ( pPlayer->GetBaseData()->SetTakeInCoin(pRet->nMoneyToTakeIn,pRet->bIsDiamond) )
			{
				msgBack.nRet = 0 ;
				msgBack.nMoneyToTakeIn = pRet->nMoneyToTakeIn ;
				msgBack.nRoomID = nRoomID ;
				msgBack.nSeatIdx = pRet->nSeatIdx ;
				msgBack.bIsDiamond = pRet->bIsDiamond ;
				CGameServerApp::SharedGameServerApp()->SendMsg(pRet->nPlayerSessionID,(char*)&msgBack,sizeof(msgBack)) ;
			}
			else
			{
				msgBack.nRet = 1 ;
				CGameServerApp::SharedGameServerApp()->SendMsg(pRet->nPlayerSessionID,(char*)&msgBack,sizeof(msgBack)) ;
			}
		}
		break;
	case MSG_TP_INFORM_STAND_UP:
		{
			stMsgInformTaxasPlayerStandUp* pRet = (stMsgInformTaxasPlayerStandUp*)prealMsg ;
			CPlayer* pPlayer = NULL ;
			if ( pRet->nUserUID )
			{
				pPlayer = GetPlayerByUserUID(pRet->nUserUID ) ;
			}
			else
			{
				pPlayer = GetPlayerBySessionID(pRet->nSessionID ) ;
			}

			if ( pPlayer == NULL )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("MSG_TP_INFORM_STAND_UP why can not find player uid from taxas server uid = %d",pRet->nSessionID ) ;
				return true ;
			}

			if ( pPlayer )
			{
				pPlayer->GetBaseData()->CacluateTaxasRoomMoney(pRet->nTakeInMoney,pRet->bIsDiamond) ;
			}
		}
		break;
	case MSG_TP_INFORM_LEAVE:
		{
			stMsgInformTaxasPlayerLeave* pRet = (stMsgInformTaxasPlayerLeave*)prealMsg ;
			CPlayer* pPlayer = GetPlayerByUserUID(pRet->nUserUID) ;
			if ( pPlayer == NULL )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("MSG_TP_INFORM_LEAVE why can not find player uid from taxas server uid = %d",pRet->nUserUID ) ;
				return true ;
			}
			pPlayer->SetStayInTaxasRoomID(0) ;
		}
		break;
	default:
		return false ;
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
			return true ;
		}
	}
	return false ;
}