#include "PlayerBaseData.h"
#include <string>
#include "MessageDefine.h"
#include "ServerMessageDefine.h"
#include "Player.h"
#include "LogManager.h"
#include <time.h>
#include "GameServerApp.h"
#include "ContinueLoginConfig.h"
#include "PlayerItem.h"
#include "PlayerEvent.h"
#include "PlayerManager.h"
#include "EventCenter.h"
#include "InformConfig.h"
#pragma warning( disable : 4996 )
#define ONLINE_BOX_RESET_TIME 60*60*3   // offline 3 hour , will reset the online box ;
CPlayerBaseData::CPlayerBaseData(CPlayer* player )
	:IPlayerComponent(player)
{
	m_eType = ePlayerComponent_BaseData ;
	m_nTakeInDiamoned = 0 ;
	m_nTakeInCoin = 0 ;
	memset(&m_stBaseData,0,sizeof(m_stBaseData)) ;
	m_bGivedLoginReward = false ;
}

CPlayerBaseData::~CPlayerBaseData()
{

}

void CPlayerBaseData::Init()
{
	m_nTakeInDiamoned = 0 ;
	m_nTakeInCoin = 0 ;
	memset(&m_stBaseData,0,sizeof(m_stBaseData)) ;
	m_stBaseData.nUserUID = GetPlayer()->GetUserUID() ;
	m_bGivedLoginReward = false ;

	m_bMoneyDataDirty = false;
	m_bTaxasDataDirty = false;
	m_bCommonLogicDataDirty = false;
	m_bPlayerInfoDataDirty = false;
	Reset();
}

void CPlayerBaseData::Reset()
{
	m_bGivedLoginReward = false ;
	m_nTakeInDiamoned = 0 ;
	m_nTakeInCoin = 0 ;

	m_bMoneyDataDirty = false;
	m_bTaxasDataDirty = false;
	m_bCommonLogicDataDirty = false;
	m_bPlayerInfoDataDirty = false;

	memset(&m_stBaseData,0,sizeof(m_stBaseData)) ;
	m_stBaseData.nUserUID = GetPlayer()->GetUserUID() ;

	stMsgDataServerGetBaseData msg ;
	msg.nUserUID = GetPlayer()->GetUserUID() ;
	SendMsg(&msg,sizeof(msg)) ;
	CLogMgr::SharedLogMgr()->PrintLog("requesting userdata for uid = %d",msg.nUserUID);
	// register new day event ;
	CEventCenter::SharedEventCenter()->RegisterEventListenner(eEvent_NewDay,this,CPlayerBaseData::EventFunc ) ;
}

