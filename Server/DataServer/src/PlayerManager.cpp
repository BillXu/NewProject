#include "PlayerManager.h"
#include "ServerMessageDefine.h"
#include "LogManager.h"
#include "Player.h"
#include "CommonDefine.h"
#include "GameServerApp.h"
#include <assert.h>
#include "EventCenter.h"
#include "PlayerBaseData.h"
#include "AutoBuffer.h"
#include "PlayerTaxas.h"
#include "PlayerMail.h"

CSelectPlayerDataCacher::CSelectPlayerDataCacher()
{
	m_vBrifData.clear();
	m_vDetailData.clear();
}

CSelectPlayerDataCacher::~CSelectPlayerDataCacher()
{
	for ( MAP_ID_DATA::value_type va : m_vBrifData )
	{
		delete va.second ;
		va.second = nullptr ;
	}
	m_vBrifData.clear() ;

	for ( MAP_ID_DATA::value_type va : m_vDetailData )
	{
		delete va.second ;
		va.second = nullptr ;
	}
	m_vDetailData.clear() ;
}

void CSelectPlayerDataCacher::removePlayerDataCache( uint32_t nUID )
{
	MAP_ID_DATA::iterator iter = m_vBrifData.find(nUID) ;
	if ( iter != m_vBrifData.end() )
	{
		delete iter->second ;
		m_vBrifData.erase(iter) ;
		return ;
	}

	iter = m_vDetailData.find(nUID) ;
	if ( iter != m_vDetailData.end() )
	{
		delete iter->second ;
		m_vDetailData.erase(iter) ;
		return ;
	}
}

void CSelectPlayerDataCacher::cachePlayerData(stMsgSelectPlayerDataRet* pmsg )
{
	if ( pmsg->nRet )
	{
		CLogMgr::SharedLogMgr()->PrintLog("cahe player data failed");
		return ;
	}

	stPlayerBrifData* pData = (stPlayerBrifData*)((char*)pmsg + sizeof(stMsgSelectPlayerDataRet));
	if ( pmsg->isDetail )
	{
		removePlayerDataCache(pData->nUserUID);
		stPlayerDetailDataClient* pCdata = new stPlayerDetailDataClient ;
		memcpy(pCdata,pData,sizeof(stPlayerDetailDataClient));
		m_vDetailData[pData->nUserUID] = pCdata ;
	}
	else
	{
		stPlayerBrifData* pCdata = new stPlayerBrifData ;
		memcpy(pCdata,pData,sizeof(stPlayerBrifData));
		m_vBrifData[pData->nUserUID] = pCdata ;
	}
}

bool CSelectPlayerDataCacher::getPlayerData(uint32_t nUID , stPlayerBrifData* pData , bool isDetail )
{
	MAP_ID_DATA::iterator iter ;
	if ( isDetail )
	{
		iter = m_vDetailData.find(nUID) ;
		if ( iter == m_vDetailData.end() )
		{
			return false ;
		}
	}
	else
	{
		iter = m_vBrifData.find(nUID) ;
		if ( iter == m_vBrifData.end() )
		{
			iter = m_vDetailData.find(nUID) ;
			if ( iter == m_vDetailData.end() )
			{
				return false ;
			}
		}
	}
 
	memcpy(pData,iter->second,isDetail ? sizeof(stPlayerDetailDataClient) : sizeof(stPlayerBrifData));
	return true ;
}

CPlayerManager::CPlayerManager()
{
	m_vOfflinePlayers.clear() ;
	m_vAllActivePlayers.clear();
}

CPlayerManager::~CPlayerManager()
{
	MAP_SESSIONID_PLAYERS::iterator iter = m_vAllActivePlayers.begin();
	for ( ; iter != m_vAllActivePlayers.end() ; ++iter )
	{
		delete iter->second ;
		iter->second = NULL ;
	}
	m_vAllActivePlayers.clear() ;

	MAP_UID_PLAYERS::iterator iter_R = m_vOfflinePlayers.begin() ;
	for ( ; iter_R != m_vOfflinePlayers.end(); ++iter_R )
	{
		delete iter_R->second ;
		iter_R->second = NULL ;
	}
	m_vOfflinePlayers.clear() ;
}

