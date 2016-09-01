#include "IScene.h"
#include "MessageDefine.h"
#include "Client.h"
#include "log4z.h"
void IScene::OnEnterScene()
{
	if ( m_bRunning )return ;
	m_bRunning = true ;
}

void IScene::OnEixtScene()
{
	if ( !m_bRunning )
		return ;
	m_bRunning = false ;
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

bool IScene::onMessage(stMsg* pmsg)
{
	return false ;
}

bool IScene::onMessage(Json::Value& jsContent )
{
	return false ;
}

void IScene::SendMsg(char* pBuffer , unsigned short nLen )
{
	m_pClient->GetNetWork()->SendMsg(pBuffer,nLen);
}