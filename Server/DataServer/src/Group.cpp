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
#define CNT_PER_PAGE 16
#define  ONLINE_APP_KEY 
struct stClubMemberChangeReqArg
{
	stGroupItem* pOwenClub ;
	uint32_t nTargetUID ;
	uint32_t nMsgType ;
};

bool stGroupItem::isRoomKeepRunning()
{
	auto pgameCenter = CGameServerApp::SharedGameServerApp()->getGameRoomCenter() ;
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
	m_isSortDirty = true ;
}

void CGroup::onConnectedSvr()
{
	Json::Value jssql ;
	uint32_t nOffset = m_vGroups.size() ;
	char pBuffer[512] = {0};
	sprintf(pBuffer,"select clubID,cityCode,ownerUID,level,deadtime from clubs where isDelete = '0' limit 10 offset %u ",nOffset);
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

		if ( nRow >= 10 ) // go on read more 
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
	case MSG_CREATE_CLUB:
		{
			auto pPlayer = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetPlayerBySessionID(nSessionID) ;
			if ( pPlayer == nullptr )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("session id = %u , player is null , can not create club",nSessionID) ;
				return true ;
			}

			uint16_t nOwnClubCnt = getClubCntByUserUID(pPlayer->GetUserUID());
			uint16_t nMaxCanCreate = pPlayer->GetBaseData()->getMaxCanCreateClubCount() ;

			Json::Value jsMsgBack ;
			jsMsgBack["newClubID"] = prealMsg["newClubID"].asUInt() ;
			if ( nOwnClubCnt >= nMaxCanCreate )
			{
				jsMsgBack["ret"] = 1 ;
				CGameServerApp::SharedGameServerApp()->sendMsg(nSessionID,jsMsgBack,nMsgType) ;
				return true ;
			}
			jsMsgBack["ret"] = 0 ;
			stGroupItem* pItem = new stGroupItem() ;
			pItem->nCityCode = prealMsg["cityCode"].asUInt() ;
			pItem->nCreaterUID = pPlayer->GetUserUID() ;
			pItem->nGroupID = prealMsg["newClubID"].asUInt() ;
			pItem->addMember(pPlayer->GetUserUID());
			addGroup(pItem) ;
			CLogMgr::SharedLogMgr()->PrintLog("player uid = %u create new club id = %u city code = %u",pPlayer->GetUserUID(),pItem->nGroupID,pItem->nCityCode) ;
			CGameServerApp::SharedGameServerApp()->sendMsg(nSessionID,jsMsgBack,nMsgType) ;
		}
		break ;
	case MSG_DISMISS_CLUB:
		{
			uint32_t nClubID = prealMsg["clubID"].asUInt() ;
			auto pPlayer = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetPlayerBySessionID(nSessionID) ;
			auto pClub = getGroupByID(nClubID) ;

			Json::Value jsMsgBack ;
			jsMsgBack["ret"] = 0 ;
			if ( pClub == nullptr || pClub->nCreaterUID != pPlayer->GetUserUID() )
			{
				jsMsgBack["ret"] = 1 ;
				CGameServerApp::SharedGameServerApp()->sendMsg( nSessionID,jsMsgBack,nMsgType) ;
				return true ;
			}

			if ( pClub->isRoomKeepRunning() )
			{
				jsMsgBack["ret"] = 2 ;
				CGameServerApp::SharedGameServerApp()->sendMsg(nSessionID,jsMsgBack,nMsgType) ;
				return true ;
			}
			dismissGroup(nClubID) ;
			CGameServerApp::SharedGameServerApp()->sendMsg( nSessionID,jsMsgBack,nMsgType) ;
			CLogMgr::SharedLogMgr()->PrintLog("player uid = %u dismiss club id = %u",pPlayer->GetUserUID(),nClubID) ;
		}
		break ;
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
			auto pgameCenter = CGameServerApp::SharedGameServerApp()->getGameRoomCenter() ;
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
#ifndef ONLINE_APP_KEY
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
				if ( pClub->isGroupFull() )
				{
					prealMsg["ret"] = 3 ;
					getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType) ;
					CLogMgr::SharedLogMgr()->PrintLog("group is full") ;
					break ;
				}

				if ( pClub->isHaveMember(nAccountUID) )
				{
					prealMsg["ret"] = 4 ;
					getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType) ;
					CLogMgr::SharedLogMgr()->PrintLog("already in group ") ;
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

			m_isSortDirty = true ;
			break ;
		}
		break ;
	case MSG_REQ_RECORMED_CLUB:
		{
			sortGroup();
			Json::Value jsArray ;
			auto iter = m_vSortedGroups.begin();
			for ( ; iter != m_vSortedGroups.end() ; ++iter )
			{               
				jsArray[jsArray.size()] = (*iter)->nGroupID;
				if ( jsArray.size() >= 10 )
				{
					break; 
				}
			}

			prealMsg["clubIDs"] = jsArray ;
			getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType) ;
		}
		break ;
	case MSG_REQ_CLUB_MEMBER:
		{
			uint32_t nClubID = prealMsg["clubID"].asUInt();
			uint32_t nPageIdx = prealMsg["pageIdx"].asUInt() ;
			auto pGroup = getGroupByID(nClubID) ;
			Json::Value jsMembers ;
			prealMsg["ret"] = 0 ;
			do 
			{
				if ( pGroup == nullptr )
				{
					prealMsg["ret"] = 1 ;
					break;
				}

				uint32_t nSkipCnt = CNT_PER_PAGE * nPageIdx ;
				if (nSkipCnt >= pGroup->getMemberCnt() )
				{
					break;
				}

				for ( auto& ref : pGroup->vMembers )
				{
					if ( nSkipCnt > 0 )
					{
						--nSkipCnt;
						continue;
					}

					jsMembers[jsMembers.size()] = ref ;

					if ( jsMembers.size() >= CNT_PER_PAGE )
					{
						break;
					}
				}
			} while (0);

			prealMsg["members"] = jsMembers ;
			getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType);
		}
		break ;
	case MSG_REQ_CITY_CLUB:
		{
			sortGroup() ;
			uint32_t nCityCode = prealMsg["cityCode"].asUInt();
			uint32_t nPageIdx = prealMsg["pageIdx"].asUInt() ;
			uint32_t nSkipCnt = CNT_PER_PAGE * nPageIdx ;

			Json::Value jsClubs ;
			if ( nSkipCnt >= m_vSortedGroups.size() )
			{
				prealMsg["clubs"] = jsClubs ;
				getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType);
				break ;
			}

			for ( auto& pClub : m_vSortedGroups )
			{
				if ( pClub->nCityCode != nCityCode )
				{
					continue;
				}

				if ( nSkipCnt > 0 )
				{
					--nSkipCnt;
					continue;
				}

				jsClubs[jsClubs.size()] = pClub->nGroupID ;

				if ( jsClubs.size() >= CNT_PER_PAGE )
				{
					break;
				}
			}
			prealMsg["clubs"] = jsClubs ;
			getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType);
		}
		break;
	case MSG_CLUB_APPLY_TO_JOIN:
		{
			uint32_t nClubID = prealMsg["clubID"].asUInt() ;
			auto text = prealMsg["text"].asString();
			auto pClub = getGroupByID(nClubID);
			uint8_t nRet = 0 ;
			do 
			{
				if ( pClub == nullptr )
				{
					nRet = 1 ;
					break;
				}
				
				if ( pClub->isGroupFull() )
				{
					nRet = 2 ;
					break; 
				}

				auto pPlayer = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetPlayerBySessionID(nSessionID);
				if ( !pPlayer )
				{
					nRet = 3 ;
					break;
				}

				if ( pClub->isHaveMember(pPlayer->GetUserUID()) )
				{
					nRet = 4 ;
					break;
				}
				
				Json::Value jArg ;
				jArg["applicantUID"] = pPlayer->GetUserUID() ;
				jArg["clubID"] = pClub ;
				jArg["text"] = text ;
			   CPlayerMailComponent::PostDlgNotice(eNotice_RecivedApplyToJoinClub,jArg,pClub->getOwnerUID() ) ;
			} while (false);
			prealMsg["ret"] = nRet ;
			getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType);
		}
		break;
	case MSG_CLUB_REPLY_APPLY_TO_JOIN:
		{
			uint32_t nClubID = prealMsg["clubID"].asUInt();
			uint32_t nApplyerUID = prealMsg["applicantUID"].asUInt() ;
			uint8_t nIsAgree = prealMsg["isAgree"].asUInt();
			auto text = prealMsg["text"].asString();
			uint8_t nRet = 0 ;
			do 
			{
				auto pGroup = getGroupByID(nClubID) ;
				if ( !pGroup )
				{
					nRet = 2 ;
					break;
				}

				if ( pGroup->isGroupFull() )
				{
					nRet = 1 ;
					break ;
				}

				auto pp = CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->GetPlayerBySessionID(nSessionID) ;
				if ( pp == nullptr || pGroup->getOwnerUID() != pp->GetUserUID() )
				{
					nRet = 3 ;
					break; 
				}

				// check add 
				if ( !nIsAgree )
				{
					break;
				}

				if ( pGroup->isHaveMember(nApplyerUID) )
				{
					nRet = 4 ;
					break;
				}

				// do add 
				CLogMgr::SharedLogMgr()->ErrorLog("do async http request ") ;
				 
			} while (false);
			prealMsg["ret"] = nRet ;
			getSvrApp()->sendMsg(nSessionID,prealMsg,nMsgType);
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
	
	// just push to the end of sorted list 
	m_vSortedGroups.push_back(pItem) ;
}

