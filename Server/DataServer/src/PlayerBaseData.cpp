#include "PlayerBaseData.h"
#include <string>
#include "MessageDefine.h"
#include "ServerMessageDefine.h"
#include "Player.h"
#include "LogManager.h"
#include <time.h>
#include "GameServerApp.h"
#include "ContinueLoginConfig.h"
#include "PlayerItem.h"
#include "PlayerEvent.h"
#include "PlayerManager.h"
#include "EventCenter.h"
#include "InformConfig.h"
#include "AutoBuffer.h"
#include "PlayerManager.h"
#include "ShopConfg.h"
#include <assert.h>
#pragma warning( disable : 4996 )
#define ONLINE_BOX_RESET_TIME 60*60*3   // offline 3 hour , will reset the online box ;
CPlayerBaseData::CPlayerBaseData(CPlayer* player )
	:IPlayerComponent(player)
{
	m_eType = ePlayerComponent_BaseData ;
	memset(&m_stBaseData,0,sizeof(m_stBaseData)) ;
	m_bGivedLoginReward = false ;
	m_strCurIP = "" ;
}

CPlayerBaseData::~CPlayerBaseData()
{

}

void CPlayerBaseData::Init()
{
	memset(&m_stBaseData,0,sizeof(m_stBaseData)) ;
	m_stBaseData.nUserUID = GetPlayer()->GetUserUID() ;
	m_bGivedLoginReward = false ;

	m_bMoneyDataDirty = false;
	m_bCommonLogicDataDirty = false;
	m_bPlayerInfoDataDirty = false;

	m_strCurIP = "" ;
	Reset();
}

void CPlayerBaseData::Reset()
{
	m_strCurIP = "" ;
	m_bGivedLoginReward = false ;

	m_bMoneyDataDirty = false;
	m_bCommonLogicDataDirty = false;
	m_bPlayerInfoDataDirty = false;

	memset(&m_stBaseData,0,sizeof(m_stBaseData)) ;
	m_stBaseData.nUserUID = GetPlayer()->GetUserUID() ;

	stMsgDataServerGetBaseData msg ;
	msg.nUserUID = GetPlayer()->GetUserUID() ;
	SendMsg(&msg,sizeof(msg)) ;
	CLogMgr::SharedLogMgr()->PrintLog("requesting userdata for uid = %d",msg.nUserUID);
	// register new day event ;
	CEventCenter::SharedEventCenter()->RegisterEventListenner(eEvent_NewDay,this,CPlayerBaseData::EventFunc ) ;

	stMsgRequestClientIp msgReq ;
	SendMsg(&msgReq,sizeof(msgReq)) ;
	CLogMgr::SharedLogMgr()->PrintLog("send request ip , sessioni id = %d",GetPlayer()->GetSessionID()) ;
}

