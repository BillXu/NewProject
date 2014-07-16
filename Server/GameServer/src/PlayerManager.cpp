#include "PlayerManager.h"
#include "ServerMessageDefine.h"
#include "LogManager.h"
#include "Player.h"
#include "CommonDefine.h"
#include "GameServerApp.h"
#include <assert.h>
#include "PlayerMail.h"
#include "EventCenter.h"

CPlayerManager::CPlayerManager()
{
	m_vAllReservePlayers.clear() ;
	m_vAllActivePlayers.clear();
	m_vWillRemovePlayers.clear();
}

CPlayerManager::~CPlayerManager()
{
	MAP_PLAYERS::iterator iter = m_vAllActivePlayers.begin();
	for ( ; iter != m_vAllActivePlayers.end() ; ++iter )
	{
		delete iter->second ;
		iter->second = NULL ;
	}
	m_vAllActivePlayers.clear() ;

	LIST_PLAYERS::iterator iter_R = m_vAllReservePlayers.begin() ;
	for ( ; iter_R != m_vAllReservePlayers.end(); ++iter_R )
	{
		delete *iter_R ;
		*iter_R = NULL ;
	}
	m_vAllReservePlayers.clear() ;

	iter_R = m_vWillRemovePlayers.begin() ;
	for ( ; iter_R != m_vWillRemovePlayers.end(); ++iter_R )
	{
		delete *iter_R ;
		*iter_R = NULL ;
	}
	m_vWillRemovePlayers.clear() ;
}

bool CPlayerManager::OnMessage( RakNet::Packet* pMsg )
{
	stMsg* pMessage = (stMsg*)pMsg->data ;
	if ( pMessage->usMsgType == MSG_DISCONNECT_CLIENT )
	{
		stMsgClientDisconnect* pDisMsg = (stMsgClientDisconnect*)pMessage ;
		CPlayer* pPlayer = GetPlayerBySessionID(pDisMsg->nSeesionID) ;
		if ( PreProcessLogicMessage(pPlayer,pMessage,pDisMsg->nSeesionID ) )
		{
			return true ;
		}
	}

	if ( MSG_VERIFY_TANSACTION == pMessage->usMsgType )
	{
		stMsgFromVerifyServer* pR = (stMsgFromVerifyServer*)pMessage ;
		CPlayer* pPlayer = GetPlayerByUserUID(pR->nBuyerPlayerUserUID,false) ;
		if ( pPlayer )
		{
			pPlayer->OnMessage(pR) ;
		}
		else
		{
			CLogMgr::SharedLogMgr()->ErrorLog( "Can not find player to process verify result!" ) ;
			// post to its mail ;
			CPlayer* pPlayer = GetFirstActivePlayer();
			CPlayerMailComponent* pMail = (CPlayerMailComponent*)pPlayer->GetComponent(ePlayerComponent_Mail) ;
			pMail->PostUnprocessedPurchaseVerify(pR->nBuyerPlayerUserUID,pR->nBuyForPlayerUserUID,pR->nShopItemID,pR->nRet == 4 ) ;
		}
		return true ;
	}

	if ( pMessage->usMsgType == MSG_TRANSER_DATA )
	{
		CHECK_MSG_SIZE(stMsgTransferData,pMsg->length);
		stMsgTransferData* pTransMsg = (stMsgTransferData*)pMessage ;
		CPlayer* pPlayer = GetPlayerBySessionID(pTransMsg->nSessionID) ;
		// unpack message ;
		stMsg* pRealMsg = (stMsg*)(pMsg->data + sizeof(stMsgTransferData)) ;
		if ( PreProcessLogicMessage(pPlayer,pRealMsg,pTransMsg->nSessionID ) )
		{
			return true ;
		}

		if ( !pPlayer )
		{
			CLogMgr::SharedLogMgr()->ErrorLog( "Can not find Player with sessionID = %d,msgtype = %d, msg from = %d , ip = %s",pTransMsg->nSessionID,pRealMsg->usMsgType,pMessage->cSysIdentifer,pMsg->systemAddress.ToString(true) ) ;
			return false ;
		}
		pPlayer->OnMessage(pRealMsg) ;
		return true ;
	}
	else if ( ID_MSG_DB2GM == pMessage->cSysIdentifer )
	{
		if ( MSG_CREATE_ROLE == pMessage->usMsgType )
		{
			stMsgGameServerCreateRoleRet* pRet = (stMsgGameServerCreateRoleRet*)pMessage ;
			stMsgCreateRole msg ;
			memcpy(&msg,pRet,sizeof(msg));
			msg.cSysIdentifer = ID_MSG_GAME2C ;
			CGameServerApp::SharedGameServerApp()->SendMsgToGateServer(pRet->nSessionID,(char*)&msg,sizeof(msg)) ;
			return true ;
		}
		stMsgDB2GM* pRet = (stMsgDB2GM*)pMessage ;
		CPlayer* pPlayer = GetPlayerBySessionID(pRet->nSessionID ) ;
		if ( pPlayer )
		{
			pPlayer->OnMessage(pMessage) ;
		}
		else
		{
			CLogMgr::SharedLogMgr()->PrintLog( "can not find player with sessionID = %d to process DB MG cSysIdentifer = %d ,usMsgType = %d ",pRet->nSessionID,pMessage->cSysIdentifer,pMessage->usMsgType ) ;
		}
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog( "Receive Unknown message cSysIdentifer = %d ,usMsgType = %d ",pMessage->cSysIdentifer,pMessage->usMsgType ) ;
	}
	return false ;
}

