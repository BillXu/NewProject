#include "PlayerMail.h"
#include "ServerMessageDefine.h"
#include "LogManager.h"
#include "Player.h"
#include <time.h>
#include "PlayerFriend.h"
#include "ShopConfg.h"
#include "PlayerBaseData.h"
#include "PlayerItem.h"
#include "GameServerApp.h"
#include "ConfigManager.h"
#include "PlayerEvent.h"
#include "AutoBuffer.h"
#define  MAX_KEEP_MAIL_CNT 10

bool arrageMailByTime( CPlayerMailComponent::stRecievedMail& left , CPlayerMailComponent::stRecievedMail& right )
{
	if ( left.nRecvTime < right.nRecvTime ) 
	{
		return true ;
	}

	if ( left.nRecvTime > right.nRecvTime )
	{
		return false ;
	}
	return true ;
}

CPlayerMailComponent::LIST_MAIL CPlayerMailComponent::s_vPublicMails ;
bool CPlayerMailComponent::s_isReadedPublic = false ;
bool CPlayerMailComponent::OnMessage( stMsg* pMsg , eMsgPort eSenderPort )
{
	if ( IPlayerComponent::OnMessage(pMsg,eSenderPort) )
	{
		return true ;
	}

	switch ( pMsg->usMsgType )
	{
	case MSG_PLAYER_READ_MAIL_LIST:
		{
			stMsgReadMailListRet* pMsgRet = (stMsgReadMailListRet*)pMsg ;
			stRecievedMail sMail ;
			sMail.eType = (eMailType)pMsgRet->pMails.eType ;
			sMail.nRecvTime = pMsgRet->pMails.nPostTime ;
			CAutoBuffer auf (pMsgRet->pMails.nContentLen + 1 );
			auf.addContent((char*)pMsg + sizeof(stMsgReadMailListRet),pMsgRet->pMails.nContentLen ) ;
			sMail.strContent = auf.getBufferPtr() ;

			if ( sMail.eType == eMail_Public )
			{
				s_vPublicMails.push_back(sMail) ;
				if ( pMsgRet->bFinal )
				{
					s_vPublicMails.sort(arrageMailByTime);
				}
				return true  ;
			}

			m_vAllMail.push_back(sMail);

			if ( pMsgRet->bFinal )
			{
				// if have unread mail should tell client ;
				ProcessOfflineEvent();
				stPlayerEvetArg arg ;
				arg.eEventType = ePlayerEvent_ReadDBOK;
				GetPlayer()->PostPlayerEvent(&arg);
				InformRecievedUnreadMails();
				CLogMgr::SharedLogMgr()->PrintLog("read mail finish uid = %d",GetPlayer()->GetUserUID());
				if ( m_vAllMail.empty() == false )
				{
					m_vAllMail.sort(arrageMailByTime);
				}
			}

		}
		break;
	case MSG_PLAYER_REQUEST_MAIL_LIST:
		{
			SendMailListToClient();
			return true ;
		}
		break;
	case MSG_PLAYER_ADVICE:
		{
			stMsgPlayerAdviceRet msgRet ;
			msgRet.nRet = 0 ;
			stMsgPlayerAdvice* pRet = (stMsgPlayerAdvice*)pMsg ;
			if ( pRet->nLen > 512 || pRet->nLen <= 7 )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("uid = %d advice too long len = %d",GetPlayer()->GetUserUID(),pRet->nLen) ;
				msgRet.nRet = 1; 
			}
			SendMsg(&msgRet,sizeof(msgRet)) ;

			if ( msgRet.nRet )
			{
				return true ;
			}

			stMsgSavePlayerAdvice msgSaveAdVice ;
			msgSaveAdVice.nUserUID = GetPlayer()->GetUserUID() ;
			msgSaveAdVice.nLen = pRet->nLen ;

			CAutoBuffer buff(msgSaveAdVice.nLen + sizeof(msgSaveAdVice));
			buff.addContent(&msgSaveAdVice,sizeof(msgSaveAdVice)) ;
			buff.addContent(((char*)pMsg)+ sizeof(stMsgPlayerAdvice) ,pRet->nLen ) ;
			SendMsg((stMsg*)buff.getBufferPtr(),buff.getContentSize()) ;
			CLogMgr::SharedLogMgr()->PrintLog("recived uid = %d advice",GetPlayer()->GetUserUID());
		}
		break;
	default:
		return false ;
	}
	return true ;
}

