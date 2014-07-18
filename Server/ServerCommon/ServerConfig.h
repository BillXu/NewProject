#pragma once
#include "IConfigFile.h"
#include "CommonDefine.h"
struct stServerConfig
{
	unsigned char nSvrType ;
	char strIPAddress[16] ;
	unsigned short nPort ;
	char strAccount[80];
	char strPassword[80];
};

class CSeverConfigMgr
	:public IConfigFile
{
public:
	CSeverConfigMgr();
	~CSeverConfigMgr();
	virtual bool OnPaser(CReaderRow& refReaderRow ) ;
	stServerConfig* GetServerConfig(eServerType cSvrType ){ return &m_vAllSvrConfig[cSvrType]; }
protected:
	stServerConfig m_vAllSvrConfig[eSvrType_Max] ;
};