#include <windows.h>
#include "TaxasServerApp.h"
#include <iostream>
#include <WinDef.h>
#include <Dbghelp.h>
#pragma comment( lib, "DbgHelp" )
#pragma comment(lib,"JsonDll.lib")
#include "LogManager.h"
DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	bool bRunning = true;
	char pBuffer[255] ;
	while(bRunning)
	{
		memset(pBuffer,0,sizeof(pBuffer)) ;
		scanf_s("%s",pBuffer,sizeof(pBuffer)) ;
		if ( strcmp(pBuffer,"exit") == 0 || strcmp(pBuffer,"Q") == 0 )
		{
			bRunning = false ;
			CTaxasServerApp* pAp = (CTaxasServerApp*)lpParam ;
			pAp->stop();
			printf("Closing!!!\n");
		}
		else
		{
			printf("Input exit or Q , to close the app \n") ;
		}
	}
	return 0;
}

void CreateThred( CTaxasServerApp* pApp )
{
	DWORD threadID;
	HANDLE hThread;
	hThread = CreateThread(NULL,0,ThreadProc,pApp,0,&threadID); // 创建线程
}


LONG WINAPI MyUnhandledExceptionFilter( struct _EXCEPTION_POINTERS* ExceptionInfo )
{
	HANDLE lhDumpFile = CreateFile(L"DumpFileForTaxasServer.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL ,NULL);

	MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
	loExceptionInfo.ExceptionPointers = ExceptionInfo;
	loExceptionInfo.ThreadId = GetCurrentThreadId();
	loExceptionInfo.ClientPointers = TRUE;
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),lhDumpFile, MiniDumpNormal, &loExceptionInfo, NULL, NULL);

	CloseHandle(lhDumpFile);

	return EXCEPTION_EXECUTE_HANDLER;
}


void RunFunc ( CTaxasServerApp* pApp )
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

#include "httpRequest.h"
#include <json/json.h>
//---test
//#include "CardPoker.h"
//#include "TaxasPokerPeerCard.h"
///test
class CCallback 
	:public CHttpRequestDelegate
{
public:
	void onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData, size_t tType )
	{
		printf("net String: %s\n",pResultData) ;
		Json::Reader reader ;
		Json::Value cValue ;
		reader.parse(pResultData,pResultData + nDatalen,cValue) ;
		printf("retcode = %d, room id = %d\n ",cValue["errcode"].asInt(),cValue["room_id"].asInt());
	}
};
int main()
{
	//CCallback bat ;
	//CHttpRequest req ;
	//req.init("https://qplusapi.gotye.com.cn:8443/api/");
	//Json::Value cValue ;
	//cValue["email"] = "378569952@qq.com" ;
	//cValue["devpwd"] = "bill007" ;
	//cValue["appkey"] = "abffee4b-deea-4e96-ac8d-b9d58f246c3f" ;
	//cValue["room_name"] = 2 ;
	//cValue["room_type"] = 1;
	//cValue["room_create_type"] = 0 ;
	//Json::StyledWriter sWrite ;
	//std::string str = sWrite.write(cValue);
	//req.setDelegate(&bat);
	//req.performRequest("CreateRoom",str.c_str(),str.size(),NULL);
	//return 0;
	//room_name":"tset01","room_type":"1","room_create_type":"0"}
	//-----test
	//CCard publicCard[5] ;

	//CCard privateCardA[2] ;
	//CCard pviatCardB[2] ;

	//CTaxasPokerPeerCard pPeerA ,pPeerB ;

	//publicCard[0].SetCard(CCard::eCard_Heart,2) ;
	//publicCard[1].SetCard(CCard::eCard_Heart,9) ;
	//publicCard[2].SetCard(CCard::eCard_Club,12) ;
	//publicCard[3].SetCard(CCard::eCard_Heart,5) ;
	//publicCard[4].SetCard(CCard::eCard_Sword,13) ;

	//privateCardA[0].SetCard(CCard::eCard_Heart,6) ;
	//privateCardA[1].SetCard(CCard::eCard_Club,11) ;

	//pviatCardB[0].SetCard(CCard::eCard_Heart,9) ;
	//pviatCardB[1].SetCard(CCard::eCard_Diamond,13) ;

	//pPeerA.AddCardByCompsiteNum(privateCardA[0].GetCardCompositeNum());
	//pPeerA.AddCardByCompsiteNum(privateCardA[1].GetCardCompositeNum());

	//pPeerB.AddCardByCompsiteNum(pviatCardB[0].GetCardCompositeNum());
	//pPeerB.AddCardByCompsiteNum(pviatCardB[1].GetCardCompositeNum());
	//for ( int i = 0 ; i < 5 ; ++i )
	//{
	//	pPeerA.AddCardByCompsiteNum(publicCard[i].GetCardCompositeNum()) ;
	//	pPeerB.AddCardByCompsiteNum(publicCard[i].GetCardCompositeNum()) ;
	//}
	//int nType = pPeerA.GetCardType();
	//int nTypeB = pPeerB.GetCardType();
	//char c = pPeerA.PK(&pPeerB) ;
	//printf( "type A = %d ,robot Type A = %d, type B = %d ,result = %d\n",nType,pPeerA.GetCardTypeForRobot(),nTypeB ,c) ;
	/////----test
	 
	CLogMgr::SharedLogMgr()->SetOutputFile("taxasSvr");
	CTaxasServerApp theApp ;
	bool bok = theApp.init() ;
	if ( !bok )
	{
		printf("init svr error\n");
		char c ;
		scanf_s("%c",&c,1);
		return 0 ;
	}
	CreateThred(CTaxasServerApp::SharedGameServerApp());
//#ifdef NDEBUG
	RunFunc(CTaxasServerApp::SharedGameServerApp());
//#endif // _DEBUG
//#ifdef _DEBUG
//	theApp.run();
//#endif // _DEBUG
	return 0 ;
}