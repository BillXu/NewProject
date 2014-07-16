#include "ClientManager.h"
#include "GateClient.h"
#include "MessageDefine.h"
#include "LogManager.h"
#include "CommonDefine.h"
#include "ServerNetwork.h"
#include "GateServer.h"
#include "ServerMessageDefine.h"
unsigned int CGateClientMgr::s_nSeesionIDProducer = 0 ;
CGateClientMgr::CGateClientMgr()
{
	m_vNetWorkIDGateClient.clear();
	m_vSessionGateClient.clear() ;
	m_vGateClientReserver.clear();
	memset(m_pMsgBuffer,0,MAX_MSG_BUFFER_LEN) ;
}

CGateClientMgr::~CGateClientMgr()
{
	MAP_SESSIONID_GATE_CLIENT::iterator iterS = m_vSessionGateClient.begin();
	for ( ; iterS != m_vSessionGateClient.end(); ++iterS )
	{
		delete  iterS->second ;
	}
	m_vSessionGateClient.clear() ;

	// just clear ; object deleted in session Gate ;
	m_vNetWorkIDGateClient.clear() ;

	LIST_GATE_CLIENT::iterator iter = m_vGateClientReserver.begin() ;
	for ( ; iter != m_vGateClientReserver.end(); ++iter )
	{
		delete *iter ;
	}
	m_vGateClientReserver.clear() ;
}

