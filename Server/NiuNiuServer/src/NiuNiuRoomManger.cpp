#include "NiuNiuRoomManger.h"
#include "NiuNiuServer.h"
#include "NiuNiuRoom.h"
#include "NiuNiuMessageDefine.h"
#include "NiuNiuRoomPlayer.h"
#include "LogManager.h"
#define ROOM_LIST_ITEM_CNT_PER_PAGE 5 
#include "AutoBuffer.h"

bool CNiuNiuRoomManager::onPublicMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)
{
	if ( ISitableRoomManager::onPublicMsg(prealMsg,eSenderPort,nSessionID) )
	{
		return true ;
	}

	switch ( prealMsg->usMsgType )
	{
	case MSG_REQUEST_MATCH_ROOM_LIST:
		{
			auto iter = m_vCreatorAndRooms.find(MATCH_MGR_UID);
			if ( iter == m_vCreatorAndRooms.end() )
			{
				return true ;
			}
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
	default:
		return false;
	}
	return true ;
}

void CNiuNiuRoomManager::sendMsg(stMsg* pmsg, uint32_t nLen , uint32_t nSessionID )
{
	CNiuNiuServerApp::getInstance()->sendMsg(nSessionID,(char*)pmsg,nLen) ;
}

IRoom* CNiuNiuRoomManager::doCreateInitedRoomObject(uint32_t nRoomID , uint16_t nRoomConfigID ,eRoomType reqSubRoomType, Json::Value& vJsValue ) 
{
	stSitableRoomConfig* pConfig = (stSitableRoomConfig*)CNiuNiuServerApp::getInstance()->getRoomConfigMgr()->GetConfigByConfigID(nRoomConfigID) ;
	if ( pConfig == nullptr )
	{
		return nullptr ;
	}

	IRoom* pRoom = doCreateRoomObject(reqSubRoomType) ;
	pRoom->init(pConfig,nRoomID,vJsValue);
	return pRoom ;
}

IRoom* CNiuNiuRoomManager::doCreateRoomObject(eRoomType reqSubRoomType)
{
	IRoom* pRoom = new CNiuNiuRoom() ;
	return pRoom ;
}
