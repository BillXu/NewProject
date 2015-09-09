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
			}

		}
		break;
	case MSG_PLAYER_REQUEST_MAIL_LIST:
		{
			SendMailListToClient();
			return true ;
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
	if ( m_vAllMail.size() )
	{
		stMsgInformNewMail msg ;
		msg.nUnreadMailCount = m_vAllMail.size() ;
		msg.eNewMailType = m_vAllMail.back().eType ;
		SendMsg(&msg,sizeof(msg)) ;
	}
}

void CPlayerMailComponent::ClearMails()
{
	m_vAllMail.clear();
}

void CPlayerMailComponent::SendMailListToClient()
{
	if ( m_vAllMail.empty() )
	{
		return ;
	}

	stMsgRequestMailListRet msgRet;
	uint8_t nSize = m_vAllMail.size() ;
	CAutoBuffer auBuff(sizeof(msgRet) + 100 );
	for ( stRecievedMail& pMail : m_vAllMail )
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
	CLogMgr::SharedLogMgr()->PrintLog("send mail to client uid = %d ,size = %d",GetPlayer()->GetUserUID(),m_vAllMail.size() ) ;

	// tell db set state 
	stMsgResetMailsState msgReset ;
	msgReset.nUserUID = GetPlayer()->GetUserUID() ;
	msgReset.tMailType = eMail_Max ;
	SendMsg(&msgReset,sizeof(msgReset)) ;

	m_vAllMail.clear() ;
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
		LIST_MAIL::iterator iter = m_vAllMail.begin() ;
		for ( ; iter != m_vAllMail.end();  )
		{
			if ( (*iter).eType  == eMail_SysOfflineEvent )
			{
				m_vAllMail.erase(iter) ;
				iter = m_vAllMail.begin() ;
				continue;
			}
			++iter ;
		}
		// tell db set state 
		stMsgResetMailsState msgReset ;
		msgReset.nUserUID = GetPlayer()->GetUserUID() ;
		msgReset.tMailType = eMail_SysOfflineEvent ;
		SendMsg(&msgReset,sizeof(msgReset)) ;
	}
}

bool CPlayerMailComponent::ProcessMail( stRecievedMail& pMail)
{
	if ( pMail.eType != eMail_SysOfflineEvent )
	{
		return false ;
	}

	CLogMgr::SharedLogMgr()->ErrorLog("process offline event here");
	return true ;
}
