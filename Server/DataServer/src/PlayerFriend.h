#pragma once 
#include "IPlayerComponent.h"
#include "CommonDefine.h"
#include <map>
#include "MessageDefine.h"
struct stEventArg ;
struct stFriendInfo
	:public stFriendBrifData
{
	stPlayerDetailData* pDetail ;
	CPlayer* pPlayer ;
	bool bHaveBrifeData;
public:
	stFriendInfo(){ pDetail = NULL ; pPlayer = NULL ;bHaveBrifeData = false; }
	~stFriendInfo(){ if (pDetail){delete pDetail;} pPlayer = NULL ;}
	void OnFriendOffline(CPlayer* pPlayerOffline);
	void OnFriendOnLine(CPlayer* pPlayer);
	bool HaveBrifData(){ if (pDetail || pPlayer)return true ; return bHaveBrifeData ;  }
	bool HaveDetailData(){ return (pDetail || pPlayer);}
	bool GetDetailData(stPlayerDetailData* pData );
	void SetDetail(stPlayerDetailData* pDetailData );
};

class CPlayerFriend
	:public IPlayerComponent
{
public:
	typedef std::map<unsigned int,stFriendInfo*> MAP_FRIENDS ;
public:
	CPlayerFriend(CPlayer* pPlayer) ;
	~CPlayerFriend();
	virtual bool OnMessage(stMsg* pMsg ) ;
	virtual void OnPlayerDisconnect();
	virtual void Reset();
	virtual void Init();
	bool IsFriendListFull(){ return false ;}
	//void OnPlayerWantAddMe(CPlayer* pPlayerWantAddMe );
	//void OnOtherReplayMeAboutAddItbeFriend(bool bAgree,CPlayer* pWhoReplyMe);
	virtual void TimerSave();
	void RemoveFriendByUID(unsigned int nPlayerUID );
	void AddFriend(unsigned int nFriendUserUID);
	static bool EventFunc(void* pUserData,stEventArg* pArg);
protected:
	void UpdateFirendInfo();
	void ClearFriendInfo();
	void SendListToClient();
	void AddFriend(CPlayer* pPlayerToAdd );
	stFriendInfo* GetFriendByUID(unsigned int nPlayerUID );
	bool IsLoadFriendBrifData();
	void OnClientRequestFriendList();
	void OnProcessEvent(stEventArg* pArg);
protected:
	MAP_FRIENDS m_vAllFriends ;
	bool m_bDirty ;
};