bool CPlayerBaseData::OnMessage( stMsg* pMsg , eMsgPort eSenderPort )
{
	if ( IPlayerComponent::OnMessage(pMsg,eSenderPort) )
	{
		return true ;
	}

	switch( pMsg->usMsgType )
	{
	case MSG_REQUEST_CLIENT_IP:
		{
			stMsgRequestClientIpRet* pRet = (stMsgRequestClientIpRet*)pMsg ;
			if ( pRet->nRet == 0 )
			{
				m_strCurIP = pRet->vIP ;
				CLogMgr::SharedLogMgr()->PrintLog("get client ip = %s session id = %d",m_strCurIP.c_str(),GetPlayer()->GetSessionID()) ;
			}
			else
			{
				CLogMgr::SharedLogMgr()->ErrorLog("cant not request client ip , uid = %d",GetPlayer()->GetUserUID()) ;
			}
		}
		break;
	case MSG_SHOP_BUY_ITEM_ORDER:
		{
			stMsgPlayerShopBuyItemOrder* pRet = (stMsgPlayerShopBuyItemOrder*)pMsg ;
			stMsgPlayerShopBuyItemOrderRet msgBack ;
			msgBack.nChannel = pRet->nChannel ;
			msgBack.nShopItemID = pRet->nShopItemID ;
			msgBack.nRet = 0 ;
			memset(msgBack.cOutTradeNo,0,sizeof(msgBack.cOutTradeNo)) ;
			memset(msgBack.cPrepayId,0,sizeof(msgBack.cPrepayId)) ;
			CShopConfigMgr* pMgr = (CShopConfigMgr*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Shop);
			stShopItem* pItem = pMgr->GetShopItem(pRet->nShopItemID);
			if ( pItem == nullptr )
			{
				msgBack.nRet = 1 ;
				SendMsg(&msgBack,sizeof(msgBack)) ;
				CLogMgr::SharedLogMgr()->ErrorLog("can not find shop item , for order uid = %d",GetPlayer()->GetUserUID()) ;
				break;
			}

			if ( msgBack.nChannel != ePay_WeChat )
			{
				msgBack.nRet = 4 ;
				SendMsg(&msgBack,sizeof(msgBack)) ;
				CLogMgr::SharedLogMgr()->ErrorLog("current must be wechat channel for order channel = %d, uid = %d",msgBack.nChannel,GetPlayer()->GetUserUID() );
				break;
			}

			if ( m_strCurIP.empty() )
			{
				msgBack.nRet = 2 ;
				SendMsg(&msgBack,sizeof(msgBack)) ;
				CLogMgr::SharedLogMgr()->ErrorLog("cur rent ip = null , for order uid = %d",GetPlayer()->GetUserUID()) ;
				break;
			}

			stMsgVerifyItemOrder msgOrder ;
			memset(msgOrder.cShopDesc,0,sizeof(msgOrder.cShopDesc));
			sprintf_s(msgOrder.cShopDesc,sizeof(msgOrder.cShopDesc),"%s",pItem->strItemName.c_str()) ;

			memset(msgOrder.cOutTradeNo,0,sizeof(msgOrder.cOutTradeNo));
			sprintf_s(msgOrder.cOutTradeNo,sizeof(msgOrder.cOutTradeNo),"%dE%dE%u",pItem->nShopItemID,GetPlayer()->GetUserUID(),(uint32_t)time(nullptr)) ;
			
			msgOrder.nPrize = pItem->nPrize * 100 ; 
			msgOrder.nChannel = pRet->nChannel ;

			memset(msgOrder.cTerminalIp,0,sizeof(msgOrder.cTerminalIp));
			sprintf_s(msgOrder.cTerminalIp,sizeof(msgOrder.cTerminalIp),"%s",m_strCurIP.c_str()) ;
			
			SendMsg(&msgOrder,sizeof(msgOrder)) ;
			CLogMgr::SharedLogMgr()->SystemLog("order shop item to verify shop item = %d , uid = %d",pItem->nShopItemID,GetPlayer()->GetUserUID()) ;
		} 
		break;
	case MSG_VERIFY_ITEM_ORDER:
		{
			stMsgVerifyItemOrderRet* pRet = (stMsgVerifyItemOrderRet*)pMsg ;
			stMsgPlayerShopBuyItemOrderRet msgBack ;
			memset(msgBack.cOutTradeNo,0,sizeof(msgBack.cOutTradeNo)) ;
			memset(msgBack.cPrepayId,0,sizeof(msgBack.cPrepayId)) ;
			msgBack.nChannel = pRet->nChannel ;
			
			std::string strTradeNo(pRet->cOutTradeNo,sizeof(pRet->cOutTradeNo));
			std::string shopItem = strTradeNo.substr(0,strTradeNo.find_first_of('E')) ;
			if ( shopItem.empty() )
			{
				msgBack.nShopItemID = 0 ;
				CLogMgr::SharedLogMgr()->ErrorLog("outTradeNo shop item is null , uid = %d",GetPlayer()->GetUserUID()) ;
			}
			else
			{
				msgBack.nShopItemID = atoi(shopItem.c_str()) ;
			}
			
			if ( pRet->nRet )
			{
				msgBack.nRet = 3 ;
			}
			else
			{
				msgBack.nRet = 0;
				memcpy(msgBack.cOutTradeNo,pRet->cOutTradeNo,sizeof(pRet->cOutTradeNo));
				memcpy(msgBack.cPrepayId,pRet->cPrepayId,sizeof(pRet->cPrepayId));
			}
			CLogMgr::SharedLogMgr()->SystemLog("shopitem id = %d shop order ret = %d, uid = %d",msgBack.nShopItemID,pRet->nRet,GetPlayer()->GetUserUID()) ;
			SendMsg(&msgBack,sizeof(msgBack)) ;
		}
		break;
	case MSG_BUY_SHOP_ITEM:
		{
			stMsgPlayerBuyShopItem* pRet = (stMsgPlayerBuyShopItem*)pMsg ;
			stMsgToVerifyServer msgVerify ;
			msgVerify.nBuyerPlayerUserUID = GetPlayer()->GetUserUID();
			msgVerify.nBuyForPlayerUserUID = msgVerify.nBuyerPlayerUserUID ;
			msgVerify.nMiUserUID = pRet->nMiUserUID ;
			msgVerify.nShopItemID = pRet->nShopItemID ;
			msgVerify.nTranscationIDLen = pRet->nBufLen ;
			msgVerify.nChannel = pRet->nChannelID ;
			CAutoBuffer buffer(sizeof(stMsgPlayerBuyShopItem) + pRet->nBufLen ) ;
			buffer.addContent(&msgVerify,sizeof(msgVerify));
			buffer.addContent(((char*)pRet) + sizeof(stMsgPlayerBuyShopItem),pRet->nBufLen);
			SendMsg((stMsg*)buffer.getBufferPtr(),buffer.getContentSize());
		}
		break;
	case MSG_VERIFY_TANSACTION:
		{
			stMsgFromVerifyServer* pRet = (stMsgFromVerifyServer*)pMsg ;
			stMsgPlayerBuyShopItemRet msgBack ;
			msgBack.nBuyShopItemForUserUID = pRet->nBuyForPlayerUserUID ;
			msgBack.nDiamoned = 0 ;
			msgBack.nSavedMoneyForVip = 0 ;
			msgBack.nShopItemID = pRet->nShopItemID ;
			msgBack.nRet = 0 ;
			if ( pRet->nRet == 4 ) // success 
			{
				CShopConfigMgr* pMgr = (CShopConfigMgr*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Shop);
				stShopItem* pItem = pMgr->GetShopItem(pRet->nShopItemID);
				if ( pItem == nullptr )
				{
					msgBack.nRet = 5 ;
					CLogMgr::SharedLogMgr()->ErrorLog("can not find shop id = %d , buyer uid = %d",pRet->nShopItemID,pRet->nBuyerPlayerUserUID) ;
				}
				else
				{
					AddMoney(pItem->nCount) ;
					CLogMgr::SharedLogMgr()->SystemLog("add coin with shop id = %d for buyer uid = %d ",pRet->nShopItemID,pRet->nBuyerPlayerUserUID) ;
				}
			}
			else
			{
				msgBack.nRet = 2 ;
				CLogMgr::SharedLogMgr()->ErrorLog("uid = %d ,shop id = %d , verify error ",pRet->nBuyerPlayerUserUID,pRet->nShopItemID) ;
			}

			msgBack.nFinalyCoin = GetAllCoin() ;
			SendMsg(&msgBack,sizeof(msgBack)) ;
		}
		break;
	case MSG_ON_PLAYER_BIND_ACCOUNT:
		{
			m_stBaseData.isRegister = true ;
			m_bPlayerInfoDataDirty = true ;
			CLogMgr::SharedLogMgr()->PrintLog("player bind account ok uid = %u",GetPlayer()->GetUserUID());
		}
		break;
	case MSG_READ_PLAYER_BASE_DATA:   // from db server ;
		{
			stMsgDataServerGetBaseDataRet* pBaseData = (stMsgDataServerGetBaseDataRet*)pMsg ;
			if ( pBaseData->nRet )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("do not exsit playerData") ;
				return true; 
			}
			memcpy(&m_stBaseData,&pBaseData->stBaseData,sizeof(m_stBaseData));
			CLogMgr::SharedLogMgr()->PrintLog("recived base data uid = %d",pBaseData->stBaseData.nUserUID);
			SendBaseDatToClient();
			CGameServerApp::SharedGameServerApp()->GetPlayerMgr()->getPlayerDataCaher().removePlayerDataCache(pBaseData->stBaseData.nUserUID) ;
			return true ;
		}
		break;
//	case MSG_PLAYER_REQUEST_NOTICE:
//		{
//// 			CGameServerApp::SharedGameServerApp()->GetBrocaster()->SendInformsToPlayer(GetPlayer()) ;
//// 			CInformConfig* pConfig = (CInformConfig*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Informs) ;
//// 			m_stBaseData.nNoticeID = pConfig->GetMaxInformID();
//		}
//		break;
	case MSG_PLAYER_MODIFY_SIGURE:
		{
			stMsgPLayerModifySigure* pMsgRet = (stMsgPLayerModifySigure*)pMsg ;
			memcpy(m_stBaseData.cSignature,pMsgRet->pNewSign,sizeof(m_stBaseData.cSignature));
			stMsgPlayerModifySigureRet ret ;
			ret.nRet = 0 ;
			SendMsg(&ret,sizeof(ret)) ;
			m_bPlayerInfoDataDirty = true ;
		}
		break;
	case MSG_PLAYER_MODIFY_NAME:
		{
			stMsgPLayerModifyName* pMsgRet = (stMsgPLayerModifyName*)pMsg ;
			stMsgPlayerModifyNameRet ret ;
			ret.nRet = 0 ;
			if ( pMsgRet->pNewName[sizeof(pMsgRet->pNewName) -1 ] != 0 )
			{
				ret.nRet = 1 ;
				CLogMgr::SharedLogMgr()->ErrorLog("name is too long uid = %d",GetPlayer()->GetUserUID());
			}
			else
			{
				memcpy(m_stBaseData.cName,pMsgRet->pNewName,sizeof(m_stBaseData.cName)) ;
				m_bPlayerInfoDataDirty = true ;
			}
			SendMsg(&ret,sizeof(ret)) ;
		}
		break;
	case MSG_PLAYER_MODIFY_PHOTO:
		{
			stMsgPlayerModifyPhoto* pPhoto = (stMsgPlayerModifyPhoto*)pMsg ;
			m_stBaseData.nPhotoID = pPhoto->nPhotoID ;
			stMsgPlayerModifyPhotoRet msgRet ;
			msgRet.nRet = 0 ;
			SendMsg(&msgRet,sizeof(msgRet)) ;
			m_bPlayerInfoDataDirty = true ;
		}
		break;
	case MSG_PLAYER_MODIFY_SEX:
		{
			stMsgPlayerModifySex* pRet = (stMsgPlayerModifySex*)pMsg ;
			m_stBaseData.nSex = pRet->nNewSex ;
			stMsgPlayerModifySexRet msgback ;
			msgback.nRet = 0 ;
			SendMsg(&msgback,sizeof(msgback)) ;
			m_bPlayerInfoDataDirty = true ;
			CLogMgr::SharedLogMgr()->SystemLog("change sex uid = %d , new sex = %d",GetPlayer()->GetUserUID(),pRet->nNewSex) ;
		}
		break;
	case MSG_PUSH_APNS_TOKEN:
		{
			//stMsgPushAPNSToken* pMsgRet = (stMsgPushAPNSToken*)pMsg ;
			//if ( 0 == pMsgRet->nGetTokenRet )
			//{
			//	bPlayerEnableAPNs = true ;
			//	memcpy(vAPNSToken,pMsgRet->vAPNsToken,32);
			//}
			//else
			//{
			//	bPlayerEnableAPNs = false ;
			//	memset(vAPNSToken,0,32 ) ;
			//}
			//stMsgPushAPNSTokenRet msg ;
			//msg.nGetTokenRet = pMsgRet->nGetTokenRet ;
			//SendMsgToClient((char*)&msg,sizeof(msg)) ;
		}
		break;
	case MSG_PLAYER_UPDATE_MONEY:
		{
 			stMsgPlayerUpdateMoney msgUpdate ;
 			msgUpdate.nFinalCoin = GetAllCoin();
 			msgUpdate.nFinalDiamoned = GetAllDiamoned();
 			SendMsg(&msgUpdate,sizeof(msgUpdate));
		}
		break;
	case MSG_GET_CONTINUE_LOGIN_REWARD:
		{
// 			stMsgGetContinueLoginReward* pGetR = (stMsgGetContinueLoginReward*)pMsg ;
// 			stMsgGetContinueLoginRewardRet msgBack ;
// 			msgBack.nRet = 0 ; //  // 0 success , 1 already getted , 2 you are not vip  ;
// 			msgBack.cRewardType = pGetR->cRewardType;
// 			msgBack.nDayIdx = m_stBaseData.nContinueDays ;
// 			msgBack.nDiamoned = GetAllDiamoned();
// 			msgBack.nFinalCoin = GetAllCoin() ;
// 			if ( m_bGivedLoginReward )
// 			{
// 				msgBack.nRet = 1 ; 
// 				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 				break;
// 			}
// 
// 			stConLoginConfig* pConfig = CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetContinueLoginConfig()->GetConfigByDayIdx(m_stBaseData.nContinueDays) ;
// 			if ( pConfig == NULL )
// 			{
// 				CLogMgr::SharedLogMgr()->ErrorLog("there is no login config for dayIdx = %d",m_stBaseData.nContinueDays ) ;
// 				msgBack.nRet = 4 ;
// 				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 				break;
// 			}
// 
// 			// give item  deponed on nContinuedDays ;
// 			if (pGetR->cRewardType == 1 )
// 			{
// 				if ( GetVipLevel() < 1 )
// 				{
// 					msgBack.nRet = 2 ;
// 					SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 					break;
// 				}
// 				// give item ;
// 				for ( int i = 0 ; i < pConfig->vItems.size(); ++i )
// 				{
// 					CPlayerItemComponent* pc = (CPlayerItemComponent*)GetPlayer()->GetComponent(ePlayerComponent_PlayerItemMgr) ;
// 					pc->AddItemByID(pConfig->vItems[i].nItemID,pConfig->vItems[i].nCount) ;
// 				}
// 				// give vip prize ;
// 				m_stBaseData.nDiamoned += pConfig->nDiamoned ;
// 				msgBack.nDiamoned = GetAllDiamoned();
// 				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 				m_bGivedLoginReward = true ;
// 				break;
// 			}
// 			else if( pGetR->cRewardType == 0 )
// 			{
// 				// gvie common prize ;
// 				m_stBaseData.nCoin += pConfig->nGiveCoin ;
// 				msgBack.nFinalCoin = GetAllCoin() ;
// 				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 				m_bGivedLoginReward = true ;
// 				break;
// 			}
// 			else
// 			{
// 				msgBack.nRet = 3 ;
// 				SendMsgToClient((char*)&msgBack,sizeof(msgBack)) ;
// 				break;
// 			}
		}
		break;
	case MSG_PLAYER_REQUEST_CHARITY_STATE:
		{
 			stMsgPlayerRequestCharityStateRet msgBack ;
 			 // 0 can get charity , 1 you coin is enough , do not need charity, 2 time not reached ;
 			msgBack.nState = 0 ;
 			msgBack.nLeftSecond = 0 ;
 			if ( GetAllCoin() > COIN_CONDITION_TO_GET_CHARITY )
 			{
 				msgBack.nState = 1 ;
 			}
 			else if ( time(NULL) - m_stBaseData.tLastTakeCharityCoinTime < TIME_GET_CHARITY_ELAPS )
 			{
 				msgBack.nState = 2 ;
 				msgBack.nLeftSecond = m_stBaseData.tLastTakeCharityCoinTime + TIME_GET_CHARITY_ELAPS - time(NULL) ;
 			}
 			SendMsg(&msgBack,sizeof(msgBack)) ;
		}
		break;
	case MSG_PLAYER_GET_CHARITY:
		{
 			stMsgPlayerGetCharityRet msgBack ;
 			// 0 success ,  1 you coin is enough , do not need charity, 2 time not reached ;
 			msgBack.nRet = 0 ;
 			msgBack.nFinalCoin = GetAllCoin();
 			msgBack.nGetCoin = 0;
 			msgBack.nLeftSecond = 0 ;
 			if ( GetAllCoin() > COIN_CONDITION_TO_GET_CHARITY )  
 			{
 				msgBack.nRet = 1 ;
 			}
 			else if ( time(NULL) - m_stBaseData.tLastTakeCharityCoinTime < TIME_GET_CHARITY_ELAPS )
 			{
 				msgBack.nRet = 2 ;
 				msgBack.nLeftSecond = m_stBaseData.tLastTakeCharityCoinTime + TIME_GET_CHARITY_ELAPS - time(NULL) ;
 			}
 			else
 			{
 				msgBack.nGetCoin = COIN_FOR_CHARITY;
 				msgBack.nLeftSecond = TIME_GET_CHARITY_ELAPS ;
 				m_stBaseData.tLastTakeCharityCoinTime = time(NULL) ;
				AddMoney(msgBack.nGetCoin);
				msgBack.nFinalCoin = GetAllCoin();
				CLogMgr::SharedLogMgr()->PrintLog("player uid = %d get charity",GetPlayer()->GetUserUID());
				m_bCommonLogicDataDirty = true ;
 			}
			CLogMgr::SharedLogMgr()->SystemLog("uid = %d , final coin = %I64d",GetPlayer()->GetUserUID(),GetAllCoin());
 			SendMsg(&msgBack,sizeof(msgBack)) ;
		}
		break;
	default:
		{
			return false ;
		}
		break;
	}
	return true ;
}

