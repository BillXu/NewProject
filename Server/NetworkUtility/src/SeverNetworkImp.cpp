 #include <WinSock2.h>
#include "SeverNetworkImp.h"
#include "Session.h"
CServerNetworkImp::CServerNetworkImp()
{
	m_acceptor = nullptr ;
}

CServerNetworkImp::~CServerNetworkImp()
{
	m_ioService.stop() ;
	if ( m_acceptor )
	{
		delete m_acceptor ;
	}

	if ( m_pIOThread )
	{
		delete m_pIOThread ;
		m_pIOThread = nullptr ;
	}

	if ( m_pIOThread2 )
	{
		delete m_pIOThread2 ;
		m_pIOThread2 = nullptr ;
	}
}

bool CServerNetworkImp::init(uint16_t nPort )
{
	tcp::endpoint endpoint(tcp::v4(), nPort);
	m_acceptor = new tcp::acceptor(m_ioService,endpoint);
	startAccept();

	m_pIOThread = new  boost::thread(boost::bind(&boost::asio::io_service::run, &m_ioService)); 
	m_pIOThread->detach();

	m_pIOThread2 = new  boost::thread(boost::bind(&boost::asio::io_service::run, &m_ioService)); 
	m_pIOThread2->detach();
	return true ;
}

void CServerNetworkImp::startAccept() 
{
	Session_ptr new_session(new CSession(m_ioService,this));  
	m_acceptor->async_accept(new_session->socket(),  
		boost::bind(&CServerNetworkImp::handleAccept, this, new_session,  
		boost::asio::placeholders::error)); 
}

void CServerNetworkImp::handleAccept(boost::shared_ptr<CSession> session,const boost::system::error_code& error) 
{
	if (!error)  
	{  
		session->start();  
		WriteLock wLock ( m_SessionMutex );
		m_vActiveSessions[session->getConnectID()] = session ;

		std::string str = session->getIPString() ;
		printf("a peer connected ip = %s \n",str.c_str()) ;
		Packet* pack = new Packet ;
		pack->_brocast = false ;
		pack->_packetType = _PACKET_TYPE_CONNECTED ;
		pack->_connectID = session->getConnectID() ;
		pack->_len = str.size();
		memset(pack->_orgdata,0,sizeof(pack->_orgdata));
		memcpy(pack->_orgdata,str.c_str(),pack->_len);
		addPacket(pack);

		session->startHeartbeatTimer();
		session->startWaitFirstMsg();
	}  

	startAccept(); //每连接上一个socket都会调用  
}

std::string CServerNetworkImp::getIPportString(uint32_t nConnectID)
{
	Session_ptr pt = getSessionByConnectID(nConnectID) ;
	if ( pt )
	{
		return pt->getIPString() ;
	}
	return "" ;
}

void CServerNetworkImp::closeSession(uint32_t nConnectID , bool bServerClose )
{
	//printf("closeSession id = %d\n",nConnectID);
	WriteLock wLock ( m_SessionMutex );
	MAP_SESSION::iterator iter = m_vActiveSessions.find(nConnectID) ;
	if ( iter != m_vActiveSessions.end() )
	{
		Session_ptr pt = iter->second ;
		std::string str = pt->getIPString();
		if ( bServerClose == false )
		{
			Packet* pack = new Packet ;
			pack->_brocast = false ;
			pack->_packetType = _PACKET_TYPE_DISCONNECT ;
			pack->_connectID = nConnectID ;
			pack->_len = str.size();
			memset(pack->_orgdata,0,sizeof(pack->_orgdata));
			memcpy(pack->_orgdata,str.c_str(),pack->_len);
			addPacket(pack);
			printf(" a peer disconnected \n") ;
		}
		else
		{
			printf("server post peer closed \n") ;
		}
		m_vActiveSessions.erase(iter) ;
		pt->close();
		//printf("a peer disconected \n") ;
	}
	//printf("closeSession end id = %d\n",nConnectID);
}

bool CServerNetworkImp::sendMsg(uint32_t nConnectID , const char* pData , size_t nLen )
{
	Session_ptr pt = getSessionByConnectID(nConnectID) ;
	if ( pt )
	{
		return pt->sendData(pData,nLen) ;
	}
	printf("cant not find session with id = %d to send msg \n",nConnectID ) ;
	return false ;
}

CServerNetworkImp::Session_ptr CServerNetworkImp::getSessionByConnectID(uint32_t nConnectID )
{
	ReadLock rLock ( m_SessionMutex );
	MAP_SESSION::iterator iter = m_vActiveSessions.find(nConnectID) ;
	if ( iter != m_vActiveSessions.end() )
	{
		return iter->second ;
	}
	printf("cant not find session with id = %d\n",nConnectID ) ;
	return nullptr ;
}

bool CServerNetworkImp::getAllPacket(LIST_PACKET& vOutPackets ) // must delete out side ;
{
	WriteLock wLock(m_PacketMutex) ;
	vOutPackets.swap(m_vRecivedPackets) ;
	return !vOutPackets.empty();
}

bool CServerNetworkImp::getFirstPacket(Packet** ppPacket ) // must delete out side ;
{
	WriteLock wLock(m_PacketMutex) ;
	if ( m_vRecivedPackets.empty() )
	{
		return false ;
	}

	LIST_PACKET::iterator iter = m_vRecivedPackets.begin() ;
	Packet* p = m_vRecivedPackets.front() ;
	*ppPacket = p ;
	m_vRecivedPackets.erase(iter) ;
	return true ;
}

void CServerNetworkImp::closePeerConnection(uint32_t nConnectID, bool bServerClose )
{
	//printf("post close id = %d\n",nConnectID);
	m_ioService.post(boost::bind(&CServerNetworkImp::closeSession, this, nConnectID, bServerClose));
}

void CServerNetworkImp::addPacket(Packet* pPacket )
{	
	WriteLock wLock(m_PacketMutex) ;
	m_vRecivedPackets.push_back(pPacket) ;
}

void CServerNetworkImp::onReivedData(uint32_t nConnectID , const char* pBuffer , size_t nLen )
{
	if ( nLen > _MSG_BUF_LEN )
	{
		printf("too big buffer from connect id = %d\n",nConnectID) ;
		return ;
	}
	Packet* pack = new Packet ;
	pack->_brocast = false ;
	pack->_packetType = _PACKET_TYPE_MSG ;
	pack->_connectID = nConnectID ;
	pack->_len = nLen ;
	memcpy(pack->_orgdata,pBuffer,pack->_len);
	addPacket(pack);
}