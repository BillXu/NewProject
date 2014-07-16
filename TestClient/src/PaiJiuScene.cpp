//#include "PaiJiuScene.h"
//#include "PaiJiuMessageDefine.h"
//#include "LogManager.h"
//#include "Client.h"
//#include "PlayerData.h"
//#define TIME_BET_DELAY 1
//CPaiJiuScene::CPaiJiuScene(CClient* pClient)
//	:IScene(pClient)
//{
//	m_eSceneType = eScene_PaiJiuRoom ;
//	m_bBanker = false ;
//	nApplyerCount = 0 ;
//	nRoundWhenBanker = 0 ;
//	nBetTimesThisRound = 0 ;
//
//	m_eTargetAction = eTargetAction_None;
//	m_fTick = 0.0F ;
//	m_fTargetTick[eTargetAction_None]= 999999999.0F;
//	m_fTargetTick[eTargetAction_Max] = 9999999999.0F;
//	m_fTargetTick[eTargetAction_Bet] = TIME_BET_DELAY ;
//}
//
//void CPaiJiuScene::OnEnterScene()
//{
//	IScene::OnEnterScene();
//}
//
//bool CPaiJiuScene::OnMessage( RakNet::Packet* pPacket )
//{
//	if ( IScene::OnMessage(pPacket) )
//	{
//		return true ;
//	}
//
//	stMsg* pMsg = (stMsg*)pPacket->data;
//	switch ( pMsg->usMsgType )
//	{
//	case MSG_PJ_ROOM_INFO:
//		{
//			stMsgPJRoomInfo* pInfo = (stMsgPJRoomInfo*)pMsg ;
//			m_eRoomState = (eRoomState)pInfo->eRoomState ;
//			nApplyerCount = pInfo->nApplyBankerCount ;
//			CLogMgr::SharedLogMgr()->PrintLog("Recieved Room Info") ;
//			if ( m_eRoomState == eRoomState_PJ_WaitBanker )
//			{
//				TryToBeBanker();
//			}
//		}
//		break;
//	case MSG_PJ_STATE_CHANGED:
//		{
//			stMsgPJRoomStateChanged* pState = (stMsgPJRoomStateChanged*)pMsg ;
//			if ( pState->nCurrentState == eRoomState_PJ_WaitForBet )
//			{
//				CLogMgr::SharedLogMgr()->PrintLog("Wait for Bet Room Info") ;
//				if ( m_bBanker == false )
//				{
//					//BetCoin();
//					m_eTargetAction = eTargetAction_Bet ;
//					m_fTick = (rand() % TIME_BET_DELAY ) / (TIME_BET_DELAY * 1.0F );
//				}
//				else
//				{
//					CLogMgr::SharedLogMgr()->PrintLog("Banker Can not Bet , so I do not Bet !") ;
//				}
//			}
//			else if ( eRoomState_PJ_WaitBanker == pState->nCurrentState )
//			{
//				printf("No Banker , Wait New Banker !\n") ;
//				if ( m_bBanker == false )
//				{
//					printf("I try to be banker !\n") ;
//					TryToBeBanker();
//				}
//				else
//				{
//					printf("I am Banker , so don't apply banker !\n" ) ;
//				}
//			}
//			else if ( eRoomState_PJ_Settlement == pState->nCurrentState )
//			{
//				stMsgPJSettlement* pset = (stMsgPJSettlement*)pMsg ;
//				m_pClient->GetPlayerData()->OnWinCoin(pset->nResultCoinOffset,false);
//				CLogMgr::SharedLogMgr()->PrintLog("Now Settlement , mycoin = %I64d, myOffsetCoin = %I64d",m_pClient->GetPlayerData()->GetCoin(false),pset->nResultCoinOffset ) ;
//				if ( m_bBanker )
//				{
//					++nRoundWhenBanker ;
//					CLogMgr::SharedLogMgr()->PrintLog("My Banker Round is %d",nRoundWhenBanker ) ;
//				}
//				nBetTimesThisRound = 0 ;
//			}
//			else if ( eRoomState_PJ_BankerSelectGoOn == pState->nCurrentState )
//			{
//				if ( !m_bBanker )
//				{
//					CLogMgr::SharedLogMgr()->PrintLog("Wait Banker make choice , I am not Banker !") ;
//					return true;
//				}
//				stMsgPJBankerChoseGoOnOrCanncel msg ;
//				if ( nApplyerCount >= 1 && nRoundWhenBanker >= 3 )
//				{
//					msg.nChoice = 2;
//					CLogMgr::SharedLogMgr()->PrintLog("I am banker , I chose unbanker ") ;
//				}
//				else
//				{
//					msg.nChoice = 0;
//					CLogMgr::SharedLogMgr()->PrintLog("I am banker , I chose Go On ") ;
//				}
//				SendMsg(&msg,sizeof(msg)) ;
//			}
//			else if ( eRoomState_PJWaitNewBankerChoseShuffle == pState->nCurrentState )
//			{
//				if ( m_bBanker )
//				{
//					stMsgPJNewBankerChoseShuffle  msg ;
//					SendMsg(&msg,sizeof(msg)) ;
//					CLogMgr::SharedLogMgr()->PrintLog("I am the new banker , I chose shuffle !") ;
//				}
//			}
//		}
//		break;
//	case MSG_PJ_BANKER_CHANGED:
//		{
//			stMsgPJBankerChanged* pret= (stMsgPJBankerChanged*)pMsg ;
//			m_bBanker = pret->newBankerSessionID == m_pClient->GetSessionID() ;
//			nRoundWhenBanker = 0 ;
//			--nApplyerCount ;
//			if ( nApplyerCount < 0 )
//			{
//				nApplyerCount = 0 ;
//			}
//			CLogMgr::SharedLogMgr()->PrintLog("Banker changed bank is me ? value = %d",m_bBanker ) ;
//		}
//		break;
//	case MSG_PJ_ROOM_APPLY_BANKER_LIST:
//		{
//			stMsgPJRoomApplyList* plist = (stMsgPJRoomApplyList*)pMsg ;
//			nApplyerCount = plist->nCount ;
//			CLogMgr::SharedLogMgr()->PrintLog("update apply list ,current count = %d " , nApplyerCount ) ;
//		}
//		break; 
//	case MSG_PJ_OTHER_APPLY_BANKER:
//		{
//			++nApplyerCount ;
//		}
//		break;
//	case MSG_PJ_ACTION_RET:
//		{
//			stMsgPJActionRet* pact = (stMsgPJActionRet*)pMsg ;
//			if ( pact->nAcionType == MSG_PJ_BET && pact->nRet == ePJ_ActRet_Success )
//			{
//				unsigned short nTimes = rand()% 6 ;
//				if ( nBetTimesThisRound <= nTimes )
//				{
//					//BetCoin();
//					m_eTargetAction = eTargetAction_Bet ;
//					m_fTick = (rand() % TIME_BET_DELAY ) / (TIME_BET_DELAY * 1.0F );
//				}
//				CLogMgr::SharedLogMgr()->PrintLog("I Bet Success!") ;
//			}
//			else if (  pact->nAcionType == MSG_PJ_BET ) 
//			{
//				CLogMgr::SharedLogMgr()->PrintLog(" I Bet Failed ! error = %d",pact->nRet ) ;
//				m_eTargetAction = eTargetAction_Max ;
//				m_fTick = 0 ;
//			}
//			++nBetTimesThisRound ;
//		}
//		break;
//	default:
//		{
//			CLogMgr::SharedLogMgr()->PrintLog("%s: unprocessed msg type msg = %d ",m_pClient->GetPlayerData()->GetName(), pMsg->usMsgType ) ;
//		}
//		break;
//	}
//	return false ;
//}
//
//void CPaiJiuScene::OnUpdate(float fDeltaTime )
//{
//	IScene::OnUpdate(fDeltaTime) ;
//	m_fTick += fDeltaTime ;
//	if ( m_eTargetAction == eTargetAction_Bet && m_fTargetTick[eTargetAction_Bet] <= m_fTick )
//	{
//		BetCoin() ;
//		m_fTick = (rand() % TIME_BET_DELAY ) / (TIME_BET_DELAY * 1.0F );
//	}
//}
//
//void CPaiJiuScene::TryToBeBanker()
//{
//	unsigned short nRate = rand() % 100 ;
//	if ( nRate >= 80 )
//	{
//		printf("I decied , I will not Apply banker , let other robot do ! \n") ;
//		return ;
//	}
//	printf("I decied , I need to apply to Be banker !\n ") ;
//	stMsgPJApplyToBeBanker msg ;
//	SendMsg(&msg,sizeof(msg)) ;
//}
//
//void CPaiJiuScene::BetCoin()
//{
//	if ( m_bBanker )
//	{
//		printf("%s: I am Banker , I will not bet\n",m_pClient->GetPlayerData()->GetName() ) ;
//		m_eTargetAction = eTargetAction_Max ;
//		m_fTick = 0 ;
//		return ;
//	}
//	unsigned int nCoinType[] = { 50,100,500,10000 };
//	int nCountIdx  = rand() % (sizeof(nCoinType) / sizeof(unsigned int )) ;
//	unsigned int nBetCoin = nCoinType[nCountIdx] ;
//	if ( nBetCoin > m_pClient->GetPlayerData()->GetCoin(false) )
//	{
//		nBetCoin = nCoinType[0] ;
//		if ( nBetCoin > m_pClient->GetPlayerData()->GetCoin(false) )
//		{
//			printf("%s: I have no money can not bet coin\n",m_pClient->GetPlayerData()->GetName() ) ;
//			m_eTargetAction = eTargetAction_Max ;
//			m_fTick = 0 ;
//			return ;
//		}
//	}
//
//	unsigned char nBetPort = rand() % ePJ_BetPort_Max ;
//	stMsgPJBet msg ;
//	msg.nBetCoin = nBetCoin ;
//	msg.nBetPort = nBetPort ;
//	SendMsg(&msg,sizeof(msg)) ;
//	printf("I am bet coin = %d\n",nBetPort ) ;
//}
