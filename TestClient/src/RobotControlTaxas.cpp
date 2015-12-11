#include "RobotControlTaxas.h"
#include "TaxasPokerScene.h"
#include "TaxasPokerPeerCard.h"
#include "TaxasMessageDefine.h"
#include "PlayerData.h"
#include "Client.h"
bool CRobotControlTaxas::init(const char* cAiFile,CTaxasPokerScene* pScene)
{
	CRobotControl::init();
	if (!m_tAiCore.init(cAiFile))
	{
		printf("load ai file error %s",cAiFile);
	}
	m_pScene = nullptr ;
	m_eState = eRobot_StandUp ;
	m_nMySeatIdx = MAX_PEERS_IN_TAXAS_ROOM ;
	m_pScene = pScene ;
	m_fTicketForCheckCanSitDown = float(rand() % 25 ) + 8;
	printf("robot enter room \n");
	return true ;
}

void CRobotControlTaxas::leave()
{
	stMsgTaxasPlayerLeave msgLeave ;
	msgLeave.nRoomID = m_pScene->getPokerData()->nRoomID ;
	m_pScene->SendMsg(&msgLeave,sizeof(msgLeave)) ;
	m_eState = eRobot_WaitLeaving ;
}

void CRobotControlTaxas::update(float fdeta )
{
	CRobotControl::update(fdeta);

	// wait to sit down ;
	if ( m_eState == eRobot_WaitPosSitDown && m_pScene->getPokerData()->nRoomID )
	{
		TryingSitDown();
	}

	if ( eRobot_StandUp == m_eState && m_pScene->getPokerData()->getPlayerCnt() < m_pScene->getClientApp()->GetPlayerData()->pRobotItem->nApplyLeaveWhenPeerCount )
	{
		m_fTicketForCheckCanSitDown -= fdeta ;
		if ( fdeta < 0 )
		{
			orderSitdown();
			m_fTicketForCheckCanSitDown = float(rand() % 25 ) + 5;
		}
		
	}

	// check lack of coin state 
	if ( m_eState == eRobot_Playing && isRobotSitDown() )
	{
		stTaxasPeerBaseData* playerData = m_pScene->getPokerData()->getTaxasPlayerData(m_nMySeatIdx);
		if ( (playerData->nStateFlag & eRoomPeer_LackOfCoin) == eRoomPeer_LackOfCoin )
		{
			standUp();
			if ( m_pScene->getPokerData()->getPlayerCnt() < m_pScene->getClientApp()->GetPlayerData()->pRobotItem->nApplyLeaveWhenPeerCount )
			{
				m_eState = eRobot_WaitPosSitDown ;
				printf("sit down failed , try seek other pos, target pos have other player \n");
			}
			else
			{
				m_eState = eRobot_StandUp ;
				printf("sit down failed , enough robot so keep standup  \n");
			}
			printf("i lack of coin , so standup , then sitdown , retake in coin name = %s\n",playerData->cName);
		}
	}
}

