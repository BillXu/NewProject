#pragma once
#include "INetwork.h"
class CServerNetworkImp
	:public INetwork,public ITcpAcceptCallback
{
public:
	CServerNetworkImp(){ m_accept = NULL ; m_nCurMaxConnectID = INVALID_CONNECT_ID ;}
	bool Start(unsigned short nPort);
protected:
	virtual bool OnStop();
	virtual bool OnPost(void *pUser);
	virtual bool OnTimer();

	virtual bool OnAccept(ITcpSocket * s);
	virtual bool OnClose();
protected:
	ITcpAccept * m_accept;

	CONNECT_ID m_nCurMaxConnectID ;
};