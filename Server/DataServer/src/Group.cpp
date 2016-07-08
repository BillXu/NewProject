#include "Group.h"
#include "LogManager.h"
#include "ISeverApp.h"
#include "AsyncRequestQuene.h"
#include "GameServerApp.h"
#include "GameRoomCenter.h"
#include "PlayerBaseData.h"
#include "Player.h"
#include "PlayerMail.h"
//#define  REQ_PAGE_MEMBER_CNT_OF_CLUB 30

struct stClubMemberChangeReqArg
{
	stGroupItem* pOwenClub ;
	uint32_t nTargetUID ;
	uint32_t nMsgType ;
};

bool stGroupItem::isRoomKeepRunning()
{
	auto pgameCenter = (CGameRoomCenter*)CGameServerApp::SharedGameServerApp()->getModuleByType(IGlobalModule::eMod_RoomCenter) ;
	return pgameCenter->getClubOwnRoomCnt(nGroupID) > 0 ;
}

bool stGroupItem::isGroupFull()
{
	return getMemberCnt() >= getCapacity() ; 
}

uint32_t stGroupItem::getCapacity()
{
	if ( m_tLevelRunOutTime )
	{
		uint32_t nNow = (uint32_t)time(nullptr) ;
		if ( nNow >= m_tLevelRunOutTime )
		{
			nLevel = 0 ;
			m_tLevelRunOutTime = 0 ;
			isDirty = true ;
		}
	}

	switch (nLevel)
	{
	case 0:
		return 40 ;
	case 1 :
		return 60 ;
	case 2:
		return 80 ;
	case 3:
		return 100 ;
	case 4:
		return 120 ;
	case 5:
		return 150 ;
	case 6:
		return 200 ;
	default:
		return 200 ;
	}
	return 40;
}

void stGroupItem::addMember(uint32_t nMemberUID )
{
#ifdef _DEBUG
	if ( isHaveMember(nMemberUID) )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("already have member uid = %u , why add twice ? group id = %u",nMemberUID,nGroupID) ;
		return ;
	}
#endif // _DEBUG
	vMembers.push_back(nMemberUID) ;
}

void stGroupItem::removeMember(uint32_t nMemberUID )
{
	auto iter = std::find(vMembers.begin(),vMembers.end(),nMemberUID);
	if ( iter != vMembers.end() )
	{
		vMembers.erase(iter);
	}
}

bool stGroupItem::isHaveMember(uint32_t nMemberUID )
{
	return std::find(vMembers.begin(),vMembers.end(),nMemberUID) != vMembers.end() ;
}

uint32_t stGroupItem::getMemberCnt() 
{
	return vMembers.size() ;
}

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
	m_pGoTyeAPI.init("https://qplusapi.gotye.com.cn:8443/api/");
	m_pGoTyeAPI.setDelegate(this);
}

