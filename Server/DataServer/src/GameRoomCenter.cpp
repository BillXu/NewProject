#include "GameRoomCenter.h"
#include "LogManager.h"
#include "ServerCommon.h"
#include "AsyncRequestQuene.h"
#include "ISeverApp.h"
CGameRoomCenter::~CGameRoomCenter()
{
	MAP_ROOM_ITEM m_vRoomIDKey ;
	for ( auto& refIter : m_vRoomIDKey )
	{
		delete refIter.second ;
		refIter.second = nullptr ;
	}
	m_vRoomIDKey.clear() ;
	m_vClubsOwner.clear();
	m_vPlayerOwners.clear() ;
	m_nCurSerailNum = 0 ;
	m_vWillUseRoomIDs.clear();
}

void CGameRoomCenter::init( IServerApp* svrApp )
{
	IGlobalModule::init(svrApp) ;
	m_isFinishedReading = false ;
}

void CGameRoomCenter::onConnectedSvr()
{
	// read max serail number 
	Json::Value jssql ;
	jssql["sql"] = "select max(serialNum) as 'maxSerial' from gameroomcenter ;"  ;
	getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DB,eAsync_DB_Select,jssql,[this](uint16_t nReqType ,const Json::Value& retContent,Json::Value& jsUserData){
		uint8_t nRow = retContent["afctRow"].asUInt() ;
		Json::Value jsData = retContent["data"];
		if ( nRow == 0 )
		{
			CLogMgr::SharedLogMgr()->PrintLog("do not read max serial number") ;
		}
		else
		{
			Json::Value jsRow = jsData[(uint32_t)0];
			m_nCurSerailNum = jsRow["maxSerial"].asUInt();
			CLogMgr::SharedLogMgr()->PrintLog("read max serial number = %u",m_nCurSerailNum);
		}
	});

	// read room item info from db svr ;
	readRoomItemsInfo();
}

void CGameRoomCenter::readRoomItemsInfo()
{
	Json::Value jssql ;
	uint32_t nOffset = m_vRoomIDKey.size() ;
	char pBuffer[512] = {0};
	sprintf(pBuffer,"select serialNum,roomID,belongClubID,creatorUID from gameroomcenter where isDelete = 0 order by createDate limit 20 offset %u",nOffset);
	jssql["sql"] = pBuffer ;
	getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DB,eAsync_DB_Select,jssql,[this](uint16_t nReqType ,const Json::Value& retContent,Json::Value& jsUserData){
		uint8_t nRow = retContent["afctRow"].asUInt() ;
		Json::Value jsData = retContent["data"];
		for ( uint8_t nIdx = 0 ; nIdx < jsData.size() ; ++nIdx )
		{
			Json::Value jsRow = jsData[nIdx];
			auto* prt = new stRoomItem ;
			prt->nSerialNumber = jsRow["serialNum"].asUInt() ;
			prt->nBelongsToClubUID = jsRow["belongClubID"].asUInt() ;
			prt->nCreator = jsRow["creatorUID"].asUInt() ;
			prt->nRoomID = jsRow["roomID"].asUInt() ;
			addRoomItem(prt,false) ;
		}

		if ( nRow >= 20 ) // go on read more 
		{
			CLogMgr::SharedLogMgr()->PrintLog("go on reader more clubs") ;
			readRoomItemsInfo();
		}
		else
		{
			m_isFinishedReading = true ;
		}
	});
}

bool CGameRoomCenter::onMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)
{
	return false ;
}

bool CGameRoomCenter::onMsg(Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID)
{
	return false ;
}

bool CGameRoomCenter::onAsyncRequest(uint16_t nRequestType , const Json::Value& jsReqContent, Json::Value& jsResult )
{
	switch ( nRequestType )
	{
	case eAsync_OnRoomDeleted:
		{
			uint32_t nRoomID = jsReqContent["roomID"].asUInt() ;
			//uint32_t nOwnerUID = jsReqContent["ownerUID"].asUInt() ;
			//uint32_t nClubID = jsReqContent["clubID"].asUInt() ;
			CLogMgr::SharedLogMgr()->PrintLog("room id = %u do deleted",nRoomID) ;
			deleteRoomItem(nRoomID);
		}
		break ;
	case eAsync_ReqRoomSerials:
		{
			if ( m_isFinishedReading == false )
			{
				jsResult["ret"] = 1 ;
				CLogMgr::SharedLogMgr()->PrintLog("still reading from db , please wait game center") ;
				break ;
			}

			jsResult["ret"] = 0 ;

			Json::Value jsRoomIDs ;
			uint32_t nRoomType = jsReqContent["roomType"].asUInt() ;
			auto iter = m_vRoomIDKey.begin();
			while ( m_vRoomIDKey.end() != (iter = std::find_if(iter,m_vRoomIDKey.end(),[nRoomType](MAP_ROOM_ITEM::value_type& refValue){ return getRoomType(refValue.first) == nRoomType; }) ) )
			{
				jsRoomIDs[jsRoomIDs.size()] = iter->second->nSerialNumber ;
				++iter ;
			}
			jsResult["serials"] = jsRoomIDs ;
			CLogMgr::SharedLogMgr()->PrintLog("req back roomType = %u , roomCnt = %u",nRoomType,jsRoomIDs.size()) ;
		}
		break ;
	default:
		return false ;
	}
	return true ;
}