void CPlayerMailComponent::Reset()
{
	ClearMails();
	stMsgReadMailList msg ;
	msg.nUserUID = GetPlayer()->GetUserUID() ;
	SendMsg(&msg,sizeof(msg)) ;

	if ( s_isReadedPublic == false )
	{
		s_isReadedPublic = true ;
		stMsgReadMailList msgpublic ;
		msgpublic.nUserUID = 0 ;
		SendMsg(&msgpublic,sizeof(msgpublic));
	}
}

void CPlayerMailComponent::Init()
{
	Reset();
}

void CPlayerMailComponent::OnOtherDoLogined()
{
	// send unread mail count to client ;
	InformRecievedUnreadMails();
}

void CPlayerMailComponent::InformRecievedUnreadMails()
{
	stMsgInformNewMail msg ;
	msg.nUnreadMailCount = m_vAllMail.size() + getNewerMailListByTime(m_tReadTimeTag) ;
	msg.nUnreadMailCount = msg.nUnreadMailCount < MAX_KEEP_MAIL_CNT ? msg.nUnreadMailCount : MAX_KEEP_MAIL_CNT ;
	msg.eNewMailType = eMail_Public ;

	if ( msg.nUnreadMailCount )
	{
		SendMsg(&msg,sizeof(msg)) ;
	}
}

void CPlayerMailComponent::OnReactive(uint32_t nSessionID )
{
	IPlayerComponent::OnReactive(nSessionID) ;
	InformRecievedUnreadMails();
}

void CPlayerMailComponent::ClearMails()
{
	m_vAllMail.clear();
}

void CPlayerMailComponent::SendMailListToClient()
{
	LIST_MAIL vSendMailList ;
	vSendMailList.assign(m_vAllMail.begin(),m_vAllMail.end()) ;
	if ( vSendMailList.size() < MAX_KEEP_MAIL_CNT )
	{
		getNewerMailListByTime(m_tReadTimeTag,&vSendMailList,MAX_KEEP_MAIL_CNT - vSendMailList.size() );
	}
	
	if ( vSendMailList.empty() )
	{
		return ;
	}

	vSendMailList.sort(arrageMailByTime);


	stMsgRequestMailListRet msgRet;
	uint8_t nSize = vSendMailList.size() ;
	CAutoBuffer auBuff(sizeof(msgRet) + 100 );
	for ( stRecievedMail& pMail : vSendMailList )
	{
		--nSize ;
		msgRet.isFinal = nSize == 0 ;
		msgRet.tMail.eType = pMail.eType ;
		msgRet.tMail.nContentLen = pMail.strContent.size() ;
		auBuff.clearBuffer();
		auBuff.addContent(&msgRet,sizeof(msgRet));
		auBuff.addContent(pMail.strContent.c_str(),msgRet.tMail.nContentLen) ;
		SendMsg((stMsg*)auBuff.getBufferPtr(),auBuff.getContentSize()) ;
	}
	CLogMgr::SharedLogMgr()->PrintLog("send mail to client uid = %d ,size = %d",GetPlayer()->GetUserUID(),vSendMailList.size() ) ;

#ifndef _DEBUG
	// tell db set state 
	stMsgResetMailsState msgReset ;
	msgReset.nUserUID = GetPlayer()->GetUserUID() ;
	msgReset.tMailType = eMail_Max ;
	SendMsg(&msgReset,sizeof(msgReset)) ;
	m_tReadTimeTag = time(nullptr) ;

	m_vAllMail.clear() ;

	stMail msgSaveTimetag ;
	msgSaveTimetag.eType = eMail_ReadTimeTag ;
	msgSaveTimetag.nContentLen = 0 ;
	msgSaveTimetag.nPostTime = m_tReadTimeTag ;
	PostMailToDB(&msgSaveTimetag,GetPlayer()->GetUserUID()) ;
#endif // DEBUG

}

