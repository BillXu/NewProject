#include "QingJiaModule.h"
#include "LogManager.h"
void CQinJiaModule::init( IServerApp* svrApp )
{
	IGlobalModule::init(svrApp);
	getTaskPool().init(this,2);
}

void CQinJiaModule::onExit()
{
	IGlobalModule::onExit();
	getTaskPool().closeAll();
}

void CQinJiaModule::update(float fDeta )
{
	IGlobalModule::update(fDeta);
	getTaskPool().update();
}

ITask::ITaskPrt CQinJiaModule::createTask( uint32_t nTaskID )
{
	auto ptr = std::make_shared<CQinjiaTask>(nTaskID) ;
	return ptr ;
}

void CQinJiaModule::sendQinJiaRequest(const char* pApi,Json::Value& jsReqData ,lpQinjiaRequestCallBack lpFunc , Json::Value jsUserData )
{
	auto pPtr = std::static_pointer_cast<CQinjiaTask>(getTaskPool().getReuseTaskObjByID(1));
	pPtr->setRequest(pApi,jsReqData,lpFunc,jsUserData);
	getTaskPool().postTask(pPtr);
}

// real task 
CQinjiaTask::CQinjiaTask( uint32_t nTaskID )
	:ITask(nTaskID)
{
	m_lpCallBack = nullptr ;
	m_tHttpRequest.init("https://qplusapi.gotye.com.cn:8443/api/");
	m_tHttpRequest.setDelegate(this);

	setCallBack([this](ITask::ITaskPrt ptr ) { if ( m_lpCallBack ){ m_lpCallBack(m_jsResultData,m_jsUserData);} } ) ;
}

uint8_t CQinjiaTask::performTask()
{
	Json::StyledWriter sWrite ;
	std::string str = sWrite.write(m_jsReqData);
	return m_tHttpRequest.performRequest(m_strApi.c_str(),str.c_str(),str.size(),nullptr);
}

void CQinjiaTask::reset()
{
	m_lpCallBack = nullptr ;
	m_jsReqData.clear();
	m_jsResultData.clear();
	m_jsUserData.clear() ;
	m_strApi.clear();
}

void CQinjiaTask::onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData , size_t nUserTypeArg)
{
	bool bSuccess = nDatalen > 0 ;
	m_jsResultData.clear(); 
	if ( bSuccess )
	{
		Json::Reader reader ;
		reader.parse(pResultData,pResultData + nDatalen,m_jsResultData) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("club gotyp request failed type = %u ",nUserTypeArg) ;
		return ;
	}

	if ( m_jsResultData["errcode"].asUInt() != 200 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("club gotype request failed error code = %u type = %u", m_jsResultData["errcode"].asUInt(),nUserTypeArg) ;
		return ;
	}
}

void CQinjiaTask::setRequest( const char* pApi,Json::Value& jsReqData ,CQinJiaModule::lpQinjiaRequestCallBack lpFunc , Json::Value jsUserData )
{
	m_strApi = pApi ;
	m_jsReqData = jsReqData ;
	m_jsReqData["email"] = "378569952@qq.com" ;
	m_jsReqData["devpwd"] = "bill007" ;
#ifndef NDEBUG
	m_jsReqData["appkey"] = "e87f31bb-e86c-4d87-a3f3-57b3da76b3d6";
#else
	m_jsReqData["appkey"] = "abffee4b-deea-4e96-ac8d-b9d58f246c3f" ;
#endif // DEBUG
	m_lpCallBack = lpFunc ;
	m_jsUserData = jsUserData ;
}