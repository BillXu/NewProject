#include "LoginPeerMgr.h"
#include "LoginPeer.h"
#include "LoginApp.h"
#include "MessageDefine.h"
#include "LogManager.h"
#include "ServerMessageDefine.h"
CLoginPeerMgr::CLoginPeerMgr(CLoginApp* app )
{
	ClearAll();
	m_pApp = app ;
}

CLoginPeerMgr::~CLoginPeerMgr()
{
	ClearAll();
}

void CLoginPeerMgr::OnMessage(RakNet::Packet* pMsg )
{
	CHECK_MSG_SIZE_VOID(stMsg,pMsg->length);
	
	stMsg* pRet = (stMsg*)pMsg->data;
	if ( pRet->usMsgType == MSG_TRANSER_DATA )
	{
		CHECK_MSG_SIZE_VOID(stMsgTransferData,pMsg->length);
		stMsgTransferData* pReal = (stMsgTransferData*)pRet ;
		char* pbuffer = (char*)pReal ;
		pbuffer += sizeof(stMsgTransferData) ;
		OnGateMessage((stMsg*)pbuffer,pReal->nSessionID ) ;
		return ;
	}

	if ( pRet->cSysIdentifer == ID_MSG_DB2LOGIN )
	{
		OnDBMessage(pRet) ;
		return ;
	}

	if ( MSG_DISCONNECT_CLIENT == pRet->usMsgType )
	{
		return ;
	}

	CLogMgr::SharedLogMgr()->PrintLog("unprocessed msg type = %d",pRet->usMsgType ) ;
	return ;
}

void CLoginPeerMgr::OnGateMessage(stMsg* pmsg ,unsigned int nSessionID )
{
	switch ( pmsg->usMsgType )
	{
	case MSG_PLAYER_REGISTER:
		{
			stMsgRegister* pMsg = (stMsgRegister*)pmsg ;

			stMsgLoginRegister msgRegister ;
			memcpy(&msgRegister,pMsg,sizeof(stMsgRegister)) ;
			msgRegister.nSessionID = nSessionID ;
			msgRegister.cSysIdentifer = ID_MSG_LOGIN2DB ;
			SendMsgToDB((char*)&msgRegister, sizeof(msgRegister) ) ;
		}
		break;
	case MSG_PLAYER_CHECK_ACCOUNT:
		{
			stMsgCheckAccount* pMsgCheck = (stMsgCheckAccount*)pmsg ;
			stMsgLoginAccountCheck msg ;
			memcpy((char*)&msg,pMsgCheck,sizeof(stMsgCheckAccount));
			msg.nSessionID = nSessionID ;
			msg.cSysIdentifer = ID_MSG_LOGIN2DB ;
			SendMsgToDB((char*)&msg, sizeof(msg) ) ;
		}
		break;
	case MSG_PLAYER_BIND_ACCOUNT:
		{
			stMsgRebindAccount* pMsg = (stMsgRebindAccount*)pmsg ;
			stMsgLoginBindAccount msgSend ;
			memcpy(&msgSend,pMsg,sizeof(stMsgRebindAccount));
			msgSend.nSessionID = nSessionID ;
			msgSend.cSysIdentifer = ID_MSG_LOGIN2DB ;
			SendMsgToDB((char*)&msgSend, sizeof(msgSend) ) ;
		}
		break;
	case MSG_MODIFY_PASSWORD:
		{
			stMsgModifyPassword* pRet = (stMsgModifyPassword*)pmsg;
			stMsgLoginModifyPassword msg ;
			memcpy(&msg,pRet,sizeof(stMsgModifyPassword));
			msg.cSysIdentifer = ID_MSG_LOGIN2DB ;
			msg.nSessionID = nSessionID ;
			SendMsgToDB((char*)&msg, sizeof(msg) ) ;
		}
		break;
	default:
		{
			CLogMgr::SharedLogMgr()->ErrorLog("Unknown message from gate type = %d",pmsg->usMsgType );
			return ;
		}	   

	}
}

