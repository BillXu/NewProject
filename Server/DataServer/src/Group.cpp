#include "Group.h"
#include "LogManager.h"
#include "ISeverApp.h"
#include "AsyncRequestQuene.h"
CGroup::~CGroup()
{
	for ( auto refItem : m_vGroups )
	{
		delete refItem.second ;
		refItem.second = nullptr ;
	}
	m_vGroups.clear() ;
}

void CGroup::init( IServerApp* svrApp )
{
	IGlobalModule::init(svrApp) ;
}

void CGroup::onConnectedSvr()
{
	Json::Value jssql ;
	uint32_t nOffset = m_vGroups.size() ;
	char pBuffer[512] = {0};
	sprintf(pBuffer,"select * from clubs where isDelete = 0 limit 20 offset %u",nOffset);
	jssql["sql"] = pBuffer ;
	getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DB,eAsync_DB_Select,jssql,[this](uint16_t nReqType ,const Json::Value& retContent,Json::Value& jsUserData){
		uint8_t nRow = retContent["afctRow"].asUInt() ;
		Json::Value jsData = retContent["data"];
		for ( uint8_t nIdx = 0 ; nIdx < jsData.size() ; ++nIdx )
		{
			Json::Value jsRow = jsData[nIdx];
			stGroupItem* prt = new stGroupItem ;
			prt->nCityCode = jsRow["cityCode"].asUInt() ;
			prt->nCreaterUID = jsRow["ownerUID"].asUInt() ;
			prt->nGroupID = jsRow["clubID"].asUInt() ;
			auto iter = m_vGroups.find(prt->nGroupID) ;
			if ( iter == m_vGroups.end() )
			{
				m_vGroups[prt->nGroupID] = prt ;
			}
			else
			{
				CLogMgr::SharedLogMgr()->ErrorLog("read from db , already have club id = %u",prt->nGroupID) ;
				delete prt ;
				prt = nullptr ;
			}
		}

		if ( nRow >= 20 ) // go on read more 
		{
			CLogMgr::SharedLogMgr()->PrintLog("go on reader more clubs") ;
			onConnectedSvr();
		}
	});

}

bool CGroup::onMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)
{
	if ( IGlobalModule::onMsg(prealMsg,eSenderPort,nSessionID) )
	{
		return true ;
	}
	return false ;
}

bool CGroup::onMsg(Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID)
{
	if ( IGlobalModule::onMsg(prealMsg,nMsgType,eSenderPort,nSessionID) )
	{
		return true ;
	}
	
	switch ( nMsgType )
	{
	case MSG_REQ_CLUB_ROOM:
		{
			Json::Value jsMsgBack ;
			jsMsgBack["ret"] = 0 ;
			auto pg = getGroupByID(prealMsg["clubID"].asUInt()) ;
			if ( pg == nullptr )
			{
				jsMsgBack["ret"] = 1 ;
				getSvrApp()->sendMsg(nSessionID,jsMsgBack,MSG_REQ_CLUB_ROOM) ;
				break ;
			}

			Json::Value jsRoomIDs ;
			for ( auto rid : pg->vAllRoomIDs )
			{
				jsRoomIDs[jsRoomIDs.size()] = rid ;
			}
			jsMsgBack["rooms"] = jsRoomIDs ;
			getSvrApp()->sendMsg(nSessionID,jsMsgBack,MSG_REQ_CLUB_ROOM) ;
		}
		break;
	default:
		return false ;
	}
	return true ;
}

stGroupItem* CGroup::getGroupByID(uint32_t nGroupID )
{
	auto iter = m_vGroups.find(nGroupID) ;
	if ( iter != m_vGroups.end() )
	{
		return iter->second ;
	}
	return nullptr ;
}

void CGroup::addGroup(stGroupItem* pItem )
{
	auto iter = m_vGroups.find(pItem->nGroupID) ;
	if ( iter != m_vGroups.end() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("already exist group id = %u  , can not add again",pItem->nGroupID) ; 
		delete pItem ;
		pItem = nullptr ;
		return ;
	}
	m_vGroups[pItem->nGroupID] = pItem ;

	Json::Value jssql ;
	char pBuffer[512] = {0};
	sprintf(pBuffer,"insert into clubs ( clubID,ownerUID,cityCode ) values ('%u','%u','%u') ;",pItem->nGroupID,pItem->nCreaterUID,pItem->nCityCode);
	jssql["sql"] = pBuffer ;
	getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DB,eAsync_DB_Add,jssql);
}

void CGroup::dismissGroup(uint32_t nGroupID )
{
	auto iter = m_vGroups.find(nGroupID) ;
	if ( iter != m_vGroups.end() )
	{
		delete iter->second ;
		iter->second = nullptr ;
		m_vGroups.erase(iter) ;

		// do delete
		Json::Value jssql ;
		char pBuffer[512] = {0};
		sprintf(pBuffer,"update clubs set isDelete = 1 where clubID = '%u';",nGroupID);
		jssql["sql"] = pBuffer;
		getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DB,eAsync_DB_Update,jssql);
		return ;
	}
	CLogMgr::SharedLogMgr()->ErrorLog("can not find dismiss group id = %u",nGroupID) ;
}

uint16_t CGroup::getClubCntByUserUID(uint32_t nUserUID)
{
	uint16_t nCnt = 0 ;
	for ( auto refIter : m_vGroups )
	{
		if ( refIter.second->nCreaterUID == nUserUID )
		{
			++nCnt ;
		}
	}
	return nCnt ;
}