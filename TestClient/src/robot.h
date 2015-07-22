#pragma once
#include "MessageDefine.h"
class CRobotState ;
class CRobot 
{
public:
	enum  eRobotState
	{
		eRobot_Idle,
		eRobot_Taxas,
		eRobot_Max,
	};
public:
	bool init();
	void onMsg(stMsg* pmsg);
	void update(float fDeta );
	bool sendMsg(stMsg* pmsg, uint16_t nLen );
	void changeToState(eRobotState eS );
protected:
	eRobotState m_eCurState;
	stPlayerBrifData m_tData ;
	CRobotState* m_vState[eRobot_Max];
};