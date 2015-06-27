#include "Session.h"
#include "ServerNetworkImp.h"
#include "depends/protocol4z/protocol4z.h"
CSession::CSession()
{
	m_pNetwork = NULL;
	m_socket = NULL;

	m_type = 1;
	m_bSeverMode = false ;
	memset(&m_recving, 0, sizeof(m_recving));
	m_curRecvLen = 0;
	memset(&m_sending, 0, sizeof(m_sending));
	m_curSendLen = 0;
	m_nConnectID = INVALID_CONNECT_ID ;
	m_bWaitDelete = 0 ;
	m_nHeatBetTimeOut = (time_t)-1 ;;
	m_nTimeSendBet = (time_t)-1 ;

	memset(m_pIPString,0,sizeof(m_pIPString));
}

CSession::~CSession()
{
	if ( m_socket && !m_bSeverMode )
	{
		//DestroyTcpSocket(m_socket);  // mostly m_socket will delete it self ;
	}
}

void CSession::InitSocketForServer(INetwork *proc, ITcpSocket *s, CONNECT_ID nConnectID )
{
	m_nHeatBetTimeOut = time(NULL) + _HEAT_BET_TIME_OUT ;
	m_bWaitDelete = false ;

	m_nTimeSendBet = time(NULL) + _HEAT_BET_SEND_TIME ;

	m_nConnectID = nConnectID ;
	m_pNetwork = proc;
	m_socket = s;
	m_socket->DoRecv(m_recving._orgdata, 2);
	m_bSeverMode = true ;

	m_pNetwork->OnNewSession(this,m_nConnectID);
	// add packet 
	Packet* p = new Packet ;
	p->_connectID = m_nConnectID ;
	p->_brocast = false ;
	p->_len = sizeof(ConnectInfo);
	p->_packetType = _PACKET_TYPE_CONNECTED ;

	ConnectInfo cInfo ;
	unsigned int nAddress ;
	m_socket->GetPeerInfo(&nAddress,&cInfo.nPort);
	memset(cInfo.strAddress,0,sizeof(cInfo.strAddress));
	in_addr addres ;
	addres.S_un.S_addr= nAddress ;
	sprintf_s((char*)cInfo.strAddress,sizeof(cInfo.strAddress),"%s",inet_ntoa(addres));
	memcpy(p->_orgdata,&cInfo,sizeof(cInfo));
	m_pNetwork->AddPacket(p);

	memset(m_pIPString,0,sizeof(m_pIPString));
	sprintf_s(m_pIPString,sizeof(m_pIPString),"%s |%d",cInfo.strAddress,cInfo.nPort);
}

bool CSession::InitSocketForClient(INetwork *proc, zsummer::network::IIOServer* pIO,const char* pip, unsigned short nPort, CONNECT_ID nConnectID)
{
	m_socket = zsummer::network::CreateTcpSocket();
	if ( !m_socket->Initialize(pIO, this) )
	{
		delete m_socket ;
		m_socket = NULL ;
		return false ;
	}
	m_socket->DoConnect(pip, nPort);

	m_nConnectID = nConnectID ;
	m_pNetwork = proc;
	m_bSeverMode = true ;
	return true ;
}

