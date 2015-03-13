#include "ServerNetworkImp.h"
#include "Session.h"
//! Æô¶¯ÓëÍ£Ö¹
bool CServerNetworkImp::Start(unsigned short nPort)
{
	m_ios = CreateIOServer();
	if (!m_ios->Initialize(this))
	{
		LOGE("manager start fail!");
		return false;
	}

	m_accept = CreateTcpAccept();
	m_accept->Initialize(m_ios, this);
	if (m_accept->OpenAccept("0.0.0.0", nPort))
	{
		LOGFMTI("OpenAccept %d success",nPort);
	}
	else
	{
		LOGFMTE("OpenAccept %d failed",nPort);
		return false;
	}
	INetwork::Start();
	return true ;
}

bool CServerNetworkImp::OnStop()
{
	return true;
}

bool CServerNetworkImp::OnPost(void *pUser)
{
	return true;
}

bool CServerNetworkImp::OnTimer()
{
	return true;
}

//! acceptµ½client
bool CServerNetworkImp::OnAccept(ITcpSocket * s)
{
	CSession * p = new CSession;
	s->Initialize(m_ios, p);
	p->InitSocketForServer(this, s,++m_nCurMaxConnectID);
	g_nTotalLinked ++;
	return true;
}

bool CServerNetworkImp::OnClose()
{
	LOGD("accept socket do close");
	DestroyTcpAccept(m_accept);
	Stop();
	return true;
}
