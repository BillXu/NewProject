#pragma once
#include "IScene.h"
#include "CommonDefine.h"
#include "GoldenData.h"
class CGoldenScene
	:public IScene
{
public:
	CGoldenScene(CClientRobot* pClient);
	bool init(const char* cRobotAiFile);
	virtual bool OnMessage( Packet* pPacket ) ;
	virtual void OnUpdate(float fDeltaTime ) ;
	CGoldenData* getPokerData(){ return &m_tData ;}
	CClientRobot* getClientApp(){ return m_pClient ;}
protected:
	CGoldenData m_tData ;
};