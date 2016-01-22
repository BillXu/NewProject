#pragma once
#include "IScene.h"
#include "TaxasPokerPeerCard.h"
#include "CommonDefine.h"
#include "NiuNiuData.h"
#include "RobotControlNiuNiu.h"
class CNiuNiuScene
	:public IScene
{
public:
	CNiuNiuScene(CClientRobot* pClient);
	bool init(const char* cRobotAiFile);
	virtual bool OnMessage( Packet* pPacket ) ;
	virtual void OnUpdate(float fDeltaTime ) ;
	stNiuNiuData* getPokerData(){ return &m_tData ;}
	CClientRobot* getClientApp(){ return m_pClient ;}
protected:
	stNiuNiuData m_tData ;
	CRobotControlNiuNiu m_tRobotControler ;
};