#pragma once
#include "robotControl.h"
#include "CommonDefine.h"
class CNiuNiuScene ;
class CRobotControlNiuNiu
	:public CRobotControl
{
public:
	enum  eActType 
	{
		eAct_TryBanker,
		eAct_Bet,
		eAct_CaculateCards,
		eAct_Max,
	};
public:
	void onCanTryBanker();
	void onCanBet();
	uint32_t getTakeInCoinWhenSitDown(){ return 0 ;}
	void doDelayAction(uint8_t nActType,void* pUserData )override ;
	void informRobotAction(uint8_t nActType);

};