#pragma once
#include "ISeverApp.h"
#include "Singleton.h"
class CNiuNiuServerApp
	:public IServerApp
	,public CSingleton<CNiuNiuServerApp>
{
public:
	uint16_t getLocalSvrMsgPortType() override ;
	bool onLogicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID ) override;
};