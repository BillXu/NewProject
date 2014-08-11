#include "RoomGoldenNew.h"
#include "GoldenRoomData.h"
#include "Player.h"
#include "PlayerBaseData.h"
#include "GoldenMessageDefine.h"
#include "LogManager.h"
bool CRoomGoldenNew::Init()
{
	m_pRoomData = new CGoldenRoomData ;
	m_pRoomData->Init();
	return true ;
}

void CRoomGoldenNew::Enter(CPlayer* pEnter )
{
	CRoomBaseNew::Enter(pEnter);

	// create room peer data ;
	stGoldenPeerData* pPeer = new stGoldenPeerData ;
	memset(pPeer,0,sizeof(stGoldenPeerData));
	memcpy(pPeer->cName,pEnter->GetBaseData()->GetData()->cName,sizeof(pPeer->cName));
	pPeer->cVipLevel = pEnter->GetBaseData()->GetData()->nVipLevel ;
	pPeer->nBetCoin = 0 ;
	pPeer->nCurCoin = pEnter->GetBaseData()->GetAllCoin() ;
	pEnter->GetBaseData()->GetData()->nCoin = 0 ;  // all coin take in to this room ;
	pPeer->nPeerState = eRoomPeer_Golden_WaitNextPlay ;
	if ( m_pRoomData->m_pData->cCurRoomState == eRoomState_Golden_WaitPeerToJoin || eRoomState_Golden_WaitPeerReady == m_pRoomData->m_pData->cCurRoomState )
	{
		pPeer->nPeerState = eRoomPeer_Golden_WaitToReady ;
	}
	pPeer->nSessionID = pEnter->GetSessionID();
	pPeer->nDefaultPhotoID = pEnter->GetBaseData()->GetData()->nDefaulPhotoID;
	pPeer->nUserDefinePhotoID = pEnter->GetBaseData()->GetData()->bIsUploadPhoto ;
	pPeer->nUserUID = pEnter->GetUserUID();

	// add room peer data to room 
	m_pRoomData->AddPeer(pPeer) ;
	if ( eRoomState_Golden_WaitPeerReady == m_pRoomData->m_pData->cCurRoomState )  // give the new comer time to select READY 
	{
		m_pRoomData->m_pData->fTimeTick = 0 ;
	}

	// send room info to the new comer ;
	SendRoomInfoToPlayer(pEnter);

	// tell others the peer come ;
	stMsgGoldenRoomEnter msgPeerEnter ;
	memcpy(&msgPeerEnter.tPeerEnterPeerData,pPeer,sizeof(msgPeerEnter.tPeerEnterPeerData));
	SendMsgBySessionID(&msgPeerEnter,sizeof(msgPeerEnter),pEnter->GetSessionID());
}

void CRoomGoldenNew::Leave(CPlayer* pLeaver)
{
	CRoomBaseNew::Leave(pLeaver);
	// do something here ;
}

unsigned char CRoomGoldenNew::CheckCanJoinThisRoom(CPlayer* pPlayer) // 0 means ok , other value means failed ;
{
	if ( m_pRoomData->GetEmptySeatCnt() > 0 )
		return 0 ;
	return  1;
}

void CRoomGoldenNew::SendRoomInfoToPlayer(CPlayer* pPlayer)
{
	stMsgGoldenRoomInfo msgInfo ;
	memcpy(&msgInfo.tData,GetRoomDataOnly(),sizeof(msgInfo.tData) );
	msgInfo.nPeerCnt = m_pRoomData->GetPlayingSeatCnt();
	msgInfo.nSelfIdx = m_pRoomData->GetPeerDataBySessionID(pPlayer->GetSessionID())->cRoomIdx ;
	
	unsigned short nSize = sizeof(msgInfo) + sizeof(stGoldenPeerData) * msgInfo.nPeerCnt ;
	char* pBuffer = new char[nSize] ;
	memset(pBuffer,0,nSize) ;
	memcpy(pBuffer,&msgInfo,sizeof(msgInfo));
	stGoldenPeerData* pPeerData = (stGoldenPeerData*)(pBuffer + sizeof(msgInfo));
	for ( int i = 0 ; i < m_pRoomData->m_pData->cMaxPlayingPeers ; ++i )
	{
		if ( m_pRoomData->m_vPeerDatas[i] )
		{
			memcpy(pPeerData,m_pRoomData->m_vPeerDatas[i],sizeof(stGoldenPeerData));
			++pPeerData ;
		}
	}
	pPlayer->SendMsgToClient(pBuffer,nSize) ;
	delete[] pBuffer ;
	pBuffer = NULL ;
}

