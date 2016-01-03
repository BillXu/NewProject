#include "IScene.h"
#include "MessageDefine.h"
#include "Client.h"
#include "LogManager.h"
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

bool IScene::OnMessage( Packet* pPacket )
{
	stMsg* pMsg = (stMsg*)pPacket->_orgdata ;
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

bool IScene::OnLostSever(Packet* pPacket )
{
	m_bServerConnected = false ;
	CLogMgr::SharedLogMgr()->PrintLog("Server Disconnect!");
	// reconnected to Server ;
	return false ;
}

bool IScene::OnConnectStateChanged( eConnectState eSate, Packet* pMsg )
{
	if ( eConnect_Accepted == eSate )
	{
		// send client verify ;
		Verifyed();
	}
	return false ;
}

void IScene::SendMsg(char* pBuffer , unsigned short nLen )
{
	m_pClient->GetNetWork()->SendMsg(pBuffer,nLen);
}