bool CPlayerBaseData::OnMessage( stMsg* pMsg , eMsgPort eSenderPort )
{
	switch( pMsg->usMsgType )
	{
	case MSG_PLAYER_BASE_DATA:   // from db server ;
		{
			stMsgDataServerGetBaseDataRet* pBaseData = (stMsgDataServerGetBaseDataRet*)pMsg ;
			if ( pBaseData->nRet )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("do not exsit playerData") ;
				return true; 
			}
			memcpy(&m_stBaseData,&pBaseData->stBaseData,sizeof(m_stBaseData));
			CLogMgr::SharedLogMgr()->PrintLog("recived base data uid = %d",pBaseData->stBaseData.nUserUID);
			SendBaseDatToClient();
			return true ;
		}
		break;
	case MSG_PLAYER_REQUEST_NOTICE:
		{
// 			CGameServerApp::SharedGameServerApp()->GetBrocaster()->SendInformsToPlayer(GetPlayer()) ;
// 			CInformConfig* pConfig = (CInformConfig*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Informs) ;
// 			m_stBaseData.nNoticeID = pConfig->GetMaxInformID();
		}
		break;
	case MSG_PLAYER_MODIFY_SIGURE:
		{
			stMsgPLayerModifySigure* pMsgRet = (stMsgPLayerModifySigure*)pMsg ;
			memcpy(m_stBaseData.cSignature,pMsgRet->pNewSign,sizeof(m_stBaseData.cSignature));
			stMsgPlayerModifySigureRet ret ;
			ret.nRet = 0 ;
			SendMsg(&ret,sizeof(ret)) ;
			m_bPlayerInfoDataDirty = true ;
		}
		break;
	case MSG_PLAYER_MODIFY_NAME:
		{
			stMsgPLayerModifyName* pMsgRet = (stMsgPLayerModifyName*)pMsg ;
			memcpy(m_stBaseData.cName,pMsgRet->pNewName,sizeof(m_stBaseData.cName)) ;
			stMsgPlayerModifyNameRet ret ;
			ret.nRet = 0 ;
			SendMsg(&ret,sizeof(ret)) ;
			m_bPlayerInfoDataDirty = true ;
		}
		break;
	case MSG_PLAYER_MODIFY_PHOTO:
		{
			stMsgPlayerModifyPhoto* pPhoto = (stMsgPlayerModifyPhoto*)pMsg ;
			m_stBaseData.nPhotoID = pPhoto->nPhotoID ;
			stMsgPlayerModifyPhotoRet msgRet ;
			msgRet.nRet = 0 ;
			SendMsg(&msgRet,sizeof(msgRet)) ;
			m_bPlayerInfoDataDirty = true ;
		}
		break;
	case MSG_PUSH_APNS_TOKEN:
		{
			//stMsgPushAPNSToken* pMsgRet = (stMsgPushAPNSToken*)pMsg ;
			//if ( 0 == pMsgRet->nGetTokenRet )
			//{
			//	bPlayerEnableAPNs = true ;
			//	memcpy(vAPNSToken,pMsgRet->vAPNsToken,32);
			//}
			//else
			//{
			//	bPlayerEnableAPNs = false ;
			//	memset(vAPNSToken,0,32 ) ;
			//}
			//stMsgPushAPNSTokenRet msg ;
			//msg.nGetTokenRet = pMsgRet->nGetTokenRet ;
			//SendMsgToClient((char*)&msg,sizeof(msg)) ;
		}
		break;
	case MSG_PLAYER_UPDATE_MONEY:
		{
// 			stMsgPlayerUpdateMoney msgUpdate ;
// 			msgUpdate.nFinalCoin = GetAllCoin();
// 			msgUpdate.nFinalDiamoned = GetAllDiamoned();
// 			msgUpdate.nTakeInCoin = GetTakeInMoney();
// 			msgUpdate.nTakeInDiamoned = (unsigned int)GetTakeInMoney(true) ;
// 			SendMsgToClient((char*)&msgUpdate,sizeof(msgUpdate));
		}
		break;
	case MSG_GET_CONTINUE_LOGIN_REWARD:
		{
// 			stMsgGetContinueLoginReward* pGetR = (stMsgGetContinueLoginReward*)pMsg ;
// 			stMsgGetContinueLoginRewardRet msgBack ;
// 			msgBack.nRet = 0 ; //  // 0 success , 1 already getted , 2 you are not vip  ;
// 			msgBack.cRewardType = pGetR->cRewardType;
// 			msgBack.nDayIdx = m_stBaseData.nContinueDays ;
// 			msgBack.nDiamoned = GetAllDiamoned();
// 			msgBack.nFinalCoin = GetAllCoin() ;
// 			if ( m_bGivedLoginReward )
// 			{
// 				msgBack.nRet = 1 ; 
// 				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 				break;
// 			}
// 
// 			stConLoginConfig* pConfig = CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetContinueLoginConfig()->GetConfigByDayIdx(m_stBaseData.nContinueDays) ;
// 			if ( pConfig == NULL )
// 			{
// 				CLogMgr::SharedLogMgr()->ErrorLog("there is no login config for dayIdx = %d",m_stBaseData.nContinueDays ) ;
// 				msgBack.nRet = 4 ;
// 				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 				break;
// 			}
// 
// 			// give item  deponed on nContinuedDays ;
// 			if (pGetR->cRewardType == 1 )
// 			{
// 				if ( GetVipLevel() < 1 )
// 				{
// 					msgBack.nRet = 2 ;
// 					SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 					break;
// 				}
// 				// give item ;
// 				for ( int i = 0 ; i < pConfig->vItems.size(); ++i )
// 				{
// 					CPlayerItemComponent* pc = (CPlayerItemComponent*)GetPlayer()->GetComponent(ePlayerComponent_PlayerItemMgr) ;
// 					pc->AddItemByID(pConfig->vItems[i].nItemID,pConfig->vItems[i].nCount) ;
// 				}
// 				// give vip prize ;
// 				m_stBaseData.nDiamoned += pConfig->nDiamoned ;
// 				msgBack.nDiamoned = GetAllDiamoned();
// 				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 				m_bGivedLoginReward = true ;
// 				break;
// 			}
// 			else if( pGetR->cRewardType == 0 )
// 			{
// 				// gvie common prize ;
// 				m_stBaseData.nCoin += pConfig->nGiveCoin ;
// 				msgBack.nFinalCoin = GetAllCoin() ;
// 				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 				m_bGivedLoginReward = true ;
// 				break;
// 			}
// 			else
// 			{
// 				msgBack.nRet = 3 ;
// 				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 				break;
// 			}
		}
		break;
	case MSG_PLAYER_REQUEST_CHARITY_STATE:
		{
// 			stMsgPlayerRequestCharityStateRet msgBack ;
// 			 // 0 can get charity , 1 you coin is enough , do not need charity, 2 time not reached ;
// 			msgBack.nState = 0 ;
// 			msgBack.nLeftSecond = 0 ;
// 			if ( GetAllCoin() > COIN_CONDITION_TO_GET_CHARITY )
// 			{
// 				msgBack.nState = 1 ;
// 			}
// 			else if ( time(NULL) - m_stBaseData.tLastTakeCharityCoinTime < TIME_GET_CHARITY_ELAPS )
// 			{
// 				msgBack.nState = 2 ;
// 				msgBack.nLeftSecond = m_stBaseData.tLastTakeCharityCoinTime + TIME_GET_CHARITY_ELAPS - time(NULL) ;
// 			}
// 			SendMsg(&msgBack,sizeof(msgBack)) ;
		}
		break;
	case MSG_PLAYER_GET_CHARITY:
		{
// 			stMsgPlayerGetCharityRet msgBack ;
// 			// 0 success ,  1 you coin is enough , do not need charity, 2 time not reached ;
// 			msgBack.nRet = 0 ;
// 			msgBack.nFinalCoin = GetAllCoin();
// 			msgBack.nGetCoin = 0;
// 			msgBack.nLeftSecond = 0 ;
// 			if ( GetAllCoin() > COIN_CONDITION_TO_GET_CHARITY )
// 			{
// 				msgBack.nRet = 1 ;
// 			}
// 			else if ( time(NULL) - m_stBaseData.tLastTakeCharityCoinTime < TIME_GET_CHARITY_ELAPS )
// 			{
// 				msgBack.nRet = 2 ;
// 				msgBack.nLeftSecond = m_stBaseData.tLastTakeCharityCoinTime + TIME_GET_CHARITY_ELAPS - time(NULL) ;
// 			}
// 			else
// 			{
// 				m_stBaseData.nCoin += COIN_FOR_CHARITY ;
// 				msgBack.nFinalCoin = GetAllCoin();
// 				msgBack.nGetCoin = COIN_FOR_CHARITY;
// 				msgBack.nLeftSecond = 0 ;
// 				m_stBaseData.tLastLoginTime = time(NULL) ;
// 			}
// 			SendMsg(&msgBack,sizeof(msgBack)) ;
		}
		break;
	default:
		{
			return false ;
		}
		break;
	}
	return true ;
}

