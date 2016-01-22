#include "NiuNiuRoomManger.h"
#include "NiuNiuServer.h"
#include "NiuNiuRoom.h"
#include "NiuNiuMessageDefine.h"
#include "NiuNiuRoomPlayer.h"
#include "LogManager.h"
#define ROOM_LIST_ITEM_CNT_PER_PAGE 5 
#include "AutoBuffer.h"
bool CNiuNiuRoomManager::init()
{
	IRoomManager::init();
	return true ;
}

bool CNiuNiuRoomManager::onPublicMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)
{
	if ( IRoomManager::onPublicMsg(prealMsg,eSenderPort,nSessionID) )
	{
		return true ;
	}

	switch ( prealMsg->usMsgType )
	{
	case MSG_REQUEST_MATCH_ROOM_LIST:
		{
			auto iter = m_vCreatorAndRooms.find(MATCH_MGR_UID);
			stRoomCreatorInfo& pC = iter->second ;
			LIST_ROOM& vMatchRooms = pC.vRooms ;
			stMsgRequestMatchRoomListRet msgRet ;
			msgRet.nItemCnt = vMatchRooms.size() ;
			msgRet.nRoomType = eRoom_NiuNiu ;
			if ( m_vRooms.empty() == false )
			{
				msgRet.nRoomType = m_vRooms.begin()->second->getRoomType();
			}

			CAutoBuffer buffer(sizeof(msgRet) + sizeof(stMsgMatchRoomItem) * msgRet.nItemCnt );
			buffer.addContent(&msgRet,sizeof(msgRet)) ;
			for ( IRoom* pRoom : vMatchRooms )
			{
				CNiuNiuRoom* pNiuRoom = (CNiuNiuRoom*)pRoom ;
				stMsgMatchRoomItem msgItem ;
				msgItem.nBaseBet = pNiuRoom->getBaseBet() ;
				msgItem.nEndTime = pRoom->getDeadTime();
				msgItem.nChapionUID = 0 ;
				pRoom->sortRoomRankItem();
				auto firt = pRoom->getSortRankItemListBegin();
				if ( firt != pRoom->getSortRankItemListEnd() )
				{
					msgItem.nChapionUID = (*firt)->nUserUID ;
				}
				msgItem.nRoomID = pRoom->getRoomID() ;
				memset(msgItem.pRoomName,0,sizeof(msgItem.pRoomName)) ;
				memcpy(msgItem.pRoomName,pRoom->getRoomName(),strlen(pRoom->getRoomName()));
				buffer.addContent(&msgItem,sizeof(msgItem));
			}
			sendMsg((stMsg*)buffer.getBufferPtr(),buffer.getContentSize(),nSessionID) ;
		}
		break;
	case MSG_REQUEST_ROOM_LIST:
		{
			std::vector<IRoom*> vActiveRoom ;
			MAP_ID_ROOM::iterator iter = m_vRooms.begin() ;
			for ( ; iter != m_vRooms.end(); ++iter )
			{
				if ( iter->second->isRoomAlive() )
				{
					vActiveRoom.push_back(iter->second) ;
				}
			}

			uint16_t nPageCnt = ( vActiveRoom.size() + ROOM_LIST_ITEM_CNT_PER_PAGE -1  ) / ROOM_LIST_ITEM_CNT_PER_PAGE ; 
			for ( uint16_t nPageIdx = 0 ; nPageIdx < nPageCnt ; ++nPageIdx )
			{
				stMsgRequestRoomListRet msgRet ;
				msgRet.nRoomType = eRoom_NiuNiu ;
				msgRet.bFinal = nPageIdx == (nPageCnt - 1 );
				msgRet.nListCnt = msgRet.bFinal ? ( vActiveRoom.size() - ( ROOM_LIST_ITEM_CNT_PER_PAGE * nPageIdx ) ) : ROOM_LIST_ITEM_CNT_PER_PAGE ;
				CAutoBuffer auBuffer(sizeof(msgRet) + sizeof(stRoomListItem) * msgRet.nListCnt );
				auBuffer.addContent((char*)&msgRet,sizeof(msgRet)) ;
				for ( uint8_t nIdx = 0 ; nIdx < ROOM_LIST_ITEM_CNT_PER_PAGE; ++nIdx )
				{
					uint16_t nRoomIdx = nPageIdx*ROOM_LIST_ITEM_CNT_PER_PAGE + nIdx ;
					if ( nRoomIdx >= vActiveRoom.size() )
					{
						break;
					}

					CNiuNiuRoom* pRoom = (CNiuNiuRoom*)vActiveRoom[nRoomIdx];
					stRoomListItem stItem ;
					memset(&stItem,0,sizeof(stItem));
					stItem.nCreatOwnerUID = pRoom->getOwnerUID() ;
					stItem.nCurrentCount = pRoom->getPlayerCntWithState(eRoomPeer_SitDown);
					stItem.nRoomID = pRoom->getRoomID();
					stItem.nSmiallBlind = pRoom->getBaseBet();
					stItem.nSeatCnt = pRoom->getSeatCount();
					stItem.nDeadTime = pRoom->getDeadTime();
					sprintf_s(stItem.vRoomName,sizeof(stItem.vRoomName),"%s",pRoom->getRoomName());
					//sprintf_s(stItem.vDesc,sizeof(stItem.vDesc),"%s",pRoom->getRoomDesc());
					auBuffer.addContent((char*)&stItem,sizeof(stItem)) ;
				}
				sendMsg((stMsg*)auBuffer.getBufferPtr(),auBuffer.getContentSize(),nSessionID);
				CLogMgr::SharedLogMgr()->PrintLog("send msg niuniu room list ") ;
			}

			if ( nPageCnt == 0 )
			{
				stMsgRequestRoomListRet msgRet ;
				msgRet.nRoomType = eRoom_NiuNiu ;
				msgRet.bFinal = true ;
				msgRet.nListCnt = 0 ;
				sendMsg(&msgRet,sizeof(msgRet),nSessionID) ;
				CLogMgr::SharedLogMgr()->PrintLog("send msg niuniu room list zero") ;
			}
		}  
		break;
	default:
		return false;
	}
	return true ;
}

