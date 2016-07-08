#include "PlayerGameData.h"
#include "Player.h"
#include "LogManager.h"
#include "GameServerApp.h"
#include "PlayerBaseData.h"
#include <json/json.h>
#include "AutoBuffer.h"
#include "TaxasPokerPeerCard.h"
#include "RoomConfig.h"
#include "AsyncRequestQuene.h"
#include "Group.h"
#include "GameRoomCenter.h"
void CPlayerGameData::Reset()
{
	IPlayerComponent::Reset();
	m_nStateInRoomID = 0;
	m_nSubRoomIdx = 0 ;
	memset(&m_vData,0,sizeof(m_vData));
	for ( auto refPtr : m_vGameRecorders )
	{
		delete refPtr ;
		refPtr = nullptr ;
	}
	m_vGameRecorders.clear() ;

	stMsgReadPlayerGameData msg ;
	msg.nUserUID = GetPlayer()->GetUserUID() ;
	SendMsg(&msg,sizeof(msg)) ;
	CLogMgr::SharedLogMgr()->PrintLog("requesting player niuniu data for uid = %d",msg.nUserUID);                                                                                                                                                                                                                                                                                                                        

	stMsgReadPlayerGameRecorder msgReadGameRecorder ;
	msgReadGameRecorder.nUserUID = GetPlayer()->GetUserUID() ;
	SendMsg(&msgReadGameRecorder,sizeof(msgReadGameRecorder)) ;
	CLogMgr::SharedLogMgr()->PrintLog("request player game recorder data  uid = %u", msgReadGameRecorder.nUserUID ) ;

}

CPlayerGameData::~CPlayerGameData()
{
	for ( auto refPtr : m_vGameRecorders )
	{
		delete refPtr ;
		refPtr = nullptr ;
	}
	m_vGameRecorders.clear() ;
}

void CPlayerGameData::Init()
{
	IPlayerComponent::Init();
	m_eType = ePlayerComponent_PlayerGameData ;
	m_nStateInRoomID = 0;
	memset(&m_vData,0,sizeof(m_vData));

	stMsgReadPlayerGameData msg ;
	msg.nUserUID = GetPlayer()->GetUserUID() ;
	SendMsg(&msg,sizeof(msg)) ;
	CLogMgr::SharedLogMgr()->PrintLog("requesting player niuniu data for uid = %d",msg.nUserUID);                                                                                                                                                                                                                                                                                                                        

	stMsgReadPlayerGameRecorder msgReadGameRecorder ;
	msgReadGameRecorder.nUserUID = GetPlayer()->GetUserUID() ;
	SendMsg(&msgReadGameRecorder,sizeof(msgReadGameRecorder)) ;
	CLogMgr::SharedLogMgr()->PrintLog("request player game recorder data  uid = %u", msgReadGameRecorder.nUserUID ) ;
}

