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
	struct stRecievedMail
	{
		uint32_t nRecvTime ;
		eMailType eType ;
		std::string strContent ;
	};
	typedef std::list<stRecievedMail> LIST_MAIL ;
public:
	CPlayerMailComponent(CPlayer* pPlayer ):IPlayerComponent(pPlayer){ClearMails();}
	~CPlayerMailComponent(){ ClearMails() ;}
	virtual bool OnMessage( stMsg* pMessage , eMsgPort eSenderPort );
	virtual void Reset();
	virtual void Init();
	virtual void OnOtherDoLogined();
	static void PostMailToDB(stMail* pMail  ,uint32_t nTargetUID );
	void ReciveMail(stMail* pMail);
	void ReciveMail(stRecievedMail& pMail);
protected:
	void ClearMails();
	void SendMailListToClient();
	void InformRecievedUnreadMails();
	void ProcessOfflineEvent();
	bool ProcessMail(stRecievedMail& pMail);
protected:
	LIST_MAIL m_vAllMail ;
};