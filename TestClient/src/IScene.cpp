#include "IScene.h"
#include "MessageDefine.h"
#include "Client.h"
void IScene::OnEnterScene()
{
	if ( m_bRunning )return ;
	m_pClient->GetNetWork()->AddMessageDelegate(this) ;
}

void IScene::OnEixtScene()
{
	if ( !m_bRunning )
		return ;
	m_pClient->GetNetWork()->RemoveMessageDelegate(this);
}

bool IScene::OnMessage( RakNet::Packet* pPacket )
{
	stMsg* pMsg = (stMsg*)pPacket->data ;
	switch(pMsg->cSysIdentifer)
	{
	case ID_MSG_VERIFY:
		{
		}
		break; 
	default:
		break; ;
	}
	return false ;
}

bool IScene::OnLostSever(RakNet::Packet* pPacket )
{
	m_bServerConnected = false ;
	//CLogMgr::SharedLogMgr()->PrintLog("Server Disconnect , trying to connected ...!");
	// reconnected to Server ;
	return false ;
}

bool IScene::OnConnectStateChanged( eConnectState eSate, RakNet::Packet* pMsg)
{
	if ( eConnect_Accepted == eSate )
	{
		// send client verify ;
		stMsg msg ;
		msg.cSysIdentifer = ID_MSG_VERIFY ;
		msg.usMsgType = MSG_VERIFY_CLIENT ;
		m_pClient->GetNetWork()->SendMsg((char*)&msg,sizeof(msg));
		Verifyed();
	}
	return false ;
}

void IScene::SendMsg(char* pBuffer , unsigned short nLen )
{
	m_pClient->GetNetWork()->SendMsg(pBuffer,nLen);
}