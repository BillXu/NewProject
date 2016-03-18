#pragma once
#include "ServerCommon.h"
#include "json/json.h"
#include "Singleton.h"
#include "AutoBuffer.h"
class IServerApp ;
class CSendPushNotification
	:public CSingleton<CSendPushNotification>
{
public:
	CSendPushNotification():m_refAutoBuffer(512){}
	void reset();
	void addTarget(uint32_t nUserUID );
	void setContent(const char* pContent,uint32_t nFlag );
	CAutoBuffer* getNoticeMsgBuffer();
protected:
	Json::Value m_arrayTargetIDs ;
	Json::Value m_tFinalContent ;
	CAutoBuffer m_refAutoBuffer ;
};