void CRoomGoldenNew::Update(float fTimeElpas, unsigned int nTimerID )
{
	GetRoomDataOnly()->fTimeTick += fTimeElpas ;
	switch ( GetRoomDataOnly()->cCurRoomState )
	{
	case eRoomState_Golden_WaitPeerToJoin:
		{
			if ( m_pRoomData->GetPlayingSeatCnt() >= 2 )
			{
				GoToState(eRoomState_Golden_WaitPeerReady);
			}
		}
		break;
	case eRoomState_Golden_WaitPeerReady:
		{
			if ( GetRoomDataOnly()->fTimeTick < TIME_GOLDEN_ROOM_WAIT_READY )
				return ;
			if ( ((CGoldenRoomData*)m_pRoomData)->GetReadyPeerCnt() >= 2 )
			{
				((CGoldenRoomData*)m_pRoomData)->OnStartGame();
				GoToState(eRoomState_Golden_DistributeCard) ;
			}
		}
		break;
	case eRoomState_Golden_DistributeCard:
		{
			if ( GetRoomDataOnly()->fTimeTick < TIME_GOLDEN_ROOM_DISTRIBUTY )
				return ;
			GoToState(eRoomState_Golden_WaitPeerAction);
		}
		break;
	case eRoomState_Golden_WaitPeerAction:
		{
			if ( GetRoomDataOnly()->fTimeTick < TIME_GOLDEN_ROOM_WAIT_ACT )
				return ;
			CPlayer* pPlayer = GetPlayerByRoomIdx( ((stRoomGoldenDataOnly*)GetRoomDataOnly())->cCurActIdx );
			if ( !pPlayer )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("why cur act peer is NULL ? Bug") ;
				if ( !GameOverCheckAndProcess() )
				{
					NextPlayerAct();
				}
			}
			else
			{
				stMsgGoldenRoomPlayerGiveUp msg ;
				OnMessage(pPlayer,&msg);
			}
		}
		break;
	case eRoomState_Golden_PKing:
		{
			if ( GetRoomDataOnly()->fTimeTick < TIME_GOLDEN_ROOM_PK )
				return ;
			
			if ( !GameOverCheckAndProcess() )
			{
				NextPlayerAct();
			}
		}
		break;
	case eRoomState_Golden_ShowingResult:
		{
			if ( GetRoomDataOnly()->fTimeTick < TIME_GOLDEN_ROOM_RESULT )
				return ;
			((CGoldenRoomData*)m_pRoomData)->OnEndGame();
			GoToState(eRoomState_Golden_WaitPeerToJoin);
		}
		break;
	default:
		CLogMgr::SharedLogMgr()->ErrorLog("unknown state %d for golden ",GetRoomDataOnly()->cCurRoomState) ;
	}
}

void CRoomGoldenNew::GoToState(unsigned char cTargetState)
{
	unsigned char nPreSate = GetRoomDataOnly()->cCurRoomState ;
	GetRoomDataOnly()->cCurRoomState  = cTargetState ;
	GetRoomDataOnly()->fTimeTick = 0 ;
	switch ( GetRoomDataOnly()->cCurRoomState )
	{
	case eRoomState_Golden_WaitPeerToJoin:
		{
			// do nothing 
		}
		break;
	case eRoomState_Golden_WaitPeerReady:
		{
			
		}
		break;
	case eRoomState_Golden_DistributeCard:
		{
			GetRoomData()->DistributeCard();
			stMsgGoldenRoomDistributy msg ;
			msg.cBankIdx = ((stRoomGoldenDataOnly*)GetRoomDataOnly())->cBankerIdx ;
			SendMsgBySessionID(&msg,sizeof(msg),0) ;
		}
		break;
	case eRoomState_Golden_WaitPeerAction:
		{
			NextPlayerAct() ;
		}
		break;
	case eRoomState_Golden_PKing:
		{
			// do nothing 
		}
		break;
	case eRoomState_Golden_ShowingResult:
		{
			// do nothing  caculate was done in GameOverCheckAndProcess function ;
		}
		break;
	default:
		CLogMgr::SharedLogMgr()->ErrorLog("unknown Target state %d for golden ",GetRoomDataOnly()->cCurRoomState) ;
	}
}