bool CPlayerGameData::OnMessage( stMsg* pMessage , eMsgPort eSenderPort)
{
	if ( IPlayerComponent::OnMessage(pMessage,eSenderPort) )
	{
		return true ;
	}
	switch ( pMessage->usMsgType )
	{
	case MSG_PLAYER_ENTER_ROOM:
		{
			stMsgPlayerEnterRoom* pRet = (stMsgPlayerEnterRoom*)pMessage ;
			if ( isNotInAnyRoom() )
			{
				stMsgSvrEnterRoom msgEnter ;
				msgEnter.cSysIdentifer = GetPlayer()->getMsgPortByRoomType(pRet->nRoomGameType) ;
				if ( msgEnter.cSysIdentifer == ID_MSG_PORT_NONE )
				{
					stMsgPlayerEnterRoomRet msgRet ;
					msgRet.nRet = 6;
					SendMsg(&msgRet,sizeof(msgRet)) ;
					CLogMgr::SharedLogMgr()->ErrorLog("player uid = %d enter game , can not find game port type = %d ",GetPlayer()->GetUserUID(), pRet->nRoomGameType ) ;
					break;
				}

				msgEnter.nRoomID = pRet->nRoomID ;
				msgEnter.nSubIdx = pRet->nSubIdx ;
				msgEnter.tPlayerData.isRegisted = GetPlayer()->GetBaseData()->isPlayerRegistered() ;
				msgEnter.tPlayerData.nCoin = GetPlayer()->GetBaseData()->getCoin() ;
				msgEnter.tPlayerData.nUserSessionID = GetPlayer()->GetSessionID() ;
				msgEnter.tPlayerData.nUserUID = GetPlayer()->GetUserUID() ;
				msgEnter.tPlayerData.nNewPlayerHaloWeight = GetPlayer()->GetBaseData()->getNewPlayerHaloWeight() ;
				msgEnter.tPlayerData.nPlayerType = GetPlayer()->GetBaseData()->getPlayerType();
				CGameServerApp::SharedGameServerApp()->sendMsg(msgEnter.tPlayerData.nUserSessionID,(char*)&msgEnter,sizeof(msgEnter)) ;

				m_nStateInRoomID = pRet->nRoomID;
				CLogMgr::SharedLogMgr()->PrintLog("player uid = %d enter to enter room id = %d ,coin = %u", GetPlayer()->GetUserUID(), m_nStateInRoomID,msgEnter.tPlayerData.nCoin ) ;
			}
			else
			{
				stMsgPlayerEnterRoomRet msgRet ;
				msgRet.nRet = 1;
				SendMsg(&msgRet,sizeof(msgRet)) ;
				CLogMgr::SharedLogMgr()->PrintLog("player uid = %d already in room , id = %d ", GetPlayer()->GetUserUID() ,m_nStateInRoomID ) ;
			}
		}
		break;
	case MSG_SVR_ENTER_ROOM:
		{
			stMsgSvrEnterRoomRet* pRet = (stMsgSvrEnterRoomRet*)pMessage ;
			stMsgPlayerEnterRoomRet msgRet ;
			msgRet.nRet = pRet->nRet;
			SendMsg(&msgRet,sizeof(msgRet)) ;

			if ( msgRet.nRet )  // enter room failed ;
			{
				m_nStateInRoomID = 0;
				m_nSubRoomIdx = 0 ;
				CLogMgr::SharedLogMgr()->PrintLog("player enter room failed ret = %d uid = %d",msgRet.nRet,GetPlayer()->GetUserUID()) ;
			}
			else
			{
				m_nStateInRoomID = pRet->nRoomID;
				m_nSubRoomIdx = (uint8_t)pRet->nSubIdx ;
				CLogMgr::SharedLogMgr()->PrintLog("player do enter oom id = %d uid = %d subIdx = %u",m_nStateInRoomID,GetPlayer()->GetUserUID(),m_nSubRoomIdx) ;
			}
		}
		break;
	case MSG_SVR_DO_LEAVE_ROOM:
		{
			m_nStateInRoomID = 0;
			stMsgSvrDoLeaveRoom* pRet = (stMsgSvrDoLeaveRoom*)pMessage ;
			CLogMgr::SharedLogMgr()->PrintLog("uid = %d leave room coin = %u , back coin = %lld, temp coin = %u",GetPlayer()->GetUserUID(),GetPlayer()->GetBaseData()->getCoin(),pRet->nCoin,GetPlayer()->GetBaseData()->getTempCoin() ) ;
			GetPlayer()->GetBaseData()->setCoin(pRet->nCoin + GetPlayer()->GetBaseData()->getTempCoin()) ;
			GetPlayer()->GetBaseData()->setTempCoin(0) ;
			GetPlayer()->GetBaseData()->addTodayGameCoinOffset(pRet->nGameOffset);
			
			m_vData[pRet->nGameType].nPlayTimes += pRet->nPlayerTimes ;
			m_vData[pRet->nGameType].nWinTimes += pRet->nWinTimes ;
			if ( m_vData[pRet->nGameType].nSingleWinMost < pRet->nSingleWinMost )
			{
				m_vData[pRet->nGameType].nSingleWinMost = pRet->nSingleWinMost ;
			}

			if ( pRet->nPlayerTimes != 0 )
			{
				m_vData[pRet->nGameType].bDirty = true ;
			}

			// decrease halo weight 
			if ( GetPlayer()->GetBaseData()->getNewPlayerHaloWeight() >= pRet->nPlayerTimes )
			{
				GetPlayer()->GetBaseData()->setNewPlayerHalo(GetPlayer()->GetBaseData()->getNewPlayerHaloWeight() - pRet->nPlayerTimes );
			}
			else
			{
				GetPlayer()->GetBaseData()->setNewPlayerHalo(0);
			}

			CLogMgr::SharedLogMgr()->PrintLog("uid = %d do leave room final coin = %u, playertimes = %u , wintimes = %u ,offset = %d",GetPlayer()->GetUserUID(), GetPlayer()->GetBaseData()->getCoin(),pRet->nPlayerTimes,pRet->nWinTimes,pRet->nGameOffset) ;
			stMsg msg ;
			msg.usMsgType = MSG_PLAYER_UPDATE_MONEY ;
			GetPlayer()->GetBaseData()->OnMessage(&msg,ID_MSG_PORT_CLIENT) ;
		}
		break;
	case MSG_SVR_DELAYED_LEAVE_ROOM:
		{
			stMsgSvrDelayedLeaveRoom* pRet = (stMsgSvrDelayedLeaveRoom*)pMessage ;
			if ( isNotInAnyRoom() )
			{
				GetPlayer()->GetBaseData()->setCoin( pRet->nCoin + GetPlayer()->GetBaseData()->getCoin() ) ;
				CLogMgr::SharedLogMgr()->PrintLog("player not enter other room just uid = %d add coin = %lld, final = %u,",GetPlayer()->GetUserUID(),pRet->nCoin,GetPlayer()->GetBaseData()->getCoin()) ;
			}
			else
			{
				GetPlayer()->GetBaseData()->setTempCoin(GetPlayer()->GetBaseData()->getTempCoin() + pRet->nCoin) ;
				CLogMgr::SharedLogMgr()->PrintLog("player enter other room so uid = %d add temp = %lld, final = %u,",GetPlayer()->GetUserUID(),pRet->nCoin,GetPlayer()->GetBaseData()->getTempCoin(),GetPlayer()->GetBaseData()->getCoin() ) ;
			}

			GetPlayer()->GetBaseData()->addTodayGameCoinOffset(pRet->nGameOffset);

			m_vData[pRet->nGameType].nPlayTimes += pRet->nPlayerTimes ;
			m_vData[pRet->nGameType].nWinTimes += pRet->nWinTimes ;
			if ( m_vData[pRet->nGameType].nSingleWinMost < pRet->nSingleWinMost )
			{
				m_vData[pRet->nGameType].nSingleWinMost = pRet->nSingleWinMost ;
			}

			if ( pRet->nPlayerTimes != 0 )
			{
				m_vData[pRet->nGameType].bDirty = true ;
			}

			CLogMgr::SharedLogMgr()->PrintLog("uid = %d delay leave room coin = %u",GetPlayer()->GetUserUID(), GetPlayer()->GetBaseData()->getCoin()) ;
			stMsg msg ;
			msg.usMsgType = MSG_PLAYER_UPDATE_MONEY ;
			GetPlayer()->GetBaseData()->OnMessage(&msg,ID_MSG_PORT_CLIENT) ;
		}
		break;
	case MSG_PLAYER_REQUEST_GAME_RECORDER:
		{
			stMsgPlayerRequestGameRecorderRet msgRet ;
			msgRet.nCnt = m_vGameRecorders.size() < 10 ? m_vGameRecorders.size() : 10 ;
			CAutoBuffer auBuffer (sizeof(msgRet) + sizeof(stRecorderItem) * msgRet.nCnt );
			auBuffer.addContent(&msgRet,sizeof(msgRet)) ;
			auto iter = m_vGameRecorders.begin() ;
			for ( uint8_t nIdx = 0 ; iter != m_vGameRecorders.end() && nIdx < msgRet.nCnt ; ++nIdx ,++iter )
			{
				stRecorderItem rItem ;
				rItem.nBuyIn = (*iter)->nBuyIn ;
				rItem.nCreateUID = (*iter)->nCreateUID ;
				rItem.nDuiringSeconds = (*iter)->nDuiringSeconds ;
				rItem.nFinishTime = (*iter)->nFinishTime ;
				rItem.nOffset = (*iter)->nOffset ;
				rItem.nRoomID = (*iter)->nRoomID ;
				rItem.nBaseBet = (*iter)->nBaseBet ;
				rItem.nClubID = (*iter)->nClubID ;
				memcpy(rItem.cRoomName,(*iter)->cRoomName,sizeof(rItem.cRoomName));
				auBuffer.addContent(&rItem,sizeof(rItem)) ;

				// delete sended recorder ;
				delete (*iter) ;
				(*iter) = nullptr ;
			}
			m_vGameRecorders.clear() ;
			SendMsg((stMsg*)auBuffer.getBufferPtr(),auBuffer.getContentSize()) ;
			CLogMgr::SharedLogMgr()->PrintLog("send game recorder cnt = %u , uid = %u",msgRet.nCnt,GetPlayer()->GetUserUID() ) ;
		}
		break ;
	case MSG_READ_PLAYER_GAME_DATA:
		{
			stMsgReadPlayerGameDataRet* pRet = (stMsgReadPlayerGameDataRet*)pMessage ;

			Json::Reader jsReader ;
			Json::Value jsValue ;
			char* pBuffer = (char*)pRet ;
			pBuffer += sizeof(stMsgReadPlayerGameDataRet);
			jsReader.parse(pBuffer,pBuffer + pRet->nJsonLen,jsValue ) ;

			if ( jsValue.empty() )
			{
				CLogMgr::SharedLogMgr()->PrintLog("this player have no game data uid = %u",GetPlayer()->GetUserUID()) ;
				break ; 
			}
			
			for ( uint8_t nIdx = 0 ; nIdx < jsValue.size() ; ++nIdx )
			{
				Json::Value jsGameData = jsValue[nIdx] ;
				if ( jsGameData.isNull() == true || jsGameData["gameType"].isNull() )
				{
					CLogMgr::SharedLogMgr()->PrintLog("why player id = %u , game data is null idx = %u",GetPlayer()->GetUserUID(),nIdx) ;
					continue;
				}

				eRoomType eType = (eRoomType)jsGameData["gameType"].asUInt();
				Json::Value jsData = jsGameData["data"] ; 
				if ( eType >= eRoom_Max || jsData.isNull() == true )
				{
					CLogMgr::SharedLogMgr()->ErrorLog("player uid = %u , game data type invalid type = %u ",GetPlayer()->GetUserUID(),eType);
					continue;
				}

				m_vData[eType].bDirty = false ;
				m_vData[eType].nChampionTimes = jsData["nChampionTimes"].asUInt() ;
				m_vData[eType].nPlayTimes = jsData["nPlayTimes"].asUInt() ;
				m_vData[eType].nRun_upTimes = jsData["nRun_upTimes"].asUInt() ;
				m_vData[eType].nSingleWinMost = jsData["nSingleWinMost"].asUInt() ;
				m_vData[eType].nThird_placeTimes = jsData["nThird_placeTimes"].asUInt() ;
				m_vData[eType].nWinTimes = jsData["nWinTimes"].asUInt() ;
				Json::Value jsMaxCard = jsData["jsMaxCard"];
				for ( uint8_t nIdx = 0 ; nIdx < jsMaxCard.size() && nIdx < MAX_TAXAS_HOLD_CARD ; ++nIdx )
				{
					m_vData[eType].vMaxCards[nIdx] = jsData[nIdx].asUInt() ;
				}
			}

			CLogMgr::SharedLogMgr()->PrintLog("finish read player game data uid = %u" , GetPlayer()->GetUserUID()) ;
		}
		break;
	case MSG_READ_PLAYER_GAME_RECORDER:
		{
			stMsgReadPlayerGameRecorderRet* pRet = (stMsgReadPlayerGameRecorderRet*)pMessage ;
			stPlayerGameRecorder* pRecorder = new stPlayerGameRecorder ;
			pRecorder->nDuiringSeconds = pRet->nDuiringSeconds ;
			pRecorder->nFinishTime = pRet->nFinishTime ;
			pRecorder->nOffset = pRet->nOffset ;
			pRecorder->nRoomID = pRet->nRoomID ;
			pRecorder->nCreateUID = pRet->nCreateUID ;
			pRecorder->nBuyIn = pRet->nBuyIn ;
			pRecorder->nBaseBet = pRet->nBaseBet ;
			pRecorder->nClubID = pRet->nClubID ;
			memcpy(pRecorder->cRoomName,pRet->cRoomName,sizeof(pRet->cRoomName));
			CLogMgr::SharedLogMgr()->PrintLog("read basebet = %u ",pRet->nBaseBet) ;
			addPlayerGameRecorder(pRecorder,false);
		}
		break ;
	case MSG_SYNC_PRIVATE_ROOM_RESULT:
		{
			stMsgSyncPrivateRoomResult* pRet = (stMsgSyncPrivateRoomResult*)pMessage ;
			stPlayerGameRecorder* pRecorder = new stPlayerGameRecorder ;
			pRecorder->nDuiringSeconds = pRet->nDuringTimeSeconds ;
			pRecorder->nFinishTime = time(nullptr) ;
			pRecorder->nOffset = pRet->nOffset ;
			pRecorder->nRoomID = pRet->nRoomID ;
			pRecorder->nCreateUID = pRet->nCreatorUID ;
			pRecorder->nBuyIn = pRet->nBuyIn ;
			pRecorder->nBaseBet = pRet->nBaseBet ; 
			pRecorder->nClubID = pRet->nClubID ;
			memcpy(pRecorder->cRoomName,pRet->cRoomName,sizeof(pRet->cRoomName));
			addPlayerGameRecorder(pRecorder) ;

			// process coin ;
			if ( isNotInAnyRoom() )
			{
				GetPlayer()->GetBaseData()->setCoin( pRet->nFinalCoin + GetPlayer()->GetBaseData()->getCoin() ) ;
				CLogMgr::SharedLogMgr()->PrintLog("sync private Room coin player not enter other room just uid = %d add coin = %u, final = %u,",GetPlayer()->GetUserUID(),pRet->nFinalCoin,GetPlayer()->GetBaseData()->getCoin()) ;
			}
			else
			{
				GetPlayer()->GetBaseData()->setTempCoin(GetPlayer()->GetBaseData()->getTempCoin() + pRet->nFinalCoin) ;
				CLogMgr::SharedLogMgr()->PrintLog("sync private Room coin  player enter other room so uid = %d add temp = %u, final = %u,",GetPlayer()->GetUserUID(),pRet->nFinalCoin,GetPlayer()->GetBaseData()->getTempCoin(),GetPlayer()->GetBaseData()->getCoin() ) ;
			}
		}
		break ;
	case MSG_READ_PLAYER_TAXAS_DATA:
		{
			stMsgReadPlayerTaxasDataRet* pRet = (stMsgReadPlayerTaxasDataRet*)pMessage ;
			if ( pRet->nRet )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("uid = %d read taxas data error",pRet->nUserUID);
				return true ;
			}

			auto& gdata = m_vData[eRoom_TexasPoker] ;
			gdata.bDirty = false ;
			memcpy(&gdata,&pRet->tData,sizeof(pRet->tData)) ;
			CLogMgr::SharedLogMgr()->PrintLog("uid taxas data  , owner rooms = ");
			sendGameDataToClient();
		}
		break;
	case MSG_REQUEST_MY_OWN_ROOMS:
		{
			stMsgRequestMyOwnRooms* pRet = (stMsgRequestMyOwnRooms*)pMessage ;
			if ( pRet->nRoomType >= eRoom_Max )
			{
				return false;
			}

			//stMsgRequestMyOwnRoomsRet msgRet ;
			//msgRet.nRoomType = pRet->nRoomType ;
			//msgRet.nCnt = 0 ;
			//if ( msgRet.nCnt == 0 )
			//{
			//	SendMsg(&msgRet,sizeof(msgRet)) ;
			//	return true ;
			//}

			//CAutoBuffer autoBuffer(sizeof(msgRet) + sizeof(uint32_t)* msgRet.nCnt);
			//autoBuffer.addContent((char*)&msgRet,sizeof(msgRet)) ;
			//MAP_ID_MYROOW::iterator iter = m_vMyOwnRooms[pRet->nRoomType].begin() ;
			//for ( ; iter != m_vMyOwnRooms[pRet->nRoomType].end() ; ++iter )
			//{
			//	uint32_t n = iter->first ;
			//	autoBuffer.addContent((char*)&n,sizeof(uint32_t));
			//}
			//SendMsg((stMsg*)autoBuffer.getBufferPtr(),autoBuffer.getContentSize()) ;
		}
		break;
	default:
		return false;
	}
	return true ;
}

