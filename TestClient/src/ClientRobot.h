#pragma once
#include "ThreadMod.h"
#include "NetWorkManager.h"
#include "IScene.h"
#include "PlayerData.h"
struct stRobotAI ;
class CClientRobot
	:public CThreadT
{
public:
	CClientRobot();
	~CClientRobot();
	void Init();
	virtual void __run(){ Run();delete this ;}
	void Run();
	CPlayerData* GetPlayerData(){return m_pPlayerData ;}
	IScene* GetCurrentScene(){ return m_pCurentScene ;}
	void ChangeScene(IScene* pTargetScene );
	unsigned int GetSessionID(){ return m_pPlayerData->GetSessionID(); }
	void SetSessionID( unsigned int nSeseID){ nSessionID = nSeseID;}
	CNetWorkMgr* GetNetWork(){ return &m_pNetWork ;}
	stRobotAI* GetRobotAI(){ return m_pRobotAI ;}
	bool SetRobotAI(stRobotAI* pRobotAi );
protected:
	CNetWorkMgr m_pNetWork ;
	CPlayerData* m_pPlayerData ;
	IScene* m_pCurentScene ;
	IScene* m_pLastScene ;
	unsigned int nSessionID ;
	stRobotAI* m_pRobotAI ;
};