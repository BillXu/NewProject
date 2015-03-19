#include "IScene.h"
#include "CommonDefine.h"
class CBacScene
	:public IScene
{
public:
	CBacScene(CClientRobot* pClient);
	virtual void OnUpdate(float fDeltaTime );
	virtual bool OnMessage( Packet* pPacket ) ;
	void DoMyAction();
	void MyAction();
protected:
	float m_fTimeTicket ;
	bool m_bCanMyAction ;
	unsigned char nBetTimes ;

	uint64_t m_vBetPort[eBC_BetPort_Max];
	uint64_t m_nMaxBankerIdleDiff ;
	uint64_t m_nMaxPairBet;   
	uint64_t m_nMaxSameBet ;  
	bool m_bLastActionFinished ;
};