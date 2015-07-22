#pragma once
struct stMsg;
class CRobotControl
{
public:
	virtual bool init();
	virtual ~CRobotControl(){}
	virtual void update(float fdeta );
	void fireDelayAction(float fDelay,void* pUserData );
	virtual void doDelayAction(void* pUserData );
	virtual bool onMsg(stMsg* pmsg);
	virtual void leave(){}
protected:
	bool m_bHaveDelayActionTask;
	float m_fDelayActionTicket ;
	void* m_pDelayActionUserData ;
};