//void CPlayerManager::ProcessMsgFromDBServer(stMsg* pMessage ,RakNet::Packet* pMsg  )
//{
//	if ( MSG_TRANSER_DATA == pMessage->usMsgType ) // game server don't prcess tranfer msg from DBServer ;
//	{
//		stMsgTransferData* pMsgTransfer = (stMsgTransferData*)pMessage ;
//		stMsg* pTargetMessage = (stMsg*)(pMsg->data + sizeof(stMsgTransferData));
//		pMsgTransfer->cSysIdentifer = ID_MSG_GM2GA ;
//		if ( pTargetMessage->usMsgType == MSG_LOGIN )  // if login ok , we alloct player ;
//		{
//			stMsgLoginRet* pTarget = (stMsgLoginRet*)pTargetMessage ;
//			if ( pTarget->bOk )
//			{
//				CPlayer* pPlayer = GetReserverPlayer();
//				pPlayer->Reset(pMsgTransfer->nTargetPeerUID) ;
//				m_vAllActivePlayers[pPlayer->GetUserUID()] = pPlayer ;
//			}
//		}
//		SendMsgToGateServer(pMsgTransfer->nTargetPeerUID,(char*)pTargetMessage,pMsg->length - sizeof(stMsgTransferData) ) ;
//	}
//	else
//	{
//		stMsgDB2GM* pMsgFromDB = (stMsgDB2GM*)pMessage ;
//		CPlayer* pTargetPlayer = GetPlayerByUserUID(pMsgFromDB->nTargetUserUID ) ;
//		if ( !pTargetPlayer )
//		{
//			CLogMgr::SharedLogMgr()->ErrorLog( "Can not find target Player, so message From DBServer will not be processed" );
//			return ;
//		}
//		pTargetPlayer->OnMessage(pMessage);
//	}
//}
//
//void CPlayerManager::processMsgFromGateServer(stMsg* pMessage ,RakNet::Packet* pMsg  )
//{
//	if ( MSG_TRANSER_DATA == pMessage->usMsgType )
//	{
//		stMsgTransferData* pMsgTransfer = (stMsgTransferData*)pMessage ;
//		stMsg* pTargetMessage = (stMsg*)(pMsg->data + sizeof(stMsgTransferData));
//		// special msg ; game server don't process , just send it to DBServer ;
//		if ( pTargetMessage->usMsgType == MSG_REGISTE || MSG_LOGIN == pTargetMessage->usMsgType )
//		{
//			SendMsgToDBServer((char*)pMsg->data,pMsg->length) ; 
//			return  ;
//		}
//		// special msg 
//		CPlayer* pTargetPlayer = GetPlayerByUserUID(pMsgTransfer->nTargetPeerUID) ;
//		if ( !pTargetPlayer )
//		{
//			CLogMgr::SharedLogMgr()->ErrorLog( "Can not find target Player, so message From Gate will not be processed" );
//			return  ;
//		}
//		pTargetPlayer->OnMessage(pTargetMessage);
//	}
//	else if (MSG_DISCONNECT == pMessage->usMsgType)
//	{
//		stMsgPeerDisconnect* pRealMsg = (stMsgPeerDisconnect*)pMessage ;
//		MAP_PLAYERS::iterator iter = m_vAllActivePlayers.find(pRealMsg->nPeerUID) ;
//		//CPlayer* pTargetPlayer = GetPlayerByUserUID(pRealMsg->nPeerUID) ;
//		if ( iter == m_vAllActivePlayers.end())
//		{
//			//CLogMgr::SharedLogMgr()->ErrorLog( "Can not find target Player, so Disconnected message From Gate will not be processed" );
//		}
//		else
//		{
//			CPlayer* pTargetPlayer = iter->second ;
//			if ( pTargetPlayer)
//			{
//				pTargetPlayer->OnDisconnect();
//				PushReserverPlayers( pTargetPlayer );
//			}
//			m_vAllActivePlayers.erase(iter) ;
//		}
//		// tell DBserver this peer discannected ;
//		pRealMsg->cSysIdentifer = ID_MSG_GM2DB ;
//		SendMsgToDBServer((char*)pMsg->data,pMsg->length) ;
//	}
//}

