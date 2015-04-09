#pragma once
#include "MessageDefine.h"
#include "CommonDefine.h"
#include "IPlayerComponent.h"
#include <string>
#include "PlayerOnlineBox.h"
struct stMsg ;
class CPlayer ;
struct stEventArg ;
class CPlayerBaseData 
	:public IPlayerComponent
{
public:
	CPlayerBaseData(CPlayer*);
	~CPlayerBaseData();
	void Init();
	bool OnMessage( stMsg* pMessage , eMsgPort eSenderPort );
	void SendBaseDatToClient();
	void Reset();
	virtual void OnOtherDoLogined(){SendBaseDatToClient();OnProcessContinueLogin();}
	void SaveCoin();
	void OnProcessContinueLogin();
	void OnPlayerDisconnect();
	virtual void TimerSave();
	uint64_t GetAllCoin(){ return m_stBaseData.nCoin + m_nTakeInCoin ;}
	unsigned int GetAllDiamoned(){ return m_stBaseData.nDiamoned + m_nTakeInDiamoned ;}
	uint64_t GetTakeInMoney(bool bDiamoned = false ){ return bDiamoned ? m_nTakeInDiamoned : m_nTakeInCoin ; }
	bool SetTakeInCoin(uint64_t nCoin, bool bDiamoned = false);
	//bool ModifyTakeInMoney(int64_t nOffset,bool bDiamond = false );
	bool ModifyMoney(int64_t nOffset,bool bDiamond = false );
	//void CaculateTakeInMoney();
	bool OnPlayerEvent(stPlayerEvetArg* pArg);
	void AddWinTimes(){ ++m_stBaseData.nWinTimes;}
	void AddLoseTimes(){ ++m_stBaseData.nLoseTimes;}
	void SetSingleWinMost(uint64_t nWin ){ if ( GetSingleWinMost() < nWin ){ m_stBaseData.nSingleWinMost = nWin ; } }
	uint64_t GetSingleWinMost(){ return m_stBaseData.nSingleWinMost ;}
	unsigned char* GetMaxCards(){ return m_stBaseData.vMaxCards ; }
	char* GetPlayerName(){ return m_stBaseData.cName ;}
	char* GetPlayerSignature(){ return m_stBaseData.cSignature ;}
	void GetPlayerBrifData(stPlayerBrifData* pData );
	void GetPlayerDetailData(stPlayerDetailData* pData );
	unsigned char GetSex(){ return m_stBaseData.nSex ;}
	unsigned char GetVipLevel(){ return m_stBaseData.nVipLevel ; }
	static bool EventFunc(void* pUserData,stEventArg* pArg);
	stServerBaseData* GetData(){ return &m_stBaseData ;}
	virtual void OnReactive(uint32_t nSessionID );
	uint16_t GetPhotoID(){ return m_stBaseData.nPhotoID ;}
	void CacluateTaxasRoomMoney(uint64_t nNewTakeIn, bool bDiamond );
public:
	friend class CPlayerOnlineBox ;
protected:
	void SaveBaseData();
	void OnNewDay(stEventArg* pArg);
private:
	stServerBaseData m_stBaseData ;
	// not store in db 
	uint64_t m_nTakeInCoin ;
	unsigned m_nTakeInDiamoned ;
	bool m_bGivedLoginReward ;

	bool m_bMoneyDataDirty;
	bool m_bTaxasDataDirty;
	bool m_bCommonLogicDataDirty ;
	bool m_bPlayerInfoDataDirty;
};