void CGameRoomCenter::addRoomItem(stRoomItem* pItem , bool isNewAdd )
{
	auto iter = m_vRoomIDKey.find(pItem->nRoomID) ;
	if ( iter != m_vRoomIDKey.end() )
	{
		CLogMgr::SharedLogMgr()->PrintLog("already have this room id object id = %u, create by uid = %u ",pItem->nRoomID,iter->second->nCreator ) ;
		delete pItem ;
		pItem = nullptr ;
		return ;
	}
	m_vRoomIDKey[pItem->nRoomID] = pItem ;
	
	// add to player owners 
	addRoomItemToOwner(m_vPlayerOwners,pItem->nCreator,pItem->nRoomID);

	// add to club owners 
	if ( pItem->nBelongsToClubUID != 0 )
	{
		addRoomItemToOwner(m_vClubsOwner,pItem->nBelongsToClubUID,pItem->nRoomID);
	}

	// remove from will use ids 
	auto iter_willUse = std::find(m_vWillUseRoomIDs.begin(),m_vWillUseRoomIDs.end(),pItem->nRoomID);
	if ( iter_willUse != m_vWillUseRoomIDs.end() )
	{
		m_vWillUseRoomIDs.erase(iter_willUse) ;
	}
	else
	{
		if ( isNewAdd )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why this room id = %u , not in will use list ?",pItem->nRoomID) ;
		};
	}

	if ( isNewAdd ) // save to db 
	{
		CLogMgr::SharedLogMgr()->PrintLog("save this item to db ") ;
		Json::Value jssql ;
		char pBuffer[512] = {0};
		sprintf(pBuffer,"insert into gameroomcenter ( serialNum,roomID,roomType,belongClubID,creatorUID,createDate ) values ('%u','%u','%u','%u','%u',now());",pItem->nSerialNumber,pItem->nRoomID,getRoomType(pItem->nRoomID),pItem->nBelongsToClubUID,pItem->nCreator);
		jssql["sql"] = pBuffer ;
		getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DB,eAsync_DB_Add,jssql);
	}
}

void CGameRoomCenter::deleteRoomItem( uint32_t nRoomID )
{
	auto iter = m_vRoomIDKey.find(nRoomID) ;
	if ( iter == m_vRoomIDKey.end() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("can not find room id = %u to delete",nRoomID) ;
		return ;
	}

	uint32_t nClubID = iter->second->nBelongsToClubUID ;
	uint32_t nCreatorID = iter->second->nCreator ;
	uint32_t nSeailNum = iter->second->nSerialNumber ;

	delete iter->second ;
	iter->second = nullptr ;
	m_vRoomIDKey.erase(iter) ;

	// do delete from db ;
	Json::Value jssql ;
	char pBuffer[512] = {0};
	sprintf(pBuffer,"update gameroomcenter set isDelete = 1 where serialNum = '%u';",nSeailNum);
	jssql["sql"] = pBuffer;
	getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DB,eAsync_DB_Update,jssql);
	
	// delete from player owners ;
	bool b = deleteRoomItemFromOwner(m_vPlayerOwners,nCreatorID,nRoomID) ;
	if ( b == false )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("delete room from player owner uid = %u , roomId = %u , failed",nCreatorID,nRoomID) ;
	}

	if ( nClubID == 0 )
	{
		return ;
	}

	b = deleteRoomItemFromOwner(m_vClubsOwner,nClubID,nRoomID) ;
	if ( !b )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("delete room from nClub owner is failed club id = %u , room id = %u",nClubID,nRoomID) ;
	}
}

bool CGameRoomCenter::addRoomItemToOwner(MAP_ROOM_OWNERS& vOwners ,uint32_t nOwnerUID ,uint32_t nRoomID)
{
	auto iterOwner = vOwners.find(nOwnerUID);
	stRoomOwnerInfo* pOwnerInfo = nullptr ;
	if ( iterOwner == vOwners.end() )
	{
		pOwnerInfo = new stRoomOwnerInfo ;
		pOwnerInfo->nOwnerUID = nOwnerUID ;
		vOwners[pOwnerInfo->nOwnerUID] = pOwnerInfo ;
	}
	else
	{
		pOwnerInfo = iterOwner->second ;
	}

	auto iterRoomID = std::find(pOwnerInfo->vRoomIDs.begin(),pOwnerInfo->vRoomIDs.end(),nRoomID );
	if ( iterRoomID == pOwnerInfo->vRoomIDs.end() )
	{
		pOwnerInfo->vRoomIDs.push_back(nRoomID) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why already add this room id = %u",nRoomID) ;
	}
	return true ;
}

