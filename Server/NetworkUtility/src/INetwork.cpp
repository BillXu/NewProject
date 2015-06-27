#include "INetwork.h"
#include "Session.h"
INetwork::INetwork()
{
	m_bRunning = false;
	m_ios = NULL ;
}

INetwork::~INetwork()
{

}

bool INetwork::GetAllPacket(VEC_PACKET& vOutPackets ) // must delete out side ;
{
	vOutPackets.clear();
	{
		CAutoLock al(m_lockRecvPackets);
		vOutPackets.swap(m_vRecievedPackets);
	}
	return vOutPackets.empty() == false ;
}

bool INetwork::SendMsg(unsigned char* pBuffer , unsigned short nLen ,CONNECT_ID nSendToOrExcpt , bool bBorcast)
{
	if ( nLen >=  _MSG_BUF_LEN )
	{
		LOGE("message is tool long");
		return false ;
	}
	Packet* packet = new Packet ;
	packet->_brocast = bBorcast ;
	packet->_connectID = nSendToOrExcpt ;
	packet->_len = nLen ;
	packet->_packetType = _PACKET_TYPE_MSG;
	memcpy(packet->_orgdata,pBuffer,nLen);

	CAutoLock al(m_lockSendPackets);
	m_vWillSendPackets.push_back(packet);
	return true ;
}

void INetwork::CloseConnection(CONNECT_ID nConnectID )
{
	Packet* pack = new Packet ;
	pack->_brocast = false ;
	pack->_packetType = _PACKET_TYPE_DISCONNECT ;
	pack->_connectID = nConnectID ;
	CAutoLock al(m_lockSendPackets);
	m_vWillSendPackets.push_back(pack);
}

char* INetwork::GetIPStringByConnectID( CONNECT_ID nConnectID )
{
	CAutoLock cl(m_lockAllSession);
	MAP_ID_SESSION::iterator iter_se = m_vAllSessions.find(nConnectID) ;
	if ( iter_se == m_vAllSessions.end() )
	{
		return "NULL";
	}
	else
	{
		return iter_se->second->GetIPString();
	}
	return "NULL" ;
}

void INetwork::OnNewSession(CSession* pAddClient,CONNECT_ID nConnectID )
{
	CAutoLock cl(m_lockAllSession);
	m_vAllSessions.insert(MAP_ID_SESSION::value_type(nConnectID,pAddClient));
}

void INetwork::RemoveSession(CONNECT_ID nConnectID )
{
	CAutoLock cl(m_lockAllSession);
	m_vAllSessions.erase(nConnectID);
}

void INetwork::AddPacket(Packet* pPacket )
{
	CAutoLock al (m_lockRecvPackets) ;
	m_vRecievedPackets.push_back(pPacket);
}

void INetwork::Start()
{
	m_bRunning = true;
	CThread::Start();
}

void INetwork::Stop()
{
	m_bRunning = false;
}

//! Ïß³Ì
void INetwork::Run()
{
	while (m_bRunning)
	{
		m_ios->RunOnce();
		ProcessSendMsg();
		CheckHeatBeat();
	}
}

void INetwork::CheckHeatBeat()
{
	VEC_SESSION vWillDelete ;
	MAP_ID_SESSION::iterator iter = m_vAllSessions.begin();
	for ( ; iter != m_vAllSessions.end(); ++iter )
	{
		if ( iter->second->CheckHeatbet() == false )
		{
			vWillDelete.push_back(iter->second) ;
		}
	}

	VEC_SESSION::iterator iterDel = vWillDelete.begin();
	for ( ; iterDel != vWillDelete.end(); ++iterDel )
	{
		LOGD( "check bet bet failed delete session");
		(*iterDel)->Close();
	}
}

void INetwork::ProcessSendMsg()
{
	VEC_PACKET vPacket ;
	vPacket.clear();
	{
		CAutoLock al(m_lockSendPackets);
		vPacket.swap(m_vWillSendPackets) ;
	}

	VEC_PACKET::iterator iter = vPacket.begin();
	for ( ; iter != vPacket.end(); ++iter )
	{
		Packet* packet = *iter ;
		if ( packet->_packetType == _PACKET_TYPE_MSG )
		{
			//printf("packet->_packetType == _PACKET_TYPE_MSG \n");
			char buf[_MSG_BUF_LEN];
			zsummer::protocol4z::WriteStream<zsummer::protocol4z::DefaultStreamHeadTraits> ws(buf, _MSG_BUF_LEN);
			ws.AppendOriginalData(packet->_orgdata,packet->_len);

			if ( packet->_brocast )
			{
				MAP_ID_SESSION::iterator iter_se = m_vAllSessions.begin();
				for ( ; iter_se != m_vAllSessions.end(); ++iter_se )
				{
					if ( packet->_connectID != iter_se->first )
					{
						iter_se->second->Send(buf,ws.GetStreamLen()) ;
					}
				}
			}
			else
			{
				MAP_ID_SESSION::iterator iter_se = m_vAllSessions.find(packet->_connectID) ;
				if ( iter_se == m_vAllSessions.end() )
				{
					LOGFMTW( "can not find connect id = %d ,send failed, all sessionSize = %d",packet->_connectID,m_vAllSessions.size() );
					MAP_ID_SESSION::iterator iter = m_vAllSessions.begin();
					for ( ; iter != m_vAllSessions.end(); ++iter )
					{
						LOGFMTW( "current connectid = %d",iter->first );
					}
				}
				else
				{
					//printf("iter_se->second send msg %d len = %d \n",packet->_connectID,ws.GetStreamLen());
					iter_se->second->Send(buf,ws.GetStreamLen()) ;
					//printf("send msg Send(buf\n");
				}
			}
		}
		else if ( packet->_packetType == _PACKET_TYPE_DISCONNECT )
		{
			MAP_ID_SESSION::iterator iter_se = m_vAllSessions.find(packet->_connectID) ;
			if ( iter_se == m_vAllSessions.end() )
			{
				LOGFMTE( "can not find connect id = %d ,disconnect failed",packet->_connectID );
			}
			else
			{
				iter_se->second->Close(); 
			}
		}
		else
		{
			LOGFMTE("unknown packet type %d",packet->_packetType);
		}
		delete packet ;
	}
	vPacket.clear();
}