bool CGateClientMgr::OnMessage( RakNet::Packet* pData )
{
	// verify identify 
	stMsg* pMsg = (stMsg*)pData->data ;
	CHECK_MSG_SIZE(stMsg,pData->length);
	if ( pMsg->cSysIdentifer == ID_MSG_VERIFY )
	{
		if ( MSG_VERIFY_GAME == pMsg->usMsgType )
		{
			m_nGameServerNetWorkID = pData->guid ;
			CLogMgr::SharedLogMgr()->SystemLog("Game Server connected ip = %s",pData->systemAddress.ToString(true)) ;
		}
		else if ( MSG_VERIFY_LOGIN == pMsg->usMsgType )
		{
			m_nLoginServerNetWorkID = pData->guid ;
			CLogMgr::SharedLogMgr()->SystemLog("Login Server connected ip = %s",pData->systemAddress.ToString(true)) ;
		}
		else if ( MSG_VERIFY_CLIENT == pMsg->usMsgType )
		{
			stGateClient* pGateClient = GetReserverGateClient();
			if ( !pGateClient )
			{
				pGateClient = new stGateClient ;
			}
			pGateClient->Reset(++s_nSeesionIDProducer,pData->guid) ;
			AddClientGate(pGateClient);
			CLogMgr::SharedLogMgr()->SystemLog("a Client connected ip = %s Session id = %d",pData->systemAddress.ToString(false),pGateClient->nSessionId) ;
		}
		else 
		{
			CLogMgr::SharedLogMgr()->SystemLog("Unknown identify from ip = %s",pData->systemAddress.ToString() ) ;
		}
		return true;
	}

	// client reconnect ;
	if ( MSG_RECONNECT == pMsg->usMsgType )
	{
		stMsgReconnect* pRet = (stMsgReconnect*)pMsg ;
		CHECK_MSG_SIZE(stMsgReconnect,pData->length);
		stGateClient* pGateClient = GetGateClientBySessionID(pRet->nSessionID) ;
		if ( pGateClient )
		{
			stGateClient* pNew = GetGateClientByNetWorkID(pData->guid);
			if ( pNew )
			{
				RemoveClientGate(pNew) ;
			}

			MAP_NETWORKID_GATE_CLIENT::iterator iter = m_vNetWorkIDGateClient.find(pGateClient->nNetWorkID);
			if ( iter != m_vNetWorkIDGateClient.end() )
			{
				m_vNetWorkIDGateClient.erase(iter) ;
			}
			else
			{
				CLogMgr::SharedLogMgr()->ErrorLog("why net peernet work id is NULL") ;
			}
			// update network id ;
			pGateClient->SetNewWorkID(pData->guid) ;
			m_vNetWorkIDGateClient[pGateClient->nNetWorkID] = pGateClient ;
		}
		stMsgReconnectRet msgback ;
		msgback.nRet = (pGateClient == NULL ? 1 : 0 ) ;
		// send msg to client ;
		CGateServer::SharedGateServer()->GetNetWork()->SendMsg((char*)&msgback,sizeof(msgback),pData->guid,false) ;
		if ( pGateClient )
		{
			CLogMgr::SharedLogMgr()->SystemLog("MSG¡¡reconnected ! session id = %d",pGateClient->nSessionId);
		}
		return true ;
	}

	// distribute msg 
	if ( ID_MSG_C2LOGIN == pMsg->cSysIdentifer || ID_MSG_C2GAME == pMsg->cSysIdentifer ) // send to Login Server  // send to game server 
	{
		stGateClient* pDstClient = GetGateClientByNetWorkID(pData->guid) ;
		if ( pDstClient == NULL )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("can not send message to Server , client is NULL") ;
			return true ;
		}

		if ( CheckServerStateOk(pDstClient,ID_MSG_C2LOGIN == pMsg->cSysIdentifer ? eSvrType_Login: eSvrType_Game ) == false )
		{
			return true ;
		}

		stMsgTransferData msgTransData ;
		msgTransData.bBroadCast = false ;
		msgTransData.nSessionID = pDstClient->nSessionId ;
		int nLne = sizeof(msgTransData) ;
		memcpy(m_pMsgBuffer,&msgTransData,nLne);
		memcpy(m_pMsgBuffer + nLne , pData->data,pData->length );
		nLne += pData->length ;
		CGateServer::SharedGateServer()->GetNetWork()->SendMsg(m_pMsgBuffer,nLne,(ID_MSG_C2LOGIN == pMsg->cSysIdentifer?m_nLoginServerNetWorkID:m_nGameServerNetWorkID),false);
	}
	else if ( MSG_TRANSER_DATA == pMsg->usMsgType ) // Distribute to client ;
	{
		CHECK_MSG_SIZE(stMsgTransferData,pData->length);
		stMsgTransferData* pTransData = (stMsgTransferData*)pMsg ;
		stGateClient* pDstClient = GetGateClientBySessionID(pTransData->nSessionID) ;
		if ( pDstClient == NULL )
		{
			stMsgClientDisconnect msg ;
			msg.nSeesionID = pTransData->nSessionID ;
			CGateServer::SharedGateServer()->GetNetWork()->SendMsg((char*)&msg,sizeof(msg),m_nGameServerNetWorkID,false) ;
			CGateServer::SharedGateServer()->GetNetWork()->SendMsg((char*)&msg,sizeof(msg),m_nLoginServerNetWorkID,false) ;
			stMsg* pm = (stMsg*)((char*)pData->data + sizeof(stMsgTransferData) );
			CLogMgr::SharedLogMgr()->ErrorLog("can not send message to client , client is NULL, so inform disconnected session id = %d, msg = %d",pTransData->nSessionID,pm->usMsgType) ;
			return true ;
		}
		char* pSendData = (char*)pData->data ;
		if ( pTransData->bBroadCast )
		{
			CGateServer::SharedGateServer()->GetNetWork()->SendMsg(pSendData + sizeof(stMsgTransferData),pData->length - sizeof(stMsgTransferData),m_nGameServerNetWorkID,pTransData->bBroadCast );
		}
		else
		{
			CGateServer::SharedGateServer()->GetNetWork()->SendMsg(pSendData + sizeof(stMsgTransferData),pData->length - sizeof(stMsgTransferData),pDstClient->nNetWorkID,pTransData->bBroadCast );
		}
		
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Unknown Distribute Destination , %d ",pMsg->cSysIdentifer ) ;
	}
	return true ;
}

void CGateClientMgr::OnNewPeerConnected(RakNet::RakNetGUID& nNewPeer, RakNet::Packet* pData )
{
	stMsg msg ;
	msg.cSysIdentifer = ID_MSG_VERIFY ;
	msg.usMsgType = MSG_VERIFY_GATE ;
	// send msg to peer ;
	CGateServer::SharedGateServer()->GetNetWork()->SendMsg((char*)&msg,sizeof(msg),pData->guid,false) ;
}

void CGateClientMgr::OnClientWaitReconnectTimeUp(stGateClient* pClient )
{
	RemoveClientGate(pClient);
}

