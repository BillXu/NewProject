#include "PaiJiuScene.h"
#include "PaiJiuMessageDefine.h"
#include "LogManager.h"
#include "ClientRobot.h"
#include "PlayerData.h"
#define TIME_BET_DELAY 1
#define TIME_APPLY_BANKER_DELAY 50
CPaiJiuScene::CPaiJiuScene(CClientRobot* pClient)
	:IScene(pClient)
{
	m_eSceneType = eScene_PaiJiuRoom ;
	m_bBanker = false ;
	nApplyerCount = 0 ;
	nRoundWhenBanker = 0 ;
	nBetTimesThisRound = 0 ;

	m_eTargetAction = eTargetAction_None;
	m_fTick = 0.0F ;
	m_fTargetTick[eTargetAction_None]= 999999999.0F;
	m_fTargetTick[eTargetAction_ApplyBanker] = 999999999.0F;
	m_fTargetTick[eTargetAction_Max] = 9999999999.0F;
	m_fTargetTick[eTargetAction_Bet] = TIME_BET_DELAY ;
	m_bWillApplyBanker = false ;
}

void CPaiJiuScene::OnEnterScene()
{
	IScene::OnEnterScene();
}

bool CPaiJiuScene::OnMessage( Packet* pPacket )
{
	if ( IScene::OnMessage(pPacket) )
	{
		return true ;
	}

	stMsg* pMsg = (stMsg*)pPacket->_orgdata;
	switch ( pMsg->usMsgType )
	{
	case MSG_PJ_ROOM_INFO:
		{
			stMsgPJRoomInfo* pInfo = (stMsgPJRoomInfo*)pMsg ;
			m_eRoomState = (eRoomState)pInfo->eRoomState ;
			nApplyerCount = pInfo->nApplyBankerCount ;
			printf("Recieved Room Info , room id = %d \n",pInfo->nRoomID) ;
			if ( m_eRoomState == eRoomState_PJ_WaitBanker )
			{
				//TryToBeBanker();
				m_bWillApplyBanker = true ;
				m_eTargetAction = eTargetAction_ApplyBanker ;
				m_fTargetTick[eTargetAction_ApplyBanker] =  (rand() % TIME_APPLY_BANKER_DELAY ) / (TIME_APPLY_BANKER_DELAY * 1.0F );
				m_fTick = 0 ;
			}
		}
		break;
	case MSG_PJ_STATE_CHANGED:
		{
			m_bWillApplyBanker = false ;
			stMsgPJRoomStateChanged* pState = (stMsgPJRoomStateChanged*)pMsg ;
			if ( pState->nCurrentState == eRoomState_PJ_WaitForBet )
			{
				CLogMgr::SharedLogMgr()->PrintLog("Wait for Bet Room Info") ;
				if ( m_bBanker == false )
				{
					//BetCoin();
					m_eTargetAction = eTargetAction_Bet ;
					m_fTick = 0;
				}
				else
				{
					CLogMgr::SharedLogMgr()->PrintLog("Banker Can not Bet , so I do not Bet !") ;
				}
			}
			else if ( eRoomState_PJ_WaitBanker == pState->nCurrentState )
			{
				printf("No Banker , Wait New Banker !\n") ;
				if ( m_bBanker == false )
				{
					printf("I try to be banker !\n") ;
					//TryToBeBanker();
					m_bWillApplyBanker = true ;
					m_eTargetAction = eTargetAction_ApplyBanker ;
					m_fTargetTick[eTargetAction_ApplyBanker] =  (rand() % TIME_APPLY_BANKER_DELAY ) / (TIME_APPLY_BANKER_DELAY * 1.0F );
					m_fTick = 0 ;
				}
				else
				{
					printf("I am Banker , so don't apply banker !\n" ) ;
				}
			}
			else if ( eRoomState_PJ_Settlement == pState->nCurrentState )
			{
				stMsgPJSettlement* pset = (stMsgPJSettlement*)pMsg ;
				m_pClient->GetPlayerData()->nMyCoin += pset->nResultCoinOffset ;
				//m_pClient->GetPlayerData()->OnWinCoin(pset->nResultCoinOffset,false);
				CLogMgr::SharedLogMgr()->PrintLog("Now Settlement , mycoin = %I64d, myOffsetCoin = %I64d",m_pClient->GetPlayerData()->nMyCoin,pset->nResultCoinOffset ) ;
				if ( m_bBanker )
				{
					++nRoundWhenBanker ;
					CLogMgr::SharedLogMgr()->PrintLog("My Banker Round is %d",nRoundWhenBanker ) ;
				}
				nBetTimesThisRound = 0 ;
			}
			else if ( eRoomState_PJ_BankerSelectGoOn == pState->nCurrentState )
			{
				if ( !m_bBanker )
				{
					CLogMgr::SharedLogMgr()->PrintLog("Wait Banker make choice , I am not Banker !") ;
					return true;
				}
				stMsgPJBankerChoseGoOnOrCanncel msg ;
				if ( nApplyerCount >= 1 && nRoundWhenBanker >= 3 )
				{
					msg.nChoice = 2;
					CLogMgr::SharedLogMgr()->PrintLog("I am banker , I chose unbanker ") ;
				}
				else
				{
					msg.nChoice = 0;
					CLogMgr::SharedLogMgr()->PrintLog("I am banker , I chose Go On ") ;
				}
				SendMsg(&msg,sizeof(msg)) ;
			}
			else if ( eRoomState_PJWaitNewBankerChoseShuffle == pState->nCurrentState )
			{
				if ( m_bBanker )
				{
					stMsgPJNewBankerChoseShuffle  msg ;
					SendMsg(&msg,sizeof(msg)) ;
					CLogMgr::SharedLogMgr()->PrintLog("I am the new banker , I chose shuffle !") ;
				}
			}
		}
		break;
	case MSG_PJ_BANKER_CHANGED:
		{
			stMsgPJBankerChanged* pret= (stMsgPJBankerChanged*)pMsg ;
			m_bBanker = pret->newBankerSessionID == m_pClient->GetSessionID() ;
			nRoundWhenBanker = 0 ;
			--nApplyerCount ;
			if ( nApplyerCount < 0 )
			{
				nApplyerCount = 0 ;
			}
			CLogMgr::SharedLogMgr()->PrintLog("Banker changed bank is me ? value = %d",m_bBanker ) ;
			m_bWillApplyBanker = false ;
		}
		break;
	case MSG_PJ_ROOM_APPLY_BANKER_LIST:
		{
			stMsgPJRoomApplyList* plist = (stMsgPJRoomApplyList*)pMsg ;
			nApplyerCount = plist->nCount ;
			CLogMgr::SharedLogMgr()->PrintLog("update apply list ,current count = %d " , nApplyerCount ) ;
		}
		break; 
	case MSG_PJ_OTHER_APPLY_BANKER:
		{
			++nApplyerCount ;
			m_bWillApplyBanker = false ;
		}
		break;
	case MSG_PJ_ACTION_RET:
		{
			stMsgPJActionRet* pact = (stMsgPJActionRet*)pMsg ;
			if ( pact->nAcionType == MSG_PJ_BET && pact->nRet == ePJ_ActRet_Success )
			{
				unsigned short nTimes = rand()% 6 ;
				if ( nBetTimesThisRound <= nTimes )
				{
					//BetCoin();
					m_eTargetAction = eTargetAction_Bet ;
					//m_fTick = (rand() % TIME_BET_DELAY ) / (TIME_BET_DELAY * 1.0F );
				}
				CLogMgr::SharedLogMgr()->PrintLog("I Bet Success!") ;
				++nBetTimesThisRound ;
			}
			else if (  pact->nAcionType == MSG_PJ_BET ) 
			{
				CLogMgr::SharedLogMgr()->PrintLog(" I Bet Failed ! error = %d",pact->nRet ) ;
				m_eTargetAction = eTargetAction_Max ;
				m_fTick = 0 ;
			}
			else if ( MSG_PJ_APPLY_BANKER == pact->nAcionType )
			{
				if ( pact->nRet != ePJ_ActRet_Success )
				{
					printf("to be banker error code = %d \n",pact->nRet);
					if ( ePJ_ActRet_Self_Money_Not_Enough == pact->nRet )
					{
						stMsgRobotAddMoney msg ;
						msg.nWantCoin = 5500000 ;
						SendMsg((char*)&msg,sizeof(msg)) ;
						printf("request add coin 5 million \n");
					}
				}
				else
				{
					printf("to be banker ok \n");
				}
				
			}
			
		}
		break;
	case MSG_ROBOT_ADD_MONEY:
		{
			stMsgRobotAddMoneyRet* pRet = (stMsgRobotAddMoneyRet*)pMsg ;
			m_pClient->GetPlayerData()->nMyCoin = pRet->nFinalCoin ;
			printf("received add coin in pj scene , apply to be banker !\n") ;
			
			m_bWillApplyBanker = true ;
			m_eTargetAction = eTargetAction_ApplyBanker ;
			m_fTargetTick[eTargetAction_ApplyBanker] =  (rand() % TIME_APPLY_BANKER_DELAY ) / (TIME_APPLY_BANKER_DELAY * 1.0F );
			m_fTick = 0 ;
			return true;
		}
		break;
	default:
		{
			CLogMgr::SharedLogMgr()->PrintLog("%s: unprocessed msg type msg = %d ",m_pClient->GetPlayerData()->GetName(), pMsg->usMsgType ) ;
		}
		break;
	}
	return false ;
}

