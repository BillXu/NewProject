#include "TaxasPokerScene.h"
#include "TaxasMessageDefine.h"
#include "Client.h"
#include "PlayerData.h"
#include "RobotAIConfig.h"
#include "CardPoker.h"
#include "LoginScene.h"
#include "RobotConfig.h"
#define TIME_ACTION_DELAY 7
CTaxasPokerScene::CTaxasPokerScene(CClientRobot* pClient)
	:IScene(pClient)
{ 
	m_eSceneType = eScene_TaxasPoker ;
}

bool CTaxasPokerScene::init(const char* cRobotAiFile)
{
	m_tData.init(this);	
	((CRobotControlTaxas*)(m_tData.getRobotControl()))->setAiFile(cRobotAiFile) ;
	return true ;
}

bool CTaxasPokerScene::OnMessage( Packet* pPacket )
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

void CTaxasPokerScene::OnUpdate(float fDeltaTime )
{
	IScene::OnUpdate(fDeltaTime);
	m_tData.getRobotControl()->update(fDeltaTime);
}
