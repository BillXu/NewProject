#pragma once
#include "robotControl.h"
#include "PeerCard.h"
class CRobotControlGolden
	:public CRobotControl
{
public:
	CRobotControlGolden();
	~CRobotControlGolden();
	void doDelayAction(uint8_t nActType,void* pUserData )override;
	void informRobotAction(uint8_t nActType)override ;
	uint32_t getTakeInCoinWhenSitDown()override ;
	void onGameBegin()override
	{
		nCardtype = CPeerCard::ePeerCard_Max ;
	}
	bool canViewCard();
	bool canPKCard();
	bool canAddBet();
	bool canGiveUp();
	uint8_t randPKTargetIdx() ;
	void onSelfSitDown()override;
	void onGameEnd()override;
protected:
	CPeerCard::ePeerCardType nCardtype ;
};