bool CPlayerBaseData::onCrossServerRequest(stMsgCrossServerRequest* pRequest, eMsgPort eSenderPort,Json::Value* vJsValue )
{
	if ( IPlayerComponent::onCrossServerRequest(pRequest,eSenderPort,vJsValue) )
	{
		return true ;
	}

	switch ( pRequest->nRequestType )
	{
	case eCrossSvrReq_DeductionMoney:
		{
			assert(vJsValue&& "must not null") ;
			assert(pRequest->nTargetID == GetPlayer()->GetUserUID() && "different object");
			bool bDiamoned = !pRequest->vArg[0];
			if ( pRequest->vArg[1] < 0 || pRequest->vArg[2] < 0 )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("why arg is < 0 , for cross deduction uid = %d",GetPlayer()->GetUserUID());
				return true ;
			}
			uint64_t nNeedMoney = pRequest->vArg[1] ;
			int64_t nAtLeast = pRequest->vArg[2];

			bool bRet = onPlayerRequestMoney(nNeedMoney,nAtLeast,bDiamoned) ;
			CLogMgr::SharedLogMgr()->PrintLog("uid = %d do deduction coin cross rquest , final diamond = %I64d, coin = %I64d ret = %b",GetPlayer()->GetUserUID(),m_stBaseData.nDiamoned,m_stBaseData.nCoin ,bRet );
			stMsgCrossServerRequestRet msgRet ;
			msgRet.cSysIdentifer = eSenderPort ;
			msgRet.nRet = bRet ? 0 : 1 ;
			msgRet.nRequestType = pRequest->nRequestType ;
			msgRet.nRequestSubType = pRequest->nRequestSubType;
			msgRet.nTargetID = pRequest->nReqOrigID ;
			msgRet.nReqOrigID = GetPlayer()->GetUserUID() ;
			msgRet.vArg[0] = pRequest->vArg[0];
			msgRet.vArg[1] = nNeedMoney ;

			if ( vJsValue )
			{
				Json::Value& retValue = *vJsValue ;
				CON_REQ_MSG_JSON(msgRet,retValue,autoBuf) ;
				CGameServerApp::SharedGameServerApp()->sendMsg(pRequest->nReqOrigID,autoBuf.getBufferPtr(),autoBuf.getContentSize());
			}
			else
			{
				CGameServerApp::SharedGameServerApp()->sendMsg(pRequest->nReqOrigID,(char*)&msgRet,sizeof(msgRet));
			}

			// save log 
			stMsgSaveLog msgLog ;
			memset(msgLog.vArg,0,sizeof(msgLog.vArg));
			msgLog.nJsonExtnerLen = 0 ;
			msgLog.nLogType = eLog_DeductionMoney ;
			msgLog.nTargetID = GetPlayer()->GetUserUID() ;
			msgLog.vArg[0] = !bDiamoned ;
			msgLog.vArg[1] = bRet ? nNeedMoney : 0;
			msgLog.vArg[2] = m_stBaseData.nCoin;
			msgLog.vArg[3] = m_stBaseData.nDiamoned ;
			msgLog.vArg[4] = pRequest->nRequestSubType ;
			if ( eCrossSvrReqSub_TaxasSitDown == pRequest->nRequestSubType )
			{
				msgLog.vArg[5] = pRequest->nReqOrigID ;
			}
			CGameServerApp::SharedGameServerApp()->sendMsg(pRequest->nReqOrigID,(char*)&msgLog,sizeof(msgLog));

		}
		break;
	case eCrossSvrReq_AddMoney:
		{
			bool bDiamoned = !pRequest->vArg[0];
			int64_t nAddCoin = pRequest->vArg[1] ;
			if ( nAddCoin < 0 )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("why add coin is < 0  uid = %d",GetPlayer()->GetUserUID());
				return true ;
			}

			uint64_t& nAddTarget = bDiamoned ? m_stBaseData.nDiamoned : m_stBaseData.nCoin ; 
			nAddTarget += nAddCoin ;
			m_bMoneyDataDirty = true ;
			CLogMgr::SharedLogMgr()->PrintLog("uid = %d do add coin cross rquest , final diamond = %I64d, coin = %I64d",GetPlayer()->GetUserUID(),m_stBaseData.nDiamoned,m_stBaseData.nCoin );

			// save log 
			stMsgSaveLog msgLog ;
			memset(msgLog.vArg,0,sizeof(msgLog.vArg));
			msgLog.nJsonExtnerLen = 0 ;
			msgLog.nLogType = eLog_AddMoney ;
			msgLog.nTargetID = GetPlayer()->GetUserUID() ;
			msgLog.vArg[0] = !bDiamoned ;
			msgLog.vArg[1] = nAddCoin;
			msgLog.vArg[2] = m_stBaseData.nCoin;
			msgLog.vArg[3] = m_stBaseData.nDiamoned ;
			msgLog.vArg[4] = pRequest->nRequestSubType ;
			if ( eCrossSvrReqSub_TaxasSitDownFailed == pRequest->nRequestSubType || eCrossSvrReqSub_TaxasStandUp == pRequest->nRequestSubType )
			{
				msgLog.vArg[5] = pRequest->nReqOrigID ;
			}
			CGameServerApp::SharedGameServerApp()->sendMsg(pRequest->nReqOrigID,(char*)&msgLog,sizeof(msgLog));
		}
		break;
	case eCrossSvrReq_SyncCoin:
		{
			m_stBaseData.nCoin = pRequest->vArg[0] ;
		}
		break;
	default:
		return false;
	}
	return true ;
}

