#pragma once
#include "MessageDefine.h"
#include "NativeTypes.h"
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
	bool OnMessage(stMsg* pMsg );
	void SendBaseDatToClient();
	void CPlayerBaseData::Reset();
	virtual void OnOtherDoLogined(){SendBaseDatToClient();OnProcessContinueLogin();}
	void SaveCoin();
	void OnProcessContinueLogin();
	void OnPlayerDisconnect();
	char CheckVipValid();  // return 1 Is LevelUp , 0 is the same , -1 is level down ;
	virtual void TimerSave();
	uint64_t GetAllCoin(){ return m_stBaseData.nCoin + m_nTakeInCoin ;}
	unsigned int GetAllDiamoned(){ return m_stBaseData.nDiamoned + m_nTakeInDiamoned ;}
	uint64_t GetTakeInMoney(bool bDiamoned = false ){ return bDiamoned ? m_nTakeInDiamoned : m_nTakeInCoin ; }
	void SetTakeInCoin(uint64_t nCoin, bool bDiamoned = false);
	bool ModifyTakeInMoney(int64_t nOffset,bool bDiamond = false );
	bool ModifyMoney(int64_t nOffset,bool bDiamond = false );
	void CaculateTakeInMoney();
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
	unsigned int GetRechargeTimes(){ return m_stBaseData.nRechargeTimes ;}
	CPlayerOnlineBox* GetOnlineBox(){ return m_pOnlineBox ; }
	static bool EventFunc(void* pUserData,stEventArg* pArg);
	stServerBaseData* GetData(){ return &m_stBaseData ;}
	unsigned char GetTitleLevel();
public:
	friend class CPlayerOnlineBox ;
protected:
	void SaveBaseData();
	void OnNewDay(stEventArg* pArg);
private:
	stServerBaseData m_stBaseData ;
	CPlayerOnlineBox* m_pOnlineBox ;
	// not store in db 
public:
	uint64_t m_nTakeInCoin ;
	unsigned m_nTakeInDiamoned ;
	bool m_bGivedLoginReward ;
};