#include "LoginScene.h"
#include "MessageDefine.h"
#include "LogManager.h"
#include "PaiJiuMessageDefine.h"
#include "PaiJiuScene.h"
#include "Client.h"
#include "TaxasPokerScene.h"
CLoginScene::CLoginScene(CClient* pNetWork ):IScene(pNetWork)
{ 
	m_eSceneType = eScene_Login ;
}

void CLoginScene::OnEnterScene()
{
	IScene::OnEnterScene();
	InformIdle();
}

void CLoginScene::OnEixtScene()
{
	IScene::OnEixtScene();
}

bool CLoginScene::OnMessage( RakNet::Packet* pPacket )
{
	stMsg* pMsg = (stMsg*)pPacket->data ;
	IScene::OnMessage(pPacket) ;
	switch (pMsg->usMsgType)
	{
	case MSG_PLAYER_CHECK_ACCOUNT:
		{
			stMsgCheckAccountRet* pRetMsg = (stMsgCheckAccountRet*)pMsg;
			 // 0 ; success ; 1 account error , 2 password error ;
			const char* pString = NULL ;
			if ( pRetMsg->nRet == 0 )
			{
				pString = "check Account Success !" ;
				m_pClient->GetPlayerData()->nUserUID = pRetMsg->nUserID ;
				stMsgPlayerEnterGame msgEnterGame ;
				msgEnterGame.nUserUID = pRetMsg->nUserID ;
				SendMsg(&msgEnterGame, sizeof(msgEnterGame)) ;
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

			CLogMgr::SharedLogMgr()->SystemLog("Account:%s Password = %s, %s", m_pClient->GetPlayerData()->GetAccount(),m_pClient->GetPlayerData()->GetPassword(), pString ) ;
			return true ;
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
		}
		break;
	case MSG_PLAYER_BASE_DATA:
		{
			// get room list ;
			//stMsgRequestRoomList msgRoomList ;
			//msgRoomList.cRoomType = m_pClient->GetPlayerData()->GetEnterRoomType() ;
			//SendMsg(&msgRoomList,sizeof(msgRoomList));
			InformIdle();
		}
		break; ;
	//case MSG_REQUEST_ROOM_LIST:
	//	{
	//		stMsgRequestRoomListRet* Ret = (stMsgRequestRoomListRet*)pMsg ;
	//		stRoomListItem* pRoomList = (stRoomListItem*)(((char*)Ret) + sizeof(stMsgRequestRoomListRet)) ;
	//		// --target room less in all 
	//		stRoomListItem* pLestAll = NULL ;
	//		unsigned short nAllLeftSeat = 0 ;
	//		//-------target in single level ;
	//		stRoomListItem* pLestInLevel = NULL ;
	//		unsigned short nSingleLevelLeft = 0 ;
	//		// specail room 
	//		stRoomListItem* pSecailRoom = NULL ;

	//		CLogMgr::SharedLogMgr()->PrintLog("%s: RecieveRoomList Type = %d", m_pClient->GetPlayerData()->GetName(),Ret->cRoomType) ;
	//		while ( Ret->nRoomCount--)
	//		{
	//			// Lest all ;
	//			if ( pRoomList->nMaxCount - pRoomList->nCurrentCount > nAllLeftSeat )
	//			{
	//				pLestAll = pRoomList ;
	//				nAllLeftSeat = pRoomList->nMaxCount - pRoomList->nCurrentCount ;
	//			}

	//			// in lest level 
	//			if ( pRoomList->cRoomLevel == m_pClient->GetPlayerData()->GetWillEnterRoomLevel() )
	//			{
	//				if ( pRoomList->nMaxCount - pRoomList->nCurrentCount > nSingleLevelLeft )
	//				{
	//					pLestInLevel = pRoomList ;
	//					nSingleLevelLeft = pRoomList->nMaxCount - pRoomList->nCurrentCount ;
	//				}

	//				if ( pRoomList->nRoomID == m_pClient->GetPlayerData()->GetEnterRoomID() )
	//				{
	//					pSecailRoom = pRoomList ;
	//				}
	//			}
	//			printf("%s: RoomID = %d , RoomLevel=%d , MaxCount =%d  , CurrentCount = %d \n", m_pClient->GetPlayerData()->GetName(), pRoomList->nRoomID,pRoomList->cRoomLevel,pRoomList->nMaxCount,pRoomList->nCurrentCount) ;
	//			++pRoomList ;
	//		}

	//		if ( pLestAll == 0 && pLestInLevel == 0 && pSecailRoom == 0 )
	//		{
	//			printf("%s: Can not Enter config Room ID = %d , RoomType = %d", m_pClient->GetPlayerData()->GetName(),m_pClient->GetPlayerData()->GetEnterRoomID(),m_pClient->GetPlayerData()->GetEnterRoomID()) ;	
	//			m_pClient->Stop();
	//			return true ;
	//		}

	//		stRoomListItem* pFinal = NULL ;
	//		if ( m_pClient->GetPlayerData()->GetWillEnterRoomLevel() < 0 )
	//		{
	//			pFinal = pLestAll ;
	//		}
	//		else if ( m_pClient->GetPlayerData()->GetEnterRoomID() <= 0 )
	//		{
	//			pFinal = pLestInLevel ;
	//		}
	//		else
	//		{
	//			pFinal = pSecailRoom ;
	//		}

	//		stMsgRoomEnter msgEnterRoom ;
	//		msgEnterRoom.nRoomID = 0;
	//		//msgEnterRoom.nRoomType = m_pClient->GetPlayerData()->GetEnterRoomType() ;
	//		//msgEnterRoom.nRoomLevel = pFinal->cRoomLevel ;
	//		SendMsg(&msgEnterRoom, sizeof(msgEnterRoom) ) ;
	//		return true ;
	//	}
	//	break; 
	case MSG_ROOM_ENTER:
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
					CLogMgr::SharedLogMgr()->ErrorLog("%s do not meet room condition !",m_pClient->GetPlayerData()->GetName()) ;
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
					CLogMgr::SharedLogMgr()->ErrorLog("%s unknown error !",m_pClient->GetPlayerData()->GetName()) ;
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
	case MSG_PJ_ROOM_INFO:
		{
			//CLogMgr::SharedLogMgr()->SystemLog("%s Default error !",m_pClient->GetPlayerData()->GetName()) ;
			//// change room scene and push this msg;
			//IScene*pScene = new CPaiJiuScene(m_pClient) ;
			//m_pClient->ChangeScene(pScene) ;
			//pScene->OnMessage(pPacket) ;
			//return true ;
		}
		break;
	case MSG_TP_ROOM_CUR_INFO:
		{
			// change room scene and push this msg;
			IScene*pScene = new CTaxasPokerScene(m_pClient) ;
			m_pClient->ChangeScene(pScene) ;
			pScene->OnMessage(pPacket) ;
			return true ;
		}
		break;
	case MSG_VERIFY_GATE:
		{

		}
		break;
	case MSG_ROBOT_ADD_MONEY:
		{
			stMsgRobotAddMoneyRet* pRet = (stMsgRobotAddMoneyRet*)pMsg ;
			m_pClient->GetPlayerData()->nMyCoin = pRet->nFinalCoin ;
			printf("received add coin !") ;
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
	stMsgCheckAccount msgCheck ;
	memset(msgCheck.cAccount,0,sizeof(msgCheck.cAccount));
	memset(msgCheck.cPassword,0,sizeof(msgCheck.cPassword));
	sprintf_s(msgCheck.cAccount,"%s",m_pClient->GetPlayerData()->GetAccount());
	sprintf_s(msgCheck.cPassword,"%s",m_pClient->GetPlayerData()->GetPassword());

	SendMsg((char*)&msgCheck,sizeof(msgCheck)) ;
}

void CLoginScene::Login( const char* pAccound , const char* pPassword )
{
	//stMsgLogin msg ;
	//msg.pAccount= NULL ;
	//msg.pAccount = NULL ;
	//msg.nAccountLen = strlen(pAccound);
	//msg.nPaswordLen = strlen(pPassword);
	//char* pbuffer = new char[msg.nPaswordLen + msg.nAccountLen + sizeof(msg)];
	//memcpy(pbuffer,(void*)&msg,sizeof(msg));
	//memcpy(pbuffer + sizeof(msg),pAccound,msg.nAccountLen) ;
	//memcpy(pbuffer + msg.nAccountLen + sizeof(msg),pPassword, msg.nPaswordLen) ;
	//m_pNetWork->SendMsg(pbuffer,msg.nAccountLen + sizeof(msg) + msg.nPaswordLen) ;
	//delete [] pbuffer ;
}

void CLoginScene::InformIdle()
{
	if ( m_pClient->GetPlayerData()->GetCoin(false) < 10000 )
	{
		stMsgRobotAddMoney msg ;
		msg.nWantCoin = 250000 ;
		SendMsg((char*)&msg,sizeof(msg)) ;
	}
	stMsgRobotInformIdle msg ;
	SendMsg((char*)&msg,sizeof(msg)) ;
}

//void CLoginScene::Register( const char* pName ,const char* pAccound , const char* pPassword , int nType )
//{
//	stMsgRegister msg ;
//	msg.nAccountType = nType ;
//	msg.nAccountLen = msg.nPaswordLen = msg.nCharacterNameLen = 0 ;
//	msg.nCharacterNameLen = strlen(pName) ;
//	if ( 0 != nType )
//	{
//		msg.nAccountLen = strlen(pAccound);
//		msg.nPaswordLen = strlen(pPassword) ;
//	}
//
//	unsigned short nLen = msg.nAccountLen + msg.nPaswordLen + msg.nCharacterNameLen + sizeof(msg
//		);
//	char* pbuffer = new char[nLen] ;
//	unsigned nLenOffset = 0 ;
//	memcpy(pbuffer + nLenOffset,&msg,sizeof(msg));
//	nLenOffset = sizeof(msg);
//	if ( 0 != msg.nAccountLen )
//	{
//		memcpy(pbuffer + nLenOffset, pAccound, msg.nAccountLen );
//		nLenOffset += msg.nAccountLen ;
//		memcpy(pbuffer + nLenOffset, pPassword, msg.nPaswordLen );
//		nLenOffset += msg.nPaswordLen ;
//	}
//	memcpy(pbuffer + nLenOffset, pName, msg.nCharacterNameLen );
//	nLenOffset += msg.nCharacterNameLen ;
//	m_pNetWork->SendMsg(pbuffer,nLen) ;
//	delete [] pbuffer ;
//}