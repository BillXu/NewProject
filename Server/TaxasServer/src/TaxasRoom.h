#pragma once
#include "ServerMessageDefine.h"
#include "RoomConfig.h"
#include "CommonData.h"
#include "CardPoker.h"
#include "Timer.h"
#include <json/json.h>
#include <cassert>
class CTaxasBaseRoomState ;

typedef std::vector<uint8_t> VEC_INT8 ;
struct stTaxasInRoomPeerDataExten
	:public stTaxasInRoomPeerData
{
	// when play stand up, nCoinInRoom is player total coin, 
	//when player sit down (nCoinInRoom + nTakeInCoin) is player total coin
	// when player sitdown again (but not leave room ) TakeIn coin first from  nCoinInRoom, if not enough
	// then rquest from data svr ;
	// when player standup ,should add 'nTakeInCoin' to  'nCoinInRoom'
	//uint64_t nCoinInRoom ; 
	//uint32_t nStateFlag ;
	uint32_t m_nReadedInformSerial; 
	uint64_t nTotalBuyInThisRoom ; // not real coin , just for record
	uint64_t nFinalLeftInThisRoom ;   // not real coin , just for record
	uint32_t nWinTimesInThisRoom ;
	uint32_t nPlayeTimesInThisRoom ;
	bool bDataDirty ;
	//bool IsHaveState( eRoomPeerState estate ) { return ( nStateFlag & estate ) == estate ; }
};

struct stTaxasPeerData
	:public stTaxasPeerBaseData
{
	uint64_t nAllBetCoin ;  // used for tell win or lose
	uint64_t nTotalBuyInThisRoom ; // used for record
	uint64_t nWinCoinThisGame ;    // used for tell win or lose
	uint32_t nWinTimes ;
	uint32_t nPlayTimes ;
	uint64_t nSingleWinMost ;
	stTaxasInRoomPeerDataExten* pHistoryData;

	bool IsHaveState( eRoomPeerState estate ) { return ( nStateFlag & estate ) == estate ; } ;
	bool IsInvalid(){ return (nSessionID == 0) && (nUserUID == 0);}
	bool BetCoin( uint64_t nBetCoin )
	{ 
		assert(pHistoryData&&"must not null");
		if ( nTakeInMoney >= nBetCoin )
		{
			pHistoryData->nFinalLeftInThisRoom -= nBetCoin ;
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
	void RemovePlayer(uint8_t nIdx)
	{
		VEC_INT8::iterator iter = vInPoolPlayerIdx.begin();
		for ( ; iter != vInPoolPlayerIdx.end(); ++iter )
		{
			if ( (*iter) == nIdx  )
			{
				vInPoolPlayerIdx.erase(iter) ;
				return ;
			}
		}
	}
};

class CTaxasRoom
	:public CTimerDelegate
{
public:
	typedef std::vector<stTaxasInRoomPeerDataExten*> VEC_IN_ROOM_PEERS ;
public:
	CTaxasRoom();
	virtual ~CTaxasRoom();
	bool Init( uint32_t nRoomID,stTaxasRoomConfig* pRoomConfig );
	void GoToState( eRoomState eState );
	virtual void Update(float fTimeElpas, unsigned int nTimerID );
	virtual CTaxasBaseRoomState* CreateRoomState( eRoomState eState );
	void SendRoomMsg(stMsg* pMsg, uint16_t nLen );
	void SendMsgToPlayer( uint32_t nSessionID, stMsg* pMsg, uint16_t nLen  );
	virtual bool OnMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID );
	bool onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue = nullptr);
	bool onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue = nullptr );
	void AddPlayer( stTaxasInRoomPeerDataExten& nPeerData );
	uint32_t GetRoomID(){ return nRoomID ;}
	bool IsPlayerInRoomWithSessionID(uint32_t nSessionID );

	void OnPlayerSitDown(uint8_t nSeatIdx , uint32_t nSessionID, uint64_t nTakeInCoin );
	void OnPlayerStandUp(uint8_t nSeatIdx );
	uint8_t GetSeatIdxBySessionID(uint32_t nSessionID );
	void OnPlayerLeaveRoom(uint32_t nPlayerSession );
	uint8_t GetCurWaitActPlayerIdx(){ return m_nCurWaitPlayerActionIdx ; }
	uint8_t OnPlayerAction( uint8_t nSeatIdx ,eRoomPeerAction act , uint64_t& nValue );  // return error code , 0 success ;
	stTaxasPeerData* GetSitDownPlayerData(uint8_t nSeatIdx);
	stTaxasPeerData* GetSitDownPlayerDataByUID(uint32_t nUserUID);

	// room life and attribute
	void onCreateByPlayer(uint32_t nUserUID );
	void setOwnerUID(uint32_t nCreatorUID );
	uint32_t getOwnerUID(){return m_nRoomOwnerUID ;}
	void addLiftTime(uint32_t nDays );
	void setDeadTime(uint32_t nDeadTime);
	void setAvataID(uint32_t nAvaID );
	void setRoomName(const char* pRoomName);
	void setRoomDesc(const char* pRoomDesc );
	void setRoomInform(const char* pRoomInform );
	bool isRoomAlive();
	void setProfit(uint64_t nProfit );
	void setCreateTime(uint32_t nTime);
	uint32_t getCreateTime(){ return m_nCreateTime;}
	void setInformSieral(uint32_t nSieaial);
	void setChatRoomID(uint64_t nChatRoomID ){ m_nChatRoomID = nChatRoomID ;}
	uint32_t getConfigID(){ return 1 ; }

	// logic function 
	uint8_t GetPlayerCntWithState(eRoomPeerState eState );
	void StartGame();
	void ResetRoomData();
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
	bool isPlayerAlreadySitDown(uint32_t nSessionID );
	stTaxasInRoomPeerDataExten* GetInRoomPlayerDataByUID( uint32_t nUID );
	void SendRoomInfoToPlayer(uint32_t nSessionID );

	// debug info ;
	void debugPlayerHistory();

	void saveUpdateRoomInfo();
	void forceDirytInfo(){ m_bRoomInfoDirty = true ;}
	void removeTaxasPlayersHistory();
protected:
	uint8_t GetFirstInvalidIdxWithState( uint8_t nIdxFromInclude , eRoomPeerState estate );
	stVicePool& GetFirstCanUseVicePool();
	void CaculateVicePool(stVicePool& pPool );
	stTaxasInRoomPeerDataExten* GetInRoomPlayerDataBySessionID( uint32_t nSessionID );
	void syncPlayerDataToDataSvr( stTaxasPeerData& pPlayerData );
	friend class CTaxasBaseRoomState ;
	friend class CTaxasStatePlayerBet ;
protected:
	bool m_bRoomInfoDirty ;
	float m_TimeSaveTicket ;
	// static data 
	uint32_t nRoomID ;
	stTaxasRoomConfig m_stRoomConfig ;
	uint32_t m_nLittleBlind;
	CTaxasBaseRoomState* m_vAllState[eRoomState_TP_MAX];

	// creator info 
	uint32_t m_nRoomOwnerUID ;
	uint32_t m_nCreateTime ;
	uint32_t m_nDeadTime ;
	uint16_t m_nAvataID ;
	char m_vRoomName[MAX_LEN_ROOM_NAME] ;
	std::string m_strRoomDesc;
	std::string m_strRoomInForm ;
	uint32_t m_nInformSerial;
	uint64_t m_nRoomProfit;
	uint64_t m_nChatRoomID ;

	// running members ;
	eRoomState m_eCurRoomState ; // eRoomState ;
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