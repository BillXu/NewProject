#include "LoginScene.h"
#include "MessageDefine.h"
#include "LogManager.h"
#include "PaiJiuMessageDefine.h"
#include "PaiJiuScene.h"
#include "ClientRobot.h"
#include "TaxasPokerScene.h"
#include "BacScene.h"
#include "TaxasMessageDefine.h"
CLoginScene::CLoginScene(CClientRobot* pNetWork ):IScene(pNetWork)
{ 
	m_eSceneType = eScene_Login ;
}

void CLoginScene::OnEnterScene()
{
	IScene::OnEnterScene();
	//InformIdle();
}

void CLoginScene::OnEixtScene()
{
	IScene::OnEixtScene();
}

bool CLoginScene::OnMessage( Packet* pPacket )
{
	stMsg* pMsg = (stMsg*)pPacket->_orgdata ;
	IScene::OnMessage(pPacket) ;
	switch (pMsg->usMsgType)
	{
	case MSG_PLAYER_REGISTER:
		{
			stMsgRegisterRet* pRet = (stMsgRegisterRet*)pMsg ;
			if ( pRet->nRet == 0 )
			{
				CLogMgr::SharedLogMgr()->SystemLog("register sucess accout = %s , password = %s,uid = %d",pRet->cAccount,pRet->cPassword ,pRet->nUserID ) ;
			}
			else
			{
				CLogMgr::SharedLogMgr()->ErrorLog("register error code = %d",pRet->nRet );
			}
		}
		break;
	case MSG_PLAYER_LOGIN:
		{
			stMsgLoginRet* pRetMsg = (stMsgLoginRet*)pMsg;
			 // 0 ; success ; 1 account error , 2 password error ;
			const char* pString = NULL ;
			if ( pRetMsg->nRet == 0 )
			{
				pString = "check Account Success !" ;
			}
			else if ( 1 == pRetMsg->nRet )
			{
				pString = "Check Account Error : Account Error !" ;
			}
			else if ( 2 == pRetMsg->nRet )
			{
				pString = "Check Account Error : password Error !" ;
			}
			else
			{
				pString = "Check account Error : unknown Error !" ;
			}

			CLogMgr::SharedLogMgr()->SystemLog("%s ret = %d, account type = %d ",pString,pRetMsg->nRet,pRetMsg->nAccountType ) ;

// 			stMsgModifyPassword msg ;
// 			msg.nUserUID = 1201;
// 			memset(msg.cNewPassword,0,sizeof(msg.cNewPassword)) ;
// 			memset(msg.cOldPassword,0,sizeof(msg.cOldPassword)) ;
// 			sprintf_s(msg.cNewPassword,"12") ;
// 			sprintf_s(msg.cOldPassword,"6") ;
// 			SendMsg(&msg,sizeof(msg)) ;

			//stMsgRebindAccount msg ;
			//msg.nCurUserUID = 1198 ;
			//memset(msg.cAccount,0,sizeof(msg.cAccount)) ;
			//memset(msg.cPassword,0,sizeof(msg.cPassword)) ;
			//sprintf_s(msg.cAccount,"2d3") ;
			//sprintf_s(msg.cPassword,"6") ;
			//SendMsg(&msg,sizeof(msg)) ;
			return true ;
		}
		break;
	case MSG_PLAYER_BIND_ACCOUNT:
		{
			stMsgRebindAccountRet* pRet = (stMsgRebindAccountRet*)pMsg ;
			CLogMgr::SharedLogMgr()->SystemLog("bind account ret = %d",pRet->nRet ) ;
		}
		break;
	case MSG_MODIFY_PASSWORD:
		{
			stMsgModifyPasswordRet* pRet = (stMsgModifyPasswordRet*)pMsg ;
			CLogMgr::SharedLogMgr()->SystemLog("modify password ret = %d ",pRet->nRet);
		}
		break;
	case MSG_ROBOT_ORDER_TO_ENTER_ROOM:
		{
			stMsgRobotOrderToEnterRoom* pOrder = (stMsgRobotOrderToEnterRoom*)pMsg;

			stMsgRoomEnter msgEnterRoom ;
			msgEnterRoom.nRoomID = pOrder->nRoomID;
			msgEnterRoom.nRoomType = pOrder->nRoomType;
			msgEnterRoom.nRoomLevel = pOrder->cLevel;
			SendMsg(&msgEnterRoom, sizeof(msgEnterRoom) ) ;
			printf("order robot to enter type = %d , room level = %d , room id = %d \n",pOrder->nRoomType,pOrder->cLevel,pOrder->nRoomID);
		}
		break;
	case MSG_PLAYER_BASE_DATA:
		{
			// get room list ;
			//stMsgRequestRoomList msgRoomList ;
			//msgRoomList.cRoomType = m_pClient->GetPlayerData()->GetEnterRoomType() ;
			//SendMsg(&msgRoomList,sizeof(msgRoomList));

			printf("recived base data\n");
			stMsgTaxasEnterRoom msg ;
			msg.nRoomID = 1 ;
			msg.nType = 0 ;
			msg.nLevel = 0 ;
			SendMsg(&msg,sizeof(msg));
			printf("enter room taxas...\n");
		}
		break; ;
	case MSG_TP_ENTER_ROOM:
		{
			// 0 success ; 1 do not meet room condition , 2 aready in room ; 3  unknown error ; 4 waiting last game settlement ;
			stMsgRoomEnterRet* pRetMsg = (stMsgRoomEnterRet*)pMsg ;
			switch ( pRetMsg->nRet )
			{
			case 0:
				{
					CLogMgr::SharedLogMgr()->SystemLog("%s EnterRoom Success !",m_pClient->GetPlayerData()->GetName()) ;
				}
				break; 
			case 1 :
				{
					CLogMgr::SharedLogMgr()->ErrorLog("%s ivalid session id  !",m_pClient->GetPlayerData()->stBaseData.cName) ;
					InformIdle();
				}
				break ;
			case 2:
				{
					CLogMgr::SharedLogMgr()->ErrorLog("%s already in room !",m_pClient->GetPlayerData()->GetName()) ;
				}
				break; 
			case 3:
				{
					CLogMgr::SharedLogMgr()->ErrorLog("%s room id error !",m_pClient->GetPlayerData()->GetName()) ;
					InformIdle();
				}
				break; 
			case 4:
				{
					CLogMgr::SharedLogMgr()->ErrorLog("%s waiting last game settlement !",m_pClient->GetPlayerData()->GetName()) ;
					InformIdle();
				}
				break;
			default:
				{
					CLogMgr::SharedLogMgr()->ErrorLog("%s Default error !",m_pClient->GetPlayerData()->GetName()) ;
					InformIdle();
				}
				break;
			}
		}
		break;
	case MSG_TP_ROOM_BASE_INFO:
		{
			// change room scene and push this msg;
			printf("recieved taxas room info...\n");
			CTaxasPokerScene* pScene = new CTaxasPokerScene(m_pClient) ;
			pScene->init("../ConfigFile/RobotAIConfig - new.xml");
			m_pClient->ChangeScene(pScene) ;
			pScene->OnMessage(pPacket) ;
			return true ;
		}
		break;
	case MSG_ROBOT_ADD_MONEY:
		{
			stMsgRobotAddMoneyRet* pRet = (stMsgRobotAddMoneyRet*)pMsg ;
			m_pClient->GetPlayerData()->stBaseData.nCoin = pRet->nFinalCoin ;
			printf("received add coin !\n") ;
			stMsgRobotInformIdle msg ;
			SendMsg((char*)&msg,sizeof(msg)) ;
			return true;
		}
		break;
	default:
		{
			CLogMgr::SharedLogMgr()->SystemLog("%s Unknown message CLoginScene msg = %d!",m_pClient->GetPlayerData()->GetName(),pMsg->usMsgType ) ;
		}
		break;
	}
	return false ;
}