//bool CPlayerManager::OnLostSever(bool bGateDown)
//{
//	MAP_PLAYERS::iterator iter = m_vAllActivePlayers.begin();
//	for ( ; iter != m_vAllActivePlayers.end(); ++iter )
//	{
//		if ( iter->second )
//		{
//			if ( bGateDown )
//			{
//				iter->second->OnGateServerLost();
//				stMsgPeerDisconnect pRealMsg;
//				// tell DBserver this peer discannected ;
//				pRealMsg.cSysIdentifer = ID_MSG_GM2DB ;
//				pRealMsg.nPeerUID = iter->first ;
//				SendMsgToDBServer((char*)&pRealMsg,sizeof(stMsgPeerDisconnect)) ;
//				// remove the peer ;
//				PushReserverPlayers(iter->second) ;
//			}
//			else 
//			{
//				iter->second->OnDBServerLost();
//			}
//		}
//	}
//
//	if ( bGateDown )
//	{
//		m_vAllActivePlayers.clear();
//	}
//	return false ;
//}

bool CPlayerManager::PreProcessLogicMessage( CPlayer*pPlayer ,stMsg* pmsg , unsigned int nSessionID  )
{
	// process new player comin ;
	if ( pmsg->usMsgType == MSG_CREATE_ROLE && pmsg->cSysIdentifer == ID_MSG_C2GAME )
	{
		stMsgCreateRole* pCreate = (stMsgCreateRole*)pmsg ;
		stMsgGameServerCreateRole msg ;
		memcpy(&msg,pCreate,sizeof(stMsgCreateRole));
		msg.cSysIdentifer = ID_MSG_GM2DB;
		msg.nSessionID = nSessionID ;
		CGameServerApp::SharedGameServerApp()->SendMsgToDBServer((char*)&msg,sizeof(msg)) ;
		return true;
	}

	if ( pmsg->usMsgType == MSG_PLAYER_ENTER_GAME )
	{
		stMsgPlayerEnterGame* pmsgenter = (stMsgPlayerEnterGame*)pmsg ;
		
		if ( pPlayer != NULL && pPlayer->GetUserUID() == pmsgenter->nUserUID )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("double nSession, this nSessionID already have player ? ") ;
			return true ;
		}

		if ( pPlayer != NULL && pPlayer->GetUserUID() != pmsgenter->nUserUID ) // switch account in main scene 
		{
			// disconnect pre player 
			CEventCenter::SharedEventCenter()->PostEvent(eEvent_PlayerOffline,pPlayer) ;
			pPlayer->OnPlayerDisconnect() ;
			{
				RemovePlayer(pPlayer,pPlayer->GetState() == CPlayer::ePlayerState_Free);
			}
		}

		if ( ProcessIsAlreadyLogin(pmsgenter->nUserUID,nSessionID) )
		{
			return true ;
		}

		CPlayer* pNew = GetReserverPlayer();
		if ( pNew )
		{
			pNew->Reset(pmsgenter->nUserUID,nSessionID) ;
		}
		else
		{
			pNew = new CPlayer ;
			pNew->Init(pmsgenter->nUserUID,nSessionID ) ;
		}
		AddPlayer(pNew) ;
		return true ;
	}

	// process player logout ;
	if ( MSG_DISCONNECT_CLIENT == pmsg->usMsgType )
	{
		if ( pPlayer )
		{
			// post online event ;
			CEventCenter::SharedEventCenter()->PostEvent(eEvent_PlayerOffline,pPlayer) ;

			pPlayer->OnPlayerDisconnect() ;
			{
				RemovePlayer(pPlayer,pPlayer->GetState() == CPlayer::ePlayerState_Free);
			}
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
	return false ;
}

CPlayer* CPlayerManager::GetPlayerBySessionID( unsigned int nSessionID )
{
	MAP_PLAYERS::iterator iter = m_vAllActivePlayers.find(nSessionID) ;
	if ( iter != m_vAllActivePlayers.end())
		return iter->second ;
	return NULL ;
}

void CPlayerManager::Update(float fDeta )
{
	ProcessWillRemovePlayer();
}

CPlayer* CPlayerManager::GetFirstActivePlayer()
{
	MAP_PLAYERS::iterator iter = m_vAllActivePlayers.begin() ;
	if ( iter == m_vAllActivePlayers.end() || iter->second == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("first actvie player is NULL") ;
		return NULL ;
	}
	return iter->second ;
}

CPlayer* CPlayerManager::GetPlayerByUserUID( unsigned int nUserUID, bool IgnorWillRemovePlayer )
{
	MAP_PLAYERS::iterator iter = m_vAllActivePlayers.begin() ;
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

	if ( IgnorWillRemovePlayer )
	{
		return NULL ;
	}

	LIST_PLAYERS::iterator iter_L = m_vWillRemovePlayers.begin() ;
	for ( ; iter_L != m_vWillRemovePlayers.end(); ++iter_L )
	{
		if ( *iter_L )
		{
			if ( nUserUID == (*iter_L)->GetUserUID() )
			{
				return *iter_L ;
			}
		}
	}
	return NULL ;
}

void CPlayerManager::PushReserverPlayers( CPlayer* pPlayer )
{
	 if ( m_vAllReservePlayers.size() >= RESEVER_GAME_SERVER_PLAYERS )
	 {
		 delete pPlayer ;
		 return ;
	 }
	 m_vAllReservePlayers.push_back(pPlayer);
}

CPlayer* CPlayerManager::GetReserverPlayer()
{
	CPlayer* pPlayer = NULL ;
	LIST_PLAYERS::iterator iter = m_vAllReservePlayers.begin();
	if ( iter != m_vAllReservePlayers.end() )
	{
		CPlayer* pPlayer = *iter ;
		m_vAllReservePlayers.erase(iter) ;
		return pPlayer ;
	}
	return pPlayer ;
}

void CPlayerManager::RemovePlayer(CPlayer*pPlayer, bool bRightNow )
{
	if ( !pPlayer )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Can not Remove NULL player !") ;
		return ;
	}
	MAP_PLAYERS::iterator iter = m_vAllActivePlayers.find(pPlayer->GetSessionID() ) ;
	if ( iter != m_vAllActivePlayers.end() )
	{
		if ( bRightNow )
		{
			PushReserverPlayers(pPlayer) ;
		}
		else
		{
			m_vWillRemovePlayers.push_back(pPlayer) ;
		}
		m_vAllActivePlayers.erase(iter) ; 
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Remove Player don't exist in active map ! , player UID = %d",pPlayer->GetUserUID() ) ;
		delete pPlayer ;
		pPlayer = NULL ;
	}
}

