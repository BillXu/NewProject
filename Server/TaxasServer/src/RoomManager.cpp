#include "RoomManager.h"
#include "TaxasServerApp.h"
#include "LogManager.h"
#include "TaxasRoom.h"
#include "ServerMessageDefine.h"
#include "RoomConfig.h"
#include "AutoBuffer.h"
#define ROOM_LIST_ITEM_CNT_PER_PAGE 5 
bool CRoomManager::onPublicMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)
{
	if ( ISitableRoomManager::onPublicMsg(prealMsg,eSenderPort,nSessionID) )
	{
		return true ;
	}

	switch ( prealMsg->usMsgType )
	{
	case MSG_TP_REQUEST_ROOM_LIST:
		{
			std::vector<IRoom*> vActiveRoom ;
			MAP_ID_ROOM::iterator iter = m_vRooms.begin() ;
			for ( ; iter != m_vRooms.end(); ++iter )
			{
				if ( iter->second->isRoomAlive() && iter->second->getOwnerUID() != MATCH_MGR_UID )
				{
					vActiveRoom.push_back(iter->second) ;
				}
			}

			uint16_t nPageCnt = ( vActiveRoom.size() + ROOM_LIST_ITEM_CNT_PER_PAGE -1  ) / ROOM_LIST_ITEM_CNT_PER_PAGE ; 
			for ( uint16_t nPageIdx = 0 ; nPageIdx < nPageCnt ; ++nPageIdx )
			{
				stMsgRequestRoomListRet msgRet ;
				msgRet.nRoomType = eRoom_TexasPoker ;
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

					CTaxasRoom* pRoom = (CTaxasRoom*)vActiveRoom[nRoomIdx];
					stRoomListItem stItem ;
					memset(&stItem,0,sizeof(stItem));
					stItem.nCreatOwnerUID = pRoom->getOwnerUID() ;
					stItem.nCurrentCount = pRoom->getPlayerCntWithState(eRoomPeer_SitDown);
					stItem.nRoomID = pRoom->getRoomID();
					stItem.nSmiallBlind = pRoom->getLittleBlind();
					stItem.nSeatCnt = pRoom->getSeatCount();
					stItem.nDeadTime = pRoom->getDeadTime();
					sprintf_s(stItem.vRoomName,sizeof(stItem.vRoomName),"%s",pRoom->getRoomName());
					//sprintf_s(stItem.vDesc,sizeof(stItem.vDesc),"%s",pRoom->getRoomDesc());
					auBuffer.addContent((char*)&stItem,sizeof(stItem)) ;
				}
				sendMsg((stMsg*)auBuffer.getBufferPtr(),auBuffer.getContentSize(),nSessionID);
			}

			if ( nPageCnt == 0 )
			{
				stMsgRequestRoomListRet msgRet ;
				msgRet.nRoomType = eRoom_TexasPoker ;
				msgRet.bFinal = true ;
				msgRet.nListCnt = 0 ;
				sendMsg(&msgRet,sizeof(msgRet),nSessionID) ;
			}
		}  
		break;
	default:
		return false;
	}
	return true ;
}

void CRoomManager::sendMsg(stMsg* pmsg, uint32_t nLen , uint32_t nSessionID )
{
	CTaxasServerApp::SharedGameServerApp()->sendMsg(nSessionID,(char*)pmsg,nLen) ;
}

IRoom* CRoomManager::doCreateInitedRoomObject(uint32_t nRoomID , uint16_t nRoomConfigID ,eRoomType reqSubRoomType, Json::Value& vJsValue)
{
	stSitableRoomConfig* pConfig = (stSitableRoomConfig*)CTaxasServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfigByConfigID(nRoomConfigID) ;
	if ( pConfig == nullptr )
	{
		return nullptr ;
	}

	IRoom* pR = doCreateRoomObject(reqSubRoomType);
	pR->init(pConfig,nRoomID,vJsValue) ;
	return pR ;
}

IRoom* CRoomManager::doCreateRoomObject(eRoomType reqSubRoomType)
{
	return new CTaxasRoom ;
}