void CLoginScene::Verifyed()
{
	//Register("hello name","23s","6",1);
	Login("test1","1");
}

void CLoginScene::Login( const char* pAccound , const char* pPassword )
{
	stMsgLogin msg ;
	memset(msg.cAccount,0,sizeof(msg.cAccount));
	memset(msg.cPassword,0,sizeof(msg.cPassword));
	sprintf_s(msg.cAccount,"%s",pAccound);
	sprintf_s(msg.cPassword,"%s",pPassword) ;
	SendMsg((char*)&msg,sizeof(msg)) ;
}

void CLoginScene::InformIdle()
{
	return ;
	if ( m_pClient->GetPlayerData()->GetCoin(false) < 200000 )
	{
		stMsgRobotAddMoney msg ;
		msg.nWantCoin = 5500000 ;
		SendMsg((char*)&msg,sizeof(msg)) ;
	}
	else
	{
		stMsgRobotInformIdle msg ;
		SendMsg((char*)&msg,sizeof(msg)) ;
	}
}

void CLoginScene::Register( const char* pName ,const char* pAccound , const char* pPassword , int nType )
{
	stMsgRegister msg ;
	msg.cRegisterType = nType ;
	msg.nChannel = 0 ;
	memset(msg.cAccount,0,sizeof(msg.cAccount)) ;
	memset(msg.cPassword,0,sizeof(msg.cPassword)) ;
	if ( nType != 0 )
	{
		sprintf_s(msg.cAccount,"%s",pAccound) ;
		sprintf_s(msg.cPassword,"%s",pPassword) ;
	}
	m_pClient->GetNetWork()->SendMsg((char*)&msg,sizeof(msg)) ;
}