#pragma once
#include "NetWorkManager.h"
#include "MessageDefine.h"
namespace cocos2d
{
	class SpriteFrame ;
}

class CClientApp
	:public CNetMessageDelegate
{
public:
	static CClientApp* getInstance();
	void destroyInstance();
	CClientApp();
	~CClientApp();
	bool init();
	void enterForground();
	void enterBackground();
	void connectToSvr();
	void disconnectFromSvr();

	// net delegate 
	virtual bool OnMessage( Packet* pMsg );
	virtual bool OnLostSever(Packet* pMsg);
	virtual bool OnConnectStateChanged( eConnectState eSate, Packet* pMsg);
	bool sendMsg(stMsg* pMsg , uint16_t nLen );
	bool isConnecting();
	void update(float fDeta );

	void addMsgDelegate(CNetMessageDelegate* pDelegate );
	void removeMsgDelegate( CNetMessageDelegate* pDelegate );
	stCommonBaseData* getPlayerData(){ return &m_tPlayerData ;}
	void setPlayerData(stCommonBaseData* pData ){ memcpy(&m_tPlayerData,pData,sizeof(m_tPlayerData));}
	static cocos2d::SpriteFrame* getCardSpriteByCompsiteNum(uint16_t nNum );
protected:
	static CClientApp* s_ClientApp;
	CNetWorkMgr* m_pNetwork ;
	CONNECT_ID m_pConnectID ;
	CNetWorkMgr::eConnectType m_eNetState;
protected:
	stCommonBaseData m_tPlayerData ;
};