bool CPlayerBaseData::onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue )
{
	if ( IPlayerComponent::onCrossServerRequestRet(pResult,vJsValue) )
	{
		return true ;
	}
	return false ;
}

void CPlayerBaseData::SendBaseDatToClient()
{
	stMsgPlayerBaseData msg ;
	memcpy(&msg.stBaseData,&m_stBaseData,sizeof(msg.stBaseData));
	msg.nSessionID = GetPlayer()->GetSessionID() ;
	SendMsg(&msg,sizeof(msg)) ;
	CLogMgr::SharedLogMgr()->PrintLog("send base data to session id = %d ",GetPlayer()->GetSessionID() );
	CLogMgr::SharedLogMgr()->SystemLog("send data uid = %d , final coin = %I64d, sex = %d",GetPlayer()->GetUserUID(),GetAllCoin(),msg.stBaseData.nSex);
}

void CPlayerBaseData::OnProcessContinueLogin()
{
// 	if ( m_stBaseData.tLastLoginTime == 0 )
// 	{
// 		m_stBaseData.nContinueDays = 1 ;
// 		m_stBaseData.tLastLoginTime = (unsigned int)time(NULL) ; 
// 	}
// 	else
// 	{
// 		time_t nCur = time(NULL) ;
// 		struct tm* pTempTimer = NULL;
// 		pTempTimer = localtime(&nCur) ;
// 		struct tm pTimeCur ;
// 		if ( pTempTimer )
// 		{
// 			pTimeCur = *pTempTimer ;
// 		}
// 		else
// 		{
// 			CLogMgr::SharedLogMgr()->ErrorLog("local time return null ?") ;
// 		}
//  
// 		pTempTimer = localtime((time_t*)&m_stBaseData.tLastLoginTime) ;
// 		struct tm pTimeLastLogin  ;
// 		if ( pTempTimer )
// 		{
// 			pTimeLastLogin = *pTempTimer ;
// 		}
// 		else
// 		{
// 			CLogMgr::SharedLogMgr()->ErrorLog("local time return null ?") ;
// 		}
// 		
// 		if ( pTimeCur.tm_year == pTimeLastLogin.tm_year && pTimeCur.tm_mon == pTimeLastLogin.tm_mon && pTimeCur.tm_yday == pTimeLastLogin.tm_yday )
// 		{
// 			m_stBaseData.tLastLoginTime = (unsigned int)nCur ;
// 			m_bGivedLoginReward = true ;
// 			return ; // do nothing ; same day ;
// 		}
// 
// 		double nDiffe = difftime(nCur,m_stBaseData.tLastLoginTime) ;
// 		bool bContine = abs(nDiffe) - 60 * 60 * 24 <= 0 ;
// 		
// 		if ( bContine )
// 		{
// 			++m_stBaseData.nContinueDays ;   // real contiune ;
// 		}
// 		else
// 		{
// 			m_stBaseData.nContinueDays = 1 ;    // disturbed ;
// 		}
// 
// 		m_stBaseData.tLastLoginTime = (unsigned int)nCur ;
// 	}
// 
// 	stMsgShowContinueLoginDlg msg ;
// 	msg.nContinueIdx = m_stBaseData.nContinueDays ;
// 	SendMsgToClient((char*)&msg,sizeof(msg)) ;
// 	// first login event ;
// 	stPlayerEvetArg evet ;
// 	evet.eEventType = ePlayerEvent_FirstLogin ;
// 	GetPlayer()->PostPlayerEvent(&evet);
}

