#include <windows.h>
#include "main.h"
#include "LoginApp.h"
#include <WinDef.h>
#include <Dbghelp.h>
#pragma comment( lib, "DbgHelp" )
#pragma comment(lib,"JsonDll.lib")
CLoginApp* pTheApp = NULL ;
BOOL WINAPI ConsoleHandler(DWORD msgType)
{    
	if ( pTheApp )
	{
		delete pTheApp ;
	}
	return TRUE;
}

LONG WINAPI MyUnhandledExceptionFilter( struct _EXCEPTION_POINTERS* ExceptionInfo )
{
	HANDLE lhDumpFile = CreateFile("DumpFileForLoginSvr.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL ,NULL);

	MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
	loExceptionInfo.ExceptionPointers = ExceptionInfo;
	loExceptionInfo.ThreadId = GetCurrentThreadId();
	loExceptionInfo.ClientPointers = TRUE;
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),lhDumpFile, MiniDumpNormal, &loExceptionInfo, NULL, NULL);

	CloseHandle(lhDumpFile);

	return EXCEPTION_EXECUTE_HANDLER;
}

void RunFunc ( CLoginApp* pApp )
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

int main()
{
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG )|_CRTDBG_LEAK_CHECK_DF) ;
	SetConsoleCtrlHandler(ConsoleHandler, TRUE); 

	//zsummer::log4z::ILog4zManager::GetInstance()->Config("server.cfg");
	//zsummer::log4z::ILog4zManager::GetInstance()->Start();

	CLoginApp theApp ;
	bool bok = theApp.init();
	if ( bok == false )
	{
		printf("init svr error \n");
		char c ;
		scanf("%c",&c) ;
		return 0 ;
	}
#ifdef NDEBUG
	RunFunc(&theApp);
#endif // _DEBUG
#ifdef _DEBUG
	theApp.run() ;
#endif // _DEBUG
	return 0 ;
}