void CPlayerBaseData::SendBaseDatToClient()
{
	stMsgPlayerBaseData msg ;
	memcpy(&msg.stBaseData,&m_stBaseData,sizeof(msg.stBaseData));
	SendMsg(&msg,sizeof(msg)) ;
	CLogMgr::SharedLogMgr()->PrintLog("send base data to session id = %d ",GetPlayer()->GetSessionID() );
}

void CPlayerBaseData::OnProcessContinueLogin()
{
// 	if ( m_stBaseData.tLastLoginTime == 0 )
// 	{
// 		m_stBaseData.nContinueDays = 1 ;
// 		m_stBaseData.tLastLoginTime = (unsigned int)time(NULL) ; 
// 	}
// 	else
// 	{
// 		time_t nCur = time(NULL) ;
// 		struct tm* pTempTimer = NULL;
// 		pTempTimer = localtime(&nCur) ;
// 		struct tm pTimeCur ;
// 		if ( pTempTimer )
// 		{
// 			pTimeCur = *pTempTimer ;
// 		}
// 		else
// 		{
// 			CLogMgr::SharedLogMgr()->ErrorLog("local time return null ?") ;
// 		}
//  
// 		pTempTimer = localtime((time_t*)&m_stBaseData.tLastLoginTime) ;
// 		struct tm pTimeLastLogin  ;
// 		if ( pTempTimer )
// 		{
// 			pTimeLastLogin = *pTempTimer ;
// 		}
// 		else
// 		{
// 			CLogMgr::SharedLogMgr()->ErrorLog("local time return null ?") ;
// 		}
// 		
// 		if ( pTimeCur.tm_year == pTimeLastLogin.tm_year && pTimeCur.tm_mon == pTimeLastLogin.tm_mon && pTimeCur.tm_yday == pTimeLastLogin.tm_yday )
// 		{
// 			m_stBaseData.tLastLoginTime = (unsigned int)nCur ;
// 			m_bGivedLoginReward = true ;
// 			return ; // do nothing ; same day ;
// 		}
// 
// 		double nDiffe = difftime(nCur,m_stBaseData.tLastLoginTime) ;
// 		bool bContine = abs(nDiffe) - 60 * 60 * 24 <= 0 ;
// 		
// 		if ( bContine )
// 		{
// 			++m_stBaseData.nContinueDays ;   // real contiune ;
// 		}
// 		else
// 		{
// 			m_stBaseData.nContinueDays = 1 ;    // disturbed ;
// 		}
// 
// 		m_stBaseData.tLastLoginTime = (unsigned int)nCur ;
// 	}
// 
// 	stMsgShowContinueLoginDlg msg ;
// 	msg.nContinueIdx = m_stBaseData.nContinueDays ;
// 	SendMsgToClient((char*)&msg,sizeof(msg)) ;
// 	// first login event ;
// 	stPlayerEvetArg evet ;
// 	evet.eEventType = ePlayerEvent_FirstLogin ;
// 	GetPlayer()->PostPlayerEvent(&evet);
}