void CPlayerBaseData::OnPlayerDisconnect()
{
	TimerSave();
	CEventCenter::SharedEventCenter()->RemoveEventListenner(eEvent_NewDay,this,CPlayerBaseData::EventFunc ) ;
}

void CPlayerBaseData::TimerSave()
{
	if ( m_bMoneyDataDirty )
	{
		m_bMoneyDataDirty = false ;
		stMsgSavePlayerMoney msgSaveMoney ;
		msgSaveMoney.nCoin = m_stBaseData.nCoin;
		msgSaveMoney.nDiamoned = m_stBaseData.nDiamoned;
		msgSaveMoney.nUserUID = GetPlayer()->GetUserUID() ;
		SendMsg((stMsgSavePlayerMoney*)&msgSaveMoney,sizeof(msgSaveMoney)) ;
		CLogMgr::SharedLogMgr()->PrintLog("player do time save coin uid = %d coin = %I64d",msgSaveMoney.nUserUID,msgSaveMoney.nCoin );
	}

	if ( m_bCommonLogicDataDirty )
	{
		m_bCommonLogicDataDirty = false ;
		stMsgSavePlayerCommonLoginData msgLogicData ;
		msgLogicData.dfLatidue = m_stBaseData.dfLatidue ;
		msgLogicData.dfLongitude = m_stBaseData.dfLongitude ;
		msgLogicData.nContinueDays = m_stBaseData.nContinueDays ;
		//msgLogicData.nExp = m_stBaseData.nExp ;
		msgLogicData.nMostCoinEver = m_stBaseData.nMostCoinEver;
		msgLogicData.nTodayCoinOffset = m_stBaseData.nTodayCoinOffset ;
		msgLogicData.nYesterdayCoinOffset = m_stBaseData.nYesterdayCoinOffset ;
		msgLogicData.nUserUID = GetPlayer()->GetUserUID() ;
		msgLogicData.nVipLevel = m_stBaseData.nVipLevel ;
		msgLogicData.tLastLoginTime = m_stBaseData.tLastLoginTime ;
		msgLogicData.tLastTakeCharityCoinTime = m_stBaseData.tLastTakeCharityCoinTime ;
		msgLogicData.tOfflineTime = m_stBaseData.tOfflineTime ;
		memcpy(msgLogicData.vJoinedClubID,m_stBaseData.vJoinedClubID,sizeof(msgLogicData.vJoinedClubID));
		SendMsg((stMsgSavePlayerMoney*)&msgLogicData,sizeof(msgLogicData)) ;
	}

	if ( m_bPlayerInfoDataDirty )
	{
		m_bPlayerInfoDataDirty = false ;
		stMsgSavePlayerInfo msgSaveInfo ;
		msgSaveInfo.nPhotoID = m_stBaseData.nPhotoID ;
		msgSaveInfo.nIsRegister = m_stBaseData.isRegister ;
		msgSaveInfo.nSex = m_stBaseData.nSex ;
		msgSaveInfo.nUserUID = GetPlayer()->GetUserUID() ;
		memcpy(msgSaveInfo.vName,m_stBaseData.cName,sizeof(msgSaveInfo.vName));
		memcpy(msgSaveInfo.vSigure,m_stBaseData.cSignature,sizeof(msgSaveInfo.vSigure));
		memcpy(msgSaveInfo.vUploadedPic,m_stBaseData.vUploadedPic,sizeof(msgSaveInfo.vUploadedPic));
		SendMsg((stMsgSavePlayerMoney*)&msgSaveInfo,sizeof(msgSaveInfo)) ;
	}
}

