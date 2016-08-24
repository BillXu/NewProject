#pragma once
#include "ServerConfig.h"
#include "ISeverApp.h"
#include "MiVerifyManager.h"
class CVerifyApp
	:public IServerApp
{
public:
	enum eMod
	{
		eMod_Pool = eDefMod_ChildDef ,
	};
public:
	bool init();
	uint16_t getLocalSvrMsgPortType()override;
	IGlobalModule* createModule( uint16_t eModuleType )override ;
};