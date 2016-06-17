#include <windows.h>
#include "DBApp.h"
#include <Dbghelp.h>
#include <iostream>
#include "ThreadMod.h"
#pragma comment( lib, "DbgHelp" )
#pragma comment(lib,"JsonDll.lib")
#include "LogManager.h"
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
				theApp.stop();
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
		pApp->run();
	}
	__except(MyUnhandledExceptionFilter(GetExceptionInformation()))
	{
	}
}

#include "MySqlData.h"
int main()
{
	//SetConsoleCtrlHandler(ConsoleHandler, TRUE); 
	CGetInput input ;
	CLogMgr::SharedLogMgr()->SetOutputFile("DBSvr");
	input.Start();
	bool bok = theApp.init();
	if ( !bok )
	{
		printf("init svr error\n");
		char c ;
		scanf("%c",&c);
		return 0;
	}
//#ifdef NDEBUG
	RunFunc(&theApp) ;
//#else
//	theApp.run();
//#endif
	Sleep(3000) ; // wait other thread finish work ;
	return 0 ; 
}