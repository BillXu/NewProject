#pragma warning(disable:4800)
#include "LoginDBManager.h"
#include "LogManager.h"
#include "DBRequest.h"
#include "ServerMessageDefine.h"
#include "LoginApp.h"
#include "DataBaseThread.h"
#define PLAYER_BRIF_DATA "playerName,userUID,sex,vipLevel,defaultPhotoID,isUploadPhoto,exp,coin,diamond"
#define PLAYER_DETAIL_DATA "playerName,userUID,sex,vipLevel,defaultPhotoID,isUploadPhoto,exp,coin,diamond,signature,singleWinMost,winTimes,loseTimes,yesterdayPlayTimes,todayPlayTimes,longitude,latitude,offlineTime"
CDBManager::CDBManager()
{
	m_vReserverArgData.clear();
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

void CDBManager::Init(CLoginApp* pApp)
{
	m_pTheApp = pApp ;
	if ( MAX_LEN_ACCOUNT < 18 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("MAX_LEN_ACCOUNT must big than 18 , or guset login will crash ") ;
	}
}

void CDBManager::OnMessage(stMsg* pmsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	// construct sql
	stArgData* pdata = GetReserverArgData() ;
	if ( pdata == NULL )
	{
		pdata = new stArgData ;
	}

	pdata->eFromPort = eSenderPort ;
	switch( pmsg->usMsgType )
	{
	case MSG_PLAYER_REGISTER:
		{
			stMsgRegister* pLoginRegister = (stMsgRegister*)pmsg ;
			pdata->nSessionID = nSessionID ;

			if ( pLoginRegister->cRegisterType == 0  )
			{
				memset(pLoginRegister->cAccount,0,sizeof(pLoginRegister->cAccount)) ;
				memset(pLoginRegister->cPassword,0,sizeof(pLoginRegister->cPassword)) ;
				sprintf_s(pLoginRegister->cAccount,"%d%d",time_t(NULL),rand()%1000 );
				sprintf_s(pLoginRegister->cPassword,"hello");
			}
			
			pdata->nExtenArg1 = pLoginRegister->cRegisterType ;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Super ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata ;
			// format sql String ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"call RegisterAccount('%s','%s',%d,%d);",pLoginRegister->cAccount,pLoginRegister->cPassword,pLoginRegister->cRegisterType,pLoginRegister->nChannel ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case MSG_PLAYER_CHECK_ACCOUNT:
		{
			stMsgCheckAccount* pLoginCheck = (stMsgCheckAccount*)pmsg ;
			pdata->nSessionID = nSessionID ;

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_High ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			// format sql String ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"call CheckAccount('%s','%s')",pLoginCheck->cAccount,pLoginCheck->cPassword ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	//case MSG_PLAYER_BIND_ACCOUNT:
	//	{
	//		stMsgLoginBindAccount* pMsgRet = (stMsgLoginBindAccount*)pmsg ;
	//		pdata->nSessionID = pMsgRet->nSessionID ;
	//		pdata->nExtenArg1 = pMsgRet->nCurUserUID ;
	//		pdata->pUserData = new char[sizeof(stMsgLoginBindAccount)] ;
	//		memcpy(pdata->pUserData,pMsgRet,sizeof(stMsgLoginBindAccount));

	//		stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
	//		pRequest->cOrder = eReq_Order_Super ;
	//		pRequest->eType = eRequestType_Update ;
	//		pRequest->nRequestUID = pmsg->usMsgType ;
	//		pRequest->pUserData = pdata;
	//		pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE account SET account = '%s', password = '%s',email = '%s',registerType = '%d' WHERE userUID = '%u'",pMsgRet->cAccount,pMsgRet->cPassword,pMsgRet->cEmail,2,pMsgRet->nCurUserUID ) ;
	//		CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
	//	}
	//	break;
	//case MSG_MODIFY_PASSWORD:
	//	{
	//		stMsgLoginModifyPassword* pMsgRet = (stMsgLoginModifyPassword*)pmsg ;
	//		pdata->nSessionID = pMsgRet->nSessionID ;
	//		pdata->pUserData = new char[MAX_LEN_PASSWORD] ;
	//		memcpy(pdata->pUserData,pMsgRet->cNewPassword,MAX_LEN_PASSWORD);

	//		stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
	//		pRequest->cOrder = eReq_Order_Super ;
	//		pRequest->eType = eRequestType_Update ;
	//		pRequest->nRequestUID = pmsg->usMsgType ;
	//		pRequest->pUserData = pdata;
	//		pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"UPDATE account SET password = '%s' WHERE userUID = '%u' && password = '%s'",pMsgRet->cNewPassword,pMsgRet->nUserUID,pMsgRet->cOldPassword ) ;
	//		CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
	//	}
	//	break;
	default:
		{
			m_vReserverArgData.push_back(pdata) ;
			CLogMgr::SharedLogMgr()->ErrorLog("unknown msg type = %d",pmsg->usMsgType ) ;
		}
	}
}

void CDBManager::OnDBResult(stDBResult* pResult)
{
	stArgData*pdata = (stArgData*)pResult->pUserData ;
	switch ( pResult->nRequestUID )
	{
	case  MSG_PLAYER_REGISTER:
		{
			stMsgRegisterRet msgRet ;
			msgRet.cRegisterType = pdata->nExtenArg1 ;
			memset(msgRet.cAccount,0,sizeof(msgRet.cAccount));
			memset(msgRet.cPassword,0,sizeof(msgRet.cPassword)) ;
			msgRet.nUserID = 0 ;
			if ( pResult->nAffectRow <= 0 )
			{
				msgRet.nRet = 1 ;
				m_pTheApp->SendMsg((char*)&msgRet,sizeof(msgRet),pdata->nSessionID) ;
				CLogMgr::SharedLogMgr()->ErrorLog("why register affect row = 0 ") ;
				return ;
			}

			CMysqlRow& pRow = *pResult->vResultRows.front() ;
			 msgRet.nRet = pRow["nOutRet"]->IntValue();
			 if ( msgRet.nRet != 0 )
			 {
				 m_pTheApp->SendMsg((char*)&msgRet,sizeof(msgRet),pdata->nSessionID) ;
				 CLogMgr::SharedLogMgr()->PrintLog("register failed duplicate account = %s",pRow["strAccount"]->CStringValue() );
				 return ;
			 }

			sprintf_s(msgRet.cAccount,"%s",pRow["strAccount"]->CStringValue());
			sprintf_s(msgRet.cPassword,"%s",pRow["strPassword"]->CStringValue());
			msgRet.nUserID = pRow["nOutUserUID"]->IntValue();
			m_pTheApp->SendMsg((char*)&msgRet,sizeof(msgRet),pdata->nSessionID) ;
			CLogMgr::SharedLogMgr()->PrintLog("register success account = %s",pRow["strAccount"]->CStringValue() );
		}
		break;
	case MSG_PLAYER_CHECK_ACCOUNT:
		{
			stMsgCheckAccountRet msgRet ;
			if ( pResult->nAffectRow > 0 )
			{
				CMysqlRow& pRow = *pResult->vResultRows.front() ;
				msgRet.nRet = pRow["nOutRet"]->IntValue() ;
				msgRet.nUserID = pRow["nOutUID"]->IntValue() ;
				CLogMgr::SharedLogMgr()->PrintLog("check accout = %s  ret = %d",msgRet.nRet ,pRow["strAccount"]->CStringValue() ) ;
			}
			else
			{
				msgRet.nRet = 1 ;  // account error ;   
				msgRet.nUserID = 0 ;
				CLogMgr::SharedLogMgr()->ErrorLog("check account  why affect row = 0 ? ") ;
			}
			m_pTheApp->SendMsg((char*)&msgRet,sizeof(msgRet),pdata->nSessionID ) ;
		}
		break;
//	case MSG_PLAYER_BIND_ACCOUNT:
//		{
//			stMsgLoginBindAccount*pBind = (stMsgLoginBindAccount*)pdata->pUserData ;
//			stMsgLoginBindAccountRet msgBack ;
//			msgBack.nCurUserUID = pBind->nCurUserUID;
//			msgBack.nSessionID = pdata->nSessionID ;
//			msgBack.nRet = 0 ;
//			memcpy(msgBack.cAccount,pBind->cAccount,sizeof(msgBack.cAccount));
//			memcpy(msgBack.cPassword,pBind->cPassword,sizeof(msgBack.cPassword));
//			memcpy(msgBack.cEmail,pBind->cEmail,sizeof(msgBack.cEmail));
//			if ( pResult->nAffectRow <= 0 )
//			{
//				msgBack.nRet = 1 ;
//			}
//			m_pTheApp->SendMsg((char*)&msgBack,sizeof(msgBack) ,pdata->m_nReqrestFromAdd); 
//			delete[] pdata->pUserData;
//			pdata->pUserData = NULL ;
//		}
//		break;
//	case MSG_MODIFY_PASSWORD:
//		{
//			char* pNewPassword = (char*)pdata->pUserData;
//			stMsgLoginModifyPasswordRet msgBack ;
//			msgBack.nSessionID = pdata->nSessionID ;
//			msgBack.nRet = pResult->nAffectRow > 0 ? 0 : 1 ;
//			memcpy(msgBack.cNewPassword,pNewPassword,MAX_LEN_PASSWORD);
//			delete[] pdata->pUserData ;
//			m_pTheApp->SendMsg((char*)&msgBack,sizeof(msgBack) ,pdata->m_nReqrestFromAdd); 
//		}
		break;
	default:
		{
			CLogMgr::SharedLogMgr()->ErrorLog("unprocessed db result msg id = %d ", pResult->nRequestUID );
		}
	}
	m_vReserverArgData.push_back(pdata) ;
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