bool CPlayerManager::OnMessage( stMsg* pMessage , eMsgPort eSenderPort , uint32_t nSessionID )
{
	if ( ProcessPublicMessage(pMessage,eSenderPort,nSessionID) )
	{
		return true ;
	}

	CPlayer* pTargetPlayer = GetPlayerBySessionID(nSessionID,true );
	if ( pTargetPlayer && pTargetPlayer->OnMessage(pMessage,eSenderPort ) )
	{
		if (pTargetPlayer->IsState(CPlayer::ePlayerState_Offline) )
		{
			pTargetPlayer->OnTimerSave(0,0);
		}
		return true  ;
	}
	else
	{
		if (pTargetPlayer == NULL )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("can not find session id = %d to process msg id = %d ,from = %d",nSessionID,pMessage->usMsgType,eSenderPort) ;
		}
		else
		{
			CLogMgr::SharedLogMgr()->ErrorLog( "unprocess msg for player uid = %d , msg = %d ,from %d ",pTargetPlayer->GetUserUID(),pMessage->usMsgType,eSenderPort ) ;
		}
	}
	return false ;
}

bool CPlayerManager::ProcessPublicMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID  )
{
	switch ( prealMsg->usMsgType )
	{
	case MSG_CROSS_SERVER_REQUEST:
		{
			stMsgCrossServerRequest* pRet = (stMsgCrossServerRequest*)prealMsg ;

			Json::Value* pJsValue = nullptr ;
			Json::Value rootValue ;
			if ( pRet->nJsonsLen )
			{
				Json::Reader reader;
				char* pstr = ((char*)&pRet->nJsonsLen) + sizeof(pRet->nJsonsLen) ;
				reader.parse(pstr,pstr + pRet->nJsonsLen,rootValue,false);
				pJsValue = &rootValue ;
			}

			if ( onCrossServerRequest(pRet,eSenderPort,pJsValue) == false )
			{
				CPlayer* pPlayer = GetPlayerByUserUID(pRet->nTargetID);
				assert(pPlayer&&"this request no one to process or target id error");
				if ( pPlayer && pPlayer->onCrossServerRequest(pRet,eSenderPort,pJsValue) )
				{
					return true ;
				}
				CLogMgr::SharedLogMgr()->ErrorLog("cross request type = %d , subType = %d ,unprocessed",pRet->nRequestType,pRet->nRequestSubType);
				return false;
			}

			return true ;
		}
		break;
	case MSG_CROSS_SERVER_REQUEST_RET:
		{
			stMsgCrossServerRequestRet* pRet = (stMsgCrossServerRequestRet*)prealMsg ;
			Json::Value* pJsValue = nullptr ;
			Json::Value rootValue ;
			if ( pRet->nJsonsLen )
			{
				Json::Reader reader;
				char* pstr = ((char*)&pRet->nJsonsLen) + sizeof(pRet->nJsonsLen) ;
				reader.parse(pstr,pstr + pRet->nJsonsLen,rootValue,false);
				pJsValue = &rootValue ;
			}

			if ( onCrossServerRequestRet(pRet,pJsValue) == false )
			{
				CPlayer* pPlayer = GetPlayerByUserUID(pRet->nTargetID);
				assert(pPlayer&&"this request no one to process or target id error");
				if ( pPlayer && pPlayer->onCrossServerRequestRet(pRet,pJsValue) )
				{
					return true ;
				}
				CLogMgr::SharedLogMgr()->ErrorLog("cross request result type = %d , subType = %d ,unprocessed",pRet->nRequestType,pRet->nRequestSubType);
				return false;
			}
			return true ;
		}
		break;
	case MSG_REQUEST_PLAYER_DATA:
		{
			stMsgRequestPlayerData* pRet = (stMsgRequestPlayerData*)prealMsg ;
			stMsgRequestPlayerDataRet msgBack ;
			msgBack.nRet = 0 ;
			msgBack.isDetail = pRet->isDetail ;
			CPlayer* pPlayer = GetPlayerByUserUID(pRet->nPlayerUID);

			stPlayerDetailDataClient stData ;
			stData.nCurrentRoomID = 0 ;
			CAutoBuffer auB (sizeof(msgBack) + sizeof(stPlayerDetailData));
			if ( pPlayer )
			{
				CPlayerTaxas* pTaxasData = (CPlayerTaxas*)pPlayer->GetComponent(ePlayerComponent_PlayerTaxas);
				stData.nCurrentRoomID = pTaxasData->getCurRoomID() ;

				if ( stData.nCurrentRoomID )  // select take in
				{
					stMsgCrossServerRequest msgReq ;
					msgReq.nJsonsLen = 0 ;
					msgReq.nReqOrigID = nSessionID ;
					msgReq.nRequestSubType = eCrossSvrReqSub_SelectPlayerData ;
					msgReq.nRequestType = eCrossSvrReq_SelectTakeIn ;
					msgReq.nTargetID = stData.nCurrentRoomID ;
					msgReq.cSysIdentifer = ID_MSG_PORT_TAXAS ;
					msgReq.vArg[0] = pRet->nPlayerUID;
					msgReq.vArg[1] = pRet->isDetail ;
					CGameServerApp::SharedGameServerApp()->sendMsg(nSessionID,(char*)&msgReq,sizeof(msgReq)) ;
					CLogMgr::SharedLogMgr()->PrintLog("select take in for player detail") ;
					return true ;
				}

				if ( pRet->isDetail )
				{
					pPlayer->GetBaseData()->GetPlayerDetailData(&stData);
					pTaxasData->getTaxasData(&stData.tTaxasData);
				}
				else
				{
					pPlayer->GetBaseData()->GetPlayerBrifData(&stData) ;
				}
				
				auB.addContent(&msgBack,sizeof(msgBack));
				auB.addContent(&stData,pRet->isDetail ? sizeof(stPlayerDetailDataClient) : sizeof(stPlayerBrifData) ) ;
				CGameServerApp::SharedGameServerApp()->sendMsg(nSessionID,auB.getBufferPtr(),auB.getContentSize()) ;
				return true ;
			}

			if ( m_tPlayerDataCaher.getPlayerData(pRet->nPlayerUID,&stData,pRet->isDetail) )
			{
				auB.addContent(&msgBack,sizeof(msgBack));
				auB.addContent(&stData,pRet->isDetail ? sizeof(stPlayerDetailDataClient) : sizeof(stPlayerBrifData) ) ;
				CGameServerApp::SharedGameServerApp()->sendMsg(nSessionID,auB.getBufferPtr(),auB.getContentSize()) ;
				CLogMgr::SharedLogMgr()->PrintLog("get player data from cahe") ;
				return true ;
			}

			CLogMgr::SharedLogMgr()->PrintLog("req detail player not online , req from db target uid = %u",pRet->nPlayerUID) ;
			stMsgSelectPlayerData msgReq ;
			msgReq.isDetail = pRet->isDetail ;
			msgReq.nReqPlayerSessionID = nSessionID  ;
			msgReq.nTargetPlayerUID = pRet->nPlayerUID ;
			CGameServerApp::SharedGameServerApp()->sendMsg(nSessionID,(char*)&msgReq,sizeof(msgReq)) ;
		}
		break;
	case MSG_SELECT_DB_PLAYER_DATA:
		{
			stMsgSelectPlayerDataRet* pRet = (stMsgSelectPlayerDataRet*)prealMsg ;
			stMsgRequestPlayerDataRet msgBack ;
			msgBack.nRet = pRet->nRet ;
			msgBack.isDetail = pRet->isDetail ;
			if ( pRet->nRet )
			{
				CGameServerApp::SharedGameServerApp()->sendMsg(pRet->nReqPlayerSessionID,(char*)&msgBack,sizeof(msgBack)) ;
			}
			else
			{
				m_tPlayerDataCaher.cachePlayerData(pRet);
				CAutoBuffer auB (sizeof(msgBack) + sizeof(stPlayerDetailDataClient));
				auB.addContent(&msgBack,sizeof(msgBack));
				uint16_t nLen = pRet->isDetail ? sizeof(stPlayerDetailDataClient) : sizeof(stPlayerBrifData) ;
				auB.addContent((char*)prealMsg + sizeof(stMsgSelectPlayerDataRet),nLen );
				CGameServerApp::SharedGameServerApp()->sendMsg(pRet->nReqPlayerSessionID,auB.getBufferPtr(),auB.getContentSize()) ;
			}
			CLogMgr::SharedLogMgr()->PrintLog("session id = %d req play data ret = %d",pRet->nReqPlayerSessionID,pRet->nRet ) ;
		}
		break;
	case MSG_PLAYER_LOGIN:
		{
			stMsgOnPlayerLogin* pmsgenter = (stMsgOnPlayerLogin*)prealMsg ;
			CPlayer* pPlayer = GetPlayerBySessionID(nSessionID) ;
			if ( pPlayer != NULL && pPlayer->GetUserUID() == pmsgenter->nUserUID )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("double nSession, this nSessionID already have player, already login , do not login again  id = %d? ",nSessionID) ;
				return true ;
			}

			if ( pPlayer != NULL && pPlayer->GetUserUID() != pmsgenter->nUserUID ) // switch account in main scene 
			{
				// disconnect pre player 
				OnPlayerOffline(pPlayer);
			}

			if ( ProcessIsAlreadyLogin(pmsgenter->nUserUID,nSessionID) )
			{
				return true ;
			}

			// is offline peer 
			MAP_UID_PLAYERS::iterator iterOfflien = m_vOfflinePlayers.begin();
			for ( ; iterOfflien != m_vOfflinePlayers.end(); ++iterOfflien )
			{
				if ( iterOfflien->second && iterOfflien->first == pmsgenter->nUserUID )
				{
					iterOfflien->second->OnReactive(nSessionID) ;
					AddPlayer(iterOfflien->second) ;
					m_vOfflinePlayers.erase(iterOfflien) ;
					return true ;
				}
			}

			CPlayer* pNew = new CPlayer ;
			pNew->Init(pmsgenter->nUserUID,nSessionID ) ;
			AddPlayer(pNew) ;
		}
		break;
	case MSG_DISCONNECT_CLIENT:
	case MSG_PLAYER_LOGOUT:
		{
			CPlayer* pPlayer = GetPlayerBySessionID(nSessionID) ;
			if ( pPlayer )
			{
				// post online event ;
				CLogMgr::SharedLogMgr()->PrintLog("player disconnect session id = %d",nSessionID);
				OnPlayerOffline(pPlayer) ;
			}
			else
			{
				CLogMgr::SharedLogMgr()->ErrorLog("client disconnect ! client is NULL session id = %d",nSessionID) ;
			}
			//#ifdef _DEBUG
			LogState();
			//#endif
		}
		break;
	case MSG_TP_ORDER_LEAVE:
		{
			stMsgOrderTaxasPlayerLeaveRet* pRet = (stMsgOrderTaxasPlayerLeaveRet*)prealMsg ;
			CPlayer* pp = GetPlayerByUserUID(pRet->nUserUID) ;
			if (!pp)
			{
				CLogMgr::SharedLogMgr()->ErrorLog("uid = %d not find , so can not inform leave",pRet->nUserUID);
			}
			else
			{
				pp->OnMessage(prealMsg,eSenderPort);
			}
		}
		break;
	case MSG_TP_INFORM_LEAVE:
		{
			stMsgInformTaxasPlayerLeave* pRet = (stMsgInformTaxasPlayerLeave*)prealMsg ;
			CPlayer* pp = GetPlayerByUserUID(pRet->nUserUID) ;
			if (!pp)
			{
				CLogMgr::SharedLogMgr()->ErrorLog("uid = %d not find , so can not inform leave",pRet->nUserUID);
			}
			else
			{
				pp->OnMessage(prealMsg,eSenderPort);
			}
		}
		break;
	case MSG_TP_SYNC_PLAYER_DATA:
		{
			stMsgSyncTaxasPlayerData* pRet = (stMsgSyncTaxasPlayerData*)prealMsg ;
			CPlayer* pp = GetPlayerByUserUID(pRet->nUserUID) ;
			if (!pp)
			{
				CLogMgr::SharedLogMgr()->ErrorLog("uid = %d not find , so can not sys data",pRet->nUserUID);
			}
			else
			{
				pp->OnMessage(prealMsg,eSenderPort);
			}
		}
		break;
	default:
		return false;
	}

	return true ;
}

