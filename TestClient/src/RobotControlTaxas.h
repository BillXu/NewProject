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
	bool init(const char* cAiFile);
	void update(float fdeta );
	void leave();
	void doDelayAction(void* pUserData );
	void setScene(CTaxasPokerScene* pScene);
	void waitAction();
	void onSelfStandUp();
	void onSelfSitDown();
	void onSitDownFailed( uint8_t nRet );
	void onWithdrawMoneyFailed();
	void onSelfGiveUp();
	void bindPlayerData(stTaxasPeerBaseData* pData);
protected:
	void standUp();
	void TryingSitDown();
protected:
	CTaxasPokerScene* m_pScene ;
	CTaxasRobotAI m_tAiCore ;

	stTaxasPeerBaseData* m_pPlayerData ;
	eRobotState m_eState ;
};