#include "VerifyApp.h"
#include "CommonDefine.h"
#include "LogManager.h"
#include "TaskPoolModule.h"

bool CVerifyApp::init()
{
	IServerApp::init();
	CLogMgr::SharedLogMgr()->SetOutputFile("VerifySvr");

	CSeverConfigMgr stSvrConfigMgr ;
	stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");
	stServerConfig* pConfig = stSvrConfigMgr.GetServerConfig(eSvrType_Center) ;
	if ( pConfig == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("center svr config is null , so can not connected to !") ;
		return false;
	}
	setConnectServerConfig(pConfig);

	CLogMgr::SharedLogMgr()->SystemLog("START verify server !") ;

	installModule(eMod_Pool);
	return true;
}

uint16_t CVerifyApp::getLocalSvrMsgPortType()
{
	return ID_MSG_PORT_VERIFY ;
}

IGlobalModule* CVerifyApp::createModule( uint16_t eModuleType )
{
	auto p = IServerApp::createModule(eModuleType) ;
	if ( p )
	{
		return p ;
	}

	if ( eModuleType == eMod_Pool )
	{
		p = new CTaskPoolModule();
	}
	return p ;
}