CPlayer* CPlayerManager::GetPlayerBySessionID( unsigned int nSessionID , bool bInclueOffline )
{
	MAP_SESSIONID_PLAYERS::iterator iter = m_vAllActivePlayers.find(nSessionID) ;
	if ( iter != m_vAllActivePlayers.end())
	{
		return iter->second ;
	}

	if ( bInclueOffline )
	{
		MAP_UID_PLAYERS::iterator iterOffline = m_vOfflinePlayers.begin();
		for ( ; iterOffline != m_vOfflinePlayers.end(); ++iterOffline )
		{
			if ( iterOffline->second->GetSessionID() == nSessionID  )
			{
				return iterOffline->second ;
			}
		}
	}
	return NULL ;
}

void CPlayerManager::Update(float fDeta )
{
	 
}

CPlayer* CPlayerManager::GetFirstActivePlayer()
{
	MAP_SESSIONID_PLAYERS::iterator iter = m_vAllActivePlayers.begin() ;
	if ( iter == m_vAllActivePlayers.end() || iter->second == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("first actvie player is NULL") ;
		return NULL ;
	}
	return iter->second ;
}

CPlayer* CPlayerManager::GetPlayerByUserUID( unsigned int nUserUID, bool bInclueOffline )
{
	MAP_SESSIONID_PLAYERS::iterator iter = m_vAllActivePlayers.begin() ;
	for ( ; iter != m_vAllActivePlayers.end(); ++iter )
	{
		if ( iter->second )
		{
			if ( nUserUID == iter->second->GetUserUID() )
			{
				return iter->second ;
			}
		}
	}

	if ( bInclueOffline )
	{
		MAP_UID_PLAYERS::iterator iter_L = m_vOfflinePlayers.find(nUserUID) ;
		if ( iter_L != m_vOfflinePlayers.end() )
		{
			return iter_L->second ;
		}
	}
	return NULL ;
}