bool CRoomGoldenNew::OnMessage(CPlayer*pSender, stMsg* pmsg)
{
	if ( CRoomBaseNew::OnMessage(pSender,pmsg) )
	{
		return true ;
	}

	switch (pmsg->usMsgType )
	{
	case MSG_GOLDEN_ROOM_PLAYER_READY:
		{
			stMsgGoldenRoomPlayerReadyRet msgBack ;
			msgBack.nRet = GetRoomData()->OnPlayerReady(pSender->GetSessionID());
			SendMsgBySessionID(&msgBack,sizeof(msgBack),pSender->GetSessionID(),false) ;

			if ( msgBack.nRet == 0 )
			{
				stMsgGoldenRoomReady msgAll ;
				msgAll.nReadyPlayerIdx = GetRoomData()->GetRoomIdxBySessionID(pSender->GetSessionID());
				SendMsgBySessionID(&msgAll,sizeof(msgAll)) ;
			}
		}
		break;
	case MSG_GOLDEN_ROOM_PLAYER_LOOK:
		{
			stMsgGoldenRoomPlayerLookRet msgBack ;
			msgBack.nRet = GetRoomData()->OnPlayerLook(pSender->GetSessionID());
			SendMsgBySessionID(&msgBack,sizeof(msgBack),pSender->GetSessionID(),false) ;

			if (msgBack.nRet == 0 )
			{
				stMsgGoldenRoomLook msgAll ;
				msgAll.cLookPlayerIdx = GetRoomData()->GetRoomIdxBySessionID(pSender->GetSessionID());
				stGoldenPeerData* peer = (stGoldenPeerData*)GetRoomData()->GetPeerDataBySessionID(pSender->GetSessionID()) ;
				memcpy(msgAll.vCard,peer->vHoldCard,sizeof(msgAll.vCard));
				SendMsgBySessionID(&msgAll,sizeof(msgAll)) ;

				if ( GetRoomData()->GetDataOnly()->cCurActIdx == GetRoomData()->GetRoomIdxBySessionID(pSender->GetSessionID()) )
				{
					GetRoomData()->GetDataOnly()->fTimeTick = 0 ;
				}
			}
		}
		break;
	case MSG_GOLDEN_ROOM_PLAYER_GIVEUP:
		{
			stMsgGoldenRoomPlayerGiveUpRet msgBack ;
			msgBack.nRet = GetRoomData()->OnPlayerGiveUp(pSender->GetSessionID());
			SendMsgBySessionID(&msgBack,sizeof(msgBack),pSender->GetSessionID(),false) ;

			if ( msgBack.nRet == 0 )
			{
				stMsgGoldenRoomGiveUp msgAll ;
				msgAll.nGiveUpIdx = GetRoomData()->GetRoomIdxBySessionID(pSender->GetSessionID()) ;
				SendMsgBySessionID(&msgAll,sizeof(msgAll)) ;

				if ( GameOverCheckAndProcess() == false )
				{
					if ( GetRoomData()->GetDataOnly()->cCurActIdx == GetRoomData()->GetRoomIdxBySessionID(pSender->GetSessionID()) )
					{
						NextPlayerAct();
					}
				}
			}
		}
		break;
	case MSG_GOLDEN_ROOM_PLAYER_FOLLOW:
		{
			uint64_t nFinalCoin = 0;

			stMsgGoldenRoomPlayerFollowRet msgBack ;
			msgBack.nRet = GetRoomData()->OnPlayerFollow(pSender->GetSessionID(),nFinalCoin) ;
			SendMsgBySessionID(&msgBack,sizeof(msgBack),pSender->GetSessionID(),false) ;

			if ( 0 == msgBack.nRet )
			{
				stMsgGoldenRoomFollow msgAll ;
				msgAll.nFollowIdx = GetRoomData()->GetRoomIdxBySessionID(pSender->GetSessionID()) ;
				msgAll.nFollowCoin = nFinalCoin ;
				SendMsgBySessionID(&msgAll,sizeof(msgAll)) ;

				NextPlayerAct();
			}
		}
		break;
	case MSG_GOLDEN_ROOM_PLAYER_ADD:
		{
			stMsgGoldenRoomPlayerAdd* pRet = (stMsgGoldenRoomPlayerAdd*)pmsg ;

			uint64_t nFinalCoin = 0 ;
			stMsgGoldenRoomPlayerAddRet msgBack ;
			msgBack.nRet = GetRoomData()->OnPlayerAdd(pSender->GetSessionID(),pRet->nAddCoin,nFinalCoin) ;
			SendMsgBySessionID(&msgBack,sizeof(msgBack),pSender->GetSessionID(),false) ;

			if ( msgBack.nRet == 0 )
			{
				stMsgGoldenRoomAdd msgAll ;
				msgAll.nActIdx = GetRoomData()->GetRoomIdxBySessionID(pSender->GetSessionID()) ;
				msgAll.nAddCoin = pRet->nAddCoin;
				SendMsgBySessionID(&msgAll,sizeof(msgAll)) ;

				NextPlayerAct();
			}
		}
		break;
	case MSG_GOLDEN_ROOM_PLAYER_PK:
		{
			stMsgGoldenRoomPlayerPK* pRet = (stMsgGoldenRoomPlayerPK*)pmsg ;
			bool bwin = false;

			stMsgGoldenRoomPlayerPKRet msgBack ;
			msgBack.cRet = GetRoomData()->OnPlayerPK(GetRoomData()->GetRoomIdxBySessionID(pSender->GetSessionID()),pRet->nPkTargetIdx,bwin) ;
			SendMsgBySessionID(&msgBack,sizeof(msgBack),pSender->GetSessionID(),false) ;

			if ( msgBack.cRet == 0 )
			{
				stMsgGoldenRoomPK msgAll ;
				msgAll.nActPlayerIdx = GetRoomData()->GetRoomIdxBySessionID(pSender->GetSessionID()) ;
				msgAll.nTargetIdx = pRet->nPkTargetIdx ;
				msgAll.bWin = bwin ;
				SendMsgBySessionID(&msgAll,sizeof(msgAll)) ;

				GoToState(eRoomState_Golden_PKing) ;
			}
		}
		break;
	default:
		return false ;
	}
	return true ;
}

