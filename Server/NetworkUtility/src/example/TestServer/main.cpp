#include "main.h"
#include "client.h"
#include "Server.h"
#include "../zsummer_server/header.h"
int main()
{
// 	CClientApp theapp ;
// 	theapp.Init();
// 	printf("start Client\n");
// 	while ( true )
// 	{
// 		theapp.RunRoop();
// 		Sleep(200);
// 	}
	///---------------------------------------
	zsummer::log4z::ILog4zManager::GetInstance()->Config("server.cfg");
	zsummer::log4z::ILog4zManager::GetInstance()->Start();
	printf("star c client , s server , input") ;
	char r ;
	r = getchar();
	if ( r == 'c')
	{
		CClientApp theapp ;
		theapp.Init();
		printf("start Client\n");
		while ( true )
		{
			theapp.RunRoop();
			Sleep(200);
		}
	}
	else if ( 's' == r )
	{
		CServerApp theapp ;
		theapp.Init();
		printf("start server\n");
		while ( true )
		{
			theapp.RunRoop();
			Sleep(200);
		}
	}
	else
	{
		printf( "Unknown select\n" ) ;
		getchar() ;
	}
	return 0 ;
}