bool CSession::OnRecv(unsigned int nRecvedLen)
{
	m_curRecvLen += nRecvedLen;

// 	static int totolRecive = 0 ;
// 	totolRecive += nRecvedLen ;
// 	printf("totolRecive = %d\n",totolRecive);

	std::pair<zsummer::protocol4z::INTEGRITY_RET_TYPE, zsummer::protocol4z::DefaultStreamHeadTraits::Integer> ret = zsummer::protocol4z::CheckBuffIntegrity<zsummer::protocol4z::DefaultStreamHeadTraits>(m_recving._orgdata, m_curRecvLen, _MSG_BUF_LEN);

	if (ret.first == zsummer::protocol4z::IRT_CORRUPTION)
	{
		LOGE("killed socket: CheckBuffIntegrity error ");
		m_socket->Close();
		return false;
	}
	if (ret.first == zsummer::protocol4z::IRT_SHORTAGE)
	{
		m_socket->DoRecv(m_recving._orgdata+m_curRecvLen, ret.second);
		return true;
	}

// 	//! 解包完成 进行消息处理
 	zsummer::protocol4z::ReadStream<zsummer::protocol4z::DefaultStreamHeadTraits> rs(m_recving._orgdata, m_curRecvLen);
	//printf( "recving idx: %d, len = %d\n",*((unsigned short*)(m_recving._orgdata+2)),m_curRecvLen);
	if ( rs.GetStreamBodyLen() == sizeof(unsigned short) )  // heat beat 
	{
		unsigned short nproid ; 
		rs >> nproid ;
		if ( nproid )
		{
			LOGFMTE("why heat bet value = %d",nproid );
		}
		m_nHeatBetTimeOut = time(NULL) + _HEAT_BET_TIME_OUT ;
		m_bWaitDelete = false ;
		//LOGD("RECIVED HEAT BET ");
	}
	else
	{
		Packet* pOkPacket = new Packet ;
		memcpy(pOkPacket->_orgdata,rs.GetStreamBody(),rs.GetStreamBodyLen());
		pOkPacket->_brocast = false;
		pOkPacket->_len = rs.GetStreamBodyLen();
		pOkPacket->_connectID = m_nConnectID;
		pOkPacket->_packetType = _PACKET_TYPE_MSG ;
		m_pNetwork->AddPacket(pOkPacket);
	}

	//! 继续收包
	m_recving._len = 0;
	m_curRecvLen = 0;
	m_socket->DoRecv(m_recving._orgdata, 2);
	return true;
}

void CSession::Send(char * buf, unsigned int len)
{
	if (m_sending._len != 0)
	{
		Packet *p = new Packet;
		memcpy(p->_orgdata, buf, len);
		p->_len = len;
		m_sendque.push(p);
		//printf("send queeu sending len = %d\n  ",p->_len);
	}
	else
	{
		memcpy(m_sending._orgdata, buf, len);
		m_sending._len= len;
		m_socket->DoSend(m_sending._orgdata, m_sending._len);
		//printf("send direct nlen = %d\n ",m_sending._len);
	}
}

bool CSession::OnConnect(bool bConnected)
{
	if ( bConnected )
	{
		printf("connect to server sucess\n");
		OnRecv(0);
		m_pNetwork->OnNewSession(this,m_nConnectID) ;
		// add packet 
		Packet* p = new Packet ;
		p->_connectID = m_nConnectID ;
		p->_brocast = false ;
		
		ConnectInfo cInfo ;
		unsigned int nAddress ;
		m_socket->GetPeerInfo(&nAddress,&cInfo.nPort);
		memset(cInfo.strAddress,0,sizeof(cInfo.strAddress));
		in_addr addres ;
		addres.S_un.S_addr= nAddress ;
		sprintf_s((char*)cInfo.strAddress,sizeof(cInfo.strAddress),"%s",inet_ntoa(addres));
		memcpy(p->_orgdata,&cInfo,sizeof(cInfo));

		p->_len = sizeof(ConnectInfo);
		p->_packetType = _PACKET_TYPE_CONNECTED;
		m_pNetwork->AddPacket(p);
		memset(m_pIPString,0,sizeof(m_pIPString));
		sprintf_s(m_pIPString,sizeof(m_pIPString),"%s | %d",cInfo.strAddress,cInfo.nPort);

		m_nTimeSendBet = time(NULL) + _HEAT_BET_SEND_TIME ;
		m_nHeatBetTimeOut = time(NULL) + _HEAT_BET_TIME_OUT ;
		m_bWaitDelete = false ;
	}
	else
	{
		Packet* p = new Packet ;
		p->_connectID = m_nConnectID ;
		p->_brocast = false ;
		p->_len = sizeof(ConnectInfo);
		p->_packetType = _PACKET_TYPE_CONNECT_FAILED ;

		ConnectInfo cInfo ;
		unsigned int nAddress ;
		m_socket->GetPeerInfo(&nAddress,&cInfo.nPort);
		memset(cInfo.strAddress,0,sizeof(cInfo.strAddress));
		in_addr addres ;
		addres.S_un.S_addr= nAddress ;
		sprintf_s((char*)cInfo.strAddress,sizeof(cInfo.strAddress),"%s",inet_ntoa(addres));
		memcpy(p->_orgdata,&cInfo,sizeof(cInfo));
		m_pNetwork->AddPacket(p);

		//LOGI("connect failed so close delete ! ");
		printf("connect failed so close socket = %X, sesion = %X \n",(int)m_socket,(int)this) ;
		// as connected failed tcpsocket will not delete it self ; so we do it ;
		delete m_socket; ;
		m_socket = nullptr ;
		delete this ;
	}
	return true;
}

