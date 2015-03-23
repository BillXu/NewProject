#pragma once 
#include "IPlayerComponent.h"
#include "NativeTypes.h"
#include <string>
#include "CommonDefine.h"
#include <list>
class CPlayer;
struct stMail;
class CPlayerMailComponent
	:public IPlayerComponent
{
public:
	typedef std::list<stMail*> LIST_MAIL ;
public:
	CPlayerMailComponent(CPlayer* pPlayer ):IPlayerComponent(pPlayer){ClearMails();}
	~CPlayerMailComponent(){ ClearMails() ;}
	virtual bool OnMessage( stMsg* pMessage , eMsgPort eSenderPort );
	virtual void OnPlayerDisconnect();
	virtual void Reset();
	virtual void Init();
	virtual void OnOtherDoLogined();
	void PostMail(stMail* pMail ,unsigned int nTargetUID  = 0);
	void PostGiftMail( CPlayer* pPlayerPresenter,unsigned int nTargetUID, unsigned short nItemID  ,unsigned short nShopItemID, unsigned int nItemCount = 1 , uint64_t nCoin = 0 , unsigned int nDiamond = 0);
	void PostUnprocessedPurchaseVerify(unsigned int nMailTargetPlayerUID,unsigned int nTaregetForPlayerUID , unsigned short nShopItemID, bool bVerifyOK );
	void PostBeAddedFriendMail( CPlayer* WhoWantAddFriend , unsigned int nMailToUserUID );
	virtual void TimerSave();
protected:
	void ClearMails();
	unsigned short GetUnprocessedMailCount();
	void SendMailListToClient(uint64_t nBegMainUID );
	void SaveMailToDB(stMail* pMail , eDBAct eOpeateType, unsigned int nOwnerUID = 0 );  // 0 update , 1 delete , 2 insterd ;
	void InformRecievedUnreadMails();
	void ProcessOfflineEvent();
	void PushNeedToUpdate(stMail* pMail , eDBAct eAct );
	bool ProcessMail(stMail* pMail,eProcessMailAct eAct );
	stMail* GetMailByMailID(uint64_t nMailID);
public:
	static uint64_t s_nCurMaxMailUID ;
protected:
	LIST_MAIL m_vAllMail ;
	LIST_MAIL m_vNeedUpdateMails[eDBAct_Max] ; // for db operation;
};