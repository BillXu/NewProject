#include <WinSock2.h>
#include "Session.h"
#include "SeverNetworkImp.h"
uint32_t CSession::s_ConnectID = 1 ;
CSession::CSession(asio::io_service& io_service,CServerNetworkImp* network )
	:m_socket(io_service),m_pNetwork(network),m_pReadIngBuffer(new CInternalBuffer()),m_tHeatBeat(io_service),m_tWaitFirstMsg(io_service)
{
	m_nConnectID = ++s_ConnectID ;
	m_bRecivedMsg = false ;
}

bool CSession::sendData(const char* pData , uint16_t nLen )
{
	InternalBuffer_ptr pBuffer( new CInternalBuffer() );
	if ( ! pBuffer->setData(pData,nLen) )
	{
		return false ;
	}
	
	WriteLock wLock(m_SendBuffersMutex);
	bool bSending = m_vWillSendBuffers.empty() == false ;
	m_vWillSendBuffers.push_back(pBuffer) ;
	if ( bSending == false )
	{
		asio::async_write(m_socket,  
			asio::buffer(m_vWillSendBuffers.front()->data(),  
			m_vWillSendBuffers.front()->length()),   
			boost::bind(&CSession::handleWrite, shared_from_this(),  
			asio::placeholders::error)); 
	}
	return true ;
}

uint32_t CSession::getConnectID()
{
	return m_nConnectID ;
}

std::string CSession::getIPString()
{
	return m_socket.remote_endpoint().address().to_string();
}

void CSession::start()
{
	asio::async_read(m_socket,  
		asio::buffer(m_pReadIngBuffer->data(), CInternalBuffer::header_length),  
		boost::bind(  
		&CSession::handleReadHeader, shared_from_this(),  
		asio::placeholders::error)); //异步读客户端发来的消息
}

void CSession::close()
{
	//printf("close close\n") ;
	m_socket.close() ;
	m_tHeatBeat.cancel();
	m_tWaitFirstMsg.cancel();
}
// handle function ;
void CSession::handleReadHeader(const asio::error_code& error)
{
	if (!error && m_pReadIngBuffer->decodeHeader())  
	{  
		asio::async_read(m_socket,  
			asio::buffer(m_pReadIngBuffer->body(), m_pReadIngBuffer->bodyLength()),  
			boost::bind(&CSession::handleReadBody, shared_from_this(),  
			asio::placeholders::error));  
	}  
	else  
	{  
		//printf("handleReadHeader close\n") ;
		m_pNetwork->closeSession(getConnectID());
	} 
}

void CSession::handleReadBody(const asio::error_code& error)
{
	if (!error)  
	{  
		if ( m_pReadIngBuffer->bodyLength() )
		{
			m_pNetwork->onReivedData(getConnectID(),m_pReadIngBuffer->body(),m_pReadIngBuffer->bodyLength());
			m_bRecivedMsg = true ;
		}

		asio::async_read(m_socket,  
			asio::buffer(m_pReadIngBuffer->data(), CInternalBuffer::header_length),  
			boost::bind(  
			&CSession::handleReadHeader, shared_from_this(),  
			asio::placeholders::error));
	}  
	else  
	{  
		//printf("handleReadBody close\n") ;
		m_pNetwork->closeSession(getConnectID());
	}  
}

void CSession::handleWrite(const asio::error_code& error)
{
	if (!error)  
	{  
		WriteLock wLock(m_SendBuffersMutex);
		m_vWillSendBuffers.pop_front();  
		if (!m_vWillSendBuffers.empty())  
		{  
			asio::async_write(m_socket,  
				asio::buffer(m_vWillSendBuffers.front()->data(),  
				m_vWillSendBuffers.front()->length()),  
				boost::bind(&CSession::handleWrite, shared_from_this(),  
				asio::placeholders::error));
		}  
	}  
	else  
	{  
		m_pNetwork->closeSession(getConnectID());
	} 
}

void CSession::startWaitFirstMsg()
{
	m_tWaitFirstMsg.expires_from_now(boost::posix_time::seconds(TIME_CHECK_FIRST_MSG));
	m_tWaitFirstMsg.async_wait(boost::bind(&CSession::handleCheckFirstMsg, this,asio::placeholders::error ));
}

void CSession::handleCheckFirstMsg(const asio::error_code& ec)
{
	if ( !ec )
	{
		if ( !m_bRecivedMsg )
		{
			printf("find a dead connect \n");
			m_pNetwork->closeSession(getConnectID());
		}
	}
}

void CSession::startHeartbeatTimer()
{
	m_tHeatBeat.expires_from_now(boost::posix_time::seconds(TIME_HEAT_BET));
	m_tHeatBeat.async_wait(boost::bind(&CSession::sendHeatBeat, this,asio::placeholders::error));
}

void CSession::handleWriteHeartbeat(const asio::error_code& ec)
{
	if(!ec){
		startHeartbeatTimer();
		//printf("send beat ok \n");
	}
	else{
		// close ;
		printf("heat beat failed \n");
		m_pNetwork->closeSession(getConnectID());
	}
}

void CSession::sendHeatBeat(const asio::error_code& ec)
{
	if ( !ec )
	{
		char p[4] = { 0 } ;
		unsigned short* psh = (unsigned short*)p;
		*psh = 3;
		p[2] = 0 ;
		p[3] = 0 ;
		asio::async_write(m_socket, asio::buffer(p, sizeof(p)),
			boost::bind(&CSession::handleWriteHeartbeat, this,
			asio::placeholders::error ));
	}
}