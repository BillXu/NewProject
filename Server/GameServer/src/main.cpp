#include "GameServerApp.h"
#include <iostream>
#include <WinDef.h>
#include <Dbghelp.h>
#pragma comment( lib, "DbgHelp" )

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	bool bRunning = true;
	char pBuffer[255] ;
	while(bRunning)
	{
		memset(pBuffer,0,sizeof(pBuffer)) ;
		scanf("%s",pBuffer) ;
		if ( strcmp(pBuffer,"exit") == 0 || strcmp(pBuffer,"Q") == 0 )
		{
			bRunning = false ;
			CGameServerApp* pAp = (CGameServerApp*)lpParam ;
			pAp->Stop();
			printf("Closing!!!\n");
		}
		else
		{
			printf("Input exit or Q , to close the app \n") ;
		}
	}
	return 0;
}

void CreateThred( CGameServerApp* pApp )
{
	DWORD threadID;
	HANDLE hThread;
	hThread = CreateThread(NULL,0,ThreadProc,pApp,0,&threadID); // 创建线程
}


LONG WINAPI MyUnhandledExceptionFilter( struct _EXCEPTION_POINTERS* ExceptionInfo )
{
	HANDLE lhDumpFile = CreateFile(L"DumpFileForGameServer.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL ,NULL);

	MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
	loExceptionInfo.ExceptionPointers = ExceptionInfo;
	loExceptionInfo.ThreadId = GetCurrentThreadId();
	loExceptionInfo.ClientPointers = TRUE;
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),lhDumpFile, MiniDumpNormal, &loExceptionInfo, NULL, NULL);

	CloseHandle(lhDumpFile);

	return EXCEPTION_EXECUTE_HANDLER;
}


void RunFunc ( CGameServerApp* pApp )
{
	// exception 
	__try
	{
		pApp->Run() ;
	}
	__except(MyUnhandledExceptionFilter(GetExceptionInformation()))
	{
	}
}

//---test
//#include "CardPoker.h"
//#include "TaxasPokerPeerCard.h"
///test
int main()
{
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
	CGameServerApp theApp ;
	theApp.Init() ;
	CreateThred(&theApp);
#ifdef NDEBUG
	RunFunc(&theApp);
#endif // _DEBUG
#ifdef _DEBUG
	theApp.Run();
#endif // _DEBUG
	return 0 ;
}