bool CPlayerGameData::OnMessage( Json::Value& recvValue , uint16_t nmsgType, eMsgPort eSenderPort )
{
	switch (nmsgType)
	{
	case MSG_CREATE_ROOM:
		{
			if ( GetPlayer()->isWaitAsyncReq() )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("async is busy , try later uid = %u",GetPlayer()->GetUserUID() ) ;
				return true;
			}
			uint32_t nClubID = recvValue["clubID"].asUInt() ;
			eRoomType eroomType = (eRoomType)recvValue["roomType"].asUInt() ;
			// if can create room  ;
			Json::Value jsMsgBack ;
			jsMsgBack["ret"] = 0 ;
			jsMsgBack["clubID"] = nClubID ;
			jsMsgBack["roomID"] = 0 ;

			if ( eroomType >= eRoom_Max )
			{
				jsMsgBack["ret"] = 3 ;
				SendMsg(jsMsgBack,nmsgType);
				break;
			}

			auto pgameCenter = (CGameRoomCenter*)CGameServerApp::SharedGameServerApp()->getModuleByType(IGlobalModule::eMod_RoomCenter) ;
			if ( pgameCenter->getPlayerOwnRoomCnt(GetPlayer()->GetUserUID()) >= getCreateRoomCntLimit() )
			{
				CLogMgr::SharedLogMgr()->PrintLog("uid = %u , create failed , already have room cnt = %u , limit = %u",GetPlayer()->GetUserUID(),pgameCenter->getPlayerOwnRoomCnt(GetPlayer()->GetUserUID()),getCreateRoomCntLimit()) ;
				jsMsgBack["ret"] = 1 ;
				SendMsg(jsMsgBack,nmsgType);
				break;
			}

			// if club room , have create club room permission or privilege ;
			if ( nClubID != 0 )
			{
				auto pg = (CGroup*)CGameServerApp::SharedGameServerApp()->getModuleByType(IGlobalModule::eMod_Group) ;
				auto group = pg->getGroupByID(nClubID) ;
				if ( group == nullptr || !group->isPlayerCanCreateRoom( GetPlayer()->GetUserUID() ) )
				{
					jsMsgBack["ret"] = 2 ;
					SendMsg(jsMsgBack,nmsgType);
					break;
				}
			}

			// async create room ;
			uint32_t nSerailNum = 0;
			uint32_t nNewRoomID = pgameCenter->generateRoomID(eroomType,nSerailNum) ;
			uint8_t nPortID = GetPlayer()->getMsgPortByRoomType(eroomType) ;
			recvValue["roomID"] = nNewRoomID ;
			recvValue["createUID"] = GetPlayer()->GetUserUID() ;
			recvValue["serialNum"] = nSerailNum ;

			Json::Value jsUserData ;
			jsUserData["roomID"] = nNewRoomID ;
			jsUserData["clubID"] = nClubID ;
			jsUserData["serialNum"] = nSerailNum ;
			GetPlayer()->startAsyncReq() ;
			CGameServerApp::SharedGameServerApp()->getAsynReqQueue()->pushAsyncRequest(nPortID,eAsync_CreateRoom,recvValue,[this,pgameCenter](uint16_t nReqType ,const Json::Value& retContent,Json::Value& jsUserData)
			{
				uint8_t nRet = retContent["ret"].asUInt() ;
				if ( nRet )
				{
					CLogMgr::SharedLogMgr()->ErrorLog("why create room not success ?") ;
				}
				GetPlayer()->endAsyncReq() ;
				
				// add game room to room center ;
				uint32_t nRoomID = jsUserData["roomID"].asUInt() ;
				uint32_t nClubID = jsUserData["clubID"].asUInt() ;

				auto pRoomItem = new CGameRoomCenter::stRoomItem ;
				pRoomItem->nBelongsToClubUID = nClubID;
				pRoomItem->nRoomID = nRoomID ;
				pRoomItem->nCreator = GetPlayer()->GetUserUID() ;
				pRoomItem->nSerialNumber = jsUserData["serialNum"].asUInt();
				pgameCenter->addRoomItem(pRoomItem,true);

				// send msg 
				Json::Value jsMsgBack ;
				jsMsgBack["ret"] = 0 ;
				jsMsgBack["clubID"] = nClubID ;
				jsMsgBack["roomID"] = nRoomID ;
				SendMsg(jsMsgBack,MSG_CREATE_ROOM);
				CLogMgr::SharedLogMgr()->PrintLog("uid = %u , create room ok ",GetPlayer()->GetUserUID()) ;

			},jsUserData) ;

		}
		break;
	case MSG_DELETE_ROOM:
		{
			if ( GetPlayer()->isWaitAsyncReq() )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("async is busy , try later uid = %u",GetPlayer()->GetUserUID() ) ;
				return true;
			}

			uint32_t nRoomID = recvValue["roomID"].asUInt() ;
			uint32_t nClubID = recvValue["clubID"].asUInt() ;
		
			Json::Value jsMsgBack ;

			auto pgameCenter = (CGameRoomCenter*)CGameServerApp::SharedGameServerApp()->getModuleByType(IGlobalModule::eMod_RoomCenter) ;
			auto pRoomItem = pgameCenter->getRoomItemByRoomID(nRoomID) ;
			// check self 
			if ( pRoomItem == nullptr || pRoomItem->nCreator != GetPlayer()->GetUserUID() )
			{
				jsMsgBack["ret"] = 1 ;
				SendMsg(jsMsgBack,MSG_DELETE_ROOM);
				return true ;
			}

			// check club 
			//if ( nClubID )
			//{
			//	auto pg = (CGroup*)CGameServerApp::SharedGameServerApp()->getModuleByType(IGlobalModule::eMod_Group) ;
			//	auto group = pg->getGroupByID(nClubID) ;
			//	if ( group == nullptr || group->isHaveRoomID(nClubID) == false )
			//	{
			//		jsMsgBack["ret"] = 3 ;
			//		SendMsg(jsMsgBack,MSG_DELETE_ROOM);
			//		return true ;
			//	}
			//}

			// do request 
			uint8_t nPortID = GetPlayer()->getMsgPortByRoomType(CGameRoomCenter::getRoomType(nRoomID)) ;
			GetPlayer()->startAsyncReq() ;
			CGameServerApp::SharedGameServerApp()->getAsynReqQueue()->pushAsyncRequest(nPortID,eAsync_DeleteRoom,recvValue,[this,pgameCenter](uint16_t nReqType ,const Json::Value& retContent,Json::Value& jsUserData)
			{
				GetPlayer()->endAsyncReq() ;

				Json::Value jsMsgBack ;
				uint8_t nRet = retContent["ret"].asUInt() ;
				jsMsgBack["ret"] = nRet ;
				SendMsg(jsMsgBack,MSG_DELETE_ROOM);

				if ( nRet )
				{
					CLogMgr::SharedLogMgr()->ErrorLog("delete error failed") ;
					return ;
				}
				

				uint32_t nRoomID = jsUserData["roomID"].asUInt() ;
				//uint32_t nClubID = jsUserData["clubID"].asUInt() ;
				pgameCenter->deleteRoomItem(nRoomID);
				/*deleteOwnRoom(nRoomID);
				if ( nClubID )
				{
				auto pg = (CGroup*)CGameServerApp::SharedGameServerApp()->getModuleByType(IGlobalModule::eMod_Group) ;
				auto group = pg->getGroupByID(nClubID) ;
				group->removeRoomID(nRoomID) ;
				}*/

				CLogMgr::SharedLogMgr()->PrintLog("uid = %u , removed room ok ",GetPlayer()->GetUserUID()) ;

			},recvValue) ;
		}
		break;
	default:
		return false ;
	}
	return true ;
}

