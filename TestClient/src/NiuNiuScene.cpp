#include "NiuNiuScene.h"
#include "Client.h"
#include "PlayerData.h"
#include "LoginScene.h"
CNiuNiuScene::CNiuNiuScene(CClientRobot* pClient)
	:IScene(pClient)
{
	
}

bool CNiuNiuScene::init(const char* cRobotAiFile)
{
	m_tData.init(this);
	return true ;
}

bool CNiuNiuScene::OnMessage( Packet* pPacket ) 
{
	if ( IScene::OnMessage(pPacket) )
	{
		return true ;
	}	
	stMsg* pMsg = (stMsg*)pPacket->_orgdata ;
	if ( MSG_PLAYER_LEAVE_ROOM == pMsg->usMsgType )
	{
		auto pLogin = new CLoginScene(getClient());
		getClient()->ChangeScene(pLogin) ;
		printf("change to login scene") ;
		return true;
	}
	m_tData.onMsg(pMsg);
	return true ;
}

void CNiuNiuScene::OnUpdate(float fDeltaTime ) 
{
	IScene::OnUpdate(fDeltaTime);
	m_tData.getRobotControl()->update(fDeltaTime);
}