void CGateClientMgr::OnPeerDisconnected(RakNet::RakNetGUID& nPeerDisconnected, RakNet::Packet* pData )
{
	// client disconnected ;
	stGateClient* pDstClient = GetGateClientByNetWorkID(pData->guid) ;
	if ( pDstClient )
	{
		if ( pData->data[0] == ID_DISCONNECTION_NOTIFICATION )
		{
			CLogMgr::SharedLogMgr()->SystemLog("client disconnect session id = %d ",pDstClient->nSessionId) ;
			OnClientWaitReconnectTimeUp(pDstClient) ;
		}
		else
		{
			CLogMgr::SharedLogMgr()->ErrorLog("client lost ip = %s session id = %d ",pData->systemAddress.ToString(false),pDstClient->nSessionId) ;
			pDstClient->StartWaitForReconnect() ;
		}
		
		return ;
	}

	// game server disconnected 
	if ( pData->guid == m_nGameServerNetWorkID )
	{
		m_nGameServerNetWorkID = RakNet::UNASSIGNED_RAKNET_GUID ;
		CLogMgr::SharedLogMgr()->ErrorLog("game server disconnect ") ;
		return ;
	}

	// login server disconnect 
	if ( pData->guid == m_nLoginServerNetWorkID )
	{
		m_nLoginServerNetWorkID = RakNet::UNASSIGNED_RAKNET_GUID ;
		CLogMgr::SharedLogMgr()->ErrorLog("Login Server disconnected") ;
		return ;
	}

	CLogMgr::SharedLogMgr()->SystemLog("not verify peer disconnected ip = %s",pData->systemAddress.ToString(true) ) ;
}

void CGateClientMgr::AddClientGate(stGateClient* pGateClient )
{
	m_vNetWorkIDGateClient[pGateClient->nNetWorkID] = pGateClient ;
	m_vSessionGateClient[pGateClient->nSessionId] = pGateClient ;
}

void CGateClientMgr::RemoveClientGate(stGateClient* pGateClient )
{
	if ( pGateClient == NULL )
		return ;

	stMsgClientDisconnect msg ;
	msg.nSeesionID = pGateClient->nSessionId ;
	CGateServer::SharedGateServer()->GetNetWork()->SendMsg((char*)&msg,sizeof(msg),m_nGameServerNetWorkID,false) ;
	CGateServer::SharedGateServer()->GetNetWork()->SendMsg((char*)&msg,sizeof(msg),m_nLoginServerNetWorkID,false) ;

	m_vNetWorkIDGateClient.erase(m_vNetWorkIDGateClient.find(pGateClient->nNetWorkID)) ;
	m_vSessionGateClient.erase(m_vSessionGateClient.find(pGateClient->nSessionId)) ;
	RakNet::RakNetGUID guid = RakNet::UNASSIGNED_RAKNET_GUID ;
	pGateClient->Reset(0,guid) ;
	m_vGateClientReserver.push_back(pGateClient) ;
}

stGateClient* CGateClientMgr::GetReserverGateClient()
{
	stGateClient* pGateClient = NULL ;
	if ( m_vGateClientReserver.empty() == false )
	{
		pGateClient = m_vGateClientReserver.front() ;
		m_vGateClientReserver.erase(m_vGateClientReserver.begin()) ;
	}
	return pGateClient ;
}

stGateClient* CGateClientMgr::GetGateClientBySessionID(unsigned int nSessionID)
{
	MAP_SESSIONID_GATE_CLIENT::iterator iter = m_vSessionGateClient.find(nSessionID) ;
	if ( iter == m_vSessionGateClient.end() )
		return NULL ;
	return iter->second ;
}

stGateClient* CGateClientMgr::GetGateClientByNetWorkID(RakNet::RakNetGUID& nNetWorkID )
{
	MAP_NETWORKID_GATE_CLIENT::iterator iter = m_vNetWorkIDGateClient.find(nNetWorkID) ;
	if ( iter != m_vNetWorkIDGateClient.end() )
		return iter->second ;
	return NULL ;
}

bool CGateClientMgr::CheckServerStateOk( stGateClient* pClient, eServerType eCheckType )
{
	stMsgServerDisconnect msg ;
	if ( (eCheckType == eSvrType_Login || eCheckType == eSvrType_Max ) && m_nLoginServerNetWorkID == RakNet::UNASSIGNED_RAKNET_GUID )  // login server disconnect ;
	{
		
		msg.nServerType = eSvrType_Login ;
		CGateServer::SharedGateServer()->GetNetWork()->SendMsg((char*)&msg,sizeof(msg),pClient->nNetWorkID,false) ;
		return false;
	}
	
	if ( (eCheckType == eSvrType_Game || eCheckType == eSvrType_Max )  && RakNet::UNASSIGNED_RAKNET_GUID == m_nGameServerNetWorkID ) // game server disconnect 
	{
		msg.nServerType = eSvrType_Game ;
		CGateServer::SharedGateServer()->GetNetWork()->SendMsg((char*)&msg,sizeof(msg),pClient->nNetWorkID,false) ;
		return false ;
	}

	return true ;
}