bool CPlayerGameData::onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue)
{
	if ( IPlayerComponent::onCrossServerRequest(pRequest,eSenderPort,vJsValue) )
	{
		return true ;
	}

	switch ( pRequest->nRequestType )
	{
	case eCrossSvrReq_LeaveRoomRet:
		{
			m_nStateInRoomID = 0 ;
			GetPlayer()->GetBaseData()->setCoin(GetPlayer()->GetBaseData()->getTempCoin() + GetPlayer()->GetBaseData()->getCoin()) ;
			GetPlayer()->GetBaseData()->setTempCoin(0) ;
			CLogMgr::SharedLogMgr()->ErrorLog("uid = %d leave room state error ",GetPlayer()->GetUserUID() ) ;
		}
		break;
	default:
		return false ;
	}
	return true ;
}

bool CPlayerGameData::onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue )
{
	if ( IPlayerComponent::onCrossServerRequestRet(pResult,vJsValue) )
	{
		return true ;
	}

	//if ( eCrossSvrReq_CreateRoom == pResult->nRequestType  )
	//{
	//	stMsgCreateRoomRet msgBack ;
	//	msgBack.nRet = pResult->nRet ;
	//	msgBack.nRoomID = pResult->vArg[1];
	//	msgBack.nRoomType = pResult->vArg[2] ;
	//	msgBack.nFinalCoin = GetPlayer()->GetBaseData()->GetAllCoin() ;
	//	if ( pResult->nRet == 0 )
	//	{
	//		if ( eRoom_Max > msgBack.nRoomType )
	//		{
	//			addOwnRoom((eRoomType)msgBack.nRoomType,msgBack.nRoomID,pResult->vArg[0]) ;
	//		}
	//		else
	//		{
	//			CLogMgr::SharedLogMgr()->ErrorLog("add my own room , unknown room type = %d , uid = %d",msgBack.nRoomType,GetPlayer()->GetUserUID()) ;
	//		}
	//		CLogMgr::SharedLogMgr()->PrintLog("uid = %d , create room id = %d , config id = %d", GetPlayer()->GetUserUID(),msgBack.nRoomID,(int32_t)pResult->vArg[0] ) ;
	//	}
	//	else
	//	{
	//		CLogMgr::SharedLogMgr()->PrintLog("result create failed give back coin uid = %d",GetPlayer()->GetUserUID());

	//		CRoomConfigMgr* pConfigMgr = (CRoomConfigMgr*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Room);

	//		stTaxasRoomConfig* pRoomConfig = (stTaxasRoomConfig*)pConfigMgr->GetConfigByConfigID(pResult->vArg[0]) ;
	//		if ( pRoomConfig == nullptr )
	//		{
	//			CLogMgr::SharedLogMgr()->ErrorLog("fuck arument error must fix now , room config id , can not find") ;
	//			return true ;
	//		}

	//		GetPlayer()->GetBaseData()->AddMoney( pRoomConfig->nRentFeePerDay *  pResult->vArg[3]);
	//		msgBack.nFinalCoin = GetPlayer()->GetBaseData()->getCoin() ;
	//	}
	//	SendMsg(&msgBack,sizeof(msgBack)) ;
	//	return true ;
	//}

	if ( eCrossSvrReq_RoomProfit == pResult->nRequestType )
	{
		stMsgCaculateRoomProfitRet msgBack ;
		msgBack.nRoomID = pResult->nReqOrigID ;
		msgBack.bDiamond = !pResult->vArg[0] ;
		msgBack.nProfitMoney = pResult->vArg[1] ;
		msgBack.nRoomType = pResult->vArg[2] ;
		msgBack.nRet = pResult->nRet ? 3 : 0 ;
		SendMsg(&msgBack,sizeof(msgBack)) ;
		if ( msgBack.nRet )
		{
			return true ;
		}
		GetPlayer()->GetBaseData()->AddMoney(msgBack.nProfitMoney,msgBack.bDiamond);
		// save log 
		stMsgSaveLog msgLog ;
		memset(msgLog.vArg,0,sizeof(msgLog.vArg));
		msgLog.nJsonExtnerLen = 0 ;
		msgLog.nLogType = eLog_AddMoney ;
		msgLog.nTargetID = GetPlayer()->GetUserUID() ;
		msgLog.vArg[0] = !msgBack.bDiamond ;
		msgLog.vArg[1] = msgBack.nProfitMoney;
		msgLog.vArg[2] = GetPlayer()->GetBaseData()->GetData()->nCoin;
		msgLog.vArg[3] = GetPlayer()->GetBaseData()->GetData()->nDiamoned ;
		msgLog.vArg[4] = eCrossSvrReq_RoomProfit ;
		msgLog.vArg[5] = pResult->nReqOrigID ;
		CGameServerApp::SharedGameServerApp()->sendMsg(pResult->nReqOrigID,(char*)&msgLog,sizeof(msgLog));

		CLogMgr::SharedLogMgr()->PrintLog("uid = %d get profit = %llu",GetPlayer()->GetUserUID(),msgBack.nProfitMoney) ;
		return true ;
	}

	if ( eCrossSvrReq_AddRentTime == pResult->nRequestType )
	{
		stMsgAddRoomRentTimeRet msgRet ;
		msgRet.nRet = pResult->nRet ? 4 : 0  ;
		msgRet.nAddDays = pResult->vArg[0] ;
		msgRet.nRoomID = pResult->nReqOrigID ;
		msgRet.nRoomType = pResult->vArg[1] ;
		SendMsg(&msgRet,sizeof(msgRet)) ;
		if ( msgRet.nRet )
		{
			GetPlayer()->GetBaseData()->AddMoney(pResult->vArg[2]) ;
		}
		CLogMgr::SharedLogMgr()->PrintLog("uid = %d add rent time = %d",GetPlayer()->GetUserUID(),msgRet.nAddDays) ;
		return true ;
	}
	return false ;
}