void CRobotControlTaxas::doDelayAction(void* pUserData )
{
	stTaxasPokerData* pData = m_pScene->getPokerData();
	uint8_t vPublicCard[5] = {0};
	uint8_t vHoldCard[2] = {0} ;

	memcpy(vPublicCard,pData->vPublicCardNums,sizeof(pData->vPublicCardNums));

	stTaxasPeerBaseData* playerData = pData->getTaxasPlayerData(m_nMySeatIdx);
	memcpy(vHoldCard,playerData->vHoldCard,sizeof(playerData->vHoldCard)); 

	uint8_t nTimesBlind = float(pData->nMostBetCoinThisRound) / float( pData->nLittleBlind * 2 ) + 0.5;

	uint8_t nRound = 0 ;
	if ( vPublicCard[4] )
	{
		nRound = 3 ;
	}
	else if ( vPublicCard[3] )
	{
		nRound = 2 ;
	}
	else if ( vPublicCard[2] )
	{
		nRound = 1 ;
	}

	uint8_t nCardType = 0 ;
	uint8_t nKeyCardFaceValue = 0 ;
	uint8_t nContributeCnt = 0 ;
	if ( 0 == nRound )
	{
		nContributeCnt = 2 ;
		CCard left , right ;
		left.RsetCardByCompositeNum(vHoldCard[0]);
		right.RsetCardByCompositeNum(vHoldCard[1]);
		nKeyCardFaceValue = left.GetCardFaceNum(true) > right.GetCardFaceNum(true) ? left.GetCardFaceNum(true) : right.GetCardFaceNum(true) ;
		if ( left.GetCardFaceNum() == right.GetCardFaceNum() )
		{
			nCardType = CTaxasPokerPeerCard::eCard_YiDui ;
		}
		else if ( left.GetType() == right.GetType() )
		{
			nCardType = CTaxasPokerPeerCard::eCard_TongHua ;
		}
		else if ( abs(left.GetCardFaceNum() - right.GetCardFaceNum()) == 1 )
		{
			nCardType = CTaxasPokerPeerCard::eCard_ShunZi ;
		}
		else
		{
			nCardType = CTaxasPokerPeerCard::eCard_GaoPai ;
		}
	}
	else
	{
		CTaxasPokerPeerCard peerCard ;
		peerCard.AddCardByCompsiteNum(vHoldCard[0]);
		peerCard.AddCardByCompsiteNum(vHoldCard[1]) ;
		for ( int nidx = 0 ; nidx < 5 ; ++nidx )
		{
			if ( vPublicCard[nidx] == 0 )
			{
				break;
			}
			peerCard.AddCardByCompsiteNum(vPublicCard[nidx]) ;
		}

		nCardType = peerCard.GetCardTypeForRobot(nContributeCnt,nKeyCardFaceValue);
	}

	int nOutAddTimesBlindBet = 0 ;
	CTaxasAINode::eAIActionType eAiAct = m_tAiCore.getAction(nRound,nCardType,nContributeCnt,nKeyCardFaceValue,nTimesBlind,nOutAddTimesBlindBet) ;

	stMsgTaxasPlayerAct msg ;
	msg.nRoomID = pData->nRoomID ;
	msg.nValue = nOutAddTimesBlindBet*pData->nLittleBlind*2 + pData->getPlayerAddCoinLowLimit(m_nMySeatIdx) ;

	switch (eAiAct)
	{
	case CTaxasAINode::eAIAct_Follow:
		msg.nPlayerAct = eRoomPeerAction_Follow;
		if ( pData->nMostBetCoinThisRound == playerData->nBetCoinThisRound )
		{
			msg.nPlayerAct = eRoomPeerAction_Pass ;
		}
		break;
	case CTaxasAINode::eAIAct_Pass:
		msg.nPlayerAct = eRoomPeerAction_Pass;
		
		if ( pData->nMostBetCoinThisRound != playerData->nBetCoinThisRound )
		{
			msg.nPlayerAct = eRoomPeerAction_GiveUp ;
		}
		break;
	case CTaxasAINode::eAIAct_Add:
		msg.nPlayerAct = eRoomPeerAction_Add;
		if ( msg.nValue >= playerData->nTakeInMoney )
		{
			msg.nValue = playerData->nTakeInMoney ;
			msg.nPlayerAct = eRoomPeerAction_AllIn;
			printf("coin too little so , all in , can not add \n");
			if ( msg.nValue > pData->getPlayerAddCoinUpLimit(playerData->nSeatIdx) )
			{
				msg.nValue = pData->getPlayerAddCoinUpLimit(playerData->nSeatIdx) ;
			}
		}
		break;
	case CTaxasAINode::eAIAct_AllIn:
		msg.nPlayerAct = eRoomPeerAction_AllIn;
		msg.nValue = playerData->nTakeInMoney ;
		if ( msg.nValue > pData->getPlayerAddCoinUpLimit(playerData->nSeatIdx) )
		{
			msg.nValue = pData->getPlayerAddCoinUpLimit(playerData->nSeatIdx) ;
		}
		break;
	default:
		break;
	}

	m_pScene->SendMsg(&msg,sizeof(msg)) ;
	printf("do my action act = %d , value = %I64d \n",msg.nPlayerAct,msg.nValue);
}

void CRobotControlTaxas::setScene(CTaxasPokerScene* pScene)
{
	m_pScene = pScene ;
}

void CRobotControlTaxas::waitAction()
{
	float nRate = (float)rand() / float(RAND_MAX);
	nRate = 1.0f + nRate * 3.0f ;
	fireDelayAction(nRate,nullptr);

	printf("wait me action \n");
}

void CRobotControlTaxas::onSelfStandUp()
{
	m_nMySeatIdx = MAX_PEERS_IN_TAXAS_ROOM ;

	if ( m_eState == eRobot_WaitStandUpReSitDown )
	{
		m_eState = eRobot_WaitPosSitDown ;
		printf("standup, but go in seek pos to sit down \n");
	}
	else if ( m_eState == eRobot_WaitLeaving )
	{
		// do nothing 
		printf("standup for leave \n");
		return ;
	}
	else
	{
		m_eState = eRobot_WaitPosSitDown ;
		printf("what situation , standup here , money lose all? \n");
	}
}

void CRobotControlTaxas::onSelfSitDown(uint8_t nMySeatIdx)
{
	if ( m_nMySeatIdx != MAX_PEERS_IN_TAXAS_ROOM  )
	{
		printf("i already sitdown why sit twice ? ");
	}

	m_nMySeatIdx = nMySeatIdx ;
	m_eState = eRobot_Playing ;
	printf("sit down success , widthdraw money \n");
}

