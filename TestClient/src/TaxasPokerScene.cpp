#include "TaxasPokerScene.h"
#include "TaxasMessageDefine.h"
#include "ClientRobot.h"
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
	memset(m_myPrivateCard,0,sizeof(m_myPrivateCard));
	m_bBiggest = false;
	m_nCurRound = 0; 
	m_nCurBetCoin = 0 ;
	memset(m_vbHavePeer,0,sizeof(m_vbHavePeer));
	m_nMaxPeerInRoom = 0 ;
	m_nCurMaxBetCoin = 0;
	m_nMyIdx = 100 ;
	m_bCanDoAction = 0 ;
	m_fTimeTicket = 0 ;
	m_bMustWin = false ;
}

bool CTaxasPokerScene::OnMessage( Packet* pPacket )
{
	stMsg* pMsg = (stMsg*)pPacket->_orgdata ;
	switch ( pMsg->usMsgType )
	{
	//case MSG_TP_ROOM_CUR_INFO:
	//	{
	//		stMsgTaxasPokerRoomInfo* pRoomInfo = (stMsgTaxasPokerRoomInfo*)pMsg;
	//		m_nMaxPeerInRoom = pRoomInfo->nMaxSeat ;
	//		m_nBigBlindCoin = pRoomInfo->nBigBlindBetCoin ;
	//		m_nMinTakeIn = pRoomInfo->nMinTakeIn ;
	//		m_nMaxTakeIn = pRoomInfo->nMaxTakeIn ;
	//		 
	//		char* pBuffer = (char*)pMsg ;
	//		pBuffer += sizeof(stMsgTaxasPokerRoomInfo);
	//		stTPRoomPeerBrifInfo* pInfo = (stTPRoomPeerBrifInfo*)pBuffer ;
	//		while ( pRoomInfo->nCurPlayerCount-- )
	//		{
	//			m_vbHavePeer[pInfo->nIdxInRoom] = true ;
	//			++pInfo ;
	//		}
	//		printf("entered room id = %d \n",pRoomInfo->nRoomID) ;
	//		printf("peers in room is %d \n",GetCurSitDownPeers()) ;
	//		TryToSitDown();
	//	}
	//	break;
	//case MSG_TP_DISTRIBUTE:
	//	{
	//		stMsgTaxasPokerPrivateDistribute* pDis = (stMsgTaxasPokerPrivateDistribute*)pMsg ;
	//		if ( pDis->nBigBlindIdx == m_nMyIdx )
	//		{
	//			m_nCurBetCoin += m_nBigBlindCoin ;
	//			m_pClient->GetPlayerData()->BetCoin(m_bDiamoned,m_nBigBlindCoin) ;
	//		}

	//		if ( pDis->nLittleBlindIdx == m_nMyIdx )
	//		{
	//			m_nCurBetCoin += (m_nBigBlindCoin * 0.5);
	//			m_pClient->GetPlayerData()->BetCoin(m_bDiamoned,m_nBigBlindCoin *0.5) ;
	//		}

	//		char* pBuffer = (char*)pMsg ;
	//		pBuffer += sizeof(stMsgTaxasPokerPrivateDistribute);
	//		stPTPeerCard* pPeerCard = (stPTPeerCard*)pBuffer ;
	//		while ( pDis->nPeerCount-- )
	//		{
	//			if ( pPeerCard->nPlayerIdx == m_nMyIdx )
	//			{
	//				memcpy(m_myPrivateCard,pPeerCard->vCardNum,sizeof(pPeerCard->vCardNum));
	//				m_pMyPeerCard.AddCardByCompsiteNum(pPeerCard->vCardNum[0]);
	//				m_pMyPeerCard.AddCardByCompsiteNum(pPeerCard->vCardNum[1]);
	//				break;
	//			}
	//			++pPeerCard ;
	//		}
	//		m_nCurRound = 0 ;

	//		m_bMustWin = m_pClient->GetPlayerData()->IsMustWin(m_bDiamoned) ;
	//		if ( m_bMustWin )
	//		{
	//			stMsgRobotCheckBiggest msgBig;
	//			SendMsg((char*)&msgBig,sizeof(msgBig)) ;
	//		}
	//	}
	//	break;
	case MSG_ROBOT_CHECK_BIGGIEST:
		{
			stMsgRobotCheckBiggestRet* pRet = (stMsgRobotCheckBiggestRet*)pMsg ;
			m_bBiggest = pRet->nRet == 1 ;
			printf("I must win , my card result is biggest ? ret = %d\n",m_bBiggest) ;
		}
		break;
//	case MSG_TP_PUBLIC_DISTRIBUTE:
//		{
//			stMsgTaxasPokerDistribute* pDisr = (stMsgTaxasPokerDistribute*)pMsg ;
//			unsigned char* pBuffer = (unsigned char*)pMsg ;
//			pBuffer += sizeof(stMsgTaxasPokerDistribute);
//// 			while ( pDisr->nCardCnt-- )
//// 			{
//// 				m_pMyPeerCard.AddCardByCompsiteNum(*pBuffer);
//// 				++pBuffer ;
//// 			}
//		}
//		break;
//	case MSG_TP_PEER_ACTION_RET:
//		{
//			// when is sit failed ;
//			stMsgTaxasPokerPeerActionRet* pRet = (stMsgTaxasPokerPeerActionRet*)pMsg ;
//			if ( pRet->nRet != eTPActRet_Successs && pRet->ePeerAct != eRoomPeerAction_SitDown)
//			{
//				printf("My Action Error , act = %d , ret = %d  myidx = %d\n",pRet->ePeerAct,pRet->nRet,m_nMyIdx ) ;
//				return true;
//			}
//
//			if ( pRet->ePeerAct == eRoomPeerAction_SitDown )
//			{
//				if ( pRet->nRet == eTPActRet_Successs )
//				{
//					m_pClient->GetPlayerData()->TakeIn(pRet->nTakeInCoin,m_bDiamoned) ;
//				}
//				else
//				{
//					switch ( pRet->nRet )
//					{
//					case eTPActRet_PosNotEmpty:
//						{
//							MyAction();
//							printf("sit error , pos Not empty , my idx = %d\n",m_nMyIdx) ;
//						}
//						break;
//					case eTPActRet_NotEnoughCoin:
//					case eTPActRet_UnknownError:
//					case eTPActRet_TakeInCoinError:
//						{
//							OnMustLeave();
//						}
//						break;
//					default:
//						{
//							printf("unknown sit down error = %d \n",pRet->nRet );
//							OnMustLeave();
//						}
//					}
//				}
//			}
//		}
//		break;
//	case MSG_TP_OTHER_PEER_ACTION:
//		{
//			stMsgTaxasPokerPeerAction* pAct = (stMsgTaxasPokerPeerAction*)pMsg ;
//			switch ( pAct->ePeerAct )
//			{
//			case eRoomPeerAction_SitDown:
//				{
//					stMsgTaxasPokerOtherPeerSitDown* pReal = (stMsgTaxasPokerOtherPeerSitDown*)pMsg ;
//					m_vbHavePeer[pReal->tPlayerInfo.nIdxInRoom] = true ;
//					printf("pos idx = %d eRoomPeerAction_SitDown\n",pReal->tPlayerInfo.nIdxInRoom) ;
//					if ( pReal->tPlayerInfo.nSessionID == m_pClient->GetPlayerData()->GetSessionID() )
//					{
//						m_nMyIdx = pReal->tPlayerInfo.nIdxInRoom ;
//						printf("I sit down ,haha , idx = %d, taked coin = %I64d \n",m_nMyIdx,pReal->tPlayerInfo.nCoin ) ;
//						m_pClient->GetPlayerData()->TakeIn(pReal->tPlayerInfo.nCoin,m_bDiamoned) ;
//					}
//				}
//				break;
//			case eRoomPeerAction_StandUp:
//				{
//					stMsgTaxasPokerOtherPeerAction* pReal = (stMsgTaxasPokerOtherPeerAction*)pMsg ;
//					m_vbHavePeer[pReal->nPlayerIdxInRoom] = false ;
//					printf("\n pos idx = %d empty\n",pReal->nPlayerIdxInRoom) ;
//					if ( pReal->nPlayerIdxInRoom == m_nMyIdx )
//					{
//						m_nMyIdx = 100 ;
//
//						// i was statnd up 
//						MyAction();
//						printf("I was stand up, why , try sit again idx = %d? \n ",m_nMyIdx) ;
//					}
//				}
//				break;
//			case eRoomPeerAction_AllIn:
//			case eRoomPeerAction_Follow:
//			case eRoomPeerAction_Add:
//				{
//					stMsgTaxasPokerOtherPeerAction* pReal = (stMsgTaxasPokerOtherPeerAction*)pMsg ;
//					if ( pReal->nPlayerIdxInRoom == m_nMyIdx )
//					{
//						uint64_t nOffset = pReal->nActionValue - m_nCurBetCoin ;
//						m_nCurBetCoin = pReal->nActionValue ;
//						m_pClient->GetPlayerData()->BetCoin(m_bDiamoned,nOffset) ;
//						printf("myidx = %d ,I coin action success act = %d , left coin = %I64d\n",m_nMyIdx,pReal->ePeerAct,m_pClient->GetPlayerData()->GetCoin(m_bDiamoned)) ;
//					}
//				}
//				break;
//			default:
//				{
//					//printf("do not mind other actions !\n") ;
//				}
//			}
//		}
//		break;
//	case MSG_TP_WAIT_PEER_ACTION:
//		{
//			stMsgTaxasPokerWaitPeerAction* pWait = (stMsgTaxasPokerWaitPeerAction*)pMsg ;
//			if ( pWait->nWaitPlayerIdx == m_nMyIdx )
//			{
//				// I must action ;
//				MyAction();
//			}
//			m_nWaitActionPeerIdx = pWait->nWaitPlayerIdx ;
//			m_nCurMaxBetCoin = pWait->nCurMaxBetCoin ;
//		}
//		break;
//	case MSG_TP_ROUND_END:
//		{
//			RoundEnd();
//		}
//		break;
	case MSG_TP_GAME_RESULT:
		{
			//stMsgTaxasPokerGameRessult* pAct = (stMsgTaxasPokerGameRessult*)pMsg;
			//unsigned char* pBuffer = (unsigned char*)pMsg ;
			//pBuffer += sizeof(stMsgTaxasPokerGameRessult);
			//while ( pAct->nWinnerCnt-- )
			//{
			//	if ( *pBuffer == m_nMyIdx )
			//	{
			//		m_pClient->GetPlayerData()->OnWinCoin(pAct->nWinCoins,m_bDiamoned);
			//		break;
			//	}
			//	++pBuffer ;
			//}
		}
		break;
	//case MSG_TP_GAME_RESULT_FINAL:
	//	{
	//		//stMsgTaxasPokerResultFinal* pFinal = (stMsgTaxasPokerResultFinal*)pMsg ;
	//		//stTPPeerCoinFinal* pPeer = (stTPPeerCoinFinal*)((char*)pMsg + sizeof(stMsgTaxasPokerResultFinal));
	//		//while ( pFinal->nPeerCount-- )
	//		//{
	//		//	if ( pPeer->bStandUp )
	//		//	{
	//		//		m_vbHavePeer[pPeer->nIdx] = false ;
	//		//		printf("\n pos idx = %d empty\n",pPeer->nIdx) ;
	//		//	}

	//		//	if ( pPeer->nIdx == m_nMyIdx )
	//		//	{
	//		//		if ( pPeer->bStandUp )
	//		//		{
	//		//			printf(" I was standed up , my idx = %d\n",m_nMyIdx);
	//		//			m_nMyIdx = 100 ;	
	//		//		}
	//		//	}
	//		//	++pPeer ;
	//		//}
	//		//GameEnd();
	//	}
	//	break;
	case MSG_ROBOT_APPLY_TO_LEAVE:
		{
			stMsgRobotApplyToLeaveRoomRet* pLeveRet = (stMsgRobotApplyToLeaveRoomRet*)pMsg ;
			if ( pLeveRet->nRet == 0 )
			{
				CLoginScene* CurentScene = new CLoginScene(m_pClient);
				m_pClient->ChangeScene(CurentScene) ;
			}
			else
			{
				if ( m_nMyIdx >= 100 )
				{
					MyAction() ;
				}
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
	if ( m_bCanDoAction == false )
	{
		return ;
	}

	m_fTimeTicket -= fDeltaTime ;
	if ( m_fTimeTicket <= 0 )
	{
		DoMyAction() ;
		m_bCanDoAction = false ;
	}
}

void CTaxasPokerScene::RoundEnd()
{
	m_nCurBetCoin = 0 ;
	++m_nCurRound;
	m_nCurMaxBetCoin = 0 ;
	time_t t ;
	t = time(NULL) ;
	printf("round end ! %d \n",t) ;
}

void CTaxasPokerScene::GameEnd()
{
	m_pMyPeerCard.Reset();
	m_nCurRound = 0;
	m_nCurBetCoin = 0 ;
	m_nCurMaxBetCoin = 0 ;
	m_bBiggest = true ;
	printf("GameEnd  nMyIdx = %d , my coin is %I64d \n",m_nMyIdx,m_pClient->GetPlayerData()->GetCoin(m_bDiamoned)) ;

	if ( GetCurSitDownPeers() >= m_pClient->GetPlayerData()->pRobotItem->nApplyLeaveWhenPeerCount )
	{
		stMsgRobotApplyToLevelRoom msg ;
		SendMsg((char*)&msg,sizeof(msg)) ;
	}
	else
	{
		if ( m_nMyIdx >= 100 )
		{
			MyAction() ;
		}
	}
}

void CTaxasPokerScene::OnMustLeave()
{
	printf("some situation , I must leave !\n") ;
	stMsgRoomLeave stLeave ;
	SendMsg((char*)&stLeave,sizeof(stLeave)) ;
	CLoginScene* CurentScene = new CLoginScene(m_pClient);
	m_pClient->ChangeScene(CurentScene) ;
}

void CTaxasPokerScene::TryToSitDown()
{
	//stMsgTaxasPokerPeerAction msgToSit ;
	//unsigned char nIdx = -1 ;
	//for ( int i = 0 ; i < m_nMaxPeerInRoom ; ++i )
	//{
	//	if ( m_vbHavePeer[i] == false )
	//	{
	//		nIdx = i ;
	//		printf("\n I apply pos idx = %d \n",i) ;
	//		break; 
	//	}
	//}

	//if ( -1 == nIdx )
	//{
	//	OnMustLeave() ;
	//	printf("can not find a empty pos to sit MustLeave \n") ;
	//	return ;
	//}

	//msgToSit.ePeerAct = eRoomPeerAction_SitDown ;
	//msgToSit.nActionValue = nIdx ;
	//msgToSit.nTakeInCoin = m_nMinTakeIn + (m_nMaxTakeIn - m_nMinTakeIn ) * 0.5;
	//SendMsg(&msgToSit,sizeof(msgToSit)) ;
}

void CTaxasPokerScene::MyAction()
{
	m_bCanDoAction = true ;
	m_fTimeTicket = (float)rand() / float(RAND_MAX) ;
	m_fTimeTicket = m_pClient->GetPlayerData()->pRobotItem->fActDelayBegin + m_fTimeTicket * (m_pClient->GetPlayerData()->pRobotItem->fActDelayEnd - m_pClient->GetPlayerData()->pRobotItem->fActDelayBegin ) ;
}

void CTaxasPokerScene::DoMyAction()
{
	if ( m_nMyIdx == 100 )
	{
		TryToSitDown() ;
		printf("I lost coin , i was stand up , i must try to sit down again, MyIdx = %d ! \n",m_nMyIdx) ;
		return ;
	}

	//stMsgTaxasPokerPeerAction msgMyAction ;
	//bool bMustWin = m_bMustWin ;
	//if ( bMustWin )
	//{
	//	printf("Do MyAction , I Must Win \n") ;
	//	if ( m_bBiggest == false )
	//	{
	//		if ( m_nCurBetCoin == m_nCurMaxBetCoin )
	//		{
	//			msgMyAction.ePeerAct = eRoomPeerAction_Pass ;
	//		}
	//		else
	//		{
	//			msgMyAction.ePeerAct = eRoomPeerAction_GiveUp ;
	//		}
	//		SendMsg(&msgMyAction,sizeof(msgMyAction)) ;
	//		printf(" I must win , bug not biggest so , i will not follow \n") ;
	//		return ;
	//	}
	//}

	//bool bMustNotGiveUP = bMustWin && m_bBiggest ;
	//unsigned char nCardType = 0 ;
	//stRobotAI* pAI = m_pClient->GetRobotAI() ;
	//if ( m_nCurRound == 0 )
	//{
	//	CCard cardA(m_myPrivateCard[0]);
	//	CCard cardB (m_myPrivateCard[1]);
	//	if ( cardA.GetCardFaceNum() == cardB.GetCardFaceNum() )  // pair ;
	//	{
	//		nCardType = 11 ;
	//		if ( cardA.GetCardFaceNum() >= 9 )
	//		{
	//			nCardType = 10;
	//		}
	//	}
	//	else if ( cardA.GetCardFaceNum() == 1 || cardB.GetCardFaceNum() == 1 )  // card With 'A '
	//	{
	//		nCardType = 12 ;
	//	}
	//	else
	//	{
	//		nCardType = 13 ; // Gao Pai ;
	//	}
	//}
	//else
	//{
	//	nCardType = m_pMyPeerCard.GetCardTypeForRobot();
	//}

	//const stBetTimesAction* pFinalAction = pAI->GetFinalAction(m_nCurRound,nCardType,m_nCurMaxBetCoin / m_nBigBlindCoin) ;
	//unsigned char nAct = pFinalAction->RandActionType();
	//if ( bMustNotGiveUP && (nAct == eRbt_Act_GiveUp || nAct == eRbt_Act_Pass ) )
	//{
	//	nAct = eRbt_Act_Add ;
	//	printf(" I must win and will not give up \n") ;
	//}

	//switch ( nAct )
	//{
	//case eRbt_Act_Follow:
	//	{
	//		printf("my idx = %d , should eRbt_Act_Follow\n",m_nMyIdx) ;
	//		if ( m_nCurBetCoin != m_nCurMaxBetCoin )  
	//		{
	//			msgMyAction.ePeerAct = eRoomPeerAction_Follow ;
	//			if ( m_nCurMaxBetCoin - m_nCurBetCoin >= m_pClient->GetPlayerData()->GetCoin(m_bDiamoned))    // can not follow , lack of money ;
	//			{
	//				printf("coin is limit !\n") ;
	//				if ( pFinalAction->RandActionWithIn(eRbt_Act_AllIn,eRbt_Act_GiveUp) == eRbt_Act_GiveUp && bMustNotGiveUP == false )
	//				{
	//					msgMyAction.ePeerAct =  eRoomPeerAction_GiveUp;
	//					printf("my idx = %d , Final eRoomPeerAction_GiveUp\n\n",m_nMyIdx) ;
	//				}
	//				else
	//				{
	//					msgMyAction.ePeerAct =  eRoomPeerAction_AllIn;
	//					printf("my idx = %d , Final eRoomPeerAction_AllIn\n\n",m_nMyIdx) ;
	//				}
	//			}
	//		}
	//		else // can not follow
	//		{

	//			if ( pFinalAction->RandActionWithIn(eRbt_Act_Add,eRbt_Act_Pass) == eRbt_Act_Pass )
	//			{
	//				msgMyAction.ePeerAct =  eRoomPeerAction_Pass;
	//				printf("coin is limit !\n") ;
	//				printf("my idx = %d , Final eRoomPeerAction_Pass\n\n",m_nMyIdx) ;
	//			}
	//			else
	//			{
	//				msgMyAction.ePeerAct =  eRoomPeerAction_Add;
	//				stAddType* padd = pFinalAction->RandAddStrage();
	//				uint64_t nDestAdd = 0 ;
	//				if ( padd->nAddType == 0 ) // times big blind ;
	//				{
	//					nDestAdd = padd->fTimes * m_nBigBlindCoin ;
	//				}
	//				else  // times my all coin ;
	//				{
	//					nDestAdd = padd->fTimes * m_pClient->GetPlayerData()->GetCoin(m_bDiamoned);
	//				}
	//				nDestAdd = nDestAdd - (nDestAdd % m_nBigBlindCoin );

	//				// I at least Need to Add coin ;
	//				uint64_t nAtLeast = ( m_nCurMaxBetCoin - m_nCurBetCoin ) * 2 ;
	//				msgMyAction.nActionValue = (nAtLeast > nDestAdd ? nAtLeast:nDestAdd);
	//				printf("my idx = %d , Final eRoomPeerAction_Add\n\n",m_nMyIdx) ;
	//				if ( msgMyAction.nActionValue >= m_pClient->GetPlayerData()->GetCoin(m_bDiamoned) )
	//				{
	//					msgMyAction.ePeerAct = eRoomPeerAction_AllIn ;
	//					printf("my idx = %d , Final eRoomPeerAction_AllIn\n\n",m_nMyIdx) ;
	//				}
	//			}
	//		}
	//	}
	//	break;
	//case eRbt_Act_Add:
	//	{
	//		printf("my idx = %d , should eRbt_Act_Add\n",m_nMyIdx) ;
	//		msgMyAction.ePeerAct = eRoomPeerAction_Add;
	//		stAddType* padd = pFinalAction->RandAddStrage();
	//		uint64_t nDestAdd = 0 ;
	//		if ( padd->nAddType == 0 ) // times big blind ;
	//		{
	//			nDestAdd = padd->fTimes * m_nBigBlindCoin ;
	//		}
	//		else  // times my all coin ;
	//		{
	//			nDestAdd = padd->fTimes * m_pClient->GetPlayerData()->GetCoin(m_bDiamoned);
	//		}
	//		nDestAdd = nDestAdd - (nDestAdd % m_nBigBlindCoin );

	//		// I at least Need to Add coin ;
	//		uint64_t nAtLeast = ( m_nCurMaxBetCoin - m_nCurBetCoin ) * 2 ;
	//		msgMyAction.nActionValue = (nAtLeast > nDestAdd ? nAtLeast:nDestAdd);
	//		if ( msgMyAction.nActionValue >= m_pClient->GetPlayerData()->GetCoin(m_bDiamoned) )
	//		{
	//			msgMyAction.ePeerAct = eRoomPeerAction_AllIn ;
	//			printf("my idx = %d , Final eRoomPeerAction_AllIn\n\n",m_nMyIdx) ;
	//		}
	//	}
	//	break;
	//case eRbt_Act_Pass:
	//	{
	//		if ( m_nCurBetCoin != m_nCurMaxBetCoin )  // can not pass
	//		{
	//			if ( pFinalAction->RandActionWithIn(eRbt_Act_Follow,eRbt_Act_GiveUp) == eRbt_Act_GiveUp  && bMustNotGiveUP == false)
	//			{
	//				msgMyAction.ePeerAct =  eRoomPeerAction_GiveUp;
	//				printf("my idx = %d , Final eRoomPeerAction_GiveUp\n\n",m_nMyIdx) ;
	//			}
	//			else
	//			{
	//				msgMyAction.ePeerAct =  eRoomPeerAction_Follow;
	//				if ( m_nCurMaxBetCoin - m_nCurBetCoin >= m_pClient->GetPlayerData()->GetCoin(m_bDiamoned))    // can not follow , lack of money ;
	//				{
	//					printf("coin is limit \n") ;
	//					if ( pFinalAction->RandActionWithIn(eRbt_Act_AllIn,eRbt_Act_GiveUp) == eRbt_Act_GiveUp  && bMustNotGiveUP == false)
	//					{
	//						msgMyAction.ePeerAct =  eRoomPeerAction_GiveUp;
	//						
	//						printf("my idx = %d , Final eRoomPeerAction_GiveUp\n\n",m_nMyIdx) ;
	//					}
	//					else
	//					{
	//						msgMyAction.ePeerAct =  eRoomPeerAction_AllIn;
	//						printf("my idx = %d , Final eRoomPeerAction_AllIn\n\n",m_nMyIdx) ;
	//					}
	//				}
	//			}
	//		}
	//		else
	//		{
	//			msgMyAction.ePeerAct = eRoomPeerAction_Pass ;
	//			printf("my idx = %d , eRoomPeerAction_Pass\n",m_nMyIdx) ;
	//		}
	//	}
	//	break;
	//case eRbt_Act_GiveUp:
	//	{
	//		printf("my idx = %d , should eRbt_Act_GiveUp\n",m_nMyIdx) ;
	//		if (  m_nCurBetCoin == m_nCurMaxBetCoin  )
	//		{
	//			msgMyAction.ePeerAct = eRoomPeerAction_Pass ;
	//			printf("my idx = %d , Final eRoomPeerAction_Pass\n\n",m_nMyIdx) ;
	//		}
	//		else
	//		{
	//			msgMyAction.ePeerAct = eRoomPeerAction_GiveUp ;
	//		}
	//	}
	//	break;
	//case eRbt_Act_AllIn:
	//	{
	//		printf("my idx = %d , should eRoomPeerAction_AllIn\n",m_nMyIdx) ;
	//		msgMyAction.ePeerAct = eRoomPeerAction_AllIn ;
	//	}
	//	break;
	//default:
	//	printf("unknown action %d \n",nAct) ;
	//	msgMyAction.ePeerAct = eRoomPeerAction_GiveUp ;
	//	break;;
	//}
	//SendMsg(&msgMyAction,sizeof(msgMyAction)) ;
}

unsigned short CTaxasPokerScene::GetCurSitDownPeers()
{
	int nCount = 0 ;
	for ( int i = 0 ; i < 15 ; ++i )
	{
		if ( m_vbHavePeer[i] )
		{
			++nCount ;
		}
	}
	return nCount ;
}