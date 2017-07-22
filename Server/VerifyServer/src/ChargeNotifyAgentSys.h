#pragma once
#include "ITask.h"
#include "httpRequest.h"
#include "json/json.h"
class ChargeNotifyAgentTask
	:public ITask
	, public CHttpRequestDelegate
{
public:
	ChargeNotifyAgentTask(uint32_t nTaskID);
	uint8_t performTask()override;
	void onHttpCallBack(char* pResultData, size_t nDatalen, void* pUserData, size_t nUserTypeArg)override;
	void setNotifyContent(uint32_t nPlayerUID, uint32_t nFee, const char* pSiealNum);
	Json::Value& getResultJson() { return m_jsResult; }
protected:
	CHttpRequest m_tHttpRequest;
	std::string m_strTransferString;
	Json::Value m_jsResult;
};