void CPlayerGameData::OnPlayerDisconnect()
{
	IPlayerComponent::OnPlayerDisconnect();
	if ( isNotInAnyRoom() == false )
	{
		stMsgCrossServerRequest msgEnter ;
		msgEnter.cSysIdentifer = GetPlayer()->getMsgPortByRoomType(CGameRoomCenter::getRoomType(m_nStateInRoomID)) ;
		msgEnter.nJsonsLen = 0 ;
		msgEnter.nReqOrigID = GetPlayer()->GetUserUID();
		msgEnter.nRequestSubType = eCrossSvrReqSub_Default ;
		msgEnter.nRequestType = eCrossSvrReq_ApplyLeaveRoom ;
		msgEnter.nTargetID = m_nStateInRoomID ;
		msgEnter.vArg[0] = m_nStateInRoomID ;
		msgEnter.vArg[1] = GetPlayer()->GetSessionID() ;
		SendMsg(&msgEnter,sizeof(msgEnter)) ;
		CLogMgr::SharedLogMgr()->PrintLog("uid = %d disconnected , apply to leave room id = %d ",GetPlayer()->GetUserUID(),m_nStateInRoomID) ;
		GetPlayer()->delayDelete();
	}
}

void CPlayerGameData::OnOtherWillLogined()
{
	IPlayerComponent::OnOtherWillLogined();
	if ( isNotInAnyRoom() == false )
	{
		stMsgCrossServerRequest msgEnter ;
		msgEnter.cSysIdentifer = GetPlayer()->getMsgPortByRoomType(CGameRoomCenter::getRoomType(m_nStateInRoomID)) ;
		msgEnter.nJsonsLen = 0 ;
		msgEnter.nReqOrigID = GetPlayer()->GetUserUID();
		msgEnter.nRequestSubType = eCrossSvrReqSub_Default ;
		msgEnter.nRequestType = eCrossSvrReq_ApplyLeaveRoom ;
		msgEnter.nTargetID = m_nStateInRoomID ;
		msgEnter.vArg[0] = m_nStateInRoomID ;
		msgEnter.vArg[1] = GetPlayer()->GetSessionID() ;
		SendMsg(&msgEnter,sizeof(msgEnter)) ;

		CLogMgr::SharedLogMgr()->PrintLog("uid = %d other device login , apply to leave room id = %d ",GetPlayer()->GetUserUID(),m_nStateInRoomID) ;
	}
}

