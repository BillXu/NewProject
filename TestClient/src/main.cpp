#include "main.h"
#include "MessageDefine.h"
#include <iostream>
#include "Client.h"
#include "RobotConfig.h"
#include "LogManager.h"
#include "RobotAIConfig.h"
#include <crtdbg.h>
#define JSON_DLL
#include <json/json.h>
#pragma comment(lib,"JsonDll.lib")
#define GATE_IP "127.0.0.1"
//#define GATE_IP "203.186.75.136"
BOOL WINAPI ConsoleHandler(DWORD msgType)
{    

	return TRUE;
} 

#define cal(a,b) a##b
#define toSt(a) #a 
class  TestC
{
public:
	TestC( int t)
	{
		a = 3 ;
		//printf("ctor of test C, %d\n",t);
	}
	template<bool b> void hello()
	{ 
		if (b) 
			printf(" TestC ok\n");
		else
		{
			printf("TestC ok2\n");
		}
	
	}
public:
	int a ;
	char p[32];
};

template<bool b,typename A > 
void hello( A* a )
{ 
	a->hello<b>();
	if (b == true)
	 printf("ok\n"); 
	else
	{
		printf("ok2\n"); 
	}
	
}

class  Test
{
public:
	Test( int t)
	{
		a = 3 ;
		//printf("ctor of test C, %d\n",t);
	}


public:
	int a ;
	char p[40];
};


template<typename A>
A gmin(A a,A b)
{
	return a < b ? a : b ;
}

#include "MemoryManager.h"

//{
	//filename:function_partial_order.cpp 
	//wirtten by saturnman #include<iostream> usingnamespacestd; 
	//function template partial order feature 
	template<typename T> 
	void func1(T t) 
	{ 
		cout<<"func1 1"<<endl; 
	} 

	template<typename T> 
	void func1(T* t)
	{
		cout<<"func1 2"<<endl;
	} 
	
	template<typename T>
	void func2(T t) 
	{ 
		cout<<"func2"<<endl;
	} 

	int hmain() {
		//test function template partial order 
		cout<<"test function template partial order."<<endl; 
		int param1 =10; int* param2 =NULL; 
		func1(param1); 
		func1(param2); 
		func2(param1); 
		func2(param2);

		Json::Value obj , obj2;
		//obj["hello"] = "value" ;
		Json::StyledWriter write ;
		
		obj[0u] = 31 ;
		obj[1u] = "v2" ;
		obj2["s"] = 12;
		obj2["ob"] = obj ;
		obj[2u] = obj ;

		std::string str = "h";
		str = write.write(obj2) ; 
		printf("%s",str.c_str());

		uint64_t left = 10 ;
		uint64_t right = 20 ;
		int nVal = left - right ;
		printf("result = %I64d, nv = %d \n",left - right,nVal );
		if ( (left - right) < -1 )
		{
			printf("ok");
		}
		else
		{
			printf("error") ;
		}
		return 0;
	}
//}
int main()
{
	hmain();
	return 0;
	//TestC t (3);
	//hello<true,TestC>(&t);
	//t.hello<false>();
	//Test tt(3) ;
	//int a = 35 ;
	//int b = 33 ;
	//auto ts = gmin<int>(33,2);
	//cal(Te,stC) ttt(3);
	//ttt.hello<false>();
	/*//---temp
	time_t tS = 0  ;
	scanf("%d\n",&tS) ;
	//printf("%d\n",tS) ;
	while ( tS != 0 )
	{
		char* pS = ctime(&tS) ;
		printf("time is %s\n",pS);
		//printf("%d strDate is : %s\n",tS,pS) ;
		scanf("%u",&tS) ;
	}
	return 0;
	
	int nN = 1 ;
	void* pt = malloc(2);
	int address = (int)pt ;
	void* pt2 = ((char*)pt + nN ) ;
	int address2 = (int)pt2 ;
	printf("elasps = %d\n",address2 - address );
	*///----
//	char* pv = new char[10] ;
//	delete[] pv ;
//	TestC* p = new TestC(3);
//	CMemoryMgr::getInstance()->debugInfo();
//	clock_t s = clock();
//#define  CNT 91960
//	TestC* vList[CNT] = {0} ;
//	int nCnt = CNT ;
//	while ( nCnt )
//	{
//		TestC* p = new TestC(3);
//		vList[--nCnt] = p;
//		p = new TestC(3);
//		p = new TestC(3);
// 		if ( nCnt == 11620 )
// 		{
// 			CMemoryMgr::getInstance()->debugInfo();
// 		}
// 		if ( nCnt == 11586 )
// 		{
// 			CMemoryMgr::getInstance()->debugInfo();
// 		}
	//}
	//
	//clock_t e = clock();
	//float dw = float(e-s)/CLOCKS_PER_SEC ;
	//printf( "befor new time = %f\n",dw ) ;
	//e = clock();
	//s = e ;
	//for each( TestC* pt in vList ) 
	//{
	//	if ( pt )
	//	{
	//		delete pt ;
	//	}
	//	//e = clock();
	//	//dw = float(e-s)/CLOCKS_PER_SEC ;
	//	//printf( "time = %f\n",dw ) ;
	//	//e = clock();
	//	//s = e ;
	//	
	//}

	//e = clock();
	//dw = float(e-s)/CLOCKS_PER_SEC ;
	//printf( "delete  time = %f\n",dw ) ;
	//s = e ;
	//nCnt = 99000 ;
	//while ( nCnt-- )
	//{
	//	Test* p = new Test(3);
	//	if ( nCnt == 1 )
	//	{
	//		//CMemoryMgr::getInstance()->debugInfo();
	//	}
	//	if ( nCnt == 2 )
	//	{
	//		//CMemoryMgr::getInstance()->debugInfo();
	//	}
	//}

	// e = clock();
	// dw = float(e-s)/CLOCKS_PER_SEC ;
	//printf( "usd time = %f\n",dw ) ;
	//CMemoryMgr::getInstance()->debugInfo();
	//return 0 ;
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG )|_CRTDBG_LEAK_CHECK_DF) ;
	//_CrtSetBreakAlloc(133);
	SetConsoleCtrlHandler(ConsoleHandler, TRUE); 
	CClientRobot* pClient = NULL;

	CRobotConfigFile nRobotConfige ;
	//CLogMgr::SharedLogMgr()->SetDisablePrint(false) ;
	CConfigReader::s_SkillRow = 1 ;
	nRobotConfige.LoadFile("../ConfigFile/RobotConfig.txt");
	CConfigReader::s_SkillRow = 0 ;
	int i = 0 ;
	CRobotConfigFile::stRobotItem* pitem = NULL ;
	int nCount = 5 ;
	while ( (pitem = nRobotConfige.EnumConfigItem() ) && nCount-- > 0 )
	{
		pClient = new CClientRobot ;
		bool bR = pClient->Init(GATE_IP);
		pClient->GetPlayerData()->SetLoginConfig(pitem) ;
		if ( !bR )
		{
			delete pClient ;
			continue; 
		}
		pClient->Start() ;
		Sleep(10);
	}
	getchar();
	return 0 ;
}