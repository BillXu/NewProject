#include <windows.h>
#include <Dbghelp.h>
#include "LogSvrApp.h"
#include <iostream>
#include "ThreadMod.h"
#pragma comment( lib, "DbgHelp" )
#pragma comment(lib,"JsonDll.lib")
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


CLogSvrApp theApp ;
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
				theApp.stop();
				break; 
			}
			Sleep(10);
		}
	}
};

LONG WINAPI MyUnhandledExceptionFilter( struct _EXCEPTION_POINTERS* ExceptionInfo )
{
	HANDLE lhDumpFile = CreateFile(L"DumpFileForlog.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL ,NULL);

	MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
	loExceptionInfo.ExceptionPointers = ExceptionInfo;
	loExceptionInfo.ThreadId = GetCurrentThreadId();
	loExceptionInfo.ClientPointers = TRUE;
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),lhDumpFile, MiniDumpNormal, &loExceptionInfo, NULL, NULL);

	CloseHandle(lhDumpFile);

	return EXCEPTION_EXECUTE_HANDLER;
}


void RunFunc ( CLogSvrApp* pApp )
{
	// exception 
	__try
	{
		pApp->run();
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
	theApp.init();
#ifdef NDEBUG
	RunFunc(&theApp) ;
#else
	theApp.run();
#endif
	theApp.onExit();
	Sleep(3000) ; // wait other thread finish work ;
	return 0 ; 
}