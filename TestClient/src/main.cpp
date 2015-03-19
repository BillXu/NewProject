#include "main.h"
#include "MessageDefine.h"
#include <iostream>
#include "ClientRobot.h"
#include "RobotConfig.h"
#include "LogManager.h"
#include "RobotAIConfig.h"
#include <crtdbg.h>

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

	// read robot ai config
	CRobotAIManager nRobotAIConfig ;
	nRobotAIConfig.LoadFile("../ConfigFile/RobotAIConfig.xml");

	CRobotConfigFile nRobotConfige ;
	//CLogMgr::SharedLogMgr()->SetDisablePrint(false) ;
	CConfigReader::s_SkillRow = 1 ;
	nRobotConfige.LoadFile("../ConfigFile/RobotConfig.txt");
	CConfigReader::s_SkillRow = 0 ;
	int i = 0 ;
	CRobotConfigFile::stRobotItem* pitem = NULL ;
	int nCount = 1 ;
	while ( (pitem = nRobotConfige.EnumConfigItem() ) && nCount-- > 0 )
	{
		pClient = new CClientRobot ;
		pClient->GetPlayerData()->SetLoginConfig(pitem) ;
		bool bR = pClient->SetRobotAI(nRobotAIConfig.GetRobotAIBy(pitem->nAIConfigID));
		if ( !bR )
		{
			delete pClient ;
			continue; 
		}
		pClient->Init() ;
		pClient->Start() ;
		Sleep(10);
	}
	getchar();
}