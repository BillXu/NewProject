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
	void OnPlayerDisconnect() override ;
	void OnOtherWillLogined()override ;
	void SaveCoin();
	void OnProcessContinueLogin();
	void TimerSave()override;
	uint64_t GetAllCoin(){ return m_stBaseData.nCoin;}
	uint64_t GetAllDiamoned(){ return m_stBaseData.nDiamoned;}
	void setCoin(int64_t nCoin ){ m_stBaseData.nCoin = nCoin ; m_bMoneyDataDirty = true ; }
	bool AddMoney(int64_t nOffset,bool bDiamond = false );
	bool decressMoney(int64_t nOffset,bool bDiamond = false );
	bool OnPlayerEvent(stPlayerEvetArg* pArg);
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
	bool isNotInAnyRoom(){ return m_nStateInRoomID == 0 && m_nStateInRoomType == eRoom_Max ; }
protected:
	bool onPlayerRequestMoney( uint64_t& nWantMoney,uint64_t nAtLeast, bool bDiamoned = false);
public:
	friend class CPlayerOnlineBox ;
protected:
	void SaveBaseData();
	void OnNewDay(stEventArg* pArg);
private:
	stServerBaseData m_stBaseData ;

	uint32_t m_nStateInRoomID ;
	uint8_t m_nStateInRoomType ;
	std::string m_strCurIP ; // ip address ; used by wechat pay ;
	// not store in db 
	bool m_bGivedLoginReward ;

	bool m_bMoneyDataDirty;
	bool m_bCommonLogicDataDirty ;
	bool m_bPlayerInfoDataDirty;
};