void CLoginPeerMgr::OnDBMessage(stMsg* pmsg )
{
	switch ( pmsg->usMsgType )
	{
	case MSG_PLAYER_REGISTER:
		{
			stMsgLoginRegisterRet* pMsgRet = (stMsgLoginRegisterRet*)pmsg ;
			stMsgTransferData msgData ;
			msgData.bBroadCast = false ;
			msgData.cSysIdentifer = ID_MSG_LOGIN2C ;
			msgData.nSessionID = pMsgRet->nSessionID ;
			
			stMsgRegisterRet msgclientRet ;
			memcpy(&msgclientRet,pMsgRet,sizeof(msgclientRet));
			msgclientRet.cSysIdentifer = ID_MSG_LOGIN2C;
		
			int nOffset = 0 ;
			memcpy(m_pMaxBuffer+ nOffset , &msgData, sizeof( msgData ) );
			nOffset += sizeof(msgData);
			
			memcpy(m_pMaxBuffer + nOffset, &msgclientRet , sizeof(msgclientRet) );
			nOffset += sizeof(msgclientRet);
			
			SendMsgToGate(m_pMaxBuffer, nOffset );
		}
		break;
	case MSG_PLAYER_CHECK_ACCOUNT:
		{
			stMsgLoginAccountCheckRet* pLoginCheckRet = (stMsgLoginAccountCheckRet*)pmsg ;
			
				
			stMsgCheckAccountRet msgcheckret ;
			memcpy(&msgcheckret,pLoginCheckRet,sizeof(msgcheckret));
			msgcheckret.cSysIdentifer = ID_MSG_LOGIN2C ;
			
			stMsgTransferData msgData ;
			msgData.bBroadCast = false ;
			msgData.nSessionID = pLoginCheckRet->nSessionID ;
			msgData.cSysIdentifer = ID_MSG_LOGIN2C ;
			
			int nOffset = 0 ;
			memcpy(m_pMaxBuffer, &msgData, sizeof(msgData));
			nOffset += sizeof(msgData);

			memcpy(m_pMaxBuffer+ nOffset , &msgcheckret,sizeof(msgcheckret));
			nOffset += sizeof(msgcheckret) ;
			SendMsgToGate(m_pMaxBuffer, nOffset) ;
		}
		break;
	case MSG_PLAYER_BIND_ACCOUNT:
		{
			stMsgLoginBindAccountRet* pRet = (stMsgLoginBindAccountRet*)pmsg;

			stMsgRebindAccountRet retMsg ;
			memcpy(&retMsg,pRet,sizeof(retMsg));
			retMsg.cSysIdentifer = ID_MSG_LOGIN2C ;
			
			stMsgTransferData msgData ;
			msgData.bBroadCast = false ;
			msgData.nSessionID = pRet->nSessionID ;
			msgData.cSysIdentifer = ID_MSG_LOGIN2C ;

			int nOffset = 0 ;
			memcpy(m_pMaxBuffer, &msgData, sizeof(msgData));
			nOffset += sizeof(msgData);

			memcpy(m_pMaxBuffer+ nOffset , &retMsg,sizeof(retMsg));
			nOffset += sizeof(retMsg) ;
			SendMsgToGate(m_pMaxBuffer, nOffset) ;
		}
		break;
	case MSG_MODIFY_PASSWORD:
		{
			stMsgLoginModifyPasswordRet* pRet = (stMsgLoginModifyPasswordRet*)pmsg ;
			stMsgModifyPasswordRet retMsg ;
			memcpy(&retMsg,pRet,sizeof(retMsg));
			retMsg.cSysIdentifer = ID_MSG_LOGIN2C ;

			stMsgTransferData msgData ;
			msgData.bBroadCast = false ;
			msgData.nSessionID = pRet->nSessionID ;
			msgData.cSysIdentifer = ID_MSG_LOGIN2C ;

			int nOffset = 0 ;
			memcpy(m_pMaxBuffer, &msgData, sizeof(msgData));
			nOffset += sizeof(msgData);

			memcpy(m_pMaxBuffer+ nOffset , &retMsg,sizeof(retMsg));
			nOffset += sizeof(retMsg) ;
			SendMsgToGate(m_pMaxBuffer, nOffset) ;
		}
		break;
	default:
		{
			CLogMgr::SharedLogMgr()->ErrorLog("Unknown message from DB type = %d",pmsg->usMsgType );
			return ;
		}	   

	}
}

CLoginPeer* CLoginPeerMgr::GetPeerBySessionID(unsigned int nSessionID )
{
	MAP_LOGIN_PEER::iterator iter = m_vAllPeers.find(nSessionID) ;
	if ( iter != m_vAllPeers.end())
	{
		return iter->second ;
	}
	return NULL ;
}

CLoginPeer* CLoginPeerMgr::GetReserverPeer()
{
	if ( m_vReseverLoginPeers.empty() )
	{
		return NULL ;
	}
	LIST_LOGIN_PEER::iterator iter = m_vReseverLoginPeers.begin() ;
	CLoginPeer* pPeer = *iter ;
	m_vReseverLoginPeers.erase(iter) ;
	return pPeer ;
}

bool CLoginPeerMgr::SendMsgToDB(const char* pBuffer , int nLen )
{
	// net work object 
	m_pApp->SendMsg(pBuffer,nLen,false);
	return true ;
}

bool CLoginPeerMgr::SendMsgToGate(const char* pBuffer , int nLen )
{
	// net work object
	m_pApp->SendMsg(pBuffer,nLen,true);
	return true ;
}

void CLoginPeerMgr::ClearAll()
{
	MAP_LOGIN_PEER::iterator iter = m_vAllPeers.begin() ;
	for ( ; iter != m_vAllPeers.end(); ++iter )
	{
		if ( iter->second )
		{
			delete iter->second ;
			iter->second = NULL ;
		}

	}
	m_vAllPeers.clear() ;

	LIST_LOGIN_PEER::iterator iter_list = m_vReseverLoginPeers.begin() ;
	for ( ; iter_list != m_vReseverLoginPeers.end(); ++iter_list )
	{
		if ( *iter_list )
		{
			delete *iter_list ;
		}
	}
	m_vReseverLoginPeers.clear() ;
}