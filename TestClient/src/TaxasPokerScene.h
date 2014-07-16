#pragma once
#include "IScene.h"
#include "TaxasPokerPeerCard.h"
class CTaxasPokerScene
	:public IScene
{
public:
	CTaxasPokerScene(CClient* pClient);
	virtual bool OnMessage( RakNet::Packet* pPacket ) ;
	virtual void OnUpdate(float fDeltaTime ) ;
	void RoundEnd();
	void GameEnd();
	void OnMustLeave();
	void TryToSitDown();
	void MyAction();
	void DoMyAction();
	unsigned short GetCurSitDownPeers();
protected:
	unsigned char m_myPrivateCard[2] ;
	CTaxasPokerPeerCard m_pMyPeerCard ;
	bool m_bBiggest ;
	bool m_bMustWin ;
	unsigned char m_nCurRound ; 
	uint64_t m_nCurBetCoin ;
	bool m_vbHavePeer[15];
	unsigned short m_nMaxPeerInRoom ;
	unsigned short m_nMyIdx ;
	uint64_t m_nMaxTakeIn ;
	uint64_t m_nMinTakeIn ;
	bool m_bDiamoned ;
	uint64_t m_nBigBlindCoin ;
	unsigned short m_nWaitActionPeerIdx ;

	uint64_t m_nCurMaxBetCoin ;

	bool m_bCanDoAction ;
	float m_fTimeTicket ;
};