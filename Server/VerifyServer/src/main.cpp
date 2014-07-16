#include "VerifyApp.h"
int main()
{
	CVerifyApp theApp ;
	theApp.Init() ;
	while ( true )
	{
		theApp.MainLoop() ;
		Sleep(5);
	}
	return 0 ;
}