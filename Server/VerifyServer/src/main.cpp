//#include "VerifyApp.h"
//#include "catch_dump_file.h"
//#include "LogManager.h"
////#include "HMAC_SHA1.h"
////#include "Md5.h"
////#include<algorithm>
//
//void RunFunc ( IServerApp* pApp )
//{
//	// exception 
//	__try
//	{
//		pApp->run() ;
//	}
//	__except(CatchDumpFile::CDumpCatch::UnhandledExceptionFilterEx(GetExceptionInformation()))
//	{
//		CLogMgr::SharedLogMgr()->SystemLog("try to recover from exception") ;
//		pApp->run() ;
//	}
//}
//
//CatchDumpFile::CDumpCatch g_exception_handler;
//
//int main()
//{
//	//CMD5 t;
//	//unsigned char p[] = "paiyouquan";
//	//t.GenerateMD5(p,strlen((const char*)p)) ;
//	//std::string strSign = t.ToString() ;
//	//transform(strSign.begin(), strSign.end(), strSign.begin(),  toupper);
//	//printf("%s",strSign.c_str());
////  	CHMAC_SHA1 tt ;
////  	char* pKey = "bKgKJPGrgjQlWFQBJpTOhw==" ;
////  	char* pM = "appId=2882303761517415923&cpOrderId=3887efdb-e91f-403f-ae4c-54cf6a43d3c4&uid=56965855" ;
////  	std::string str = tt.HMAC_SHA1((BYTE*)pM,strlen(pM),(BYTE*)pKey,strlen(pKey)) ;
////	zsummer::log4z::ILog4zManager::GetInstance()->Config("server.cfg");
////	zsummer::log4z::ILog4zManager::GetInstance()->Start();
//	_CrtSetReportMode(_CRT_ASSERT, 0);
//	CVerifyApp theApp ;
//	bool bok = theApp.init() ;
//	if ( !bok )
//	{
//		printf("init svr error\n");
//		char c ;
//		scanf("%c",&c);
//		return 0 ;
//	}
//	RunFunc(&theApp);
//	theApp.onExit();
//	return 0 ;
//}

#include "VerifyApp.h"
#include "Application.h"
int main()
{
	CVerifyApp theApp ;
	CApplication theAplication(&theApp);
	theAplication.startApp();
	return 0 ;
}