void CPlayerBaseData::OnPlayerDisconnect()
{
	m_bCommonLogicDataDirty = true ;
	TimerSave();
	CEventCenter::SharedEventCenter()->RemoveEventListenner(eEvent_NewDay,this,CPlayerBaseData::EventFunc ) ;
}

void CPlayerBaseData::TimerSave()
{
	if ( m_bMoneyDataDirty )
	{
		m_bMoneyDataDirty = false ;
		stMsgSavePlayerMoney msgSaveMoney ;
		msgSaveMoney.nCoin = m_stBaseData.nCoin ;
		msgSaveMoney.nDiamoned = m_stBaseData.nDiamoned ;
		msgSaveMoney.nUserUID = GetPlayer()->GetUserUID() ;
		SendMsg((stMsgSavePlayerMoney*)&msgSaveMoney,sizeof(msgSaveMoney)) ;
	}
	
	if ( m_bTaxasDataDirty )
	{
		m_bTaxasDataDirty = false ;
		stMsgSavePlayerTaxaPokerData msgSavePokerData ;
		msgSavePokerData.nLoseTimes = m_stBaseData.nLoseTimes ;
		msgSavePokerData.nSingleWinMost = m_stBaseData.nSingleWinMost ;
		msgSavePokerData.nWinTimes = m_stBaseData.nWinTimes ;
		memcpy(msgSavePokerData.vMaxCards,m_stBaseData.vMaxCards,sizeof(msgSavePokerData.vMaxCards));
		msgSavePokerData.nUserUID = GetPlayer()->GetUserUID() ;
		SendMsg((stMsgSavePlayerMoney*)&msgSavePokerData,sizeof(msgSavePokerData)) ;
	}

	if ( m_bCommonLogicDataDirty )
	{
		m_bCommonLogicDataDirty = false ;
		stMsgSavePlayerCommonLoginData msgLogicData ;
		msgLogicData.dfLatidue = m_stBaseData.dfLatidue ;
		msgLogicData.dfLongitude = m_stBaseData.dfLongitude ;
		msgLogicData.nContinueDays = m_stBaseData.nContinueDays ;
		//msgLogicData.nExp = m_stBaseData.nExp ;
		msgLogicData.nMostCoinEver = m_stBaseData.nMostCoinEver;
		msgLogicData.nTodayCoinOffset = m_stBaseData.nTodayCoinOffset ;
		msgLogicData.nYesterdayCoinOffset = m_stBaseData.nYesterdayCoinOffset ;
		msgLogicData.nUserUID = GetPlayer()->GetUserUID() ;
		msgLogicData.nVipLevel = m_stBaseData.nVipLevel ;
		msgLogicData.tLastLoginTime = m_stBaseData.tLastLoginTime ;
		msgLogicData.tLastTakeCharityCoinTime = m_stBaseData.tLastTakeCharityCoinTime ;
		msgLogicData.tOfflineTime = m_stBaseData.tOfflineTime ;
		memcpy(msgLogicData.vJoinedClubID,m_stBaseData.vJoinedClubID,sizeof(msgLogicData.vJoinedClubID));
		SendMsg((stMsgSavePlayerMoney*)&msgLogicData,sizeof(msgLogicData)) ;
	}

	if ( m_bPlayerInfoDataDirty )
	{
		m_bPlayerInfoDataDirty = false ;
		stMsgSavePlayerInfo msgSaveInfo ;
		msgSaveInfo.nPhotoID = m_stBaseData.nPhotoID ;
		msgSaveInfo.nUserUID = GetPlayer()->GetUserUID() ;
		memcpy(msgSaveInfo.vName,m_stBaseData.cName,sizeof(msgSaveInfo.vName));
		memcpy(msgSaveInfo.vSigure,m_stBaseData.cSignature,sizeof(msgSaveInfo.vSigure));
		memcpy(msgSaveInfo.vUploadedPic,m_stBaseData.vUploadedPic,sizeof(msgSaveInfo.vUploadedPic));
		SendMsg((stMsgSavePlayerMoney*)&msgSaveInfo,sizeof(msgSaveInfo)) ;
	}
}