void CRobotControlTaxas::onSitDownFailed( uint8_t nRet )
{
	if ( nRet == 1 )
	{
		if ( m_pScene->getPokerData()->getPlayerCnt() < m_pScene->getClientApp()->GetPlayerData()->pRobotItem->nApplyLeaveWhenPeerCount )
		{
			m_eState = eRobot_WaitPosSitDown ;
			printf("sit down failed , try seek other pos, target pos have other player \n");
		}
		else
		{
			m_eState = eRobot_StandUp ;
			printf("sit down failed , enough robot so keep standup  \n");
		}		
		
	}
	else if ( 2 == nRet )
	{
		standUp();
		m_eState = eRobot_StandUp ;
		printf("sit down failed , already sit down, standup again  \n");
	}
	else
	{
		leave();
		printf("sit down failed nRet = %d, I just leave \n",nRet );
	}
}

void CRobotControlTaxas::onSelfGiveUp()
{
	if ( isRobotSitDown() == false )
	{
		printf("robot do not sitdown , how to give up ? \n");
		return ;
	}

	if ( m_pScene->getPokerData()->getPlayerCnt() >= m_pScene->getClientApp()->GetPlayerData()->pRobotItem->nApplyLeaveWhenPeerCount )
	{
		standUp();
		m_eState = eRobot_StandUp ;
	}
	else
	{
		stTaxasPeerBaseData* playerData = m_pScene->getPokerData()->getTaxasPlayerData(m_nMySeatIdx);
		if ( playerData->nTakeInMoney > m_pScene->getPokerData()->nMaxTakeIn * 4 )
		{
			standUp();
			m_eState = eRobot_WaitStandUpReSitDown ;

			printf("standup for save coin, win too much haha \n");
		}
	}
}

void CRobotControlTaxas::orderSitdown()
{
	if ( eRobot_StandUp == m_eState )
	{
		m_eState = eRobot_WaitPosSitDown ;
		printf("try to sit down robot too few\n");
	}
}

void CRobotControlTaxas::onRecivedAllRoomPlayers( uint8_t nCnt )
{
	if ( nCnt < m_pScene->getClientApp()->GetPlayerData()->pRobotItem->nApplyLeaveWhenPeerCount )
	{
		orderSitdown();
	}
}

void CRobotControlTaxas::onWithdrawMoneyFailed()
{
	printf("take in coin error , not enough \n");
	leave();
}

void CRobotControlTaxas::standUp()
{
	stMsgTaxasPlayerStandUp msg ;
	msg.nRoomID = m_pScene->getPokerData()->nRoomID ;
	m_pScene->SendMsg(&msg,sizeof(msg)) ;
	stTaxasPeerBaseData* playerData = m_pScene->getPokerData()->getTaxasPlayerData(m_nMySeatIdx);
	if ( playerData )
	{
		m_pScene->getClientApp()->GetPlayerData()->OnWinCoin(playerData->nTakeInMoney,false);
	}
}

void CRobotControlTaxas::TryingSitDown()
{
	if ( m_eState == eRobot_WaitPosSitDown && m_pScene->getPokerData()->nRoomID )
	{
		stMsgTaxasPlayerSitDown sitDown ;
		sitDown.nRoomID = m_pScene->getPokerData()->nRoomID ;
		uint8_t nIdx = -1 ;
		for ( int idx = rand() % MAX_PEERS_IN_TAXAS_ROOM  ; idx < m_pScene->getPokerData()->nMaxSeat + MAX_PEERS_IN_TAXAS_ROOM ; ++idx )
		{
			uint8_t nFindIdx = idx % MAX_PEERS_IN_TAXAS_ROOM ;
			if ( m_pScene->getPokerData()->getTaxasPlayerData(nFindIdx)->nUserUID == 0 )
			{
				nIdx = nFindIdx ;
				break; ;
			}
		}

		if ( nIdx != -1 )
		{
			sitDown.nSeatIdx = nIdx ;
			sitDown.nTakeInMoney = m_pScene->getPokerData()->nMaxTakeIn * 0.8 ;
			sitDown.nTakeInMoney = sitDown.nTakeInMoney <= m_pScene->getClientApp()->GetPlayerData()->GetCoin(false) ? sitDown.nTakeInMoney : m_pScene->getClientApp()->GetPlayerData()->GetCoin(false) ;
			m_pScene->SendMsg(&sitDown,sizeof(sitDown));
			m_eState = eRobot_SitingDown;
			printf("find a seat , sit down \n");
		}	
	}
}

void CRobotControlTaxas::onGameEnd( uint8_t nCnt )
{
	if ( nCnt >= m_pScene->getClientApp()->GetPlayerData()->pRobotItem->nApplyLeaveWhenPeerCount )
	{
		printf("too many robot try leave : idx = %d \n",m_nMySeatIdx ) ;
		if ( ( rand() % 100) <= 30  )
		{
			printf("too many robot rand do standup : idx = %d \n",m_nMySeatIdx ) ;
			standUp();
			m_eState = eRobot_StandUp ; 
		}
	}
}