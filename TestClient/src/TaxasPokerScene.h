#pragma once
#include "IScene.h"
#include "TaxasPokerPeerCard.h"
#include "CommonDefine.h"
#include "TaxasPokerData.h"
#include "RobotControlTaxas.h"
class CTaxasPokerScene
	:public IScene
{
public:
	CTaxasPokerScene(CClientRobot* pClient);
	bool init(const char* cRobotAiFile);
	virtual bool OnMessage( Packet* pPacket ) ;
	virtual void OnUpdate(float fDeltaTime ) ;
	CTaxasPokerData* getPokerData(){ return &m_tData ;}
	CClientRobot* getClientApp(){ return m_pClient ;}
protected:
	CTaxasPokerData m_tData ;
};