void CGroup::dismissGroup(uint32_t nGroupID )
{
	auto iter = m_vGroups.find(nGroupID) ;
	if ( iter != m_vGroups.end() )
	{
		// remove from sorted list 
		auto siter = std::find(m_vSortedGroups.begin(),m_vSortedGroups.end(),iter->second) ;
		if ( siter == m_vSortedGroups.end() )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why can not find the group in sorted list") ;
		}
		else
		{
			m_vSortedGroups.erase(siter);
		}
		// do delete 
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
		m_isSortDirty = true ;
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
		m_isSortDirty = true ;
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
#ifndef ONLINE_APP_KEY
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

void CGroup::sortGroup()
{
	if ( !m_isSortDirty )
	{
		return ;
	}

	m_isSortDirty = false ;

	m_vSortedGroups.clear();
	for ( auto& refP : m_vGroups )
	{
		m_vSortedGroups.push_back(refP.second) ;
	}

	auto pf = [](
		stGroupItem* pLeft , stGroupItem* pRight )->bool{
			if ( pRight->nLevel == pLeft->nLevel )
			{
				return pRight->getMemberCnt() < pLeft->getMemberCnt();
			}
			return pRight->nLevel < pLeft->nLevel ;
	} ;
	m_vSortedGroups.sort(pf);
}