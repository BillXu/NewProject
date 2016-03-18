#include "SeverUtility.h"
#include "AutoBuffer.h"
#include "ServerMessageDefine.h"
void CSendPushNotification::reset()
{
	m_arrayTargetIDs.clear() ;
	m_tFinalContent.clear() ;
	m_refAutoBuffer.clearBuffer();
}

void CSendPushNotification::addTarget(uint32_t nUserUID )
{
	m_arrayTargetIDs[m_arrayTargetIDs.size()] = nUserUID ;
}

void CSendPushNotification::setContent(const char* pContent,uint32_t nFlag )
{
	m_tFinalContent["flag"] = nFlag ;
	m_tFinalContent["content"] = pContent ;
}

CAutoBuffer* CSendPushNotification::getNoticeMsgBuffer()
{
	if ( m_tFinalContent.isNull() || m_arrayTargetIDs.isNull() )
	{
		printf("push notice argument not finish\n") ;
		return nullptr;
	}

	m_tFinalContent["targets"] = m_arrayTargetIDs ;
	Json::StyledWriter writer ;
	std::string str = writer.write(m_tFinalContent) ;
	
	stMsgPushNotice msgpush ;
	msgpush.nJonsLen = str.size() ;
	m_refAutoBuffer.addContent(&msgpush,sizeof(msgpush)) ;
	m_refAutoBuffer.addContent(str.c_str(),msgpush.nJonsLen) ;
	return &m_refAutoBuffer ;
}