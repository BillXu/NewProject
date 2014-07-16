#include "main.h"
#include "LoginApp.h"
CLoginApp* pTheApp = NULL ;
BOOL WINAPI ConsoleHandler(DWORD msgType)
{    
	if ( pTheApp )
	{
		delete pTheApp ;
	}
	return TRUE;
}
int main()
{
	SetConsoleCtrlHandler(ConsoleHandler, TRUE); 
	CLoginApp* pTheApp = new CLoginApp ;
	pTheApp->Init();
	pTheApp->MainLoop() ;
	return 0 ;
}