#include "ChargeNotifyAgentSys.h"
#include <json/json.h>
#include "VerifyRequest.h"
#include <cassert>
#include "log4z.h"
#include "ConfigDefine.h"
#include <iostream>
ChargeNotifyAgentTask::ChargeNotifyAgentTask(uint32_t nTaskID)
	:ITask(nTaskID)
{
	m_tHttpRequest.init("http://q.youhoox.com/index.php");
	//#if defined(GAME_IN_REVIEW)
	//#endif 
	m_tHttpRequest.setDelegate(this);
}

void ChargeNotifyAgentTask::setNotifyContent(uint32_t nPlayerUID, uint32_t nFee, const char* trade_no )
{
	std::ostringstream ss;
	ss << "ct=notify&ac=pay&player_uid=" << nPlayerUID << "&amount=" << nFee << "&trade_no=" << trade_no ;
	m_strTransferString = ss.str();
}

uint8_t ChargeNotifyAgentTask::performTask()
{
	if (m_strTransferString.empty())
	{
		return 1;
	}
	auto ret = m_tHttpRequest.performRequest(nullptr, m_strTransferString.c_str(), m_strTransferString.size(), nullptr);
	if (ret)
	{
		return 0;
	}
	return 1;
}

void ChargeNotifyAgentTask::onHttpCallBack(char* pResultData, size_t nDatalen, void* pUserData, size_t nUserTypeArg)
{
	assert(pResultData != nullptr && "must not null");
	Json::Reader reader;
	m_jsResult.clear();
	if (reader.parse(pResultData, pResultData + nDatalen, m_jsResult))
	{

	}
	else
	{
		printf("parse json data error , from hei zi Agent Svr \n");
		return;
	}
	return;
}