void CNiuNiuRoomManager::sendMsg(stMsg* pmsg, uint32_t nLen , uint32_t nSessionID )
{
	CNiuNiuServerApp::getInstance()->sendMsg(nSessionID,(char*)pmsg,nLen) ;
}

bool CNiuNiuRoomManager::onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue )
{
	if ( IRoomManager::onCrossServerRequest(pRequest,eSenderPort,vJsValue) )
	{
		return true ;
	}

	switch ( pRequest->nRequestType )
	{
	case eCrossSvrReq_EnterRoom:
		{
			stMsgCrossServerRequestRet resultBack ;
			FILL_CROSSE_REQUEST_BACK(resultBack,pRequest,eSenderPort);
			resultBack.vArg[1] = eRoom_NiuNiu ;
			resultBack.vArg[2] = pRequest->vArg[1] ;
			IRoom* pRoom = nullptr ;
			if ( resultBack.vArg[3] == 0 )
			{
				pRoom = GetRoomByID(pRequest->vArg[1]) ;
			}
			else
			{
				pRoom = getRoomByConfigID(pRequest->vArg[1]) ;
			}

			if ( pRoom == nullptr )
			{
				resultBack.nRet = 1 ;
				sendMsg(&resultBack,sizeof(resultBack),0) ;
				CLogMgr::SharedLogMgr()->ErrorLog("can not find room id = %d , for uid = %d to enter",(uint32_t)pRequest->vArg[1],pRequest->nTargetID) ;
				return true ;
			}
			resultBack.vArg[2] = pRoom->getRoomID() ;

			IRoomPlayer* pPlayer = pRoom->getReusePlayerObject();
			pPlayer->setSessionID(pRequest->vArg[0]) ;
			pPlayer->setUserUID(pRequest->nReqOrigID);
			pPlayer->setCoin(pRequest->vArg[2]) ;
			if ( pRoom->addRoomPlayer(pPlayer) == false )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("this peer already in this room uid = %d",pPlayer->getUserUID()) ;
				delete pPlayer ;
				pPlayer = nullptr ;
				resultBack.nRet = 1 ;
				sendMsg(&resultBack,sizeof(resultBack),0) ;
				return true ;
			}
			pRoom->sendRoomInfoToPlayer(pPlayer->getSessionID());
			sendMsg(&resultBack,sizeof(resultBack),0) ;
			CLogMgr::SharedLogMgr()->PrintLog("send niuniu room info to player uid = %d",pPlayer->getUserUID()) ;
		}
		break;
	default:
		//CLogMgr::SharedLogMgr()->PrintLog("un processed request type = %d",pRequest->nRequestType) ;
		return false;
	}
	return true ;
}

