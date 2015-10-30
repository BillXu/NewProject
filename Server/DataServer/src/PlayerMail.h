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
	void OnReactive(uint32_t nSessionID ) override ;
	static void PostMailToDB(stMail* pMail  ,uint32_t nTargetUID );
	static void PostMailToDB(const char* pContent, uint16_t nContentLen ,uint32_t nTargetUID );
	static void PostPublicMail(stRecievedMail& pMail);
	static uint16_t getNewerMailListByTime( uint32_t nTimeTag, LIST_MAIL* vOutMail = nullptr,uint16_t nMaxOutCnt = 20);
	void ReciveMail(stMail* pMail);
	void ReciveMail(const char* pContent, uint16_t nContentLen );
	void ReciveMail(stRecievedMail& pMail);
protected:
	void ClearMails();
	void SendMailListToClient();
	void InformRecievedUnreadMails();
	void ProcessOfflineEvent();
	bool ProcessMail(stRecievedMail& pMail);
protected:
	LIST_MAIL m_vAllMail ;
	static LIST_MAIL s_vPublicMails ;
	static bool s_isReadedPublic ;
	uint32_t m_tReadTimeTag ;
};