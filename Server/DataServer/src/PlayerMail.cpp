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
uint64_t CPlayerMailComponent::s_nCurMaxMailUID = 0 ;
bool CPlayerMailComponent::OnMessage( stMsg* pMsg , eMsgPort eSenderPort )
{
	if ( IPlayerComponent::OnMessage(pMsg,eSenderPort) )
	{
		return true ;
	}

	switch ( pMsg->usMsgType )
	{
	case MSG_PLAYER_GET_MAIL_LIST:
		{
			if ( pMsg->cSysIdentifer == ID_MSG_C2GAME  )
			{
				stMsgRequestMailList* pRet = (stMsgRequestMailList*)pMsg ;
				SendMailListToClient(pRet->nBeginMailUID);
			}
			else if ( ID_MSG_DB2GM == pMsg->cSysIdentifer )
			{
				stMsgGameServerGetMailListRet* pMsgRet = (stMsgGameServerGetMailListRet*)pMsg ;
				char* pBuffer = (char*)pMsg ;
				pBuffer += sizeof(stMsgGameServerGetMailListRet);
				while ( pMsgRet->nMailCount--)
				{
					stMail* pMail = new stMail ;
					memcpy(pMail,pBuffer,sizeof(stMail));
					pBuffer += sizeof(stMail);

					if ( pMail->nContentLen > 0 )
					{
						pMail->pContent = new char[pMail->nContentLen] ;
						memcpy(pMail->pContent,pBuffer,pMail->nContentLen);
					}
					pBuffer += pMail->nContentLen ;
					m_vAllMail.push_back(pMail) ;
				}

				// if have unread mail should tell client ;
				ProcessOfflineEvent();
				stPlayerEvetArg arg ;
				arg.eEventType = ePlayerEvent_ReadDBOK;
				GetPlayer()->PostPlayerEvent(&arg);
				InformRecievedUnreadMails();
			}
			else
			{
				CLogMgr::SharedLogMgr()->ErrorLog("from unknown server msg : MSG_PLAYER_GET_MAIL_LIST") ;
			}
		}
		break;
	case MSG_PLAYER_PROCESSED_MAIL:
		{
			stMsgPlayerProcessedMailRet msgBack ;
			stMsgPlayerProcessedMail* pMsgRet = (stMsgPlayerProcessedMail*)pMsg ;
			msgBack.eProcessAct = pMsgRet->eProcessAct ;
			msgBack.nMailUIDProcessed = pMsgRet->nMailUIDProcessed ;
			stMail* pMail = GetMailByMailID(pMsgRet->nMailUIDProcessed) ;
			if ( pMail == NULL )
			{
				msgBack.nRet = 1 ;
			}
			else if ( 0 != pMail->eProcessAct )
			{
				msgBack.nRet = 2 ;
			}
			else
			{
				ProcessMail(pMail,(eProcessMailAct)pMsgRet->eProcessAct) ;
			}
			SendMsg(&msgBack,sizeof(msgBack)) ;
		}
		break;
	default:
		return false ;
	}
	return true ;
}

void CPlayerMailComponent::OnPlayerDisconnect()
{
	// save mails to db ;
	TimerSave();
}

void CPlayerMailComponent::Reset()
{
	//ClearMails();
	//stMsgGameServerGetMailList msg ;
	//msg.nSessionID = GetPlayer()->GetSessionID() ;
	//msg.nUserUID = GetPlayer()->GetUserUID() ;
	//SendMsgToDB((char*)&msg,sizeof(msg)) ;
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
	//if ( GetUnprocessedMailCount() > 0 )
	//{
	//	stMsgReceivedNewMail msg ;
	//	msg.nUnreadMailCount = GetUnprocessedMailCount() ;
	//	SendMsgToClient((char*)&msg,sizeof(msg)) ;
	//}
}

