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
	if ( MAX_LEN_ACCOUNT < 20 )
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
				memset(pLoginRegister->cName,0,sizeof(pLoginRegister->cName)) ;

				// rand a name and account 
				for ( uint8_t nIdx  = 0 ; nIdx < 8 ; ++nIdx )
				{
					char acc , cName ;
					acc = rand() % 50 ;
					if ( acc <= 25 )
					{
						acc = 'a' + acc ;
					}
					else
					{
						acc = 'A' + (acc - 25 );
					}

					cName = rand() % 50 ;
					if ( acc <= 25 )
					{
						cName = 'a' + cName ;
					}
					else
					{
						cName = 'A' + (cName - 25 );
					}
					pLoginRegister->cAccount[nIdx] = acc ;
					pLoginRegister->cName[nIdx] = cName ;
				}
				sprintf_s(pLoginRegister->cPassword,"hello");
			}
			
			pdata->nExtenArg1 = pLoginRegister->cRegisterType ;
			if ( strlen(pLoginRegister->cAccount) >= MAX_LEN_ACCOUNT || strlen(pLoginRegister->cPassword) >= MAX_LEN_PASSWORD )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("pLoginRegister password or account len is too long ");
				m_vReserverArgData.push_back(pdata) ;
				break; 
			}

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
	case MSG_PLAYER_LOGIN:
		{
			stMsgLogin* pLoginCheck = (stMsgLogin*)pmsg ;
			pdata->nSessionID = nSessionID ;
			// must end with \0
			if ( strlen(pLoginCheck->cAccount) >= MAX_LEN_ACCOUNT || strlen(pLoginCheck->cPassword) >= MAX_LEN_PASSWORD )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("password or account len is too long ");
				m_vReserverArgData.push_back(pdata) ;
				break; 
			}

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_High ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata ;
			// format sql String ;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"call CheckAccount('%s','%s')",pLoginCheck->cAccount,pLoginCheck->cPassword ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case MSG_PLAYER_BIND_ACCOUNT:
		{
			stMsgRebindAccount* pMsgRet = (stMsgRebindAccount*)pmsg ;
			pdata->nSessionID = nSessionID ;
			pdata->nExtenArg1 = pMsgRet->nCurUserUID ;
			if ( strlen(pMsgRet->cAccount) >= MAX_LEN_ACCOUNT || strlen(pMsgRet->cPassword) >= MAX_LEN_PASSWORD )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("MSG_PLAYER_BIND_ACCOUNT password or account len is too long ");
				m_vReserverArgData.push_back(pdata) ;
				break; 
			}

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Super ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"call RebindAccount(%d,'%s','%s')",pMsgRet->nCurUserUID,pMsgRet->cAccount,pMsgRet->cPassword ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case MSG_MODIFY_PASSWORD:
		{
			stMsgModifyPassword* pMsgRet = (stMsgModifyPassword*)pmsg ;
			pdata->nSessionID = nSessionID ;
			pdata->nExtenArg1 = pMsgRet->nUserUID ;
			if ( strlen(pMsgRet->cOldPassword) >= MAX_LEN_PASSWORD || strlen(pMsgRet->cNewPassword) >= MAX_LEN_PASSWORD )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("MSG_MODIFY_PASSWORD password or account len is too long ");
				m_vReserverArgData.push_back(pdata) ;
				break; 
			}

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Super ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"call ModifyPassword(%d,'%s','%s')",pMsgRet->nUserUID,pMsgRet->cOldPassword,pMsgRet->cNewPassword ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
	case MSG_RESET_PASSWORD:
		{
			stMsgResetPassword* pMsgRet = (stMsgResetPassword*)pmsg ;
			pdata->nSessionID = nSessionID ;
			if ( strlen(pMsgRet->cAccount) >= MAX_LEN_ACCOUNT || strlen(pMsgRet->cNewPassword) >= MAX_LEN_PASSWORD )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("MSG_MODIFY_PASSWORD password or account len is too long ");
				m_vReserverArgData.push_back(pdata) ;
				break; 
			}

			stDBRequest* pRequest = CDBRequestQueue::SharedDBRequestQueue()->GetReserveRequest();
			pRequest->cOrder = eReq_Order_Super ;
			pRequest->eType = eRequestType_Select ;
			pRequest->nRequestUID = pmsg->usMsgType ;
			pRequest->pUserData = pdata;
			pRequest->nSqlBufferLen = sprintf_s(pRequest->pSqlBuffer,"call ResetPassword('%s','%s')",pMsgRet->cAccount,pMsgRet->cNewPassword ) ;
			CDBRequestQueue::SharedDBRequestQueue()->PushRequest(pRequest) ;
		}
		break;
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
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgRet,sizeof(msgRet));
				CLogMgr::SharedLogMgr()->ErrorLog("why register affect row = 0 ") ;
				return ;
			}

			CMysqlRow& pRow = *pResult->vResultRows.front() ;
			 msgRet.nRet = pRow["nOutRet"]->IntValue();
			 if ( msgRet.nRet != 0 )
			 {
				 m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgRet,sizeof(msgRet));
				 CLogMgr::SharedLogMgr()->PrintLog("register failed duplicate account = %s",pRow["strAccount"]->CStringValue() );
				 return ;
			 }

			sprintf_s(msgRet.cAccount,"%s",pRow["strAccount"]->CStringValue());
			sprintf_s(msgRet.cPassword,"%s",pRow["strPassword"]->CStringValue());
			msgRet.nUserID = pRow["nOutUserUID"]->IntValue();

			// request db to create new player data 
			//stMsgRequestDBCreatePlayerData msgCreateData ;
			//msgCreateData.nUserUID = msgRet.nUserID ;
			//msgCreateData.isRegister = msgRet.cRegisterType != 0 ;
			//m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgCreateData,sizeof(msgCreateData)) ;

			// tell client the success register result ;
			m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgRet,sizeof(msgRet));
			CLogMgr::SharedLogMgr()->PrintLog("register success account = %s",pRow["strAccount"]->CStringValue() );

			stMsgLoginSvrInformGateSaveLog msglog ;
			msglog.nlogType = eLog_Register ;
			msglog.nUserUID = msgRet.nUserID  ;
			m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msglog,sizeof(msglog)) ;
		}
		break;
	case MSG_PLAYER_LOGIN:
		{
			stMsgLoginRet msgRet ;
			msgRet.nAccountType = 0 ;
			uint32_t nUserUID = 0 ;
			if ( pResult->nAffectRow > 0 )
			{
				CMysqlRow& pRow = *pResult->vResultRows.front() ;
				msgRet.nRet = pRow["nOutRet"]->IntValue() ;
				msgRet.nAccountType = pRow["nOutRegisterType"]->IntValue() ;
				nUserUID = pRow["nOutUID"]->IntValue() ;
				CLogMgr::SharedLogMgr()->PrintLog("check accout = %s  ret = %d",pRow["strAccount"]->CStringValue(),msgRet.nRet  ) ;
			}
			else
			{
				msgRet.nRet = 1 ;  // account error ;   
				CLogMgr::SharedLogMgr()->ErrorLog("check account  why affect row = 0 ? ") ;
			}
			m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgRet,sizeof(msgRet) ) ;
			// tell data svr login success 
			if (msgRet.nRet == 0 )
			{
				stMsgOnPlayerLogin msgData ;
				msgData.nUserUID =  nUserUID;
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgData,sizeof(msgData) ) ;

				stMsgLoginSvrInformGateSaveLog msglog ;
				msglog.nlogType = eLog_Login ;
				msglog.nUserUID = nUserUID ;
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msglog,sizeof(msglog)) ;
			}
		}
		break;
	case MSG_PLAYER_BIND_ACCOUNT:
		{
			stMsgRebindAccountRet msgBack ;
			msgBack.nRet = 0 ;
			if ( pResult->nAffectRow > 0 )
			{
				CMysqlRow& pRow = *pResult->vResultRows.front() ;
				msgBack.nRet = pRow["nOutRet"]->IntValue() ;
			}
			else
			{
				msgBack.nRet = 3 ;
				CLogMgr::SharedLogMgr()->ErrorLog("uid = %d ,bind account error db ",pdata->nExtenArg1) ;
			}
			m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgBack,sizeof(msgBack)); 
			CLogMgr::SharedLogMgr()->PrintLog("rebind account ret = %d , userUID = %d",msgBack.nRet,pdata->nExtenArg1 ) ;

			if ( msgBack.nRet == 0 )
			{
				stMsgOnPlayerBindAccount msgInfom ;
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgInfom,sizeof(msgInfom)); 

				stMsgLoginSvrInformGateSaveLog msglog ;
				msglog.nlogType = eLog_BindAccount ;
				msglog.nUserUID = pdata->nExtenArg1 ;
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msglog,sizeof(msglog)) ;
			}
		}
		break;
	case MSG_MODIFY_PASSWORD:
		{
			stMsgModifyPasswordRet msgBack ;
			msgBack.nRet = 0 ;
			if ( pResult->nAffectRow > 0 )
			{
				CMysqlRow& pRow = *pResult->vResultRows.front() ;
				msgBack.nRet = pRow["nOutRet"]->IntValue() ;
			}
			else
			{
				msgBack.nRet = 3 ;
				CLogMgr::SharedLogMgr()->ErrorLog("uid = %d , modify password error db ",pdata->nExtenArg1) ;
			}
			CLogMgr::SharedLogMgr()->PrintLog("uid = %d modify password ret = %d",pdata->nExtenArg1,msgBack.nRet ) ;
			m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgBack,sizeof(msgBack)); 

			if ( msgBack.nRet == 0 )
			{
				stMsgLoginSvrInformGateSaveLog msglog ;
				msglog.nlogType = eLog_ModifyPwd ;
				msglog.nUserUID = pdata->nExtenArg1 ;
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msglog,sizeof(msglog)) ;
			}
		}
		break;
	case MSG_RESET_PASSWORD:
		{
			stMsgResetPasswordRet msgBack ;
			msgBack.nRet = 0 ;
			uint32_t nUID = 0 ;
			if ( pResult->nAffectRow > 0 )
			{
				CMysqlRow& pRow = *pResult->vResultRows.front() ;
				msgBack.nRet = pRow["nOutRet"]->IntValue() ;
				nUID = pRow["nUID"]->IntValue() ;
			}
			else
			{
				msgBack.nRet = 1 ;
			}
			CLogMgr::SharedLogMgr()->PrintLog("uid = %d modify password ret = %d",nUID,msgBack.nRet ) ;
			m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msgBack,sizeof(msgBack)); 

			if ( msgBack.nRet == 0 )
			{
				stMsgLoginSvrInformGateSaveLog msglog ;
				msglog.nlogType = eLog_ResetPassword ;
				msglog.nUserUID = nUID ;
				m_pTheApp->sendMsg(pdata->nSessionID,(char*)&msglog,sizeof(msglog)) ;
			}
		}
		break;
	default:
		{
			CLogMgr::SharedLogMgr()->ErrorLog("unprocessed login db result msg id = %d ", pResult->nRequestUID );
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