bool CRoomGoldenNew::GameOverCheckAndProcess()
{
	char nWinerIdx = GetRoomData()->GameOverCheckAndProcess();
	if ( nWinerIdx < 0 )
	{
		return false ;
	}

	stMsgGoldenRoomResult msgResult ;
	msgResult.cWinnerIdx = nWinerIdx ;
	msgResult.nWinCoin = ((stRoomGoldenDataOnly*)GetRoomDataOnly())->nAllBetCoin ;
	stGoldenPeerData* pdata = (stGoldenPeerData*)GetRoomData()->GetPeerDataByIdx(nWinerIdx) ;
	memcpy(msgResult.vCard,pdata->vHoldCard,sizeof(msgResult.vCard));
	SendMsgBySessionID(&msgResult,sizeof(msgResult),0) ;

	GoToState(eRoomState_Golden_ShowingResult) ;
	return true ;
}

void CRoomGoldenNew::NextPlayerAct()
{
	stRoomGoldenDataOnly* pData = (stRoomGoldenDataOnly*)GetRoomDataOnly();
	char nIdx = GetRoomData()->GetNextActIdx();
	if ( nIdx < 0 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why next act peer is less 0 , game over ? ") ;
		GameOverCheckAndProcess();
		return ;
	}
	stMsgGoldenRoomInformAct msg ;
	msg.nActIdx = nIdx ;
	msg.nCurMaxBet = pData->nCurMaxBet ;
	msg.nRound = pData->nRound ;
	msg.nTotalBetCoin = pData->nAllBetCoin ;
	SendMsgBySessionID(&msg,sizeof(msg)) ;
}

CGoldenRoomData* CRoomGoldenNew::GetRoomData()
{
	CGoldenRoomData* pData = (CGoldenRoomData*)m_pRoomData ;
	return pData ;
}