void CPlayerGameData::TimerSave()
{
	for ( uint8_t nIdx = eRoom_None; nIdx < eRoom_Max ; ++nIdx )
	{
		auto& gameData = m_vData[nIdx] ;
		if ( gameData.bDirty == false )
		{
			continue;
		}

		gameData.bDirty = false ;

		Json::Value jsValue ;
		jsValue["gameType"] = nIdx ;

		Json::Value jsData ;
		jsData["nWinTimes"] = gameData.nWinTimes ;
		jsData["nChampionTimes"] = gameData.nChampionTimes ;
		jsData["nPlayTimes"] = gameData.nPlayTimes ;
		jsData["nRun_upTimes"] = gameData.nRun_upTimes ;
		jsData["nSingleWinMost"] = (uint32_t)gameData.nSingleWinMost ;
		jsData["nThird_placeTimes"] = gameData.nThird_placeTimes ;

		Json::Value jsMaxCard ;
		for ( uint8_t nCardIdx = 0 ; nCardIdx < MAX_TAXAS_HOLD_CARD ; ++nCardIdx ) 
		{
			jsMaxCard[nCardIdx] = gameData.vMaxCards[nCardIdx] ;
		}
		jsData["vMaxCards"] = jsMaxCard ;
		jsValue["data"] = jsData ;

		Json::StyledWriter jsWrite ;
		std::string str = jsWrite.write(jsValue) ;

		stMsgSavePlayerGameData msgSave ;
		msgSave.nGameType = nIdx ;
		msgSave.nUserUID = GetPlayer()->GetUserUID() ;
		msgSave.nJsonLen = str.size() ;
		CAutoBuffer auBuffer ( sizeof(msgSave) + msgSave.nJsonLen );
		
		auBuffer.addContent(&msgSave,sizeof(msgSave)) ;
		auBuffer.addContent(str.c_str(),msgSave.nJsonLen) ;
		SendMsg((stMsg*)auBuffer.getBufferPtr(),auBuffer.getContentSize()) ;
	}
}

