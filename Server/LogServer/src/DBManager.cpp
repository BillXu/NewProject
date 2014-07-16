#pragma warning(disable:4800)
#include "DBManager.h"
#include "LogManager.h"
#include "DBRequest.h"
#include "ServerMessageDefine.h"
#include "DBApp.h"
#include "DataBaseThread.h"
CDBManager::CDBManager(CDBServerApp* theApp )
{
	m_vReserverArgData.clear();
	m_pTheApp = theApp ;
}

CDBManager::~CDBManager()
{
	LIST_ARG_DATA::iterator iter = m_vReserverArgData.begin() ;
	for ( ; iter != m_vReserverArgData.end() ; ++iter )
	{
		if ( *iter )
		{
			delete *iter ;
			*iter = NULL ;
		}
	}
	m_vReserverArgData.clear() ;
}

void CDBManager::Init()
{
	// register funcs here ;
	//stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
	//pRequest->cOrder = eReq_Order_High ;
	//pRequest->eType = eRequestType_Select ;
	//pRequest->nRequestUID = -1;
	//pRequest->pUserData = NULL;
	//pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"(select max(Account.UserUID) FROM Account)") ;
	//CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
}

void CDBManager::OnMessage(RakNet::Packet* packet)
{
	// construct sql
	stMsg* pmsg = (stMsg*)packet->data ;
	if ( pmsg->usMsgType != MSG_SAVE_DB_LOG )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("unknown message") ;
		return ;
	}

	stDBLog* pLog = (stDBLog*)(((char*)pmsg) + sizeof(stMsgToLogDBServer));
	unsigned int nCurNow = (unsigned int)time(NULL) ;
	switch ( pLog->eLogType )
	{
	case eDBLog_Login:
		{
			stPlayerLoginDBLog* pLoginLog = (stPlayerLoginDBLog*)pLog;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Normal ;
			pRequest->eType = eRequestType_Add ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			// format sql String ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"INSERT INTO`log_login` (`userUID`, `playerName`, `loginTime`, `logoutTime`, `onlineTime`,`coin`, `diamoned`) \
																	 VALUES ('%u', '%s', '%u', '%u', '%u', '%I64d', '%u');",pLoginLog->nUserUID,pLoginLog->cPlayerName,pLoginLog->nLoginTime,nCurNow,nCurNow - pLoginLog->nLoginTime,pLoginLog->nCurCoin,pLoginLog->nCurDiamond ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case eDBLog_Impawn:
		{
			stPlayerImpawnDBLog* pPawnLog = (stPlayerImpawnDBLog*)pLog;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Normal ;
			pRequest->eType = eRequestType_Add ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			// format sql String ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,
				"INSERT INTO`log_impawn` (`userUID`, `playerName`, `impawnItemID`, `itemCount`, `impawnCoin`, `impawnDiamoned`,`coin`, `diamoned`, `logTime`) VALUES ('%u', '%s', '%u', '%u', '%I64d','%u', '%I64d', '%u','%u');",
			 pPawnLog->nUserUID,pPawnLog->cPlayerName,pPawnLog->nImpawnItemID,pPawnLog->nItemCount,pPawnLog->nImpawnCoin,pPawnLog->nImpawnDiamoned,pPawnLog->nCurCoin,pPawnLog->nCurDiamond,nCurNow ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case eDBLog_MissionReward:
		{
			stPlayerGetMissionRewardDBlog* pRealLog = (stPlayerGetMissionRewardDBlog*)pLog;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Normal ;
			pRequest->eType = eRequestType_Add ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			// format sql String ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,
				"INSERT INTO`log_mission` (`userUID`, `playerName`, `missionID`, `rewardCoin`,`coin`, `diamoned`, `logTime`) VALUES ('%u', '%s', '%u', '%u', '%I64d','%u','%u');",
				pRealLog->nUserUID,pRealLog->cPlayerName,pRealLog->nMissionID,pRealLog->nGetCoin,pRealLog->nCurCoin,pRealLog->nCurDiamond,nCurNow ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case eDBLog_Shop:
		{
			stPlayShopDBLog* pRealLog = (stPlayShopDBLog*)pLog;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Normal ;
			pRequest->eType = eRequestType_Add ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			// format sql String ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,
				"INSERT INTO`log_shop` (`userUID`, `playerName`, `moneyType`, `spendMoney`, `channel`, `shopItemID`, `shopItemCount`,`coin`, `diamoned`, `logTime`) VALUES ('%u', '%s', '%u', '%u','%u','%u','%I64d','%u','%u');",
				pRealLog->nUserUID,pRealLog->cPlayerName,pRealLog->nMoneyType,pRealLog->nSpendMoney,pRealLog->cChannel,pRealLog->nShopID,pRealLog->nShopCnt,pRealLog->nCurCoin,pRealLog->nCurDiamond,nCurNow ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case eDBLog_OtherMoneyOffset:
		{
			stPlayerOtherMoneyActDBLog* pRealLog = (stPlayerOtherMoneyActDBLog*)pLog;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Normal ;
			pRequest->eType = eRequestType_Add ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			// format sql String ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,
				"INSERT INTO`log_moneyoffsetotheraction` (`userUID`, `playerName`, `actionType`, `coinOffset`, `diamonedOffset`,`coin`, `diamoned`, `logTime`) VALUES ('%u', '%s', '%u', '%I64d','%d','%I64d','%u','%u');",
				pRealLog->nUserUID,pRealLog->cPlayerName,pRealLog->cActType,pRealLog->nCoinOffset,pRealLog->nDiamondOffset,pRealLog->nCurCoin,pRealLog->nCurDiamond,nCurNow ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case eDBLog_StayInRoom:
		{
			stPlayerInRoomDBLog* pRealLog = (stPlayerInRoomDBLog*)pLog;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Normal ;
			pRequest->eType = eRequestType_Add ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			// format sql String ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,
				"INSERT INTO`log_playinroom` (`userUID`, `playerName`, `enterRoomTime`, `coinBeforEnter`, `coinExit`, `exitRoomTime`, `roomID`,`stayInRoomTime`) VALUES ('%u', '%s', '%u', '%I64d','%I64d','%u','%u','%u');",
				pRealLog->nUserUID,pRealLog->cPlayerName,pRealLog->nEnterRoomTime,pRealLog->nCoinBeforEnterRoom,pRealLog->nCurCoin,nCurNow,pRealLog->nRoomID,nCurNow - pRealLog->nEnterRoomTime) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case eDBLog_PresentAsset:
		{
			stPlayerPresentAssertDBLog* pRealLog = (stPlayerPresentAssertDBLog*)pLog;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Normal ;
			pRequest->eType = eRequestType_Add ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			// format sql String ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,
				"INSERT INTO`log_presentasset` (`userUID`, `playerName`, `targetPlayerUID`, `assetItemID`, `assetCount`, `presentReason`,`coin`, `diamoned`, `logTime`) VALUES ('%u', '%s', '%u', '%u','%u','%u','%I64d','%u','%u');",
				pRealLog->nUserUID,pRealLog->cPlayerName,pRealLog->nTargetPlayerUID,pRealLog->nAssertID,pRealLog->nAssetCount,pRealLog->nPresentReason,pRealLog->nCurCoin,pRealLog->nCurDiamond,nCurNow ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
	case eDBLog_RobotCoin:
		{
			stRobotCoinDBLog* pRealLog = (stRobotCoinDBLog*)pLog;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Normal ;
			pRequest->eType = eRequestType_Add ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			// format sql String ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,
				"INSERT INTO`log_robotcoin` (`robotZoneOffset1`, `robotZoneOffset2`, `robotZoneOffset3`, `robotZoneOffset4`, `robotTotalOffset`,`logTime`) VALUES ('%I64d', '%I64d', '%I64d', '%I64d','%I64d','%u');",
				pRealLog->vRoomLevelRobotOffset[0],pRealLog->vRoomLevelRobotOffset[1],pRealLog->vRoomLevelRobotOffset[2],pRealLog->vRoomLevelRobotOffset[3],pRealLog->vAllRobotOffset,nCurNow ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	default:
		{
			CLogMgr::SharedLogMgr()->ErrorLog("unknown log type = %d",pLog->eLogType ) ;
		}
		break;
	}
}

void CDBManager::OnDBResult(stDBResult* pResult)
{

}

CDBManager::stArgData* CDBManager::GetReserverArgData()
{
	LIST_ARG_DATA::iterator iter = m_vReserverArgData.begin() ;
	if ( iter != m_vReserverArgData.end() )
	{
		stArgData* p = *iter ;
		m_vReserverArgData.erase(iter) ;
		p->Reset();
		return p ;
	}
	return NULL ;
}
