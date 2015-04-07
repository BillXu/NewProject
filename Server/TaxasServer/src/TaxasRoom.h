#pragma once
#include "MessageDefine.h"
#include "RoomConfig.h"
#include "CommonData.h"
class CTaxasBaseRoomState ;

struct stTaxasPeerData
	:public stTaxasPeerBaseData
{
	uint64_t nAllBetCoin ;
	uint64_t nTotalBuyInThisRoom ;
};

class CTaxasRoom
{
public:
	typedef std::vector<stTaxasInRoomPeerData*> VEC_IN_ROOM_PEERS ;
public:
	CTaxasRoom();
	~CTaxasRoom();
	bool Init( uint32_t nRoomID,stTaxasRoomConfig* pRoomConfig );
	void GoToState( eRoomState eState );
	void Update(float fDeta );
	virtual CTaxasBaseRoomState* CreateRoomState( eRoomState eState );
	void SendRoomMsg(stMsg* pMsg, uint16_t nLen );
	void SendMsgToPlayer( uint32_t nSessionID, stMsg* pMsg, uint16_t nLen  );
	virtual void OnMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID );
protected:
	// static data 
	uint32_t nRoomID ;
	stTaxasRoomConfig m_stRoomConfig ;
	CTaxasBaseRoomState* m_vAllState[eRoomState_TP_MAX];

	// running members ;
	eRoomState cCurRoomState ; // eeRoomState ;
	uint8_t nBankerIdx ;
	uint8_t cLittleBlindIdx ;
	uint8_t cBigBlindIdx ;
	int8_t cCurWaitPlayerActionIdx ;
	uint64_t  nCurMainBetPool ;
	uint64_t  nMostBetCoinThisRound;
	uint8_t vPublicCardNums[TAXAS_PUBLIC_CARD] ; 
	uint8_t nBetRound ; //valid value , 0,1 , 2 , 3 ,4 
	stTaxasPeerData m_vAllPlayers[MAX_PEERS_IN_TAXAS_ROOM] ;
	VEC_IN_ROOM_PEERS m_vAllPeers ;
};