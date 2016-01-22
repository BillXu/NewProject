#include "NiuNiuScene.h"
CNiuNiuScene::CNiuNiuScene(CClientRobot* pClient)
	:IScene(pClient)
{

}

bool CNiuNiuScene::init(const char* cRobotAiFile)
{
	m_tRobotControler.init(this) ;
	return true ;
}

bool CNiuNiuScene::OnMessage( Packet* pPacket ) 
{
	if ( IScene::OnMessage(pPacket) )
	{
		return true ;
	}	
	stMsg* pMsg = (stMsg*)pPacket->_orgdata ;
	m_tData.onMsg(pMsg);
	m_tRobotControler.onMsg(pMsg);
	switch ( pMsg->usMsgType )
	{
	case MSG_ROOM_ENTER_NEW_STATE:
		{
			stMsgRoomEnterNewState* pRet = (stMsgRoomEnterNewState*)pMsg ;
			if ( pRet->nNewState == eRoomState_NN_TryBanker )
			{
				m_tRobotControler.onCanTryBanker();
			}
			else if ( eRoomState_NN_StartBet == pRet->nNewState )
			{
				m_tRobotControler.onCanBet() ;
			}
			else if ( eRoomState_NN_GameResult == pRet->nNewState )
			{
				m_tRobotControler.onGameEnd() ;
			}
		}
		break;
	case MSG_NN_DISTRIBUTE_4_CARD:
		{
			m_tData.onStartNewGame() ;
		}
		break;
	default:
		return false ;
	}
	return true ;
}

void CNiuNiuScene::OnUpdate(float fDeltaTime ) 
{
	IScene::OnUpdate(fDeltaTime);
	m_tRobotControler.update(fDeltaTime);
}