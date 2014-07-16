#include "ServerConfig.h"
#include "LogManager.h"
CSeverConfigMgr::CSeverConfigMgr()
{
	memset(m_vAllSvrConfig,0,sizeof(m_vAllSvrConfig)) ;
}

CSeverConfigMgr::~CSeverConfigMgr()
{

}

bool CSeverConfigMgr::OnPaser(CReaderRow& refReaderRow )
{
	unsigned char cSvrType = refReaderRow["svrType"]->IntValue();
	if ( cSvrType >= eSvrType_Max )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("server config error , type error , type = %d",cSvrType) ;
		return false;
	}
	m_vAllSvrConfig[cSvrType].nPort = refReaderRow["svrPort"]->IntValue();
	m_vAllSvrConfig[cSvrType].nSvrType = cSvrType;
	if (strlen(refReaderRow["svrIP"]->StringValue().c_str()) >= 16 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("too long ip address = %s",refReaderRow["svrIP"]->StringValue().c_str()) ;
		return false ;
	}
	memcpy(m_vAllSvrConfig[cSvrType].strIPAddress,refReaderRow["svrIP"]->StringValue().c_str(),strlen(refReaderRow["svrIP"]->StringValue().c_str()));
	memcpy(m_vAllSvrConfig[cSvrType].strAccount,refReaderRow["account"]->StringValue().c_str(),strlen(refReaderRow["account"]->StringValue().c_str()));
	memcpy(m_vAllSvrConfig[cSvrType].strPassword,refReaderRow["password"]->StringValue().c_str(),strlen(refReaderRow["password"]->StringValue().c_str()));
	return true ;
}