#include "NiuNiuRoomManger.h"
#include "NiuNiuServer.h"
#include "NiuNiuRoom.h"
#include "NiuNiuMessageDefine.h"
#include "NiuNiuRoomPlayer.h"
#include "LogManager.h"
bool CNiuNiuRoomManager::init()
{
	IRoomManager::init();

	// temp create room ;
	IRoom* pRoom = doCreateInitedRoomObject(1,2,eRoom_NiuNiu);
	assert(pRoom&&"create room must success");
	if ( pRoom == nullptr )
	{
		return true ;
	}
	m_vRooms[pRoom->getRoomID()] = pRoom ;
	// temp code 
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
	default:
		break;
	}
	return false ;
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
	stMsgReadRoomInfo msgRead ;
	msgRead.nRoomType = eRoom_NiuNiu ;
	sendMsg(&msgRead,sizeof(msgRead),0) ;
	CLogMgr::SharedLogMgr()->PrintLog("read niu niu room info ") ;
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

	if ( vAcitveRooms.empty() )  // if all room is empty , then just rand a room to enter ;
	{
		vAcitveRooms.insert(vAcitveRooms.begin(),vRooms.begin(),vRooms.end()) ;
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