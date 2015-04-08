#pragma once
#include "MessageDefine.h"
#include "RoomConfig.h"
#include "CommonData.h"
#include "CardPoker.h"
class CTaxasBaseRoomState ;

typedef std::vector<uint8_t> VEC_INT8 ;

struct stTaxasPeerData
	:public stTaxasPeerBaseData
{
	uint64_t nAllBetCoin ;
	uint64_t nTotalBuyInThisRoom ;
	uint64_t nWinCoinThisGame ; 

	bool IsHaveState( eRoomPeerState estate ) { return ( nStateFlag & estate ) == estate ; } ;
	bool IsInvalid(){ return (nSessionID == 0) && (nUserUID == 0);}
	bool BetCoin( uint64_t nBetCoin )
	{ 
		if ( nTakeInMoney >= nBetCoin )
		{
			nTakeInMoney -= nBetCoin ;
			nAllBetCoin += nBetCoin ;
			nBetCoinThisRound += nBetCoin ;
			return true ;
		} 
		return false ;
	}
};

struct stPlayedPeerRecord
{
	int64_t nMoneyOffset ;
	uint64_t nMoneyLastLeft ;
};

struct stVicePool
{
	uint8_t nIdx ;
	bool bUsed ;
	uint64_t nCoin ;
	VEC_INT8 vInPoolPlayerIdx ;
	VEC_INT8 vWinnerIdxs ;

	void Reset(){ bUsed = false ; nCoin = 0 ; vInPoolPlayerIdx.clear() ; vWinnerIdxs.clear(); }
};

class CTaxasRoom
{
public:
	typedef std::vector<stTaxasInRoomPeerData*> VEC_IN_ROOM_PEERS ;
public:
	CTaxasRoom();
	virtual ~CTaxasRoom();
	bool Init( uint32_t nRoomID,stTaxasRoomConfig* pRoomConfig );
	void GoToState( eRoomState eState );
	void Update(float fDeta );
	virtual CTaxasBaseRoomState* CreateRoomState( eRoomState eState );
	void SendRoomMsg(stMsg* pMsg, uint16_t nLen );
	void SendMsgToPlayer( uint32_t nSessionID, stMsg* pMsg, uint16_t nLen  );
	virtual void OnMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID );

	// logic function 
	uint8_t GetPlayerCntWithState(eRoomPeerState eState );
	void StartGame();
	void DistributePrivateCard();
	void PreparePlayersForThisRoundBet();
	uint8_t InformPlayerAct();
	void OnPlayerActTimeOut();
	bool IsThisRoundBetOK();
	uint8_t CaculateOneRoundPool();  // return produced vice pool cunt this round ;
	uint8_t DistributePublicCard(); // return dis card cnt ;
	uint8_t CaculateGameResult(); //return pool cnt ;
	uint64_t GetAllBetCoinThisRound();
	bool IsPublicDistributeFinish();
protected:
	uint8_t GetFirstInvalidIdxWithState( uint8_t nIdxFromInclude , eRoomPeerState estate );
	stVicePool& GetFirstCanUseVicePool();
	void CaculateVicePool(stVicePool& pPool );
protected:
	// static data 
	uint32_t nRoomID ;
	stTaxasRoomConfig m_stRoomConfig ;
	uint32_t m_nLittleBlind;
	CTaxasBaseRoomState* m_vAllState[eRoomState_TP_MAX];

	// running members ;
	eRoomState m_eCurRoomState ; // eeRoomState ;
	uint8_t m_nBankerIdx ;
	uint8_t m_nLittleBlindIdx ;
	uint8_t m_nBigBlindIdx ;
	int8_t m_nCurWaitPlayerActionIdx ;
	uint64_t  m_nCurMainBetPool ;
	uint64_t  m_nMostBetCoinThisRound;
	uint8_t m_vPublicCardNums[TAXAS_PUBLIC_CARD] ; 
	uint8_t m_nBetRound ; //valid value , 0,1 , 2 , 3 ,4 
	stTaxasPeerData m_vSitDownPlayers[MAX_PEERS_IN_TAXAS_ROOM] ;
	stVicePool m_vAllVicePools[MAX_PEERS_IN_TAXAS_ROOM] ;
	VEC_IN_ROOM_PEERS m_vAllPeers ;
	CPoker m_tPoker ;
};