void CPlayerMailComponent::PostMailToDB(stMail* pMail ,uint32_t nTargetUID  )
{
	stMsgSaveMail msgSave ;
	msgSave.nUserUID = nTargetUID ;
	memcpy(&msgSave.pMailToSave,pMail,sizeof(stMail));
	CAutoBuffer auBuffer(sizeof(msgSave) + pMail->nContentLen );
	auBuffer.addContent(&msgSave,sizeof(msgSave)) ;
	auBuffer.addContent(((char*)pMail)+sizeof(stMail),pMail->nContentLen) ;
	CGameServerApp::SharedGameServerApp()->sendMsg(nTargetUID,auBuffer.getBufferPtr(),auBuffer.getContentSize()) ;
}

void CPlayerMailComponent::PostMailToDB(const char* pContent, uint16_t nContentLen ,uint32_t nTargetUID )
{
	stMail* mail = new stMail ;
	mail->eType = eMail_PlainText ;
	mail->nPostTime = time(nullptr) ;
	mail->nContentLen = nContentLen;
	CAutoBuffer buff(sizeof(stMail)+mail->nContentLen);
	buff.addContent(mail,sizeof(stMail)) ;
	buff.addContent(pContent,mail->nContentLen) ;
	PostMailToDB((stMail*)buff.getBufferPtr(),nTargetUID);
	delete mail ;
	mail = nullptr ;
}

void CPlayerMailComponent::ReciveMail(stMail* pMail)
{
	PostMailToDB(pMail,GetPlayer()->GetUserUID());
	stRecievedMail sMail ;
	sMail.eType = (eMailType)pMail->eType ;
	sMail.nRecvTime = pMail->nPostTime ;
	CAutoBuffer auB(pMail->nContentLen + 1 );
	auB.addContent((char*)pMail + sizeof(stMail),pMail->nContentLen) ;
	sMail.strContent = auB.getBufferPtr() ;

	m_vAllMail.push_back(sMail);
	InformRecievedUnreadMails();
	if ( m_vAllMail.size() > MAX_KEEP_MAIL_CNT )
	{
		m_vAllMail.erase(m_vAllMail.begin()) ;
	}
}

void CPlayerMailComponent::ReciveMail(const char* pContent, uint16_t nContentLen )
{
	stMail* mail = new stMail ;
	mail->eType = eMail_PlainText ;
	mail->nPostTime = time(nullptr) ;
	mail->nContentLen = nContentLen;
	CAutoBuffer buff(sizeof(stMail)+mail->nContentLen);
	buff.addContent(mail,sizeof(stMail)) ;
	buff.addContent(pContent,mail->nContentLen) ;
	ReciveMail((stMail*)buff.getBufferPtr());
	delete mail ;
	mail = nullptr ;
}

void CPlayerMailComponent::ReciveMail(stRecievedMail& refMail)
{
	stMsgSaveMail msgSave ;
	msgSave.nUserUID = GetPlayer()->GetUserUID() ;
	msgSave.pMailToSave.eType = refMail.eType ;
	msgSave.pMailToSave.nContentLen = refMail.strContent.size() ;
	msgSave.pMailToSave.nPostTime = refMail.nRecvTime ;

	CAutoBuffer auBuffer(sizeof(msgSave) + msgSave.pMailToSave.nContentLen );
	auBuffer.addContent(&msgSave,sizeof(msgSave)) ;
	auBuffer.addContent(refMail.strContent.c_str(),msgSave.pMailToSave.nContentLen) ;
	CGameServerApp::SharedGameServerApp()->sendMsg(msgSave.nUserUID,auBuffer.getBufferPtr(),auBuffer.getContentSize()) ;

	m_vAllMail.push_back(refMail);
	InformRecievedUnreadMails();
	if ( m_vAllMail.size() > MAX_KEEP_MAIL_CNT )
	{
		m_vAllMail.erase(m_vAllMail.begin()) ;
	}
}

