//#include <windows.h>
//#include "MJServer.h"
//#include <iostream>
//#include <WinDef.h>
//#include <Dbghelp.h>
//#pragma comment( lib, "DbgHelp" )
//#ifdef NDEBUG
//#pragma comment(lib,"JsonDll.lib")
//#else
//#pragma comment(lib,"JsonDllD.lib")
//#endif
//#include "log4z.h"
//DWORD WINAPI ThreadProc(LPVOID lpParam)
//{
//	bool bRunning = true;
//	char pBuffer[255] ;
//	while(bRunning)
//	{
//		memset(pBuffer,0,sizeof(pBuffer)) ;
//		scanf_s("%s",pBuffer,sizeof(pBuffer)) ;
//		if ( strcmp(pBuffer,"exit") == 0 || strcmp(pBuffer,"Q") == 0 )
//		{
//			bRunning = false ;
//			IServerApp* pAp = (IServerApp*)lpParam ;
//			pAp->stop();
//			printf("Closing!!!\n");
//		}
//		else
//		{
//			printf("Input exit or Q , to close the app, current = %s \n", pBuffer) ;
//		}
//	}
//	return 0;
//}
//
//void CreateThred( IServerApp* pApp )
//{
//	DWORD threadID;
//	HANDLE hThread;
//	hThread = CreateThread(NULL,0,ThreadProc,pApp,0,&threadID); // 创建线程
//}
//
//
//LONG WINAPI MyUnhandledExceptionFilter( struct _EXCEPTION_POINTERS* ExceptionInfo )
//{
//	HANDLE lhDumpFile = CreateFile(L"DumpFileForNiuNiuServer.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL ,NULL);
//
//	MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
//	loExceptionInfo.ExceptionPointers = ExceptionInfo;
//	loExceptionInfo.ThreadId = GetCurrentThreadId();
//	loExceptionInfo.ClientPointers = TRUE;
//	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),lhDumpFile, MiniDumpNormal, &loExceptionInfo, NULL, NULL);
//
//	CloseHandle(lhDumpFile);
//
//	return EXCEPTION_EXECUTE_HANDLER;
//}
//
//
//void RunFunc ( IServerApp* pApp )
//{
//	// exception 
//	__try
//	{
//		pApp->run() ;
//	}
//	__except(MyUnhandledExceptionFilter(GetExceptionInformation()))
//	{
//	}
//}
//
//
//#include "MJRoomPlayer.h"
//#include "MJPeerCardNew.h"
//#include "MJCard.h"
//#include "MJHuPaiInfo.h"
//#include "HZMJPlayerCard.h"
//int main()
//{
//	//CMJHuPaiInfo tInfo ;
//	//std::vector<uint8_t> vCards ;
//	//uint8_t nValue ;
//
//	//nValue = CMJCard::makeCardNumber(eCT_Wan,2);
//	//vCards.push_back(nValue);
//	//nValue = CMJCard::makeCardNumber(eCT_Wan,2);
//	//vCards.push_back(nValue);
//
//	//nValue = CMJCard::makeCardNumber(eCT_Wan, 2);
//	//vCards.push_back(nValue);
//
//	//nValue = CMJCard::makeCardNumber(eCT_Wan, 3);
//	//vCards.push_back(nValue);
//
//	//nValue = CMJCard::makeCardNumber(eCT_Wan, 4);
//	//vCards.push_back(nValue);
//	//nValue = CMJCard::makeCardNumber(eCT_Wan,3);
//	//vCards.push_back(nValue);
//
//	//nValue = CMJCard::makeCardNumber(eCT_Wan,6);
//	//vCards.push_back(nValue);
//	//nValue = CMJCard::makeCardNumber(eCT_Wan,6);
//	//vCards.push_back(nValue);
//
//	//nValue = CMJCard::makeCardNumber(eCT_Wan,4);
//	//vCards.push_back(nValue);
//	//nValue = CMJCard::makeCardNumber(eCT_Wan,4);
//	//vCards.push_back(nValue);
//
//	//nValue = CMJCard::makeCardNumber(eCT_Wan,5);
//	//vCards.push_back(nValue);
//	//nValue = CMJCard::makeCardNumber(eCT_Wan,5);
//	//vCards.push_back(nValue);
//
//	//nValue = CMJCard::makeCardNumber(eCT_Wan,6);
//	//vCards.push_back(nValue);
//	//nValue = CMJCard::makeCardNumber(eCT_Wan,7);
//	//vCards.push_back(nValue);
//	
//	//bool b = tInfo.parseHuPaiInfo(vCards);
//	//if ( b )
//	//{
//	//	printf("real hu \n") ;
//	//}
//	//return 0 ;
//	//CMJPeerCardNew tPeer ;
//	//tPeer.addHoldCard(CMJCard::makeCardNumber(eCT_Wan, 2));
//	//tPeer.addHoldCard(CMJCard::makeCardNumber(eCT_Wan, 2));
//	//tPeer.addHoldCard(CMJCard::makeCardNumber(eCT_Wan,2));
//	//tPeer.addHoldCard(CMJCard::makeCardNumber(eCT_Wan,3));
//	//tPeer.addHoldCard(CMJCard::makeCardNumber(eCT_Wan, 4));
//	//tPeer.addHoldCard(CMJCard::makeCardNumber(eCT_Wan, 6));
//	//tPeer.addHoldCard(CMJCard::makeCardNumber(eCT_Wan, 6));
//
//	////bool isP = tPeer.isCardCanPeng(CMJCard::makeCardNumber(eCT_Jian,2));
//	//bool isHu = tPeer.isCardCanHu(CMJCard::makeCardNumber(eCT_Wan, 5));
//	//if ( isHu )
//	//{
//	//	printf("right") ;
//	//}
//	//return 0 ;
//	//tPeer.addHoldCard(CMJCard::makeCardNumber(eCT_Feng,2));
//	//tPeer.addHoldCard(CMJCard::makeCardNumber(eCT_Feng,2));
//	////tPeer.addHoldCard(CMJCard::makeCardNumber(eCT_Feng,2));
//	//tPeer.addHoldCard(CMJCard::makeCardNumber(eCT_Feng,3));
//	//tPeer.addHoldCard(CMJCard::makeCardNumber(eCT_Feng,3));
//	//tPeer.addHoldCard(CMJCard::makeCardNumber(eCT_Feng,3));
//
//	//std::vector<CMJPeerCardNew::stEatPair> vec ;
//	//bool b = tPeer.isCardCanEat(CMJCard::makeCardNumber(eCT_Feng,2),vec) ;
//	//for ( auto ref : vec )
//	//{
//	//	printf("--------\n");
//	//	CMJCard::debugSinglCard(ref.nCard[0]);
//	//	CMJCard::debugSinglCard(ref.nCard[1]);
//	//}
//
//	//auto bokt = tPeer.isCardCanAnGang(CMJCard::makeCardNumber(eCT_Feng,2));
//	//if ( bokt )
//	//{
//	//	printf("ok \n");
//	//}
//	//else
//	//{
//	//	printf(" not ok \n");
//	//}
//
//	//tPeer.onPeng(CMJCard::makeCardNumber(eCT_Feng,2));
//	//tPeer.debugPeerCardInfo();
//
//	//tPeer.onBuGang(CMJCard::makeCardNumber(eCT_Feng,2),CMJCard::makeCardNumber(eCT_Feng,3));
//	//tPeer.debugPeerCardInfo();
//
//	//std::vector<uint8_t> vec ;
//	//tPeer.isHaveAnGangCards(vec) ;
//	//printf("ttt-\n");
//	//for ( auto& ref : vec )
//	//{
//	//	CMJCard::debugSinglCard(ref) ;
//	//}
//
//	//tPeer.onAnGang(CMJCard::makeCardNumber(eCT_Feng,3),CMJCard::makeCardNumber(eCT_Wan,8));
//
//	//return 0;
//	//CMJRoomPlayer m_tRoomPlayer ;
//	//m_tRoomPlayer.addDistributeCard(17) ;
//	//m_tRoomPlayer.addDistributeCard(17) ;
//	//m_tRoomPlayer.addDistributeCard(20) ;
//	//m_tRoomPlayer.addDistributeCard(21) ;
//	//m_tRoomPlayer.addDistributeCard(22) ;
//
//	//m_tRoomPlayer.addDistributeCard(35) ;
//	//m_tRoomPlayer.addDistributeCard(36) ;
//	//m_tRoomPlayer.addDistributeCard(37) ;
//
//	//m_tRoomPlayer.addDistributeCard(38) ;
//	//m_tRoomPlayer.addDistributeCard(38) ;
//	//m_tRoomPlayer.addDistributeCard(40) ;
//	//m_tRoomPlayer.addDistributeCard(40) ;
//	//m_tRoomPlayer.addDistributeCard(39) ;
//	////m_tRoomPlayer.addDistributeCard(41) ;
//
//	////m_tRoomPlayer.onPengCard(41) ;
//	////m_tRoomPlayer.fetchCard(17);
//	////m_tRoomPlayer.updateSelfOperateCards();
//	////m_tRoomPlayer.canGangWithCard(17,true);
//	//uint8_t nGen = 0;
//	//bool b = m_tRoomPlayer.doHuPaiFanshu(39,nGen);
//	//return 0 ;
//	//printf("\n\u5317\u4eac");
//
//	// test new chard ;
//	IMJPlayerCard* pPlayerCard = new HZMJPlayerCard();
//	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 1));
//	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 1));
//
//	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 1));
//	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 6));
//
//	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 2));
//	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 2));
//
//	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 2));
//	//pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 2));
//	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 3));
//
//	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 4));
//	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 4));
//
//	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 3));
//	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 3));
//
//	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 6));
//
//	//pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 5));
//	pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 3));
//	
//	//pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 3));
//	if (pPlayerCard->isTingPai())
//	{
//		printf("do ting \n");
//	}
//	
//	if (pPlayerCard->isHoldCardCanHu())
//	{
//		printf("hold card can hu \n");
//	}
//
//	if (((HZMJPlayerCard*)pPlayerCard)->isBaoTou())
//	{
//		printf( "yes is bao tou!" );
//	}
//
//	if (((HZMJPlayerCard*)pPlayerCard)->canHoldCard7PairHu())
//	{
//		printf("7 pair hu , hao hua cnt = %u", ((HZMJPlayerCard*)pPlayerCard)->get7PairHuHaoHuaCnt());
//	}
//	//
//	CMJServerApp* theApp = CMJServerApp::getInstance() ;
//	bool bok = theApp->init() ;
//	CLogMgr::SharedLogMgr()->SetOutputFile("MJSvr");
//	if ( !bok )
//	{
//		printf("init data svr error , start up error\n");
//		char c ;
//		scanf_s("%c",&c);
//		return 0 ;
//	}
//	CreateThred(theApp);
//#ifdef NDEBUG
//	RunFunc(theApp);
//#endif // _DEBUG
//#ifdef _DEBUG
//	theApp->run();
//#endif // _DEBUG
//	return 0 ;
//}