bool CPlayerBaseData::SetTakeInCoin(uint64_t nCoinOffset, bool bDiamoned)
{
 	//CaculateTakeInMoney();
 	if ( bDiamoned == false )
 	{
 		if ( nCoinOffset > GetAllCoin() )
		{
			return false ;
		}
 		m_nTakeInCoin = min(nCoinOffset,GetAllCoin()); 
 		m_stBaseData.nCoin -= m_nTakeInCoin ;
 	}
 	else
 	{
		if ( nCoinOffset > GetAllDiamoned() )
		{
			return false ;
		}
 		m_nTakeInDiamoned = (unsigned int)min(nCoinOffset,GetAllDiamoned()); ;
 		m_stBaseData.nDiamoned -= (unsigned int)m_nTakeInDiamoned ;
 	}

	m_bMoneyDataDirty = true ;
	return true ;
}

bool CPlayerBaseData::ModifyMoney(int64_t nOffset,bool bDiamond  )
{
	if ( bDiamond )
	{
		if ( nOffset < 0 && (-1*nOffset) > m_stBaseData.nDiamoned )
		{
			return false ;
		}

		m_stBaseData.nDiamoned += (int)nOffset ;
	}
	else
	{
		if ( nOffset < 0 && (-1*nOffset) > m_stBaseData.nCoin )
		{
			return false ;
		}
		m_stBaseData.nCoin += nOffset ;
	}
	m_bMoneyDataDirty = true ;
	return true ;
}

//bool CPlayerBaseData::ModifyTakeInMoney(int64_t nOffset,bool bDiamond )
//{
// 	if ( bDiamond )
// 	{
// 		if ( nOffset < 0 && (-1*nOffset) > m_nTakeInDiamoned )
// 		{
// 			return false ;
// 		}
// 
// 		m_nTakeInDiamoned += (int)nOffset ;
// 	}
// 	else
// 	{
// 		if ( nOffset < 0 && (-1*nOffset) > m_nTakeInCoin )
// 		{
// 			return false ;
// 		}
// 		m_nTakeInCoin += nOffset ;
// 	}
//	return true ;
//}