void CPlayerManager::OnPlayerOffline( CPlayer*pPlayer )
{
	if ( !pPlayer )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Can not Remove NULL player !") ;
		return ;
	}

	CEventCenter::SharedEventCenter()->PostEvent(eEvent_PlayerOffline,pPlayer) ;
	pPlayer->OnPlayerDisconnect() ;

	MAP_SESSIONID_PLAYERS::iterator iter = m_vAllActivePlayers.find(pPlayer->GetSessionID() ) ;
	if ( iter == m_vAllActivePlayers.end() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why pPlayer uid = %d , not active ? shuold active " , pPlayer->GetUserUID() ) ;
		delete pPlayer ;
		pPlayer = NULL ;
		return ;
	}
	m_vAllActivePlayers.erase(iter) ;

	// do not cacher player 
	delete pPlayer ;
	pPlayer = nullptr ;
	return  ;

	// do not cacher player 
	 
	MAP_UID_PLAYERS::iterator iterOffline = m_vOfflinePlayers.find(pPlayer->GetUserUID()) ;
	if ( iterOffline != m_vOfflinePlayers.end() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why player uid = %d already in offline list ? " , iterOffline->first ) ;
		
		delete pPlayer ;
		pPlayer = NULL ;
		return ;
	}
	m_vOfflinePlayers[pPlayer->GetUserUID()] = pPlayer ;
}

