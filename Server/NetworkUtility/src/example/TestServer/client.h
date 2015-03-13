#pragma once
#include "../zsummer_server/ClientNetworkImp.h"
class CClientApp
{
public:
	bool Init();
	void RunRoop();
	void ProcessMsg();
	void SendMsg(const char* p );
protected:
	CClientNetwork* m_pNetWork;
	CONNECT_ID m_nServerID ;
};