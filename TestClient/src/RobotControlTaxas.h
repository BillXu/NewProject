#pragma once
#include "robotControl.h"
#include "TaxasRobotAIConfig.h"
#include "CommonData.h"
class CRobotControlTaxas
	:public CRobotControl
{
public:
	void setAiFile(const char* pAiFileName );
	void informRobotAction(uint8_t nActType)override;
	uint32_t getTakeInCoinWhenSitDown()override;
	void doDelayAction(uint8_t nActType,void* pUserData )override;
protected:
	CTaxasRobotAI m_tAiCore ;
};