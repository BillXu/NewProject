#pragma once
#include "robotControl.h"
#include "TaxasRobotAIConfig.h"
#include "CommonData.h"
class CTaxasPokerScene ;
class CRobotControlTaxas
	:public CRobotControl
{
public:
	enum  eRobotState
	{
		eRobot_StandUp,
		eRobot_WaitLeaving,
		eRobot_WaitPosSitDown,
		eRobot_SitingDown,
		eRobot_Playing,
		eRobot_WaitStandUpReSitDown,
		eRobot_Max,
	};
public:
	bool init(const char* cAiFile,CTaxasPokerScene* pScene);
	void update(float fdeta );
	void leave();
	void doDelayAction(void* pUserData );
	void setScene(CTaxasPokerScene* pScene);
	void waitAction();
	void onSelfStandUp();
	void onSelfSitDown( uint8_t nMySeatIdx );
	void onSitDownFailed( uint8_t nRet );
	void onWithdrawMoneyFailed();
	void onSelfGiveUp();
	uint8_t getMySeatIdx(){ return m_nMySeatIdx ; }
	bool isRobotSitDown(){ return m_nMySeatIdx != MAX_PEERS_IN_TAXAS_ROOM ; }
protected:
	void standUp();
	void TryingSitDown();
protected:
	CTaxasPokerScene* m_pScene ;
	CTaxasRobotAI m_tAiCore ;

	eRobotState m_eState ;
	uint8_t m_nMySeatIdx ;
};