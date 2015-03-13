#pragma once
#include "../zsummer_server/ServerNetworkImp.h"
class CServerApp
{
public:
	bool Init();
	void RunRoop();
	void ProcessMsg();
	void SendMsg(CONNECT_ID nTarget, const char* p);
protected:
	CServerNetworkImp* m_pNetWork;
	CONNECT_ID m_nServerID ;
};