#include "VerifyApp.h"
int main()
{
	zsummer::log4z::ILog4zManager::GetInstance()->Config("server.cfg");
	zsummer::log4z::ILog4zManager::GetInstance()->Start();

	CVerifyApp theApp ;
	bool bok = theApp.init() ;
	if ( !bok )
	{
		printf("init svr error\n");
		char c ;
		scanf("%c",&c);
		return 0 ;
	}
	theApp.run();
	theApp.onExit();
	return 0 ;
}