#include "MJPlayerCard.h"
#include "MJCard.h"
#include "XLMJPlayerCard.h"
void tempTest()
{
	//	// test new chard ;
	XLMJPlayerCard* pPlayerCard = new XLMJPlayerCard();
		
	pPlayerCard->setQueType(eCT_Tiao);
		pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 9));
		pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 9));

		pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 1));
		pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 2));
		pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 3));

		pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 1));
		pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 1));
		pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 1));
	
		pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 7));
		pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 8));
		pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 9));

		pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 7));
		pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 8));
		//pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 9));

		//pPlayerCard->onMingGang(CMJCard::makeCardNumber(eCT_Tong, 7), CMJCard::makeCardNumber(eCT_Wan, 1));
		/*pPlayerCard->onPeng(CMJCard::makeCardNumber(eCT_Tong, 5));
		pPlayerCard->onPeng(CMJCard::makeCardNumber(eCT_Tong, 2));
		pPlayerCard->onPeng(CMJCard::makeCardNumber(eCT_Wan, 8));
		pPlayerCard->onPeng(CMJCard::makeCardNumber(eCT_Wan, 2));*/
		uint32_t nT;
		uint8_t nBeiShu;
		uint8_t nGen;
		auto p = pPlayerCard->onDoHu(false, CMJCard::makeCardNumber(eCT_Wan, 9), nT, nBeiShu, nGen);
		auto p2 = pPlayerCard->onDoHu(false, CMJCard::makeCardNumber(eCT_Wan, 9), nT, nBeiShu, nGen);
		//eFanxingType eType = (eFanxingType)nT;
		if (!p)
		{
			printf("bug");
		}
		printf("beishu = %u\n",p);
		if (pPlayerCard->isTingPai())
		{
			printf("do ting \n");
		}

}

#include "MJServer.h"
#include "Application.h"
int main()
{
	tempTest();
	CApplication theAplication(CMJServerApp::getInstance());
	theAplication.startApp();
	return 0;
}