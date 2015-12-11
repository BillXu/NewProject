#pragma once
#include "TaxasPokerPeerCard.h"
#include "TaxasRobotAIConfig.h"
class CTaxasRobotBrain
{
public:
	CTaxasRobotBrain(const char* aiFile ) ;
	void resetState();
	void recivedCard( CCard& card );
	CTaxasAINode::eAIActionType getAction(unsigned int nMostBetTimesBlind, int& addTimesBlind, char nDemondRound = -1 );
private:
	CTaxasPokerPeerCard m_tPeerCard;
	CTaxasRobotAI m_tAiCore ;
};