void CGroup::onConnectedSvr()
{
	Json::Value jssql ;
	uint32_t nOffset = m_vGroups.size() ;
	char pBuffer[512] = {0};
	sprintf(pBuffer,"select * from clubs where isDelete = '0' limit 20 offset %u ",nOffset);
	std::string str = pBuffer ;
	jssql["sql"] = pBuffer ;
	getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DB,eAsync_DB_Select,jssql,[this](uint16_t nReqType ,const Json::Value& retContent,Json::Value& jsUserData){
		uint8_t nRow = retContent["afctRow"].asUInt() ;
		Json::Value jsData = retContent["data"];
		for ( uint8_t nIdx = 0 ; nIdx < jsData.size() ; ++nIdx )
		{
			Json::Value jsRow = jsData[nIdx];
			stGroupItem* prt = new stGroupItem() ;
			prt->nCityCode = jsRow["cityCode"].asUInt() ;
			prt->nCreaterUID = jsRow["ownerUID"].asUInt() ;
			prt->nGroupID = jsRow["clubID"].asUInt() ;
			prt->nLevel = jsRow["level"].asUInt() ;
			prt->m_tLevelRunOutTime = jsRow["deadtime"].asUInt() ;
			//prt->nCurCnt = jsRow["curCnt"].asUInt() ;
			prt->isCntDirty = true ;
			auto iter = m_vGroups.find(prt->nGroupID) ;
			if ( iter == m_vGroups.end() )
			{
				m_vGroups[prt->nGroupID] = prt ;

				CLogMgr::SharedLogMgr()->PrintLog("request room members group id = %u",prt->nGroupID) ;
				reqGroupMembers(prt) ;
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
		else
		{
			// temp set ;
			//onTimeSave() ;
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
	case MSG_REQ_PLAYER_JOINED_CLUBS:
		{
			uint32_t nReqUID = prealMsg["uid"].asUInt() ;

			Json::Value jsJoinedClubs ;
			for ( auto vclub : m_vGroups )
			{
				if ( vclub.second->isHaveMember(nReqUID) )
				{
					jsJoinedClubs[jsJoinedClubs.size()] = vclub.second->nGroupID ;
				}
			}
			prealMsg["clubIDs"] = jsJoinedClubs ;
			getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType) ;
		}
		break;
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
			auto pgameCenter = (CGameRoomCenter*)CGameServerApp::SharedGameServerApp()->getModuleByType(IGlobalModule::eMod_RoomCenter) ;
			std::vector<uint32_t> vAllRoomIDs ;
			pgameCenter->getClubOwnRooms(vAllRoomIDs,pg->nGroupID) ;
			for ( auto rid : vAllRoomIDs )
			{
				jsRoomIDs[jsRoomIDs.size()] = rid ;
			}
			jsMsgBack["rooms"] = jsRoomIDs ;
			getSvrApp()->sendMsg(nSessionID,jsMsgBack,MSG_REQ_CLUB_ROOM) ;
		}
		break;
	case MSG_REQ_CLUB_INFO:
		{
			uint32_t nGroupID = prealMsg["groupID"].asUInt() ;
			auto pGroup = getGroupByID(nGroupID) ;
			if ( !pGroup )
			{
				prealMsg["ret"] = 1 ;
				getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType) ;
				break ;
			}

			prealMsg["ret"] = 0 ;
			prealMsg["curCnt"] = pGroup->getMemberCnt() ;
			prealMsg["capacity"] = pGroup->getCapacity();
			prealMsg["level"] = pGroup->nLevel ;
			prealMsg["deadTime"] = pGroup->m_tLevelRunOutTime ;
			getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType) ;
		}
		break;
	case MSG_CLUB_ADD_MEMBER:
	case MSG_CLUB_DELETE_MEMBER:
		{
			uint32_t nGroupID = prealMsg["groupID"].asUInt();
			uint32_t nAccountUID = prealMsg["userAccount"].asUInt() ;
			auto pClub = getGroupByID(nGroupID) ;
			prealMsg["ret"] = 0 ;
			if ( pClub == nullptr )
			{
				prealMsg["ret"] = 1 ;
				getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType) ;
				break ;
			}

			// do add member 
			Json::Value cValue ;
			cValue["email"] = "378569952@qq.com" ;
			cValue["devpwd"] = "bill007" ;
#ifdef _DEBUG
			cValue["appkey"] = "e87f31bb-e86c-4d87-a3f3-57b3da76b3d6";
#else
			cValue["appkey"] = "abffee4b-deea-4e96-ac8d-b9d58f246c3f" ;
#endif // DEBUG

			//cValue["appkey"] = "abffee4b-deea-4e96-ac8d-b9d58f246c3f" ;
			cValue["group_id"] = std::to_string(nGroupID);
			cValue["user_account"] = std::to_string(nAccountUID);
			Json::StyledWriter sWrite ;
			std::string str = sWrite.write(cValue);

			if ( nMsgType == MSG_CLUB_ADD_MEMBER )
			{
				if ( pClub->isGroupFull() || pClub->isHaveMember(nAccountUID) )
				{
					prealMsg["ret"] = 3 ;
					getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType) ;
					CLogMgr::SharedLogMgr()->PrintLog("group is full") ;
					break ;
				}

				stClubMemberChangeReqArg* pArg = new stClubMemberChangeReqArg ;
				pArg->nMsgType = nMsgType ;
				pArg->nTargetUID = nAccountUID;
				pArg->pOwenClub = pClub ;

				m_pGoTyeAPI.performRequest("AddGroupMember",str.c_str(),str.size(),pArg,eReq_AddMember );
				CLogMgr::SharedLogMgr()->PrintLog("add member accountUid  = %u",nAccountUID) ;
			}
			else
			{
				if ( pClub->isHaveMember(nAccountUID) == false )
				{
					prealMsg["ret"] = 2 ;
					getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType) ;
					break;
				}

				stClubMemberChangeReqArg* pArg = new stClubMemberChangeReqArg ;
				pArg->nMsgType = nMsgType ;
				pArg->nTargetUID = nAccountUID;
				pArg->pOwenClub = pClub ;

				m_pGoTyeAPI.performRequest("DelGroupMember",str.c_str(),str.size(),pArg,eReq_DeleteMember );
				CLogMgr::SharedLogMgr()->PrintLog("delete member accountUid  = %u",nAccountUID) ;
			}
			getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType) ;

			// tell result ;
			eNoticeType eType = nMsgType == MSG_CLUB_ADD_MEMBER ? eNotice_BeAddedToClub : eNotice_BeRemoveFromClub ;
			Json::Value jsArg;
			jsArg["clubID"] = nGroupID;
			CPlayerMailComponent::PostDlgNotice(eType,jsArg,nAccountUID) ;
			CLogMgr::SharedLogMgr()->PrintLog("club member changed dlg notice etype = %u ,targetUID = %u",eType,nAccountUID) ;
		}
		break ;
	case MSG_REQ_LEVEL_UP_CLUB:
		{
			static uint32_t vLevelPriceDiamon[] = { 15,30,3000,4500,7200 } ;
			
			uint32_t nclubID = prealMsg["clubID"].asUInt();
			uint32_t nLevel = prealMsg["level"].asUInt() ;
			auto pClub = getGroupByID(nclubID) ;
			if ( !pClub )
			{
				prealMsg["ret"] = 1 ;
				getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType) ;
				break ;
			}

			pClub->getCapacity() ; // update the level and time;
			uint16_t nTargetLevel = prealMsg["level"].asUInt() ;
			if ( nTargetLevel == 0 || nTargetLevel < pClub->nLevel )
			{
				prealMsg["ret"] = 4 ;
				getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType) ;
				break ;
			}

			if ( nTargetLevel > sizeof(vLevelPriceDiamon) / sizeof(uint32_t) )
			{
				prealMsg["ret"] = 3 ;
				getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType) ;
				break ;
			}

			uint32_t nDiamondPrice = vLevelPriceDiamon[nTargetLevel-1] ;
			auto pPlayer = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetPlayerBySessionID(nSessionID) ;
			if ( !pPlayer )
			{
				prealMsg["ret"] = 3 ;
				getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType) ;
				CLogMgr::SharedLogMgr()->ErrorLog("session id = %u not online , so can not do this operate") ;
				break ;
			}

			if ( pPlayer->GetBaseData()->GetAllDiamoned() < nDiamondPrice )
			{
				prealMsg["ret"] = 2 ;
				getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType) ;
				break ;
			}

			pPlayer->GetBaseData()->decressMoney(nDiamondPrice,true) ;
			uint32_t nAddDuiringTime = 15 * 24 * 60 * 60;
			if ( pClub->nLevel == nTargetLevel )
			{
				pClub->m_tLevelRunOutTime += nAddDuiringTime;
			}
			else
			{
				pClub->m_tLevelRunOutTime = (uint32_t)time(nullptr) + nAddDuiringTime ;
			}
			pClub->nLevel = nTargetLevel ;
			pClub->isDirty = true ;

			prealMsg["ret"] = 0 ;
			prealMsg["deadTime"] = pClub->m_tLevelRunOutTime ;
			getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType) ;
			break ;
		}
		break ;
	case MSG_REQ_RECORMED_CLUB:
		{
			Json::Value jsArray ;
			auto iter = m_vGroups.rbegin();
			for ( ; iter != m_vGroups.rend() ; ++iter )
			{               
				jsArray[jsArray.size()] = iter->second->nGroupID;
				if ( jsArray.size() >= 10 )
				{
					break; 
				}
			}

			prealMsg["clubIDs"] = jsArray ;
			getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType) ;
		}
		break ;
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
	sprintf(pBuffer,"insert into clubs ( clubID,ownerUID,cityCode,createDate ) values ('%u','%u','%u',now()) ;",pItem->nGroupID,pItem->nCreaterUID,pItem->nCityCode);
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