void CPlayerManager::AddPlayer(CPlayer*pPlayer)
{
	if ( !pPlayer )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Can not Add NULL player !") ;
		return ;
	}
	MAP_PLAYERS::iterator iter = m_vAllActivePlayers.find(pPlayer->GetSessionID() ) ;
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

void CPlayerManager::ProcessWillRemovePlayer()
{
	LIST_PLAYERS vRemove ;
	LIST_PLAYERS::iterator iter = m_vWillRemovePlayers.begin();
	for ( ; iter != m_vWillRemovePlayers.end(); ++iter )
	{
		if ( (*iter)->GetState() == CPlayer::ePlayerState_Free )
		{
			vRemove.push_back(*iter) ;
		}
	}

	iter = vRemove.begin();
	for ( ; iter != vRemove.end() ; ++iter )
	{
		LIST_PLAYERS::iterator R = m_vWillRemovePlayers.begin() ;
		for ( ; R != m_vWillRemovePlayers.end(); ++R )
		{
			if ( (*iter) == (*R))
			{
				(*R)->OnPlayerDisconnect();
				PushReserverPlayers(*iter) ;
				m_vWillRemovePlayers.erase(R) ;
				break; 
			}
		}
	}
	vRemove.clear() ;
}

void CPlayerManager::LogState()
{
	CLogMgr::SharedLogMgr()->SystemLog( "Active Player: %d   WillRemove Player: %d  Reserver Player: %d" ,m_vAllActivePlayers.size(),m_vWillRemovePlayers.size(),m_vAllReservePlayers.size()) ;
}

bool CPlayerManager::ProcessIsAlreadyLogin(unsigned int nUserID ,unsigned nSessionID )
{
	// is active player
	CPlayer* pPlayer = NULL ;
	MAP_PLAYERS::iterator  iter_ma = m_vAllActivePlayers.begin() ;
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

	// is in will leave player ?
	LIST_PLAYERS::iterator iter = m_vWillRemovePlayers.begin();
	for ( ; iter != m_vWillRemovePlayers.end(); ++iter )
	{
		pPlayer = *iter ;
		if ( pPlayer == NULL )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("Why empty m_vWillRemovePlayers player here ?") ;
			continue; 
		}

		if ( pPlayer->GetUserUID() == nUserID )
		{
			pPlayer->OnAnotherClientLoginThisPeer(nSessionID) ;
			m_vWillRemovePlayers.erase(iter) ;
			AddPlayer(pPlayer) ;
			return true ;
		}
	}
	return false ;
}