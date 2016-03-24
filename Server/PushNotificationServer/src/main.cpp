#include "PushNotificationServer.h"
#pragma comment(lib,"JsonDll.lib")
int main()
{
	//CMD5 t;
	//unsigned char p[] = "paiyouquan";
	//t.GenerateMD5(p,strlen((const char*)p)) ;
	//std::string strSign = t.ToString() ;
	//transform(strSign.begin(), strSign.end(), strSign.begin(),  toupper);
	//printf("%s",strSign.c_str());
	//  	CHMAC_SHA1 tt ;
	//  	char* pKey = "bKgKJPGrgjQlWFQBJpTOhw==" ;
	//  	char* pM = "appId=2882303761517415923&cpOrderId=3887efdb-e91f-403f-ae4c-54cf6a43d3c4&uid=56965855" ;
	//  	std::string str = tt.HMAC_SHA1((BYTE*)pM,strlen(pM),(BYTE*)pKey,strlen(pKey)) ;
	//	zsummer::log4z::ILog4zManager::GetInstance()->Config("server.cfg");
	//	zsummer::log4z::ILog4zManager::GetInstance()->Start();
	//uint32_t nArg = 2 ;
	//int8_t fRate = -3 ;
	//if ( nArg + fRate < 0 )
	//{
	//	int b = 3 ;
	//}
	//else
	//{
	//	int a = 0 ;
	//}
	//int32_t nValue = nArg + fRate ;
	//int8_t tt = nArg - fRate ;
	//int64_t m = tt ;
	bool bok = CPushNotificationServer::getInstance()->init();
	if ( !bok )
	{
		printf("init svr error\n");
		char c ;
		scanf("%c",&c);
		return 0 ;
	}
	CPushNotificationServer::getInstance()->run();
	CPushNotificationServer::getInstance()->onExit();
	return 0 ;
}