bool CGameRoomCenter::deleteRoomItemFromOwner(MAP_ROOM_OWNERS& vOwners ,uint32_t nOwnerUID , uint32_t nRoomID )
{
	// remove from owner
	auto iterOwner = vOwners.find(nOwnerUID) ;
	if (iterOwner == vOwners.end() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why do not have the creator uid = %u recorder in owner container ? roomID = %u " , nOwnerUID,nRoomID ) ;
		return false;
	}

	auto pOwner = iterOwner->second ;
	auto iterID = std::find(pOwner->vRoomIDs.begin(),pOwner->vRoomIDs.end(),nRoomID) ;
	if ( iterID != pOwner->vRoomIDs.end() )
	{
		pOwner->vRoomIDs.erase(iterID);
		if ( pOwner->vRoomIDs.empty() )
		{
			delete pOwner ;
			pOwner = nullptr ;
			vOwners.erase(iterOwner) ;
		}
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("uid = %u ,do not own room id = %u , why delete it ",nOwnerUID,nRoomID) ;
	}
	return true ;
}

uint8_t CGameRoomCenter::getRoomType(uint32_t nRoomID)
{
	uint32_t nTypeNumber = nRoomID / 100000 ;
	uint32_t nRealID = nRoomID % 100000 ;
	uint32_t narg = nRealID % 7 + 1;
	return (nTypeNumber - narg) ;
}

uint32_t CGameRoomCenter::generateRoomID(eRoomType eType,uint32_t& nserailNum )
{
	uint32_t nRoomID = 0 ;
	auto iter = m_vRoomIDKey.begin() ;
	auto iter_willUse = m_vWillUseRoomIDs.end();
	uint32_t nTryTimes = 0 ;
	nserailNum = ++m_nCurSerailNum ;
	do 
	{
		uint32_t nRealID = rand() % 100000 ;
		uint32_t narg = nRealID % 7 + 1;
		uint32_t nTypeNumber = eType + narg ;
		nRoomID = nTypeNumber * 100000 + nRealID ;	

		iter = m_vRoomIDKey.find(nRoomID) ;
		++nTryTimes ;
		if ( nTryTimes > 1 )
		{
			CLogMgr::SharedLogMgr()->PrintLog("try times = %u to generate room id ",nTryTimes);
		}

		if ( !m_vWillUseRoomIDs.empty() )
		{
			iter_willUse = std::find(m_vWillUseRoomIDs.begin(),m_vWillUseRoomIDs.end(),nRoomID) ;
		}
	}
	while (iter != m_vRoomIDKey.end() || iter_willUse != m_vWillUseRoomIDs.end() ) ;

	m_vWillUseRoomIDs.push_back(nRoomID) ;
	return nRoomID ;
}

CGameRoomCenter::stRoomItem* CGameRoomCenter::getRoomItemByRoomID(uint32_t nRoomID )
{
	auto iter = m_vRoomIDKey.find(nRoomID) ;
	if ( iter != m_vRoomIDKey.end() )
	{
		return iter->second ;
	}
	return nullptr ;
}

uint16_t CGameRoomCenter::getPlayerOwnRoomCnt(uint32_t nPlayerUID)
{
	auto iter = m_vPlayerOwners.find(nPlayerUID) ;
	if ( iter != m_vPlayerOwners.end() )
	{
		return iter->second->vRoomIDs.size() ;
	}
	return 0 ;
}

uint16_t CGameRoomCenter::getClubOwnRoomCnt(uint32_t nClubID )
{
	auto iter = m_vClubsOwner.find(nClubID) ;
	if ( iter != m_vClubsOwner.end() )
	{
		return iter->second->vRoomIDs.size(); 
	}
	return 0 ;
}

uint16_t CGameRoomCenter::getClubOwnRooms(std::vector<uint32_t>& vRoomIDs , uint32_t nClubID )
{
	auto iter = m_vClubsOwner.find(nClubID) ;
	if ( iter != m_vClubsOwner.end() )
	{
		auto pOwnInfor = iter->second ;
		vRoomIDs.clear();
		vRoomIDs.assign(pOwnInfor->vRoomIDs.begin(),pOwnInfor->vRoomIDs.end()) ;
		return vRoomIDs.size();
	}
	return 0 ;
}