uint16_t CPlayerGameData::getCreateRoomCntLimit()
{
	return GetPlayer()->GetBaseData()->getMaxCanCreteRoomCount() ;
}

void CPlayerGameData::addPlayerGameRecorder(stPlayerGameRecorder* pRecorder , bool isSaveDB  )
{
#ifdef _DEBUG
	for ( auto pRec : m_vGameRecorders  )
	{
		if ( pRec->nRoomID == pRecorder->nRoomID )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("do not add duplicate room recorder player uid = %d",GetPlayer()->GetUserUID()) ;
			return ;
		}
	} 
#endif
	m_vGameRecorders.push_front(pRecorder);
	if ( !isSaveDB )
	{
		return;
	}

	// save player recorder to db ;
	stMsgSavePlayerGameRecorder msgSave ;
	msgSave.nDuiringSeconds = pRecorder->nDuiringSeconds ;
	msgSave.nFinishTime = pRecorder->nFinishTime ;
	msgSave.nOffset = pRecorder->nOffset ;
	msgSave.nRoomID = pRecorder->nRoomID ;
	msgSave.nRoomType = CGameRoomCenter::getRoomType(pRecorder->nRoomID); ;
	msgSave.nUserUID = GetPlayer()->GetUserUID() ;
	msgSave.nCreateUID = pRecorder->nCreateUID ;
	msgSave.nBuyIn = pRecorder->nBuyIn ;
	msgSave.nBaseBet = pRecorder->nBaseBet ; 
	msgSave.nClubID = pRecorder->nClubID ;
	memcpy(msgSave.cRoomName,pRecorder->cRoomName,sizeof(msgSave.cRoomName));
	SendMsg(&msgSave,sizeof(msgSave)) ;
	
	CLogMgr::SharedLogMgr()->PrintLog("save player game recorder room id = %u , baseBet = %d",pRecorder->nRoomID, pRecorder->nBaseBet) ;
}

void CPlayerGameData::sendGameDataToClient()
{
	// send niuniu data ;
	stMsgPlayerBaseDataNiuNiu msg ;
	memcpy(&msg.tNiuNiuData,&m_vData[eRoom_NiuNiu],sizeof(msg.tNiuNiuData));
	SendMsg(&msg,sizeof(msg)) ;

	// send taxas data 
	stMsgPlayerBaseDataTaxas msgT ;
	memcpy(&msgT.tTaxasData,&m_vData[eRoom_TexasPoker],sizeof(msgT.tTaxasData));
	SendMsg(&msgT,sizeof(msgT)) ;
}