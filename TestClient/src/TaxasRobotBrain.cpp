#include "TaxasRobotBrain.h"
CTaxasRobotBrain::CTaxasRobotBrain(const char* aiFile )
{
	assert(m_tAiCore.init(aiFile) && "pls check file not exist or format error" ) ;
}

void CTaxasRobotBrain::resetState()
{
	m_tPeerCard.Reset();
}

void CTaxasRobotBrain::recivedCard( CCard& card )
{
	if ( m_tPeerCard.getAllCardCnt() < 7 )
	{
		m_tPeerCard.AddCardByCompsiteNum(card.GetCardCompositeNum()) ;
	}
	else
	{
		assert( 0 && "you forget reset state or add too more cards, logic error");
	}
}

CTaxasAINode::eAIActionType CTaxasRobotBrain::getAction(unsigned int nMostBetTimesBlind, int& addTimesBlind, char nDemondRound )
{
	unsigned char nRound = 0 ;
	switch ( m_tPeerCard.getAllCardCnt() )
	{
	case 2:
		{
			nRound = 0 ;
		}
		break;
	case 5:
		{
			nRound = 1 ;
		}
		break;
	case 6:
		{
			nRound = 2 ;
		}
		break;
	case 7:
		{
			nRound = 3 ;
		}
		break;
	default:
		assert(0 && "receive card error ,pls check you logic" );
		return CTaxasAINode::eAIAct_Pass;
	}

	if ( nDemondRound != -1 )
	{
		if ( nRound >= nDemondRound )
		{
			nRound = nDemondRound ;
		}
		else
		{
			assert(0 && "receieved cards not enough for your demond round " );
		}
	}

	unsigned char nCardType = 0 ;
	unsigned char nKeyCardFaceValue = 0 ;
	unsigned char nContributeCnt = 0 ;
	if ( 0 == nRound )
	{
		nContributeCnt = 2 ;
		CCard left , right ;
		unsigned char vHoldCard[2] = {m_tPeerCard.GetPrivateCard(0),m_tPeerCard.GetPrivateCard(1)} ;
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
		nCardType = m_tPeerCard.GetCardTypeForRobot(nContributeCnt,nKeyCardFaceValue);
	}

    return m_tAiCore.getAction(nRound,nCardType,nContributeCnt,nKeyCardFaceValue,nMostBetTimesBlind,addTimesBlind) ;
}