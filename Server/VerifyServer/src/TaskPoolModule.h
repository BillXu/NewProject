#pragma once
#include "IGlobalModule.h"
#include "TaskPool.h"
struct stVerifyRequest ;
class CTaskPoolModule
	:public IGlobalModule
	,public ITaskFactory
{
public:
	enum eTask
	{
		eTask_WechatOrder,
		eTask_AppleVerify,
		eTask_WechatVerify,
		eTask_DBVerify,
		eTask_Max,
	};
public:
	void init( IServerApp* svrApp )override ;
	void onExit()override ;
	bool onMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)override ;
	bool onMsg(Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID)override ;
	void update(float fDeta )override ;
	void testFunc();
	ITask::ITaskPrt createTask( uint32_t nTaskID )override ;
	CTaskPool& getPool(){ return m_tTaskPool ;}

protected:
	// logic 
	void onWechatOrder( stMsg* pOrder, eMsgPort eSenderPort , uint32_t nSessionID );
	void onVerifyMsg( stMsg* pOrder, eMsgPort eSenderPort , uint32_t nSessionID );
	void sendVerifyResult(std::shared_ptr<stVerifyRequest> & pResult );
protected:
	CTaskPool m_tTaskPool ;
};