void CGroup::onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData , size_t nUserTypeArg)
{
	bool bSuccess = nDatalen > 0 ;
	Json::Value jsResult ;
	if ( bSuccess )
	{
		Json::Reader reader ;
		reader.parse(pResultData,pResultData + nDatalen,jsResult) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("club gotyp request failed type = %u ",nUserTypeArg) ;
		return ;
	}

	if ( jsResult["errcode"].asUInt() != 200 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("club gotype request failed error code = %u type = %u", jsResult["errcode"].asUInt(),nUserTypeArg) ;
		return ;
	}

	if ( eReq_DeleteMember == nUserTypeArg )
	{
		stClubMemberChangeReqArg* pArg = (stClubMemberChangeReqArg*)pUserData ;
		auto pClub = (stGroupItem*)pArg->pOwenClub;
		uint32_t nUID = pArg->nTargetUID ;
		pClub->removeMember(nUID);
		delete pArg  ;
		pArg = nullptr ;
		 
		CLogMgr::SharedLogMgr()->PrintLog("delete club member ok, uid = %u ",nUID) ;
	}
	else if ( eReq_AddMember == nUserTypeArg )
	{
		stClubMemberChangeReqArg* pArg = (stClubMemberChangeReqArg*)pUserData ;
		auto pClub = (stGroupItem*)pArg->pOwenClub;
		uint32_t nUID = pArg->nTargetUID ;
		pClub->addMember(nUID);
		delete pArg  ;
		pArg = nullptr ;
		CLogMgr::SharedLogMgr()->PrintLog("add club member ok uid = %u ",nUID) ;
	}
	else if ( eReq_RefreshCnt == nUserTypeArg )
	{
		//Json::Value jsGropList = jsResult["group_list"];
		//for ( uint32_t nIdx = 0 ; nIdx < jsGropList.size() ; ++nIdx )
		//{
		//	auto jsginfo = jsGropList[nIdx] ;
		//	uint32_t nClubID = jsginfo["group_id"].asUInt() ;
		//	uint32_t nCnt = jsginfo["number"].asUInt() ;
		//	CLogMgr::SharedLogMgr()->PrintLog("refresh cnt result club id = %u, curCnt = %u",nClubID,nCnt) ;

		//	auto gGr = getGroupByID(nClubID) ;
		//	if ( gGr && gGr->nCurCnt != nCnt )
		//	{
		//		gGr->nCurCnt = nCnt ;
		//		gGr->isDirty = true ;
		//	}

		//	if ( gGr != nullptr && (gGr->isCntDirty = false ) ){}
		//}
	}
	else if ( eReq_GroupMembers == nUserTypeArg )
	{
		auto pClub = (stGroupItem*)pUserData;
		Json::Value jsMembers = jsResult["user_list"] ;
		for ( uint16_t nIdx = 0 ; nIdx < jsMembers.size() ; ++nIdx )
		{
			auto jsM = jsMembers[nIdx] ;
			uint32_t nUID = atoi(jsM.asCString()) ;
			CLogMgr::SharedLogMgr()->PrintLog("req member to add group id = %u , uid = %u",pClub->nGroupID,nUID);
			pClub->addMember(nUID) ;
		}

		//if ( jsMembers.size() >= REQ_PAGE_MEMBER_CNT_OF_CLUB )
		//{
		//	CLogMgr::SharedLogMgr()->PrintLog("req next page member cnt for club id = %u",pClub->nGroupID) ;
		//	reqGroupMembers(pClub) ;
		//}
	}
	else 
	{
		CLogMgr::SharedLogMgr()->PrintLog("unknown club req type") ;
	}
}

