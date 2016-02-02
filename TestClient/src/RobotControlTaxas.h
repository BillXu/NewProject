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
		eRobot_StandingUp,
		eRobot_StandUp,

		eRobot_StandingUpForSaveCoin,

		eRobot_StandingUpForLeave,

		eRobot_Leave,
		eRobot_WantSitDown,

		eRobot_SitingDown,
		eRobot_SitDown,

		eRobot_Playing = eRobot_SitDown,

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
	void onSitdownPlayerCntChanged( uint8_t nCurCnt );
	uint8_t getMySeatIdx(){ return m_nMySeatIdx ; }
	bool isRobotSitDown(){ return m_nMySeatIdx != MAX_PEERS_IN_TAXAS_ROOM ; }
	void onEnterRoom();
	void onGameEnd( uint8_t nCnt);
	void setState( eRobotState eState, bool isNeedSvrBack = true );
	void onRoomDead();
protected:
	void standUp();
	void findSeatIdxSitDown();
protected:
	CTaxasPokerScene* m_pScene ;
	CTaxasRobotAI m_tAiCore ;

	eRobotState m_eState ;
	uint8_t m_nMySeatIdx ;
	float m_fTicketForCheckCanSitDown ;
	float m_fBugStateTime ;
};