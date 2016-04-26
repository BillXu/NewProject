#include "RobotControlTaxas.h"
#include "TaxasPokerScene.h"
#include "TaxasPokerPeerCard.h"
#include "TaxasMessageDefine.h"
#include "PlayerData.h"
#include "Client.h"

void CRobotControlTaxas::setAiFile(const char* pAiFileName )
{
	if (!m_tAiCore.init(pAiFileName))
	{
		printf("load ai file error %s",pAiFileName);
	}
}

void CRobotControlTaxas::informRobotAction(uint8_t nActType)
{
	// just do bet ;
	fireDelayAction(0,0,nullptr) ;
	printf("it is my turn to act uid = %d\n",getUserUID()) ;
}

uint32_t CRobotControlTaxas::getTakeInCoinWhenSitDown()
{
	CTaxasPokerData* pData = (CTaxasPokerData*)getRoomData();
	return ( pData->nMiniTakeIn + pData->nMaxTakeIn ) * 0.7 ;
}

void CRobotControlTaxas::doDelayAction(uint8_t nActType,void* pUserData )
{
	
	CTaxasPokerData* pData = (CTaxasPokerData*)getRoomData();
	uint8_t vPublicCard[5] = {0};
	uint8_t vHoldCard[2] = {0} ;

	memcpy(vPublicCard,pData->vPublicCardNums,sizeof(pData->vPublicCardNums));

	stTaxasPlayer* playerData = (stTaxasPlayer*)pData->getPlayerByIdx(getSeatIdx());
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

	if ( vHoldCard[0] == 0 )
	{
		printf("uid = %u hold card is null round = %u \n",playerData->nUserUID,nRound);
		return ;
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
	msg.nRoomID = pData->getRoomID() ;
	msg.nValue = nOutAddTimesBlindBet*pData->nLittleBlind*2 + pData->getPlayerAddCoinLowLimit(getSeatIdx()) ;

	if ( eAiAct == CTaxasAINode::eAIAct_Pass && getTempHalo() > 70 )
	{
		eAiAct = CTaxasAINode::eAIAct_Follow ;
	}

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
		if ( msg.nValue >= playerData->nCoin )
		{
			msg.nValue = playerData->nCoin ;
			msg.nPlayerAct = eRoomPeerAction_AllIn;
			printf("coin too little so , all in , can not add \n");
			if ( msg.nValue > pData->getPlayerAddCoinUpLimit(playerData->nIdx) )
			{
				msg.nValue = pData->getPlayerAddCoinUpLimit(playerData->nIdx) ;
			}
		}
		break;
	case CTaxasAINode::eAIAct_AllIn:
		msg.nPlayerAct = eRoomPeerAction_AllIn;
		msg.nValue = playerData->nCoin ;
		if ( msg.nValue > pData->getPlayerAddCoinUpLimit(playerData->nIdx) )
		{
			msg.nValue = pData->getPlayerAddCoinUpLimit(playerData->nIdx) ;
		}
		break;
	default:
		CRobotControl::doDelayAction(nActType,pUserData);
		return;
	}

	printf("do my act = %d , value = %d uid = %d,curMost = %lld, i betCoin = %d\n",msg.nPlayerAct,msg.nValue,getUserUID(),pData->nMostBetCoinThisRound,playerData->nBetCoinThisRound) ;
	sendMsg(&msg,sizeof(msg)) ;

	if ( msg.nPlayerAct == eRoomPeerAction_GiveUp && playerData->nCoin > pData->nMaxTakeIn * 2 )
	{
		printf("win too many coin , should stand up uid = %u\n",playerData->nUserUID) ;
		standUp();
	}

	if ( msg.nPlayerAct == eRoomPeerAction_GiveUp && playerData->bDelayLeave )
	{
		leaveRoom();
	}
}