void CPlayerMailComponent::PostPublicMail(stRecievedMail& pMail)
{
	s_vPublicMails.push_back(pMail);
	if ( s_vPublicMails.size() > MAX_KEEP_MAIL_CNT )
	{
		s_vPublicMails.erase(s_vPublicMails.begin()) ;
	}

	stMsgSaveMail msgSave ;
	msgSave.nUserUID = 0 ;
	msgSave.pMailToSave.eType = eMail_Public ;
	msgSave.pMailToSave.nContentLen = pMail.strContent.size() ;
	msgSave.pMailToSave.nPostTime = pMail.nRecvTime ;

	CAutoBuffer auBuffer(sizeof(msgSave) + msgSave.pMailToSave.nContentLen );
	auBuffer.addContent(&msgSave,sizeof(msgSave)) ;
	auBuffer.addContent(pMail.strContent.c_str(),msgSave.pMailToSave.nContentLen) ;
	CGameServerApp::SharedGameServerApp()->sendMsg(msgSave.nUserUID,auBuffer.getBufferPtr(),auBuffer.getContentSize()) ;
	CLogMgr::SharedLogMgr()->PrintLog("save public mail") ;
}

uint16_t CPlayerMailComponent::getNewerMailListByTime( uint32_t nTimeTag, LIST_MAIL* vOutMail,uint16_t nMaxOutCnt )
{
	LIST_MAIL::reverse_iterator iter = s_vPublicMails.rbegin() ;
	uint16_t nGetCnt = 0 ;
	while ( iter != s_vPublicMails.rend() )
	{
		if ( nGetCnt >= nMaxOutCnt )
		{
			break; 
		}

		stRecievedMail& pRef = *iter ;
		if ( pRef.nRecvTime < nTimeTag )
		{
			break;
		}
		++nGetCnt;
		if ( vOutMail )
		{
			vOutMail->push_back(pRef) ;
		}
	}
	return nGetCnt ;
}

void CPlayerMailComponent::ProcessOfflineEvent()
{
	bool bNeedDel = false ;
	for ( stRecievedMail& refMail : m_vAllMail )
	{
		if ( ProcessMail(refMail) )
		{
			bNeedDel = true ;
		}
	}

	if ( bNeedDel )
	{
		bool bDelOffline = false ;
		bool bDelTimeTag = false ;
		LIST_MAIL::iterator iter = m_vAllMail.begin() ;
		for ( ; iter != m_vAllMail.end();  )
		{
			if ( (*iter).eType  == eMail_SysOfflineEvent || (*iter).eType  == eMail_ReadTimeTag )
			{
				if ( (*iter).eType  == eMail_SysOfflineEvent )
				{
					bDelOffline = true ;
				}

				if ( (*iter).eType  == eMail_ReadTimeTag )
				{
					bDelTimeTag = true ;
				}
				
				m_vAllMail.erase(iter) ;
				iter = m_vAllMail.begin() ;
				continue;
			}
			++iter ;
		}

		if ( bDelOffline )
		{
			// tell db set state 
			stMsgResetMailsState msgReset ;
			msgReset.nUserUID = GetPlayer()->GetUserUID() ;
			msgReset.tMailType = eMail_SysOfflineEvent ;
			SendMsg(&msgReset,sizeof(msgReset)) ;
		}

		if ( bDelTimeTag )
		{
			// tell db set state 
			stMsgResetMailsState msgReset ;
			msgReset.nUserUID = GetPlayer()->GetUserUID() ;
			msgReset.tMailType = eMail_ReadTimeTag ;
			SendMsg(&msgReset,sizeof(msgReset)) ;
		}

	}
}

bool CPlayerMailComponent::ProcessMail( stRecievedMail& pMail)
{
	if ( pMail.eType != eMail_SysOfflineEvent && eMail_ReadTimeTag != pMail.eType )
	{
		return false ;
	}

	if ( eMail_ReadTimeTag == pMail.eType )
	{
		m_tReadTimeTag = pMail.nRecvTime ;
	}
	CLogMgr::SharedLogMgr()->ErrorLog("process offline event here");
	return true ;
}
