#include "main.h"
#include "MessageDefine.h"
#include <iostream>
#include "Client.h"
#include "RobotConfig.h"
#include "LogManager.h"
#include "RobotAIConfig.h"
#include <crtdbg.h>
#define GATE_IP "127.0.0.1"
//#define GATE_IP "203.186.75.136"
BOOL WINAPI ConsoleHandler(DWORD msgType)
{    

	return TRUE;
} 

int main()
{
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
	*///----
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
}