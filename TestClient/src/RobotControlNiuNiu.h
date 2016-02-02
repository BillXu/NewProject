#pragma once
#include "robotControl.h"
#include "CommonDefine.h"
class CNiuNiuScene ;
class CRobotControlNiuNiu
	:public CRobotControl
{
public:
	enum  eRobotControlState
	{
		eRcs_StandUp,
		eRcs_Leave,
		eRcs_StandingUp,
		eRcs_SitingDown,
		eRcs_SitDown,
		eRcs_WaitToSitDown,
		eRcs_Max,
	};
public:
	bool init(CNiuNiuScene * pScene );
	void onCanTryBanker();
	void onCanBet();
	void onGameEnd(); 
	void doDelayAction(void* pUserData )override ;
	void update(float fdeta )override ;
	void updateWaitSitdown(float fdeta );
	bool onMsg(stMsg* pmsg)override ;
	void waitToSitdown();
	void standUp();
protected:
	CNiuNiuScene* m_pScene ;
	uint8_t m_nSelfIdx ;
	eRobotControlState m_eState ;

	float m_fWaitSitDownTicket ;
};