#include "LoginScene.h"
#include "MessageDefine.h"
#include "LogManager.h"
#include "PaiJiuMessageDefine.h"
#include "PaiJiuScene.h"
 
#include "TaxasPokerScene.h"
#include "BacScene.h"
#include "TaxasMessageDefine.h"
#include "Client.h"
#include "NiuNiuScene.h"
#define  TIME_DELAY_ENTER_ROOM (40 )
#define  TIME_DELAY_LOGIN (45)
CLoginScene::CLoginScene(CClientRobot* pNetWork ):IScene(pNetWork)
{ 
	m_eSceneType = eScene_Login ;
	m_fDelyTick = 0 ;
	m_eCurState = els_Normal ;
}

void CLoginScene::OnEnterScene()
{
	IScene::OnEnterScene();
	//InformIdle();
	m_eCurState = els_Normal ;
	if ( m_pClient->GetPlayerData()->isLackOfCoin() )
	{
		m_eCurState = els_WaitCoin ;

		stMsgRobotAddMoney msg ;
		msg.nWantCoin = 60000 ;
		SendMsg((char*)&msg,sizeof(msg)) ;
	}
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

			printf("recived base data donot tell robot type \n");
			stMsgTellPlayerType msg ;
			msg.nPlayerType = ePlayer_Robot ;
			SendMsg(&msg, sizeof(msg)) ;

			//uint8_t nCnt = 6 ;
			//while ( nCnt )
			//{
			//	stMsgCreateRoom msgCreate ;
			//	msgCreate.nConfigID = 29 ;
			//	msgCreate.nMinites = 30 ;
			//	msgCreate.nRoomType = nCnt % 2 == 0 ? eRoom_TexasPoker : eRoom_NiuNiu;
			//	if ( eRoom_TexasPoker == msgCreate.nRoomType )
			//	{
			//		msgCreate.nConfigID = 2 ;
			//	}
			//	memset(msgCreate.vRoomName,0,sizeof(msgCreate.vRoomName)) ;
			//	sprintf_s(msgCreate.vRoomName,"roname = %d",nCnt);
			//	SendMsg(&msgCreate, sizeof(msgCreate)) ;
			//	--nCnt ;
			//}
			//CLogMgr::SharedLogMgr()->PrintLog("create room ok");
			//return true ;
			////uint8_t nCnt = 30 ;
			////while ( nCnt)
			////{
			////	stMsgDeleteRoom msgDel ;
			////	msgDel.nRoomType = eRoom_TexasPoker;
			////	msgDel.nRoomID = nCnt-- ;
			////	SendMsg(&msgDel, sizeof(msgDel)) ;
			////}
			////CLogMgr::SharedLogMgr()->PrintLog("delete room ok");
			doEnterGame();
		}
		break; ;
	case MSG_PLAYER_ENTER_ROOM:
		{
			stMsgPlayerEnterRoomRet* pRet = (stMsgPlayerEnterRoomRet*)pMsg ;
			if ( pRet->nRet )
			{
				printf("enter niuniu room failed ret = %d\n",pRet->nRet) ;
				delayEnterRoom() ;
			}
			else
			{
				printf("enter niuniu room success\n") ;
			}
		}
		break;
	case MSG_ROOM_INFO:
		{
			stMsgRoomInfo* pRet = (stMsgRoomInfo*)pMsg ;
			if ( pRet->nRoomType == eRoom_TexasPoker )
			{
				// change room scene and push this msg;
				printf("recieved taxas room info...\n");
				CTaxasPokerScene* pScene = new CTaxasPokerScene(m_pClient) ;
				//pScene->init("../ConfigFile/RobotAIConfig - new.xml");
				char pBuffer[255] = { 0 };
				sprintf_s(pBuffer,"../ConfigFile/%s",m_pClient->GetPlayerData()->pRobotItem->strAiFileName.c_str());
				pScene->init(pBuffer);
				m_pClient->ChangeScene(pScene) ;
				pScene->OnMessage(pPacket) ;
			}
			else if ( eRoom_NiuNiu == pRet->nRoomType )
			{
				CNiuNiuScene* pScene = new CNiuNiuScene(m_pClient) ;
				pScene->init(nullptr) ;
				m_pClient->ChangeScene(pScene) ;
				pScene->OnMessage(pPacket) ;
			}
			else
			{
				printf("unknown room type = %u\n",pRet->nRoomType) ;
			}

			return true ;
		}
		break;
	case MSG_ADD_MONEY:
		{
			stMsgRobotAddMoneyRet* pRet = (stMsgRobotAddMoneyRet*)pMsg ;
			m_pClient->GetPlayerData()->stBaseData.nCoin = pRet->nFinalCoin ;
			printf("received add coin !\n") ;
			delayEnterRoom();
			m_pClient->GetPlayerData()->setIsLackOfCoin(false) ;
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

void CLoginScene::doEnterGame()
{
	stMsgPlayerEnterRoom msg ;
	msg.nRoomGameType = m_pClient->GetPlayerData()->getDstGameType() ;
	msg.nRoomID = m_pClient->GetPlayerData()->getDstRoomID() ;
	SendMsg(&msg,sizeof(msg));
	printf("enter room type = %d id = %d...\n",msg.nRoomGameType,msg.nRoomID);
	
	m_eCurState = els_Normal ;
}

void CLoginScene::delayEnterRoom()
{
	m_fDelyTick = TIME_DELAY_ENTER_ROOM ;
	m_eCurState = els_WaitEnterRoom ;
}

void CLoginScene::Verifyed()
{
	srand(m_pClient->GetPlayerData()->pRobotItem->nRobotID);
	//Register("hello name","23s","6",1);
	Login(m_pClient->GetPlayerData()->pRobotItem->strAccount.c_str(),m_pClient->GetPlayerData()->pRobotItem->strPassword.c_str());
	//Register("name",m_pClient->GetPlayerData()->pRobotItem->strAccount.c_str(),m_pClient->GetPlayerData()->pRobotItem->strPassword.c_str(),1);
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

void CLoginScene::OnUpdate(float fDeltaTime )
{
	IScene::OnUpdate(fDeltaTime);

	if ( els_Normal != m_eCurState && els_WaitCoin != m_eCurState )
	{
		m_fDelyTick -= fDeltaTime ;
	}
	else 
	{
		return ;
	}

	if ( m_fDelyTick < 0 )
	{
		if ( els_WaitEnterRoom == m_eCurState )
		{
			printf("try again to enter room \n") ;
			doEnterGame() ;
		}
		else if ( els_WaitLogin == m_eCurState )
		{
			Login(m_pClient->GetPlayerData()->pRobotItem->strAccount.c_str(),m_pClient->GetPlayerData()->pRobotItem->strPassword.c_str());
			m_eCurState = els_Normal ;
		}
	}

}