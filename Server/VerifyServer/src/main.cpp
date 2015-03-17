#include "VerifyApp.h"
int main()
{
	zsummer::log4z::ILog4zManager::GetInstance()->Config("server.cfg");
	zsummer::log4z::ILog4zManager::GetInstance()->Start();

	CVerifyApp theApp ;
	theApp.Init() ;
	while ( true )
	{
		theApp.MainLoop() ;
		Sleep(5);
	}
	return 0 ;
}