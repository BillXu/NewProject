#pragma once
#include "IScene.h"
class commonCmdScene
	:public IScene
{
public:
	commonCmdScene( CClientRobot* pClient ):IScene(pClient){}
	bool onMessage(stMsg* pmsg)override;
	bool onMessage(Json::Value& jsContent )override;
};