bool CPlayerBaseData::onPlayerRequestMoney(uint64_t& nCoinOffset,uint64_t nAtLeast, bool bDiamoned)
{
	bool invalidAtLeast = (nAtLeast != 0 && nAtLeast < nCoinOffset );

 	if ( bDiamoned == false )
 	{
 		if ( nCoinOffset > GetAllCoin() )
		{
			if ( invalidAtLeast && GetAllCoin() >= nAtLeast )
			{
				nCoinOffset = nAtLeast ;
				m_stBaseData.nCoin -= nCoinOffset ;
				m_bMoneyDataDirty = true ;
				return true ;
			}
			return false ;
		}
 		//m_nTaxasPlayerCoin += nCoinOffset ;   //add after recieved comfirm msg 
 		m_stBaseData.nCoin -= nCoinOffset ;
 	}
 	else
 	{
		if ( nCoinOffset > GetAllDiamoned() )
		{
			if ( invalidAtLeast && GetAllDiamoned() >= nAtLeast )
			{
				nCoinOffset = nAtLeast ;
				m_stBaseData.nDiamoned -= nCoinOffset ;
				m_bMoneyDataDirty = true ;
				return true ;
			}

			return false ;
		}
 		// m_nTaxasPlayerDiamoned += nCoinOffset; ; //add after recieved comfirm msg 
 		m_stBaseData.nDiamoned -= nCoinOffset ;
 	}
	m_bMoneyDataDirty = true ;
	return true ;
}

