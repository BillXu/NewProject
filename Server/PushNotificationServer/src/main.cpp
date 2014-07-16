#include "PushNotificationServer.h"
int main()
{
	printf("start APNS server\n") ;
	CPushNotificationServer theApp ;
	theApp.Init();
	theApp.Update();
	return 0 ;
}