//void CPlayerBaseData::CaculateTakeInMoney()
//{
//	m_stBaseData.nDiamoned += m_nTakeInDiamoned;
//	m_stBaseData.nCoin += m_nTakeInCoin ;
//	m_nTakeInCoin = 0;
//	m_nTakeInDiamoned = 0 ;
//}

bool CPlayerBaseData::OnPlayerEvent(stPlayerEvetArg* pArg)
{
// 	if ( pArg->eEventType == ePlayerEvent_ReadDBOK )
// 	{
// 		SendBaseDatToClient();
// 		OnProcessContinueLogin();
// 		// send new notice inform 
// 		CInformConfig* pConfig = (CInformConfig*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Informs) ;
// 		if ( pConfig->GetMaxInformID() > m_stBaseData.nNoticeID )
// 		{
// 			stMsgInformNewNotices msgInfom ;
// 			msgInfom.cNewNoticeCount = pConfig->GetMaxInformID() - m_stBaseData.nNoticeID ;
// 			msgInfom.cNewNoticeCount = min(msgInfom.cNewNoticeCount,pConfig->GetInformCount());
// 			SendMsgToClient((char*)&msgInfom,sizeof(msgInfom)) ;
// 		}
// 		// post online event ;
// 		CEventCenter::SharedEventCenter()->PostEvent(eEvent_PlayerOnline,this->GetPlayer()) ;
// 	}
// 	else if ( pArg->eEventType == ePlayerEvent_Recharge )
// 	{
// 		//stPlayerEventArgRecharge* pA = (stPlayerEventArgRecharge*)pArg ;
// 		//nTotalSpendRMB += pA->nRMB ;
// 		//unsigned char nLevel = nVipLevel ;
// 		//CheckVipValid();
// 		//if ( nVipLevel > nLevel )
// 		//{
// 		//	stMsgPlayerUpdateVipLevel msg ;
// 		//	msg.nCurVIPLevel = nVipLevel ;
// 		//	SendMsgToClient((char*)&msg,sizeof(msg)) ;
// 		//}
// 		CLogMgr::SharedLogMgr()->SystemLog("recharge event happened") ;
// 		++m_stBaseData.nRechargeTimes;
// 	}
	return false ;
}

void CPlayerBaseData::GetPlayerBrifData(stPlayerBrifData* pData )
{
	if ( !pData )
	{
		return ;
	}
	memcpy(pData,&m_stBaseData,sizeof(stPlayerBrifData));
}

void CPlayerBaseData::GetPlayerDetailData(stPlayerDetailData* pData )
{
	if ( !pData )
	{
		return ;
	}
	memcpy(pData,&m_stBaseData,sizeof(stPlayerDetailData));
}

bool CPlayerBaseData::EventFunc(void* pUserData,stEventArg* pArg)
{
// 	CPlayerBaseData* pBaseData = (CPlayerBaseData*)pUserData ;
// 	pBaseData->OnNewDay(pArg);
	return false ;
}

void CPlayerBaseData::OnNewDay(stEventArg* pArg)
{
// 	m_stBaseData.nYesterdayPlayTimes = m_stBaseData.nTodayPlayTimes ;
// 	m_stBaseData.nTodayPlayTimes = 0 ;
// 	m_stBaseData.nYesterdayWinCoin = m_stBaseData.nTodayWinCoin ;
// 	m_stBaseData.nTodayWinCoin = 0 ;
}

void CPlayerBaseData::OnReactive(uint32_t nSessionID )
{
	CEventCenter::SharedEventCenter()->RemoveEventListenner(eEvent_NewDay,this,CPlayerBaseData::EventFunc ) ;
}

void CPlayerBaseData::CacluateTaxasRoomMoney(uint64_t nNewTakeIn, bool bDiamond )
{
	if ( bDiamond )
	{
		m_stBaseData.nDiamoned += nNewTakeIn ;
		m_nTakeInDiamoned = 0 ;
	}
	else
	{
		m_stBaseData.nCoin += nNewTakeIn ;
		m_nTakeInCoin = 0 ;
	}

	m_bMoneyDataDirty = true ;
}
