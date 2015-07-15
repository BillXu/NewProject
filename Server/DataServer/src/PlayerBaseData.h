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
	uint64_t GetAllCoin(){ return m_stBaseData.nCoin;}
	unsigned int GetAllDiamoned(){ return m_stBaseData.nDiamoned;}
	bool ModifyMoney(int64_t nOffset,bool bDiamond = false );
	bool OnPlayerEvent(stPlayerEvetArg* pArg);
	//void AddWinTimes(){ ++m_stBaseData.nWinTimes;}
	//void AddLoseTimes(){ ++m_stBaseData.nLoseTimes;}
	//void SetSingleWinMost(uint64_t nWin ){ if ( GetSingleWinMost() < nWin ){ m_stBaseData.nSingleWinMost = nWin ; m_bTaxasDataDirty = true; } }
	//uint64_t GetSingleWinMost(){ return m_stBaseData.nSingleWinMost ;}
	//unsigned char* GetMaxCards(){ return m_stBaseData.vMaxCards ; }
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

	// taxas data ;
	void caculateMoneyWhenLeaveTaxasRoom(bool bNormalLave , uint64_t nTakeInCoin , bool bDiamoned);
	bool onTaxasPlayerRequestMoney( uint64_t nWantMoney, bool bDiamoned = false);
	bool onTaxasPlayerRequestMoneyComfirm( bool bSucess, uint64_t nWantMoney, bool bDiamoned = false);
	void onSyncTaxasPlayerData();
public:
	friend class CPlayerOnlineBox ;
protected:
	void SaveBaseData();
	void OnNewDay(stEventArg* pArg);
private:
	stServerBaseData m_stBaseData ;
	// not store in db 
	bool m_bGivedLoginReward ;
	uint64_t m_nTaxasPlayerDiamoned   ;
	uint64_t m_nTaxasPlayerCoin  ;

	bool m_bMoneyDataDirty;
	bool m_bTaxasDataDirty;
	bool m_bCommonLogicDataDirty ;
	bool m_bPlayerInfoDataDirty;
};