#pragma once
#include "ThreadMod.h"
#include "NetWorkManager.h"
#include "IScene.h"
#include "PlayerData.h"
struct stRobotAI ;
class CClientRobot
	:public CThreadT
	,public CNetMessageDelegate
{
public:
	CClientRobot();
	~CClientRobot();
	bool Init(const char* pIPString, unsigned short nPort = 50002 );
	virtual void __run(){ Run();delete this ;}
	void Run();
	CPlayerData* GetPlayerData(){return m_pPlayerData ;}
	IScene* GetCurrentScene(){ return m_pCurentScene ;}
	void ChangeScene(IScene* pTargetScene );
	unsigned int GetSessionID(){ return m_pPlayerData->GetSessionID(); }
	void SetSessionID( unsigned int nSeseID){ nSessionID = nSeseID;}
	CNetWorkMgr* GetNetWork(){ return &m_pNetWork ;}
	bool OnLostSever()override ;
	bool OnConnectStateChanged( eConnectState eSate, Packet* pMsg )override ;
	void processReconnect( float fDelt );
	bool OnMessage( Packet* pMsg );
protected:
	CNetWorkMgr m_pNetWork ;
	CPlayerData* m_pPlayerData ;
	IScene* m_pCurentScene ;
	IScene* m_pLastScene ;
	unsigned int nSessionID ;
	stRobotAI* m_pRobotAI ;
	std::string m_strAiFile ,m_strPassword, m_strAccount;

	std::string m_strTargetIP ;
	uint16_t m_nTargetPort ;

	bool m_bWaitReonnect ;
	float m_fReconnectTick ;
};