bool CNiuNiuRoomManager::onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue )
{
	if ( IRoomManager::onCrossServerRequestRet(pResult,vJsValue) )
	{
		return true ;
	}
	CLogMgr::SharedLogMgr()->PrintLog("un processed requestRet type = %d",pResult->nRequestType) ;
	return false ;
}

void CNiuNiuRoomManager::onConnectedToSvr()
{
	if ( m_vRooms.empty() )
	{
		stMsgReadRoomInfo msgRead ;
		msgRead.nRoomType = eRoom_NiuNiu ;
		sendMsg(&msgRead,sizeof(msgRead),0) ;
		CLogMgr::SharedLogMgr()->PrintLog("read niu niu room info ") ;
	}
}

IRoom* CNiuNiuRoomManager::doCreateInitedRoomObject(uint32_t nRoomID , uint16_t nRoomConfigID ,eRoomType reqSubRoomType ) 
{
	stSitableRoomConfig* pConfig = (stSitableRoomConfig*)CNiuNiuServerApp::getInstance()->getRoomConfigMgr()->GetConfigByConfigID(nRoomConfigID) ;
	if ( pConfig == nullptr )
	{
		return nullptr ;
	}

	IRoom* pRoom = new CNiuNiuRoom() ;
	pRoom->init(pConfig,nRoomID);
	return pRoom ;
}

IRoom* CNiuNiuRoomManager::getRoomByConfigID(uint32_t nRoomConfigID )
{
	MAP_CONFIG_ROOMS::iterator iter = m_vCongfigIDRooms.find(nRoomConfigID) ;
	if ( iter == m_vCongfigIDRooms.end() )
	{
		return nullptr ;
	}

	LIST_ROOM& vRooms = iter->second ;
	if ( vRooms.empty() )
	{
		return nullptr ;
	}

	LIST_ROOM vAcitveRooms ;
	LIST_ROOM vEmptyRooms ;
	for ( IRoom* pRoom : vRooms )
	{
		if ( pRoom == nullptr || pRoom->isRoomAlive() == false )
		{
			continue; 
		}

		if ( ((ISitableRoom*)pRoom)->getPlayerCntWithState(eRoomPeer_SitDown) )
		{
			vAcitveRooms.push_back(pRoom) ;
		}
		else
		{
			vEmptyRooms.push_back(pRoom) ;
		}
	}

	if ( vAcitveRooms.empty() && vEmptyRooms.empty() )
	{
		return nullptr ;
	}

	if ( vAcitveRooms.empty() )  // if all room is empty , then just rand a room to enter ;
	{
		vAcitveRooms.insert(vAcitveRooms.begin(),vEmptyRooms.begin(),vEmptyRooms.end()) ;
	}
	else if ( vAcitveRooms.size() <= 10 )  // put some empty rooms in 
	{
		uint8_t naddEmtpy = 0 ;
		for ( IRoom* pRoom : vEmptyRooms )
		{
			if ( naddEmtpy > 8 )
			{
				break; ;
			}

			if ( naddEmtpy % 2 == 0 )
			{
				vAcitveRooms.push_back(pRoom) ;
			}
			else
			{
				vAcitveRooms.insert(vAcitveRooms.begin(),pRoom) ;
			}

			++naddEmtpy ;
		}
	}

	uint16_t nStartIdx = rand() % vAcitveRooms.size() ;
	uint16_t iter_idx = 0 ;
	for( IRoom* pRoom : vAcitveRooms )
	{
		if ( iter_idx != nStartIdx )
		{
			++iter_idx ;
			continue;
		}

		return pRoom ;

	}
	return nullptr ;
}