void CPlayerMailComponent::ClearMails()
{
	LIST_MAIL::iterator iter = m_vAllMail.begin() ;
	for ( ; iter != m_vAllMail.end(); ++iter )
	{
		if ( (*iter)->pContent )
		{
			delete[] (*iter)->pContent ;
			(*iter)->pContent = NULL ;
		}

		delete *iter ;
		*iter = NULL ;
	}
	m_vAllMail.clear() ;

	for ( int i = 0 ; i < eDBAct_Max ; ++i )
	{
		LIST_MAIL& pMailList = m_vNeedUpdateMails[i] ;
		if ( i != eDBAct_Delete )
		{
			pMailList.clear() ;
			continue;
		}

		LIST_MAIL::iterator iter = pMailList.begin() ;
		for ( ; iter != pMailList.end(); ++iter )
		{
			stMail*p = *iter ;
			if ( p->pContent )
			{
				delete [] p->pContent ;
			}
			delete p ;
		}
		pMailList.clear();
	}
}

unsigned short CPlayerMailComponent::GetUnprocessedMailCount()
{
	unsigned short nCount = 0 ;
	LIST_MAIL::iterator iter = m_vAllMail.begin() ;
	for ( ; iter != m_vAllMail.end(); ++iter )
	{
		if ( (*iter)->eProcessAct == 0 )
		{
			++nCount ;
		}
	}
	return nCount ;
}

void CPlayerMailComponent::SendMailListToClient(uint64_t nBegMainUID)
{
	//LIST_MAIL vSendMail ;
	//LIST_MAIL::iterator iter = m_vAllMail.begin() ;
	//unsigned short nTotalLen = sizeof(stMsgRequestMailListRet) ;
	//for ( ; iter != m_vAllMail.end(); ++iter )
	//{
	//	if ( (*iter)->nMailUID >= nBegMainUID )
	//	{
	//		vSendMail.push_back(*iter) ;
	//		nTotalLen += sizeof(stMail);
	//		nTotalLen += (*iter)->nContentLen ;
	//	}
	//}

	//stMsgRequestMailListRet msg ;
	//msg.nMailCount = vSendMail.size() ;
	//unsigned short nOffset = 0 ;
	//char* pBuffer = new char[nTotalLen] ;
	//memcpy(pBuffer,&msg,sizeof(msg));
	//nOffset += sizeof(msg);
	//iter = vSendMail.begin() ;
	//for ( ; iter != vSendMail.end(); ++iter )
	//{
	//	stMail* pMail = *iter ;
	//	memcpy(pBuffer + nOffset , pMail,sizeof(stMail));
	//	nOffset += sizeof(stMail);

	//	memcpy(pBuffer + nOffset , pMail->pContent, pMail->nContentLen );
	//	nOffset += pMail->nContentLen ;
	//}
	//SendMsgToClient(pBuffer,nOffset) ;
	//delete[] pBuffer ;
}

void CPlayerMailComponent::SaveMailToDB(stMail* pMail , eDBAct eOpeateType,unsigned int nOwnerUID )
{
	//// 0 update , 1 delete , 2 insterd ;
	//if ( nOwnerUID <= 0 )
	//{
	//	nOwnerUID = GetPlayer()->GetUserUID() ;
	//}
	//stMsgGameServerSaveMail msg ;
	//msg.nSessionID = GetPlayer()->GetSessionID() ;
	//msg.nUserUID = nOwnerUID ;
	//msg.nOperateType = eOpeateType ;
	//if ( eOpeateType == eDBAct_Add )
	//{
	//	char* pBuffer = new char[sizeof(msg) + sizeof(stMail)+ pMail->nContentLen] ;
	//	int nOffset = 0 ;
	//	
	//	memcpy(pBuffer,&msg,sizeof(msg));
	//	nOffset += sizeof(msg);

	//	memcpy(pBuffer + nOffset,pMail,sizeof(stMail));
	//	nOffset += sizeof(stMail);

	//	memcpy(pBuffer + nOffset , pMail->pContent, pMail->nContentLen);
	//	nOffset += pMail->nContentLen ;
	//	SendMsgToDB(pBuffer,nOffset) ;
	//	delete[] pBuffer;
	//}
	//else
	//{
	//	char* pBuffer = new char[sizeof(msg) + sizeof(stMail)] ;
	//	int nOffset = 0 ;

	//	memcpy(pBuffer,&msg,sizeof(msg));
	//	nOffset += sizeof(msg);

	//	memcpy(pBuffer + nOffset,pMail,sizeof(stMail));
	//	nOffset += sizeof(stMail);
	//	SendMsgToDB(pBuffer,nOffset) ;
	//	delete[] pBuffer;
	//}
}

