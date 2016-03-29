#include <windows.h>
#include "GameServerApp.h"
#include <iostream>
#include <WinDef.h>
#include <Dbghelp.h>
#include "LogManager.h"
#pragma comment( lib, "DbgHelp" )
#pragma comment(lib,"JsonDll.lib")

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	bool bRunning = true;
	char pBuffer[255] ;
	while(bRunning)
	{
		memset(pBuffer,0,sizeof(pBuffer)) ;
		scanf("%s",pBuffer) ;
		if ( strcmp(pBuffer,"exit") == 0 || strcmp(pBuffer,"Q") == 0 )
		{
			bRunning = false ;
			CGameServerApp* pAp = (CGameServerApp*)lpParam ;
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

void CreateThred( CGameServerApp* pApp )
{
	DWORD threadID;
	HANDLE hThread;
	hThread = CreateThread(NULL,0,ThreadProc,pApp,0,&threadID); // 创建线程
}


LONG WINAPI MyUnhandledExceptionFilter( struct _EXCEPTION_POINTERS* ExceptionInfo )
{
	HANDLE lhDumpFile = CreateFile(L"DumpFileForDataServer.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL ,NULL);

	MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
	loExceptionInfo.ExceptionPointers = ExceptionInfo;
	loExceptionInfo.ThreadId = GetCurrentThreadId();
	loExceptionInfo.ClientPointers = TRUE;
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),lhDumpFile, MiniDumpNormal, &loExceptionInfo, NULL, NULL);

	CloseHandle(lhDumpFile);

	return EXCEPTION_EXECUTE_HANDLER;
}


void RunFunc ( CGameServerApp* pApp )
{
	// exception 
	__try
	{
		pApp->run() ;
	}
	__except(MyUnhandledExceptionFilter(GetExceptionInformation()))
	{
	}
}
#include <time.h>

int main()
{
	//-----
	//char p[20] = {0} ;
	//sprintf_s(p,"12E44E54") ;
	//std::string str (p,20);
	//std::string strNo = str.substr(0,str.find_first_of('E')) ;
	//int a = atoi(strNo.c_str());
	//printf("%d",a);
	//-----
	CGameServerApp theApp ;
	bool bok = theApp.init() ;
	CLogMgr::SharedLogMgr()->SetOutputFile("DataSvr");
	if ( !bok )
	{
		printf("init data svr error , start up error\n");
		char c ;
		scanf("%c",&c);
		return 0 ;
	}
	CreateThred(&theApp);
	RunFunc(&theApp);
	return 0 ;
}