void CPlayerManager::AddPlayer(CPlayer*pPlayer)
{
	if ( !pPlayer )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Can not Add NULL player !") ;
		return ;
	}
	MAP_SESSIONID_PLAYERS::iterator iter = m_vAllActivePlayers.find(pPlayer->GetSessionID() ) ;
	if ( iter != m_vAllActivePlayers.end() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Player to add had existed in active map ! , player UID = %d",pPlayer->GetUserUID() ) ;
		delete pPlayer ;
		pPlayer = NULL ;
	}
	else
	{
		m_vAllActivePlayers[pPlayer->GetSessionID()]= pPlayer ;
	}
}

void CPlayerManager::LogState()
{
	CLogMgr::SharedLogMgr()->SystemLog( "Active Player: %d   Offline Player: %d " ,m_vAllActivePlayers.size(),m_vOfflinePlayers.size() ) ;
}

bool CPlayerManager::ProcessIsAlreadyLogin(unsigned int nUserID ,unsigned nSessionID )
{
	// is active player
	CPlayer* pPlayer = NULL ;
	MAP_SESSIONID_PLAYERS::iterator  iter_ma = m_vAllActivePlayers.begin() ;
	for ( ; iter_ma != m_vAllActivePlayers.end(); ++iter_ma )
	{
		pPlayer = iter_ma->second ;
		if ( !pPlayer )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("Why empty active player here ?") ;
			continue; 
		}

		if ( pPlayer->GetUserUID() == nUserID )
		{
			pPlayer->OnAnotherClientLoginThisPeer(nSessionID) ;
			m_vAllActivePlayers.erase(iter_ma) ;
			AddPlayer(pPlayer) ;
			CLogMgr::SharedLogMgr()->PrintLog("other decivec login");
			return true ;
		}
	}
	return false ;
}

