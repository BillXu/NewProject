#pragma once
#include "NetWorkManager.h"
#include "MessageDefine.h"
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

	// net delegate 
	virtual bool OnMessage( Packet* pMsg );
	virtual bool OnLostSever(Packet* pMsg);
	virtual bool OnConnectStateChanged( eConnectState eSate, Packet* pMsg);
	bool sendMsg(stMsg* pMsg , uint16_t nLen );
	bool isConnecting();
	void update(float fDeta );
protected:
	static CClientApp* s_ClientApp;
	CNetWorkMgr* m_pNetwork ;
	CONNECT_ID m_pConnectID ;
	CNetWorkMgr::eConnectType m_eNetState;
};