void CPlayerMailComponent::PostGiftMail( CPlayer* pPlayerPresenter,unsigned int nTargetUID, unsigned short nItemID ,unsigned short nShopItemID, unsigned int nItemCount , uint64_t nCoin , unsigned int nDiamond )
{
	stMail* pMail = new stMail ;
	pMail->eProcessAct = 0 ;
	pMail->eType = eMail_PresentGift ;
	pMail->nContentLen = sizeof(stMailGiftContent);
	pMail->nMailUID = ++s_nCurMaxMailUID ;
	pMail->nPostTime = (unsigned int)time(NULL) ;
	pMail->pContent = new char[pMail->nContentLen] ;
	stMailGiftContent* pContent = (stMailGiftContent*)pMail->pContent ;
	pPlayerPresenter->GetBaseData()->GetPlayerBrifData(&pContent->stPresenter);
	pContent->nCount = nItemCount ;
	pContent->nItemID = nItemID ;
	pContent->nPresentCoin = nCoin ;
	pContent->nPrsentDiamond = nDiamond ;
	pContent->nShopItemID = nShopItemID ;

	PostMail(pMail,nTargetUID) ;
}

void CPlayerMailComponent::PostUnprocessedPurchaseVerify( unsigned int nMailTargetPlayerUID ,unsigned int nTaregetBuyForPlayerUID , unsigned short nShopItemID, bool bVerifyOK )
{
	stMail* pMail = new stMail ;
	pMail->eProcessAct = 0 ;
	pMail->eType = eMail_UnProcessedPurchaseVerify ;
	pMail->nContentLen = sizeof(stMailUnprocessedPurchaseVerifyContent);
	pMail->nMailUID = ++s_nCurMaxMailUID ;
	pMail->nPostTime = (unsigned int)time(NULL) ;
	pMail->pContent = new char[pMail->nContentLen] ;

	stMailUnprocessedPurchaseVerifyContent* pContent = (stMailUnprocessedPurchaseVerifyContent*)pMail->pContent ;
	pContent->nShopItemID = nShopItemID ;
	pContent->nTaregetForPlayerUID = nTaregetBuyForPlayerUID ;
	pContent->bVerifyOK = bVerifyOK ;
	PostMail(pMail,nMailTargetPlayerUID) ;
}

void CPlayerMailComponent::PostBeAddedFriendMail( CPlayer* WhoAddMeTobeFriend, unsigned int nMailToUserUID )
{
	stMail* pMail = new stMail ;
	pMail->eProcessAct = 0 ;
	pMail->eType = eMail_BeAddedFriend ;
	pMail->nContentLen = sizeof(stMailBeAddedFriend);
	pMail->nMailUID = ++s_nCurMaxMailUID ;
	pMail->nPostTime = (unsigned int)time(NULL) ;
	pMail->pContent = new char[pMail->nContentLen] ;

	stMailBeAddedFriend* pContent = (stMailBeAddedFriend*)pMail->pContent ;
	WhoAddMeTobeFriend->GetBaseData()->GetPlayerBrifData(&pContent->peerWhoWantAddMe) ;
	PostMail(pMail,nMailToUserUID) ;
}