bool CPlayerManager::onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue)
{
	if ( pRequest->nRequestType == eCrossSvrReq_Inform )
	{
		CPlayer* pp = GetPlayerByUserUID(pRequest->nTargetID) ;
		if ( pp )
		{
			((CPlayerMailComponent*)pp->GetComponent(ePlayerComponent_Mail))->ReciveMail(((char*)pRequest) + sizeof(stMsgCrossServerRequest),pRequest->nJsonsLen);
		}
		else
		{
			CPlayerMailComponent::PostMailToDB(((char*)pRequest) + sizeof(stMsgCrossServerRequest),pRequest->nJsonsLen,pRequest->nTargetID);
		}
		return true ;
	}
	return false ;
}

bool CPlayerManager::onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue)
{
	if ( eCrossSvrReq_SelectTakeIn == pResult->nRequestType && eCrossSvrReqSub_SelectPlayerData == pResult->nRequestSubType )
	{
		stMsgRequestPlayerDataRet msgBack ;
		msgBack.nRet = 0 ;
		msgBack.isDetail = pResult->vArg[3] ;
		CPlayer* pPlayer = GetPlayerByUserUID(pResult->vArg[0]);
		if ( pPlayer == nullptr )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why this player is null , can not , funck!") ;
			return true ;
		}

		stPlayerDetailDataClient stData ;
		stData.nCurrentRoomID = 0 ;
		CAutoBuffer auB (sizeof(msgBack) + sizeof(stPlayerDetailDataClient));
		if ( pPlayer )
		{
			CPlayerTaxas* pTaxasData = (CPlayerTaxas*)pPlayer->GetComponent(ePlayerComponent_PlayerTaxas);
			stData.nCurrentRoomID = pResult->nReqOrigID;
			if ( msgBack.isDetail )
			{
				pPlayer->GetBaseData()->GetPlayerDetailData(&stData);
				pTaxasData->getTaxasData(&stData.tTaxasData);
				if ( vJsValue )
				{
					stData.tTaxasData.nPlayTimes += (*vJsValue)["playTimes"].asUInt();
					stData.tTaxasData.nWinTimes += (*vJsValue)["winTimes"].asUInt();
					stData.tTaxasData.nSingleWinMost = (*vJsValue)["singleMost"].asUInt() < stData.tTaxasData.nSingleWinMost ? stData.tTaxasData.nSingleWinMost : (*vJsValue)["singleMost"].asUInt();
				}
				else
				{
					CLogMgr::SharedLogMgr()->PrintLog("targe player not sit down uid = %llu",pResult->vArg[0]);
				}
			}
			else
			{
				pPlayer->GetBaseData()->GetPlayerBrifData(&stData) ;
			}

			if ( pResult->vArg[1] )
			{
				stData.nCoin += pResult->vArg[2] ;
			}
			else
			{
				stData.nDiamoned += pResult->vArg[2] ;
			}
			stData.nCurrentRoomID = pResult->nReqOrigID;
			auB.addContent(&msgBack,sizeof(msgBack));
			auB.addContent(&stData,msgBack.isDetail ? sizeof(stPlayerDetailDataClient) : sizeof(stPlayerBrifData) ) ;
			CGameServerApp::SharedGameServerApp()->sendMsg(pResult->nTargetID,auB.getBufferPtr(),auB.getContentSize()) ;
			CLogMgr::SharedLogMgr()->PrintLog("select take in ret , send player data");
			return true ;
		}
		return true ;
	}
	return false ;
}