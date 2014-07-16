#include "main.h"
#include "MessageDefine.h"
#include "RakNetTypes.h"
#include "RakPeerInterface.h"
#include <iostream>
#include "Client.h"
#include "RobotConfig.h"
#include "LogManager.h"
#include "RobotAIConfig.h"
BOOL WINAPI ConsoleHandler(DWORD msgType)
{    

	return TRUE;
} 

int main()
{
	SetConsoleCtrlHandler(ConsoleHandler, TRUE); 
	CClient* pClient = NULL;

	// read robot ai config
	CRobotAIManager nRobotAIConfig ;
	nRobotAIConfig.LoadFile("../ConfigFile/RobotAIConfig.xml");

	CRobotConfigFile nRobotConfige ;
	CLogMgr::SharedLogMgr()->SetDisablePrint(false) ;
	CConfigReader::s_SkillRow = 1 ;
	nRobotConfige.LoadFile("../ConfigFile/RobotConfig.txt");
	CConfigReader::s_SkillRow = 0 ;
	int i = 0 ;
	CRobotConfigFile::stRobotItem* pitem = NULL ;
	while ( (pitem = nRobotConfige.EnumConfigItem() ) )
	{
		pClient = new CClient ;
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