bool CPlayerBaseData::AddMoney(int64_t nOffset,bool bDiamond  )
{
	if ( bDiamond )
	{
		m_stBaseData.nDiamoned += (int)nOffset ;
	}
	else
	{
		m_stBaseData.nCoin += nOffset ;
	}
	m_bMoneyDataDirty = true ;
	return true ;
}

bool CPlayerBaseData::decressMoney(int64_t nOffset,bool bDiamond )
{
	if ( bDiamond )
	{
		if ( m_stBaseData.nDiamoned < nOffset )
		{
			return false ;
		}
		m_stBaseData.nDiamoned -= nOffset ;
		m_bMoneyDataDirty = true ;
		return true ;
	}

	if ( m_stBaseData.nCoin < nOffset )
	{
		return false ;
	}
	m_stBaseData.nCoin -= nOffset ;
	m_bMoneyDataDirty = true ;
	return true ;
}

bool CPlayerBaseData::OnPlayerEvent(stPlayerEvetArg* pArg)
{
	return false ;
}

void CPlayerBaseData::GetPlayerBrifData(stPlayerBrifData* pData )
{
	if ( !pData )
	{
		return ;
	}
	memcpy(pData,&m_stBaseData,sizeof(stPlayerBrifData));
}

void CPlayerBaseData::GetPlayerDetailData(stPlayerDetailData* pData )
{
	if ( !pData )
	{
		return ;
	}
	memcpy(pData,&m_stBaseData,sizeof(stPlayerDetailData));
}

