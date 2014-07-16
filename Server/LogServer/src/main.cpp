#include "DBApp.h"
#include <Dbghelp.h>
#include <iostream>
#include "ThreadMod.h"
#pragma comment( lib, "DbgHelp" )
//#include "mutex.h"
//#include <my_global.h>
//#include "mysql.h"
//#include "DataBaseThread.h"
//#include "MySqlData.h"
//#include "DBRequest.h"
//#include "RakNetTypes.h"
//#include "RakPeerInterface.h"
//#include "ServerNetwork.h"
//#include "DBPlayerManager.h"


CDBServerApp theApp ;
class CGetInput
	:public CThreadT
{
	void __run()
	{
		char c ;
		while ( 1 )
		{
			c = getchar();
			if ( c == 'q' || c == 'Q')
			{
				theApp.Stop();
				break; 
			}
			Sleep(10);
		}
	}
};

LONG WINAPI MyUnhandledExceptionFilter( struct _EXCEPTION_POINTERS* ExceptionInfo )
{
	HANDLE lhDumpFile = CreateFile(L"DumpFileForDB.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL ,NULL);

	MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
	loExceptionInfo.ExceptionPointers = ExceptionInfo;
	loExceptionInfo.ThreadId = GetCurrentThreadId();
	loExceptionInfo.ClientPointers = TRUE;
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),lhDumpFile, MiniDumpNormal, &loExceptionInfo, NULL, NULL);

	CloseHandle(lhDumpFile);

	return EXCEPTION_EXECUTE_HANDLER;
}


void RunFunc ( CDBServerApp* pApp )
{
	// exception 
	__try
	{
		while (  pApp->IsRunning() )
		{
			pApp->MainLoop() ;
			Sleep(5);
		}
	}
	__except(MyUnhandledExceptionFilter(GetExceptionInformation()))
	{
	}
}

int main()
{
	//SetConsoleCtrlHandler(ConsoleHandler, TRUE); 
	CGetInput input ;
	input.Start();
	theApp.Init();
#ifdef NDEBUG
	RunFunc(&theApp) ;
#else
	while (  theApp.IsRunning() )
	{
		theApp.MainLoop() ;
		Sleep(5);
	}
#endif
	theApp.OnExit();
	Sleep(3000) ; // wait other thread finish work ;
	return 0 ; 
}