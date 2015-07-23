#include "Client.h"
#include "MessageDefine.h"
#include "CommonDefine.h"
#include "LoginScene.h"
CClientRobot::CClientRobot()
{
	m_pPlayerData = new CPlayerData ;
	m_pCurentScene = NULL ;
	m_pLastScene = NULL ;
}

CClientRobot::~CClientRobot()
{
	delete m_pPlayerData ;
	if ( m_pCurentScene )
	{
		m_pCurentScene->OnEixtScene();
		delete m_pCurentScene ;
	}
}

void CClientRobot::Init()
{
	m_pNetWork.SetupNetwork();
	m_pCurentScene = new CLoginScene(this);
	m_pCurentScene->OnEnterScene();
	//m_pNetWork.ConnectToServer("127.0.0.1",50001,"123456") ;  // inner net ;-
	m_pNetWork.ConnectToServer("203.186.75.136",50001,"123456") ;  // Out net ;
	m_pNetWork.AddMessageDelegate(m_pPlayerData) ;
}

void CClientRobot::Run()
{
	clock_t nTick = clock();
	while ( 1  )
	{
		m_pNetWork.ReciveMessage();

		if ( m_pCurentScene)
		{
			float fElasp = (float)(clock() - nTick ) / (float)CLOCKS_PER_SEC ;
			nTick = clock();
			m_pCurentScene->OnUpdate(fElasp);
		}
		Sleep(2);
		if ( m_pLastScene )
		{
			delete m_pLastScene ;
			m_pLastScene = NULL ;
		}
	}
	m_pNetWork.ShutDown();
}

void CClientRobot::ChangeScene(IScene* pTargetScene )
{
	if ( m_pCurentScene )
	{
		m_pCurentScene->OnEixtScene() ;
		m_pCurentScene = m_pCurentScene;
	}
	pTargetScene->OnEnterScene() ;
	m_pCurentScene = pTargetScene ;
}

bool CClientRobot::SetRobotAI(stRobotAI* pRobotAi )
{
	if ( pRobotAi == NULL )
	{
		printf( "Robot AI Is NULL\n" ) ;
		return false;
	}
	m_pRobotAI = pRobotAi ;
	return true ;
}