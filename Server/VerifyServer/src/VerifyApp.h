#pragma once
#include "VerifyRequest.h"
#include "AppleVerifyManager.h"
#include "DBVerifyManager.h"
#include "ServerNetwork.h"
#include "ServerConfig.h"
#include "ISeverApp.h"
#include "MiVerifyManager.h"
class CVerifyApp
	:public IServerApp
{
public:
	CVerifyApp();
	~CVerifyApp();
	void update(float fDeta );
	bool init();
	stVerifyRequest* GetRequestToUse();
	void PushVerifyRequestToReuse(stVerifyRequest* pRequest );
	void FinishVerifyRequest(stVerifyRequest* pRequest);
	bool onLogicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID )override;
	uint16_t getLocalSvrMsgPortType();
protected:
	LIST_VERIFY_REQUEST m_vListRequest ;
	CServerNetwork* m_pNetwork ;
	CAppleVerifyManager m_AppleVerifyMgr ;
	CMiVerifyManager m_MiVerifyMgr ;
	CDBVerifyManager m_DBVerifyMgr ;
};