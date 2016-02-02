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
	memset(&m_tData,0,sizeof(m_tData)) ;
	return m_tRobotControler.init(cRobotAiFile,this) ;
}

bool CTaxasPokerScene::OnMessage( Packet* pPacket )
{
	stMsg* pmsg = (stMsg*)pPacket->_orgdata ;
	if ( MSG_TP_ROOM_STAND_UP != pmsg->usMsgType )
	{
		m_tData.onMsg(pmsg);
	}

	switch ( pmsg->usMsgType )
	{
	case MSG_TP_ROOM_BASE_INFO:
		{
			stMsgTaxasRoomInfoBase* pRet = (stMsgTaxasRoomInfoBase*)pmsg ;
			if ( pRet->eCurRoomState == eRoomState_TP_WaitJoin )
			{
				m_tRobotControler.onEnterRoom();
			}
		}
		break;
	case MSG_TP_ROOM_PLAYER_DATA:
		{
			stMsgTaxasRoomInfoPlayerData* pRet = (stMsgTaxasRoomInfoPlayerData*)pmsg;
			if ( pRet->bIsLast )
			{
				m_tRobotControler.onEnterRoom();
			}
		}
		break;
	case MSG_TP_PLAYER_SIT_DOWN:
		{
			stMsgTaxasPlayerSitDownRet* pRet = (stMsgTaxasPlayerSitDownRet*)pmsg ;
			if ( pRet->nRet )
			{
				m_tRobotControler.onSitDownFailed(pRet->nRet);
			}
		}
		break;
	case MSG_TP_WITHDRAWING_MONEY:
		{
			stMsgWithdrawingMoneyRet* pRet = (stMsgWithdrawingMoneyRet*)pmsg ;
			if ( pRet->nRet )
			{
				m_tRobotControler.onWithdrawMoneyFailed();
			}
		}
		break;
	case MSG_TP_ROOM_SIT_DOWN:
		{
			stMsgTaxasRoomSitDown* pRet = (stMsgTaxasRoomSitDown*)pmsg ;
			if ( pRet->tPlayerData.nSeatIdx < MAX_PEERS_IN_TAXAS_ROOM )
			{
				if ( pRet->tPlayerData.nUserUID == getClientApp()->GetPlayerData()->getUserUID())
				{
					m_tRobotControler.onSelfSitDown(pRet->tPlayerData.nSeatIdx);
				}
			}
		}
		break;
	case MSG_TP_ROOM_STAND_UP:
		{
			stMsgTaxasRoomStandUp* pRet = (stMsgTaxasRoomStandUp*)pmsg ;
			if ( pRet->nSeatIdx < MAX_PEERS_IN_TAXAS_ROOM )
			{
				if ( pRet->nSeatIdx == m_tRobotControler.getMySeatIdx() )
				{
					m_tRobotControler.onSelfStandUp() ;
				}
			}

			m_tData.onMsg(pmsg);
		}
		break;
	case MSG_TP_WAIT_PLAYER_ACT:
		{
			stMsgTaxasRoomWaitPlayerAct* pRet = (stMsgTaxasRoomWaitPlayerAct*)pmsg ;
			if ( m_tRobotControler.getMySeatIdx() == pRet->nActPlayerIdx )
			{
				m_tRobotControler.waitAction() ;
			}
		}
		break;
	case MSG_TP_ROOM_ACT:
		{
			stMsgTaxasRoomAct* pret = (stMsgTaxasRoomAct*)pmsg;
			if ( pret->nPlayerIdx == m_tRobotControler.getMySeatIdx() )
			{
				if ( pret->nPlayerAct == eRoomPeerAction_GiveUp )
				{
					m_tRobotControler.onSelfGiveUp() ;
				}
			}
		}
		break;
	case MSG_TP_ONE_BET_ROUND_RESULT:
		{
			getPokerData()->resetBetRoundState();
		}
		break;
	case MSG_TP_GAME_RESULT:
		{
			stMsgTaxasRoomGameResult* pRet = (stMsgTaxasRoomGameResult*)pmsg ;
			if ( pRet->bIsLastOne )
			{
				printf("recive last result\n") ;
				getPokerData()->resetRuntimeData();
				m_tRobotControler.onGameEnd(m_tData.getPlayerCnt());
			}
		}
		break;
	case MSG_ROBOT_CHECK_BIGGIEST:
		{
			stMsgRobotCheckBiggestRet* pRet = (stMsgRobotCheckBiggestRet*)pmsg ;
		}
		break;
	case MSG_TP_PLAYER_LEAVE:
		{
			stMsgTaxasRoomLeave* pLeveRet = (stMsgTaxasRoomLeave*)pmsg ;
			if ( pLeveRet->nUserUID == getClientApp()->GetPlayerData()->getUserUID() )
			{
				getClientApp()->GetPlayerData()->setIsLackOfCoin(true);
				CLoginScene* CurentScene = new CLoginScene(m_pClient);
				m_pClient->ChangeScene(CurentScene) ;
			}
		}
		break;
	case MSG_PLAYER_BE_ADDED_FRIEND:
		{
			stMsgPlayerBeAddedFriend* pret = (stMsgPlayerBeAddedFriend*)pmsg ;
			printf("uid = %d want add me friend \n",pret->nPlayerUserUID);

			stMsgPlayerBeAddedFriendReply msgReply ;
			msgReply.bAgree = true ;
			msgReply.nReplayToPlayerUserUID = pret->nPlayerUserUID ;
			SendMsg(&msgReply,sizeof(msgReply));
		}
		break;
	case MSG_PLAYER_BE_ADDED_FRIEND_REPLY:
		{
			stMsgPlayerBeAddedFriendReplyRet* pret = (stMsgPlayerBeAddedFriendReplyRet*)pmsg ;
			printf("recive replay ret = %d , new friend uid = %d\n",pret->nRet,pret->nNewFriendUserUID) ;
		}
		break;
	case MSG_TP_ENTER_STATE:
		{
			stMsgTaxasRoomEnterState* pRet = (stMsgTaxasRoomEnterState*)pmsg ;
			if ( pRet->nNewState == eRoomState_Dead )
			{
				m_tRobotControler.onRoomDead() ;
			}
		}
		break;
	default:
		break;
	}
	return true ;
}

void CTaxasPokerScene::OnUpdate(float fDeltaTime )
{
	IScene::OnUpdate(fDeltaTime);
	m_tRobotControler.update(fDeltaTime);
}