bool CPlayerBaseData::EventFunc(void* pUserData,stEventArg* pArg)
{
	return false ;
}

void CPlayerBaseData::OnNewDay(stEventArg* pArg)
{
// 	m_stBaseData.nYesterdayPlayTimes = m_stBaseData.nTodayPlayTimes ;
// 	m_stBaseData.nTodayPlayTimes = 0 ;
// 	m_stBaseData.nYesterdayWinCoin = m_stBaseData.nTodayWinCoin ;
// 	m_stBaseData.nTodayWinCoin = 0 ;
}

void CPlayerBaseData::OnReactive(uint32_t nSessionID )
{
	CEventCenter::SharedEventCenter()->RegisterEventListenner(eEvent_NewDay,this,CPlayerBaseData::EventFunc ) ;
	CLogMgr::SharedLogMgr()->PrintLog("player reactive send base data");
	SendBaseDatToClient();

	stMsgRequestClientIp msgReq ;
	SendMsg(&msgReq,sizeof(msgReq)) ; 
	CLogMgr::SharedLogMgr()->PrintLog("send request ip , sessioni id = %d",GetPlayer()->GetSessionID()) ;
}

void CPlayerBaseData::OnOtherDoLogined()
{ 	
	stMsgRequestClientIp msgReq ;
	SendMsg(&msgReq,sizeof(msgReq)) ; 
	CLogMgr::SharedLogMgr()->PrintLog("send request ip , sessioni id = %d",GetPlayer()->GetSessionID()) ;

	SendBaseDatToClient();
	OnProcessContinueLogin();
}

