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
	virtual bool onCrossServerRequest(stMsgCrossServerRequest* pRequest, eMsgPort eSenderPort,Json::Value* vJsValue = nullptr );
	virtual bool onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue = nullptr );
	bool OnMessage( stMsg* pMessage , eMsgPort eSenderPort );
	void SendBaseDatToClient();
	void Reset();
	virtual void OnOtherDoLogined();
	void SaveCoin();
	void OnProcessContinueLogin();
	void OnPlayerDisconnect();
	void TimerSave()override;
	uint64_t GetAllCoin(){ return m_stBaseData.nCoin;}
	uint64_t GetAllDiamoned(){ return m_stBaseData.nDiamoned;}
	bool AddMoney(int64_t nOffset,bool bDiamond = false );
	bool decressMoney(int64_t nOffset,bool bDiamond = false );
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
protected:
	bool onPlayerRequestMoney( uint64_t& nWantMoney,uint64_t nAtLeast, bool bDiamoned = false);
public:
	friend class CPlayerOnlineBox ;
protected:
	void SaveBaseData();
	void OnNewDay(stEventArg* pArg);
private:
	stServerBaseData m_stBaseData ;
	std::string m_strCurIP ; // ip address ; used by wechat pay ;
	// not store in db 
	bool m_bGivedLoginReward ;

	bool m_bMoneyDataDirty;
	bool m_bCommonLogicDataDirty ;
	bool m_bPlayerInfoDataDirty;
};