void CPlayerMailComponent::PostMail(stMail* pMail ,unsigned int nTargetUID  )
{
	if ( nTargetUID <= 0 )
	{
		nTargetUID = GetPlayer()->GetUserUID();
	}

	if ( nTargetUID == GetPlayer()->GetUserUID() )
	{
		m_vAllMail.push_back(pMail) ;
		m_vNeedUpdateMails[eDBAct_Add].push_back(pMail) ;
		InformRecievedUnreadMails();
	}
	else
	{
		SaveMailToDB(pMail,eDBAct_Add,nTargetUID) ;
		if ( pMail->pContent )
		{
			delete[] pMail->pContent ;
			pMail->pContent = NULL ;
		}
		delete pMail ;
		pMail = NULL ;
	}
}

void CPlayerMailComponent::TimerSave()
{
	for ( int i = 0 ; i < eDBAct_Max ; ++i )
	{
		LIST_MAIL& pMailList = m_vNeedUpdateMails[i] ;
		LIST_MAIL::iterator iter = pMailList.begin() ;
		for ( ; iter != pMailList.end(); ++iter )
		{
			SaveMailToDB(*iter,(eDBAct)i) ;

			if ( i == eDBAct_Delete )
			{
				stMail*p = *iter ;
				if ( p->pContent )
				{
					delete [] p->pContent ;
				}
				delete p ;
			}
		}
		pMailList.clear();
	}
}

void CPlayerMailComponent::ProcessOfflineEvent()
{
	//LIST_MAIL::iterator iter = m_vAllMail.begin();
	//for ( ; iter != m_vAllMail.end(); )
	//{
	//	stMail* pItem = *iter ;
	//	if ( eMail_UnProcessedPurchaseVerify == pItem->eType )
	//	{
	//		stMsgFromVerifyServer pR ;
	//		stMailUnprocessedPurchaseVerifyContent* pContent = (stMailUnprocessedPurchaseVerifyContent*)pItem->pContent ;
	//		if ( pContent->bVerifyOK == false  )
	//		{
	//			pR.nRet = 0 ;
	//		}
	//		else
	//		{
	//			pR.nRet = 4 ;
	//		}
	//		pItem->eProcessAct = ePro_Mail_DoYes ;
	//		pR.nBuyerPlayerUserUID = GetPlayer()->GetUserUID() ;
	//		pR.nBuyForPlayerUserUID = pContent->nTaregetForPlayerUID ;
	//		pR.nShopItemID = pContent->nShopItemID ;
	//		GetPlayer()->OnMessage(&pR) ;
	//		// push to need to update 
	//		PushNeedToUpdate(pItem,eDBAct_Delete) ;
	//		m_vAllMail.erase(iter) ;
	//		iter = m_vAllMail.begin() ;   // invalid iterator ;
	//		continue; 
	//	}
	//	++iter ;
	//}
}

void CPlayerMailComponent::PushNeedToUpdate(stMail* pMail , eDBAct eAct )
{
	if ( pMail == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("need update mail is null") ;
		return ;
	}
	m_vNeedUpdateMails[eAct].push_back(pMail) ;
}
bool CPlayerMailComponent::ProcessMail(stMail* pMail,eProcessMailAct eAct )
{
	if ( ePro_Mail_Delete == eAct )
	{
		LIST_MAIL::iterator iter = m_vAllMail.begin() ;
		for ( ; iter != m_vAllMail.end() ; ++iter )
		{
			if ( *iter == pMail )
			{
				m_vAllMail.erase(iter) ;
				break; 
			}
		}
		PushNeedToUpdate(pMail,eDBAct_Delete) ;
		return true ;
	}
	CLogMgr::SharedLogMgr()->ErrorLog("not process this mail , write code ,quick ") ;
	return true ;
}

stMail* CPlayerMailComponent::GetMailByMailID(uint64_t nMailID)
{
	LIST_MAIL::iterator iter = m_vAllMail.begin() ;
	for ( ; iter != m_vAllMail.end(); ++iter )
	{
		if ( (*iter)->nMailUID == nMailID )
		{
			return *iter ;
		}
	}
	return NULL ;
}