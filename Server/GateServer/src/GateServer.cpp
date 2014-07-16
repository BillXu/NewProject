#include "GateServer.h"
#include "Timer.h"
#include "CommonDefine.h"
#define MAX_INCOME_PLAYER 20000
CGateServer* CGateServer::s_GateServer = NULL ;
CGateServer* CGateServer::SharedGateServer()
{
	return s_GateServer ;
}

CGateServer::CGateServer()
{
	m_bRunning = true ;
	m_pNetWork = NULL ;
	m_pGateManager = NULL ;
	m_pTimerManager = NULL ;
	if ( s_GateServer )
	{
		assert(0&&"only once should");
	}
}

CGateServer::~CGateServer()
{
	m_pNetWork->ShutDown() ;
	delete m_pNetWork ;
	delete m_pGateManager ;
	delete m_pTimerManager ;
	s_GateServer = NULL ;
}

void CGateServer::Init()
{
	if ( s_GateServer )
	{
		assert(0&&"only once should");
	}
	s_GateServer = this ;
	// client player mgr ;
	m_pGateManager = new CGateClientMgr ;
	
	m_stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");
	// net work 
	stServerConfig* pGateConfig = m_stSvrConfigMgr.GetServerConfig(eSvrType_Gate) ;
	m_pNetWork = new CServerNetwork ;
	m_pNetWork->StartupNetwork(pGateConfig->nPort,MAX_INCOME_PLAYER,pGateConfig->strPassword);
	m_pNetWork->AddDelegate(m_pGateManager) ;
	// timer manager ;
	m_pTimerManager = new CTimerManager ;

}

void CGateServer::RunLoop()
{
	while ( m_bRunning )
	{
		if ( m_pNetWork )
		{
			m_pNetWork->RecieveMsg() ;
		}

		if ( m_pTimerManager )
		{
			m_pTimerManager->Update();
		}
		Sleep(2);
	}

	m_pNetWork->ShutDown() ;
}