bool CSession::CheckHeatbet()
{
	time_t tNow = time(NULL) ;
	if ( tNow >= m_nTimeSendBet )
	{
		unsigned short nHeartBet = 0 ;
 		char buf[10] = {0};
 		zsummer::protocol4z::WriteStream<zsummer::protocol4z::DefaultStreamHeadTraits> ws(buf, 10);
 		ws << nHeartBet ;
 		this->Send(buf,ws.GetStreamLen()) ;

		m_nTimeSendBet = tNow + _HEAT_BET_SEND_TIME ;
		//LOGD("send heat bet");
	}

	if ( tNow >= m_nHeatBetTimeOut )
	{
		if ( m_bWaitDelete )
		{
			LOGD("heat bet time out will delete");
			return false ;
		}
		else
		{
			LOGD("heat bet time out just close");
			m_bWaitDelete = true ;
			m_nHeatBetTimeOut = tNow + 2 ;
			Close();
		}
	}
	return true ;
}

bool CSession::OnSend(unsigned int nSentLen)
{
	m_curSendLen += nSentLen;
	if (m_curSendLen < m_sending._len)
	{
		m_socket->DoSend(&m_sending._orgdata[m_curSendLen], m_sending._len - m_curSendLen);
	}
	else if (m_curSendLen == m_sending._len)
	{
// 		m_process->AddTotalSendCount(1);
// 		m_process->AddTotalSendLen(m_curSendLen);
		m_curSendLen = 0;
		if (m_sendque.empty())
		{
			m_sending._len = 0;
		}
		else
		{
			Packet *p = m_sendque.front();
			m_sendque.pop();
			memcpy(m_sending._orgdata, p->_orgdata, p->_len);
			m_sending._len = p->_len;
			delete p;
			m_socket->DoSend(m_sending._orgdata, m_sending._len);
		}
	}
	return true;
}

bool CSession::OnClose()
{
	Packet* p = new Packet ;
	p->_connectID = m_nConnectID ;
	p->_brocast = false ;
	p->_len = sizeof(ConnectInfo);
	p->_packetType = _PACKET_TYPE_DISCONNECT ;

	ConnectInfo cInfo ;
	unsigned int nAddress ;
	m_socket->GetPeerInfo(&nAddress,&cInfo.nPort);
	memset(cInfo.strAddress,0,sizeof(cInfo.strAddress));
	in_addr addres ;
	addres.S_un.S_addr= nAddress ;
	sprintf_s((char*)cInfo.strAddress,sizeof(cInfo.strAddress),"%s",inet_ntoa(addres));
	memcpy(p->_orgdata,&cInfo,sizeof(cInfo));
	m_pNetwork->AddPacket(p);

	m_pNetwork->RemoveSession(m_nConnectID);

	LOGI("Client Closed!");
	delete this; //! 安全的自删除源于底层的彻底的异步分离
	AtomicAdd(&g_nTotalCloesed, 1);
	return true;
}

void CSession::Close()
{
	if ( m_socket )
	{
		LOGD(" CSession::Close ");
		m_socket->Close();
	}
}
