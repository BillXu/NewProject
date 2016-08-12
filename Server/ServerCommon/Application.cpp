#include "Application.h"
#include "LogManager.h"
#pragma comment(lib,"JsonDll.lib")
#include <cassert>
CApplication::CApplication(IServerApp* pApp )
{
	CLogMgr::SharedLogMgr()->SetOutputFile(nullptr);
	m_pApp = pApp ;
}

IServerApp* CApplication::getApp()
{
	return m_pApp ;
}

void CApplication::stopRuning()
{
	m_isRunning = false ;
}

bool& CApplication::isRunning()
{
	return this->m_isRunning ;
}

void CApplication::startApp()
{
	_CrtSetReportMode(_CRT_ASSERT, 0);

	auto nRet = m_pApp->init();
	assert(nRet && "init svr error");
	if ( nRet == false )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("svr init error") ;
		m_pApp = nullptr ;
		return ;
	}

	// create console input thread 
	DWORD threadID;
	HANDLE hThread;
	m_isRunning = true ;
	hThread = CreateThread(NULL,0,CApplication::consoleInput,this,0,&threadID); // 创建线程

	// run loop ;
	uint32_t nRunloop = 0 ;
	while ( m_pApp && isRunning() )
	{
		++nRunloop ;
		runAppLoop();
		CLogMgr::SharedLogMgr()->PrintLog("try another loop = %u",nRunloop) ;
		Sleep(800);
	}
}

void CApplication::runAppLoop()
{
	__try
	{
		m_pApp->run() ;
	}
	__except(CatchDumpFile::CDumpCatch::UnhandledExceptionFilterEx(GetExceptionInformation()))
	{
		CLogMgr::SharedLogMgr()->SystemLog("try to recover from exception") ;
	}
}

DWORD WINAPI CApplication::consoleInput(LPVOID lpParam)
{
	auto application = ((CApplication*)lpParam);
	char pBuffer[255] ;
	while(application->isRunning())
	{
		memset(pBuffer,0,sizeof(pBuffer)) ;
		scanf_s("%s",pBuffer,sizeof(pBuffer)) ;
		if ( strcmp(pBuffer,"exit") == 0 || strcmp(pBuffer,"Q") == 0 )
		{
			application->stopRuning();
			IServerApp* pAp = application->getApp() ;
			pAp->stop();
			printf("Closing!!!\n");
		}
		else
		{
			printf("Input exit or Q , to close the app \n") ;
		}
	}
	return 0;
}