void CGroup::onTimeSave()
{
	char pBuffer[200] = { 0 } ;
	//Json::Value jsreqlist ;
	for ( auto ref : m_vGroups )
	{
		auto pG = ref.second ;

		//if ( pG->isCntDirty )
		//{
		//	jsreqlist[jsreqlist.size()] = pG->nGroupID ; //std::to_string(pG->nGroupID) ;
		//}

		if ( pG->isDirty == false )
		{
			continue;
		}
		memset(pBuffer,0,sizeof(pBuffer));
		Json::Value jsReq ;
		sprintf_s(pBuffer,sizeof(pBuffer),"update clubs set level = '%u' , deadtime = '%u' , curCnt = '%u' where clubID = '%u' limit 1;",pG->nLevel,pG->m_tLevelRunOutTime,pG->getMemberCnt(),pG->nGroupID);
		jsReq["sql"] = pBuffer ;
		getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DB,eAsync_DB_Update,jsReq);
		pG->isDirty = false ;
	}

	//if ( jsreqlist.empty() )
	//{
	//	return ;
	//}
//	// refresh cnt ;
//	Json::Value cValue ;
//	cValue["email"] = "378569952@qq.com" ;
//	cValue["devpwd"] = "bill007" ;
//#ifdef _DEBUG
//	cValue["appkey"] = "e87f31bb-e86c-4d87-a3f3-57b3da76b3d6";
//#else
//	cValue["appkey"] = "abffee4b-deea-4e96-ac8d-b9d58f246c3f" ;
//#endif // DEBUG
//	cValue["group_id_list"] = jsreqlist;
//	Json::StyledWriter sWrite ;
//	std::string str = sWrite.write(cValue);
//	m_pGoTyeAPI.performRequest("GetGroupDetail",str.c_str(),str.size(),nullptr,eReq_RefreshCnt );
}

void CGroup::reqGroupMembers(stGroupItem* pGroup )
{
	// refresh cnt ;
	Json::Value cValue ;
	cValue["email"] = "378569952@qq.com" ;
	cValue["devpwd"] = "bill007" ;
#ifdef _DEBUG
	cValue["appkey"] = "e87f31bb-e86c-4d87-a3f3-57b3da76b3d6";
#else
	cValue["appkey"] = "abffee4b-deea-4e96-ac8d-b9d58f246c3f" ;
#endif // DEBUG
	//cValue["index"] = pGroup->getMemberCnt() / REQ_PAGE_MEMBER_CNT_OF_CLUB ;
	//cValue["count"] = REQ_PAGE_MEMBER_CNT_OF_CLUB ;
	cValue["group_id"] = pGroup->nGroupID ;
	Json::StyledWriter sWrite ;
	std::string str = sWrite.write(cValue);
	m_pGoTyeAPI.performRequest("GetGroupUserList",str.c_str(),str.size(),pGroup,eReq_GroupMembers );
}