void CPaiJiuScene::OnUpdate(float fDeltaTime )
{
	IScene::OnUpdate(fDeltaTime) ;
	m_fTick += fDeltaTime ;
	if ( m_bWillApplyBanker && m_eTargetAction == eTargetAction_ApplyBanker && m_fTargetTick[eTargetAction_ApplyBanker] <= m_fTick )
	{
		//BetCoin() ;
		//m_fTick = (rand() % TIME_BET_DELAY ) / (TIME_BET_DELAY * 1.0F );
		m_bWillApplyBanker = false;
		m_fTick = 0 ;
		TryToBeBanker();
	}
}

void CPaiJiuScene::TryToBeBanker()
{
// 	unsigned short nRate = rand() % 100 ;
// 	if ( nRate >= 80 )
// 	{
// 		printf("I decied , I will not Apply banker , let other robot do ! \n") ;
// 		return ;
// 	}
	printf("I decied , I need to apply to Be banker !\n ") ;
	stMsgPJApplyToBeBanker msg ;
	msg.bApplyToJoinList = 1 ;
	SendMsg(&msg,sizeof(msg)) ;
}

void CPaiJiuScene::BetCoin()
{
	if ( m_bBanker )
	{
		printf("%s: I am Banker , I will not bet\n",m_pClient->GetPlayerData()->GetName() ) ;
		m_eTargetAction = eTargetAction_Max ;
		m_fTick = 0 ;
		return ;
	}
	unsigned int nCoinType[] = { 50,100,500,10000 };
	int nCountIdx  = rand() % (sizeof(nCoinType) / sizeof(unsigned int )) ;
	unsigned int nBetCoin = nCoinType[nCountIdx] ;
	if ( nBetCoin > m_pClient->GetPlayerData()->nMyCoin)
	{
		nBetCoin = nCoinType[0] ;
		if ( nBetCoin > m_pClient->GetPlayerData()->nMyCoin )
		{
			printf("%s: I have no money can not bet coin\n",m_pClient->GetPlayerData()->GetName() ) ;
			m_eTargetAction = eTargetAction_Max ;
			m_fTick = 0 ;
			return ;
		}
	}

	unsigned char nBetPort = rand() % ePJ_BetPort_Max ;
	stMsgPJBet msg ;
	msg.nBetCoin = nBetCoin ;
	msg.nBetPort = nBetPort ;
	SendMsg(&msg,sizeof(msg)) ;
	printf("I am bet coin = %d\n",nBetPort ) ;
}
