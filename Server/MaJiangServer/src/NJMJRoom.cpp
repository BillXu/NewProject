#include "NJMJRoom.h"
#include "NJMJPlayer.h"
#include "NJMJPlayerCard.h"
#include "log4z.h"
#include "IMJPoker.h"
#include "ServerMessageDefine.h"
#include "NJRoomStateWaitPlayerAct.h"
#include "MJRoomStateWaitReady.h"
#include "MJRoomStateWaitPlayerChu.h"
#include "MJRoomStateWaitPlayerAct.h"
#include "MJRoomStateGameEnd.h"
#include "NJRoomStateDoPlayerAct.h"
#include "MJRoomStateAskForPengOrHu.h"
#include "IGameRoomManager.h"
#include "NJRoomStateBuHua.h"
#include "NJRoomStateStartGame.h"
#include "MJRoomStateAskForRobotGang.h"
#include "NJMJPlayerRecorderInfo.h"
#include <ctime>
#include "MJPrivateRoom.h"
#include "MJReplayFrameType.h"
#define PUNISH_COIN_BASE 5 
#define AN_GANG_COIN_BASE 5 
#define MING_GANG_COIN_BASE 10
#define HU_GANG_COIN_BASE 10
bool NJMJRoom::init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSeialNum, uint32_t nRoomID, Json::Value& vJsValue)
{
	IMJRoom::init(pRoomMgr, pConfig, nSeialNum, nRoomID, vJsValue);
	m_isHuangZhuang = false;
	m_isBiXiaHu = false;
	m_isWillBiXiaHu = false;
	m_isBankerHu = false;
	m_isEnableBixiaHu = false;
	m_isEnableHuaZa = false ;
	m_isKuaiChong = false;
	m_nKuaiChongPool = 0;
	m_nInitKuaiChongPool = 0;
	m_isJingYuanZiMode = false;
	m_isEnableWaiBao = false;
	m_isEnableSiLianFeng = false;
	m_isBaoPaiHappend = false;
	m_isSiLianFengFaQian = false;
	m_pPrivateRoom = nullptr;
	m_vSettle.clear();
	m_isLianZhuang = false;
	m_tChuedCards.clear();
	if ( vJsValue["initCoin"].isNull() == false)
	{
		m_nInitCoin = vJsValue["initCoin"].asUInt();
		m_isJingYuanZiMode = vJsValue["initCoin"].asUInt() < 500;
	}

	if (vJsValue["isHuaZa"].isNull() || vJsValue["isHuaZa"].isUInt() == false)
	{
		LOGFMTE("argument is not proper room id = %u , hua za ? " , getRoomID());
	}
	else
	{
		m_isEnableHuaZa = vJsValue["isHuaZa"].asUInt() == 0 ? false : true;
	}

	if (vJsValue["isSiLianFeng"].isNull() || vJsValue["isSiLianFeng"].isUInt() == false)
	{
		LOGFMTE("argument is not proper room id = %u ,isSiLianFeng ? ", getRoomID());
	}
	else
	{
		m_isEnableSiLianFeng = vJsValue["isSiLianFeng"].asUInt() == 0 ? false : true;
	}

	if (vJsValue["isWaiBao"].isNull() || vJsValue["isWaiBao"].isUInt() == false)
	{
		LOGFMTE("argument is not proper room id = %u ,isWaiBao ? ", getRoomID());
	}
	else
	{
		m_isEnableWaiBao = vJsValue["isWaiBao"].asUInt() == 0 ? false : true;
	}

	if (vJsValue["isBiXiaHu"].isNull() || vJsValue["isBiXiaHu"].isUInt() == false)
	{
		LOGFMTE("argument is not proper room id = %u , isBiXiaHu  ? ", getRoomID());
	}
	else
	{
		m_isEnableBixiaHu = vJsValue["isBiXiaHu"].asUInt() == 0 ? false : true;
	}

	if ( vJsValue["isKuaiChong"].isNull() || vJsValue["isKuaiChong"].isUInt() == false)
	{
		LOGFMTE("argument is not proper room id = %u , isKuaiChong  ? ", getRoomID());
	}
	else
	{
		m_isKuaiChong = vJsValue["isKuaiChong"].asUInt() == 0 ? false : true;
	}

	if ( m_isKuaiChong )
	{
		if (vJsValue["kuaiChongCoin"].isNull() || vJsValue["kuaiChongCoin"].isUInt() == false)
		{
			LOGFMTE("argument is not proper room id = %u , kuaiChongCoin  ? ", getRoomID());
		}
		else
		{
			m_nKuaiChongPool = vJsValue["kuaiChongCoin"].asUInt();
			m_nInitKuaiChongPool = m_nKuaiChongPool;
		}
	}

	m_tPoker.initAllCard(eMJ_NanJing);
	// create state and add state ;
	IMJRoomState* vState[] = {
		new CMJRoomStateWaitReady(), new MJRoomStateWaitPlayerChu(), new NJRoomStateWaitPlayerAct(), new NJRoomStateStartGame(), new NJRoomStateBuHua()
		, new MJRoomStateGameEnd(), new NJRoomStateDoPlayerAct(), new MJRoomStateAskForPengOrHu(), new MJRoomStateAskForRobotGang()
	};
	for (uint8_t nIdx = 0; nIdx < sizeof(vState) / sizeof(IMJRoomState*); ++nIdx)
	{
		addRoomState(vState[nIdx]);
	}
	setInitState(vState[0]);

	// init banker
	m_nBankerIdx = - 1;

	// init room recorder info 
	auto pRoomRecorder = (NJMJRoomRecorder*)getRoomRecorder().get();
	pRoomRecorder->setJingYuanZi(m_isJingYuanZiMode, m_nInitCoin, m_isEnableWaiBao );
	pRoomRecorder->setKuaiChong( m_isKuaiChong,m_nInitKuaiChongPool);
	pRoomRecorder->setHuaZaBiXiaHu(m_isEnableHuaZa,m_isEnableBixiaHu,m_isEnableSiLianFeng );
	return true;
}

void NJMJRoom::willStartGame()
{
	IMJRoom::willStartGame();
	m_vSettle.clear();
	m_tChuedCards.clear();
	m_isWillProcessChuPaiFaQian = false;
	m_nChuedCard = 0;
	m_nChuPaiPlayerIdx = -1;

	if ((uint8_t)-1 == m_nBankerIdx )
	{
		m_nBankerIdx = 0;
	}
	else
	{
		if ( !m_isLianZhuang )
		{
			m_nBankerIdx = (m_nBankerIdx + 1) % MAX_SEAT_CNT;
		}
	}

	m_isBiXiaHu = false;

	if (m_isWillBiXiaHu)
	{
		m_isBiXiaHu = true;
	}

	m_isWillBiXiaHu = false;

	m_isBankerHu = false;
	m_isHuangZhuang = false;
	m_isBaoPaiHappend = false;
	m_isSiLianFengFaQian = false;
	m_isLianZhuang = false;
}

void NJMJRoom::packStartGameMsg(Json::Value& jsMsg)
{
	IMJRoom::packStartGameMsg(jsMsg);
	jsMsg["isBiXiaHu"] = isBiXiaHu() ? 1 : 0;
}

void NJMJRoom::startGame()
{
	IMJRoom::startGame();
	// bind room to player card 
	// check di hu 
	for (auto& pPlayer : m_vMJPlayers)
	{
		if (pPlayer == nullptr)
		{
			LOGFMTE("room id = %u , start game player is nullptr", getRoomID() );
			continue;
		}

		auto pPlayerCard = (NJMJPlayerCard*)pPlayer->getPlayerCard();
		pPlayerCard->bindRoom(this,pPlayer->getIdx());

		if ( pPlayerCard->getHoldCardHuaCnt() > 0 ) //have hua can not di hu or tian hu ;
		{
			continue;
		}

		// avoid di hu and tian hu 
		//uint8_t nNewCard = 0;
		//doAvoidPlayerTianHuOrDiHu(pPlayer->getIdx(), nNewCard,false );
	}

	Json::Value jsMsg;
	packStartGameMsg(jsMsg);
	sendRoomMsg(jsMsg, MSG_ROOM_START_GAME);

	// replay arg 
	Json::Value jsReplayInfo;
	jsReplayInfo["roomID"] = getRoomID();
	jsReplayInfo["time"] = (uint32_t)time(nullptr);
	jsReplayInfo["isKuaiChong"] = m_isKuaiChong ? 1 : 0 ;
	jsReplayInfo["kuaiChongPool"] = m_isKuaiChong ? m_nKuaiChongPool : 0;
	jsReplayInfo["yuanZiCoin"] = m_nInitCoin;
	jsReplayInfo["isHuaZa"] = m_isEnableHuaZa ? 1 : 0;
	jsReplayInfo["isBiXiaHu"] = isBiXiaHu() ? 1 : 0;
	jsReplayInfo["isWaiBao"] = isEnableWaiBao() ? 1 : 0;
	jsReplayInfo["isSiLianFeng"] = isEnableSiLianFeng() ? 1 : 0;
	getGameReplay()->setReplayRoomInfo(jsReplayInfo);
}

void NJMJRoom::getSubRoomInfo(Json::Value& jsSubInfo)
{
	jsSubInfo["isBiXiaHu"] = isBiXiaHu() ? 1 : 0;
	jsSubInfo["isWaiBao"] = m_isEnableWaiBao ? 1 : 0;
	jsSubInfo["isRoomBiXiaHu"] = m_isEnableBixiaHu ? 1 : 0;
	jsSubInfo["isHuaZa"] = m_isEnableHuaZa ? 1 : 0;
	jsSubInfo["isKuaiChong"] = isKuaiChong() ? 1 : 0;
	jsSubInfo["isSiLianFeng"] = isEnableSiLianFeng() ? 1 : 0;
	if (isKuaiChong())
	{
		jsSubInfo["kuaiChongCoin"] = m_nInitKuaiChongPool;
		jsSubInfo["curKuaiChongCoin"] = m_nKuaiChongPool;
	}
}

void NJMJRoom::onGameDidEnd()
{
	IMJRoom::onGameDidEnd();
	if (getDelegate())
	{
		getDelegate()->onDidGameOver(this);
		return;
	}
}

void NJMJRoom::onGameEnd()
{
	// svr: { isLiuJu : 0 , detail : [ {idx : 0 , offset : 23 }, ...  ], realTimeCal : [ { actType : 23, detial : [ {idx : 2, offset : -23 } ]  } , ... ] } 
	Json::Value jsMsg;
	Json::Value jsDetial;

	auto ptrSingleRecorder = getRoomRecorder()->createSingleRoundRecorder();
	ptrSingleRecorder->init( getRoomRecorder()->getRoundRecorderCnt(), (uint32_t)time(nullptr), getGameReplay()->getReplayID() );
	getRoomRecorder()->addSingleRoundRecorder(ptrSingleRecorder);

	bool isAnyOneHu = false;
	for (auto& ref : m_vMJPlayers)
	{
		Json::Value js;
		if (ref)
		{
			js["idx"] = ref->getIdx();
			js["offset"] = ref->getOffsetCoin();
			js["waiBaoOffset"] = ((NJMJPlayer*)ref)->getWaiBaoOffset();
			js["final"] = ref->getCoin();
			jsDetial[jsDetial.size()] = js;

			auto pPlayerRecorderInfo = std::make_shared<NJMJPlayerRecorderInfo>();
			pPlayerRecorderInfo->init(ref->getUID(), ref->getOffsetCoin(), ((NJMJPlayer*)ref)->getWaiBaoOffset());
			ptrSingleRecorder->addPlayerRecorderInfo(pPlayerRecorderInfo);
		}

		if (ref && ref->haveState(eRoomPeer_AlreadyHu))
		{
			isAnyOneHu = true;
			continue;
		}
	}

	jsMsg["isLiuJu"] = isAnyOneHu ? 0 : 1;
	jsMsg["detail"] = jsDetial;

	m_isHuangZhuang = !isAnyOneHu;
	if ( m_isHuangZhuang )  // huang zhuang ;
	{
		m_isWillBiXiaHu = true;

		Json::Value jsReal;
		settleInfoToJson(jsReal);
		jsMsg["realTimeCal"] = jsReal;
	}

	bool isNextBiXiaWhu = m_isEnableBixiaHu && m_isWillBiXiaHu ;
	jsMsg["isNextBiXiaHu"] = isNextBiXiaWhu ? 1 : 0;
	m_isLianZhuang = isLianZhuang();
	jsMsg["nNextBankIdx"] = m_isLianZhuang ? m_nBankerIdx : ((m_nBankerIdx + 1) % MAX_SEAT_CNT);

	sendRoomMsg(jsMsg, MSG_ROOM_NJ_GAME_OVER);
	// send msg to player ;
	IMJRoom::onGameEnd();
}

IMJPlayer* NJMJRoom::doCreateMJPlayer()
{
	return new NJMJPlayer();
}

IMJPoker* NJMJRoom::getMJPoker()
{
	return &m_tPoker;
}

bool NJMJRoom::isGameOver()
{
	if ( IMJRoom::isGameOver() )
	{
		return true;
	}

	for (auto& ref : m_vMJPlayers)
	{
		if (ref && ref->haveState(eRoomPeer_AlreadyHu))
		{
			return true;
		}
	}

	if ( isInternalShouldClosedAll() )
	{
		return true;
	}

	return false;
}

void NJMJRoom::onPlayerMo(uint8_t nIdx)
{
	IMJRoom::onPlayerMo(nIdx);
	auto player = (NJMJPlayer*)getMJPlayerByIdx(nIdx);
	player->clearHuaGangFlag();
	player->clearBuHuaFlag();
}

void NJMJRoom::onPlayerBuHua(uint8_t nIdx, uint8_t nHuaCard)
{
	auto player = (NJMJPlayer*)getMJPlayerByIdx(nIdx);
	player->signBuHuaFlag();

	auto pActCard = (NJMJPlayerCard*)player->getPlayerCard();
	auto nNewCard = getMJPoker()->distributeOneCard();
	pActCard->onBuHua(nHuaCard, nNewCard);

	// check tian hu di hu 
	if ( eRoomState_NJ_Auto_Buhua == m_pCurState->getStateID() )
	{
		//uint8_t nChangedCard = 0;
		//if (pActCard->getHoldCardHuaCnt() == 0 && doAvoidPlayerTianHuOrDiHu(nIdx, nChangedCard, true) )
		//{
		//	// check tian hu or di hu 
		//	nNewCard = nChangedCard;
		//	LOGFMTD("do change di hu or tian hu card = %u",nNewCard);
		//}
	}

	// send msg ;
	Json::Value msg;
	msg["idx"] = nIdx;
	msg["actType"] = eMJAct_BuHua;
	msg["card"] = nHuaCard;
	msg["gangCard"] = nNewCard;
	sendRoomMsg(msg, MSG_ROOM_ACT);

	Json::Value jsFrameArg;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_BuHua, (uint32_t)time(nullptr));
	jsFrameArg["idx"] = nIdx;
	jsFrameArg["hua"] = nHuaCard;
	jsFrameArg["newCard"] = nNewCard;
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);
}

void NJMJRoom::onPlayerHuaGang(uint8_t nIdx, uint8_t nGangCard )
{
	auto pPlayerWin = (NJMJPlayer*)getMJPlayerByIdx(nIdx);
	auto pActCard = (NJMJPlayerCard*)pPlayerWin->getPlayerCard();
	auto nNewCard = getMJPoker()->distributeOneCard();
	pActCard->onHuaGang(nGangCard, nNewCard);
	pPlayerWin->signHuaGangFlag();

	// check tian hu di hu 
	if (eRoomState_NJ_Auto_Buhua == m_pCurState->getStateID())
	{
		//uint8_t nChangedCard = 0;
		//if (pActCard->getHoldCardHuaCnt() == 0 && doAvoidPlayerTianHuOrDiHu(nIdx, nChangedCard, true))
		//{
		//	// check tian hu or di hu 
		//	nNewCard = nChangedCard;
		//	LOGFMTD("do change di hu or tian hu card = %u", nNewCard);
		//}
	}

	// send msg ;
	Json::Value msg;
	msg["idx"] = nIdx;
	msg["actType"] = eMJAct_HuaGang;
	msg["card"] = nGangCard;
	msg["gangCard"] = nNewCard;
	sendRoomMsg(msg, MSG_ROOM_ACT);

	m_isWillBiXiaHu = true;

	// do caculate ;
	stSettle st;
	st.eSettleReason = eMJAct_HuaGang;
	uint16_t nWin = 0;
	uint16_t nLosePerPlayer = HU_GANG_COIN_BASE * (isBiXiaHu() ? 2 : 1);
	for (uint8_t nCheckIdx = 0; nCheckIdx < 4; ++nCheckIdx)
	{
		if (nIdx == nCheckIdx)
		{
			continue;
		}

		auto pPlayer = getMJPlayerByIdx(nCheckIdx);
		if ((uint8_t)-1 != pActCard->getSongGangIdx())
		{
			pPlayer = getMJPlayerByIdx( pActCard->getSongGangIdx() );
		}

		uint16_t nLose = nLosePerPlayer;
		if (isKuaiChong())
		{
			if (m_nKuaiChongPool < nLose)
			{
				nLose = m_nKuaiChongPool;
			}

			m_nKuaiChongPool -= nLose;
			st.addLose(pPlayer->getIdx(), 0);
		}
		else
		{
			if (pPlayer->getCoin() < nLose)
			{
				nLose = pPlayer->getCoin();
			}
			pPlayer->addOffsetCoin(-1 * (int32_t)nLose);
			st.addLose(pPlayer->getIdx(), nLose);
		}
		nWin += nLose;
	}

	pPlayerWin->addOffsetCoin(nWin);
	st.addWin(nIdx, nWin);
	addSettle(st);

	Json::Value jsFrameArg;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_HuaGang, (uint32_t)time(nullptr));
	jsFrameArg["idx"] = nIdx;
	jsFrameArg["hua"] = nGangCard;
	jsFrameArg["newCard"] = nNewCard;
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);
}

void NJMJRoom::onPlayerPeng(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	IMJRoom::onPlayerPeng(nIdx,nCard,nInvokeIdx);
	auto pActPlayer = getMJPlayerByIdx(nIdx);
	auto pActCard = (NJMJPlayerCard*)pActPlayer->getPlayerCard();
	pActCard->addActSign(nCard, nInvokeIdx, eMJAct_Peng);
}

void NJMJRoom::onPlayerMingGang(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	IMJRoom::onPlayerMingGang(nIdx, nCard, nInvokeIdx);
	auto pActPlayer = getMJPlayerByIdx(nIdx);
	auto pActCard = (NJMJPlayerCard*)pActPlayer->getPlayerCard();

	if (pActCard->getSongGangIdx() == (uint8_t)-1)
	{
		pActCard->setSongGangIdx(nInvokeIdx);
	}

	pActCard->addActSign(nCard, nInvokeIdx, eMJAct_MingGang);

	auto pInvokerPlayer = getMJPlayerByIdx(nInvokeIdx);

	// do cacualte ;
	stSettle st;
	st.eSettleReason = eMJAct_MingGang;
	uint16_t nLose = MING_GANG_COIN_BASE * (isBiXiaHu() ? 2 : 1);
	if (isKuaiChong())
	{
		if (nLose > m_nKuaiChongPool )
		{
			nLose = m_nKuaiChongPool;
		}

		m_nKuaiChongPool -= nLose;
		st.addLose(pInvokerPlayer->getIdx(), 0);
	}
	else
	{
		if (nLose > pInvokerPlayer->getCoin())
		{
			nLose = pInvokerPlayer->getCoin();
		}

		pInvokerPlayer->addOffsetCoin(-1 * (int32_t)nLose);
		st.addLose(pInvokerPlayer->getIdx(), nLose);
	}

	pActPlayer->addOffsetCoin(nLose);
	st.addWin(nIdx,nLose);
	addSettle(st);
}

void NJMJRoom::onPlayerAnGang(uint8_t nIdx, uint8_t nCard)
{
	IMJRoom::onPlayerAnGang(nIdx, nCard);
	
	auto pPlayerWin = (NJMJPlayer*)getMJPlayerByIdx(nIdx);
	auto pActCard = (NJMJPlayerCard*)pPlayerWin->getPlayerCard();
	// do caculate ;
	stSettle st;
	st.eSettleReason = eMJAct_AnGang;
	uint16_t nWin = 0;
	uint16_t nLosePerPlayer = AN_GANG_COIN_BASE * (isBiXiaHu() ? 2 : 1 );
	for (uint8_t nCheckIdx = 0; nCheckIdx < 4; ++nCheckIdx)
	{
		if (nIdx == nCheckIdx)
		{
			continue;
		}

		auto pPlayer = getMJPlayerByIdx(nCheckIdx);
		if ((uint8_t)-1 != pActCard->getSongGangIdx())
		{
			pPlayer = getMJPlayerByIdx(pActCard->getSongGangIdx());
		}

		uint16_t nLose = nLosePerPlayer;

		if (isKuaiChong())
		{
			if (nLose > m_nKuaiChongPool)
			{
				nLose = m_nKuaiChongPool;
			}

			m_nKuaiChongPool -= nLose;
			st.addLose(pPlayer->getIdx(), 0);
		}
		else
		{
			if (pPlayer->getCoin() < nLose)
			{
				nLose = pPlayer->getCoin();
			}
			pPlayer->addOffsetCoin(-1 * (int32_t)nLose);
			st.addLose(pPlayer->getIdx(), nLose);
		}
		nWin += nLose;
	}
	
	pPlayerWin->addOffsetCoin(nWin);
	st.addWin(nIdx, nWin);
	addSettle(st);
	pActCard->addActSign(nCard, nIdx, eMJAct_AnGang );
}

void NJMJRoom::onPlayerBuGang(uint8_t nIdx, uint8_t nCard)
{
	IMJRoom::onPlayerBuGang(nIdx, nCard);
	auto pActPlayer = getMJPlayerByIdx(nIdx);
	auto pActCard = (NJMJPlayerCard*)pActPlayer->getPlayerCard();

	auto nInvokeIdx = pActCard->getSongGangIdx();
	if ((uint8_t)-1 == nInvokeIdx)
	{
		nInvokeIdx = pActCard->getInvokerPengIdx(nCard);
	}

	if ((uint8_t)-1 == nInvokeIdx)
	{
		LOGFMTE("room id = %u, bu gang do not have idx = %u peng = %u",getRoomID(),nIdx,nCard);
		return;
	}

	auto pInvokerPlayer = getMJPlayerByIdx(nInvokeIdx);

	// do cacualte ;
	stSettle st;
	st.eSettleReason = eMJAct_BuGang;
	uint16_t nLose = MING_GANG_COIN_BASE * (isBiXiaHu() ? 2 : 1);
	if (isKuaiChong())
	{
		if (nLose > m_nKuaiChongPool)
		{
			nLose = m_nKuaiChongPool;
		}

		m_nKuaiChongPool -= nLose;
		st.addLose(pInvokerPlayer->getIdx(), 0);
	}
	else
	{
		if (nLose > pInvokerPlayer->getCoin())
		{
			nLose = pInvokerPlayer->getCoin();
		}

		pInvokerPlayer->addOffsetCoin(-1 * (int32_t)nLose);
		st.addLose(pInvokerPlayer->getIdx(), nLose);
	}


	pActPlayer->addOffsetCoin(nLose);
	st.addWin(nIdx, nLose);
	addSettle(st);

	if (pActCard->getSongGangIdx() == (uint8_t)-1)
	{
		pActCard->setSongGangIdx(nInvokeIdx);
	}
}

void NJMJRoom::settleInfoToJson(Json::Value& jsRealTime)
{
	for (auto& ref : m_vSettle)
	{
		Json::Value jsItem, jsRDetail;
		jsItem["actType"] = ref.eSettleReason;

		for (auto& refl : ref.vLoseIdx)
		{
			Json::Value jsPlayer;
			jsPlayer["idx"] = refl.first;
			jsPlayer["offset"] = -1 * refl.second;
			jsRDetail[jsRDetail.size()] = jsPlayer;
		}

		for (auto& refl : ref.vWinIdxs)
		{
			Json::Value jsPlayer;
			jsPlayer["idx"] = refl.first;
			jsPlayer["offset"] = refl.second;
			jsRDetail[jsRDetail.size()] = jsPlayer;
		}
		jsItem["detial"] = jsRDetail;
		jsRealTime[jsRealTime.size()] = jsItem;
	}
}

void NJMJRoom::onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx )
{
	if (vHuIdx.empty())
	{
		LOGFMTE("why hu vec is empty ? room id = %u",getRoomID());
		return;
	}

	auto iterBankWin = std::find(vHuIdx.begin(),vHuIdx.end(),getBankerIdx());
	m_isBankerHu = iterBankWin != vHuIdx.end();
	if (m_isBankerHu )
	{
		m_isWillBiXiaHu = true;
	}

	Json::Value jsDetail;
	Json::Value jsRealTime;
	Json::Value jsMsg;
	
	settleInfoToJson(jsRealTime);

	bool isZiMo = vHuIdx.front() == nInvokeIdx;
	jsMsg["isZiMo"] = isZiMo ? 1 : 0;
	jsMsg["realTimeCal"] = jsRealTime;
	jsMsg["huCard"] = nCard;
	if (isZiMo)
	{
		onPlayerZiMo(nInvokeIdx, nCard, jsDetail);
		jsMsg["detail"] = jsDetail;
		sendRoomMsg(jsMsg, MSG_ROOM_NJ_PLAYER_HU);
		return;
	}

	// check dian piao ;
	if ( vHuIdx.size() > 1 )  // yi pao duo xiang 
	{
		m_isWillBiXiaHu = true;
	}

	auto pLosePlayer = getMJPlayerByIdx(nInvokeIdx);
	if ( !pLosePlayer )
	{
		LOGFMTE( "room id = %u lose but player idx = %u is nullptr",getRoomID(),nInvokeIdx );
		return;
	}

	//{ dianPaoIdx : 23 , isRobotGang : 0 , nLose : 23, huPlayers : [{ idx : 234 , win : 234 , baoPaiIdx : 2 , huardSoftHua : 23, vhuTypes : [ eFanxing , ] } , .... ] } 
	jsDetail["dianPaoIdx"] = pLosePlayer->getIdx();
	jsDetail["isRobotGang"] = pLosePlayer->haveDecareBuGangFalg() ? 1 : 0;
	pLosePlayer->addDianPaoCnt();
	Json::Value jsHuPlayers;
	uint32_t nTotalLose = 0;
	// adjust caculate order 
	std::vector<uint8_t> vOrderHu;
	if (vHuIdx.size() > 1)
	{
		for (uint8_t offset = 1; offset <= 3; ++offset)
		{
			auto nCheckIdx = nInvokeIdx + offset;
			nCheckIdx = nCheckIdx % 4;
			auto iter = std::find(vHuIdx.begin(), vHuIdx.end(), nCheckIdx);
			if (iter != vHuIdx.end())
			{
				vOrderHu.push_back(nCheckIdx);
			}
		}
	}
	else
	{
		vOrderHu.swap(vHuIdx);
	}

	for (auto& nHuIdx : vOrderHu)
	{
		auto pHuPlayer = getMJPlayerByIdx(nHuIdx);
		if (pHuPlayer == nullptr)
		{
			LOGFMTE("room id = %u hu player idx = %u , is nullptr",getRoomID(),nHuIdx);
			continue;
		}
		pHuPlayer->addHuCnt();
		
		Json::Value jsHuPlayer;
		jsHuPlayer["idx"] = pHuPlayer->getIdx();
		pHuPlayer->setState(eRoomPeer_AlreadyHu);

		auto pHuPlayerCard = (NJMJPlayerCard*)pHuPlayer->getPlayerCard();

		std::vector<uint16_t> vType;
		uint16_t nHuHuaCnt = 0;
		uint16_t nHardSoftHua = 0;
		bool isSpecailHu = false;
		pHuPlayerCard->onDoHu(false, nCard,isCardByPenged(nCard), vType, nHuHuaCnt, nHardSoftHua, isSpecailHu,nInvokeIdx );
		// check da hu for will bi xia hu 
		{
			auto iterPing = std::find(vType.begin(), vType.end(), eFanxing_PingHu);
			auto iterMengQing = std::find(vType.begin(), vType.end(), eFanxing_MengQing);
			if (vType.size() == 1 && (iterPing != vType.end() || iterMengQing != vType.end()))
			{

			}
			else
			{
				m_isWillBiXiaHu = true;  // da hu , will be xia hu 
			}
		}
 

		auto nAllHuaCnt = nHuHuaCnt + nHardSoftHua * ( isHuaZa() ? 2 : 1 );
		if (isBiXiaHu())
		{
			nAllHuaCnt *= 2;
		}

		if (pLosePlayer->haveDecareBuGangFalg()) // robot gang ;
		{
			nAllHuaCnt *= 3;  // robot gang means bao pai, and zi mo ; menas zi mo 
		}

		// you ren dui 3 zui , dan shi qing yi se  huo zhe dui dui hu de shi hou  ye suan kuai zhao 
		uint8_t nBaoPaiIdx = pHuPlayerCard->getSpecailHuBaoPaiKuaiZhaoIdx();
		if ( false == isSpecailHu && (uint8_t)-1 != nBaoPaiIdx )
		{
			auto iterQinYiSe = std::find(vType.begin(), vType.end(), eFanxing_QingYiSe);
			auto iterDuiDuiHu = std::find(vType.begin(), vType.end(), eFanxing_DuiDuiHu);
			if ( ( iterQinYiSe != vType.end() || iterDuiDuiHu != vType.end()))
			{
				isSpecailHu = true;   // dang suan wai bao ;
			}
			else
			{
				nBaoPaiIdx = -1;  // should not bao pai , xiao hu ;
			}
		}

		// qing yi se ,peng 3 zui , dian pao Need bao pai 
		if (isSpecailHu == false && (uint8_t)-1 == nBaoPaiIdx)
		{
			auto iterQinYiSe = std::find(vType.begin(), vType.end(), eFanxing_QingYiSe);
			if ( iterQinYiSe != vType.end())
			{
				IMJPlayerCard::VEC_CARD vHoldCard;
				pHuPlayerCard->getHoldCard(vHoldCard);
				if (vHoldCard.size() <= 5)
				{
					isSpecailHu = true;
					nBaoPaiIdx = nInvokeIdx;
				}
			}
		}

		if ( isSpecailHu && (uint8_t)-1 == nBaoPaiIdx)  // meiyou dui 3 zui , you dang qian di si zui de ren bao pai 
		{
			nBaoPaiIdx = nInvokeIdx;
		}

		if ( (uint8_t)-1 == nBaoPaiIdx && pLosePlayer->haveDecareBuGangFalg() )  // qiang gang bao pai 
		{
			nBaoPaiIdx = pLosePlayer->getIdx();
		}

		jsHuPlayer["huardSoftHua"] = nHardSoftHua;
		jsHuPlayer["isKuaiZhaoHu"] = isSpecailHu ? 1 : 0;
		Json::Value jsHuTyps;
		for (auto& refHu : vType)
		{
			jsHuTyps[jsHuTyps.size()] = refHu;
		}
		jsHuPlayer["vhuTypes"] = jsHuTyps;
		if ( (uint8_t)-1 != nBaoPaiIdx ) // do cun zai bao pai 
		{
			jsHuPlayer["baoPaiIdx"] = nBaoPaiIdx;

			m_isWillBiXiaHu = true;
			m_isBaoPaiHappend = true;
		}

		LOGFMTD("room id = %u winner = %u all huaCnt = %u lose uid =%u",getRoomID(),pHuPlayer->getUID(),nAllHuaCnt, pLosePlayer->getUID());
		auto pDoLosePlayer = getMJPlayerByIdx(nBaoPaiIdx);
		if ( pDoLosePlayer == nullptr)
		{
			pDoLosePlayer = pLosePlayer;
		}

		// do kou qian 
		if (isKuaiChong())
		{
			if (nAllHuaCnt > m_nKuaiChongPool)
			{
				nAllHuaCnt = m_nKuaiChongPool;
			}
			m_nKuaiChongPool -= nAllHuaCnt;
		}
		else 
		{
			if ( isSpecailHu && isEnableWaiBao() )
			{
				auto nBaseCoin = max(100, m_nInitCoin);
				nAllHuaCnt = isBiXiaHu() ? nBaseCoin : (nBaseCoin * 0.5);

				((NJMJPlayer*)pDoLosePlayer)->addWaiBaoOffset(-1 * (int32_t)nAllHuaCnt);
			}
			else
			{
				if (nAllHuaCnt > pDoLosePlayer->getCoin())
				{
					nAllHuaCnt = pDoLosePlayer->getCoin();
				}
				pDoLosePlayer->addOffsetCoin(-1 * (int32_t)nAllHuaCnt);

				if (pDoLosePlayer->getIdx() == nInvokeIdx)
				{
					nTotalLose += nAllHuaCnt;
				}
			}
		}

		if (isEnableWaiBao() && isSpecailHu)
		{
			((NJMJPlayer*)pHuPlayer)->addWaiBaoOffset(nAllHuaCnt);
		}
		else
		{
			pHuPlayer->addOffsetCoin(nAllHuaCnt);
		}

		jsHuPlayer["win"] = nAllHuaCnt;
		jsHuPlayers[jsHuPlayers.size()] = jsHuPlayer;
	}

	jsDetail["nLose"] = nTotalLose;
	jsDetail["huPlayers"] = jsHuPlayers;
	jsMsg["detail"] = jsDetail;
	sendRoomMsg(jsMsg, MSG_ROOM_NJ_PLAYER_HU);
	LOGFMTD("room id = %u hu end ",getRoomID() );
}

void NJMJRoom::onPlayerZiMo( uint8_t nIdx, uint8_t nCard, Json::Value& jsDetail )
{
	auto pZiMoPlayer = (NJMJPlayer*)getMJPlayerByIdx(nIdx);
	if (pZiMoPlayer == nullptr)
	{
		LOGFMTE("room id = %u zi mo player is nullptr idx = %u ",getRoomID(),nIdx);
		return;
	}
	pZiMoPlayer->addZiMoCnt();
	pZiMoPlayer->setState(eRoomPeer_AlreadyHu);
	// svr :{ huIdx : 234 , baoPaiIdx : 2 , winCoin : 234,huardSoftHua : 23, isGangKai : 0 ,vhuTypes : [ eFanxing , ], LoseIdxs : [ {idx : 1 , loseCoin : 234 }, .... ]   }
	jsDetail["huIdx"] = nIdx;

	auto pHuPlayerCard = (NJMJPlayerCard*)pZiMoPlayer->getPlayerCard();
	std::vector<uint16_t> vType;
	uint16_t nHuHuaCnt = 0;
	uint16_t nHardSoftHua = 0;
	bool bIsSpecailHu = false;
	pHuPlayerCard->onDoHu(true, nCard, isCardByPenged(nCard), vType, nHuHuaCnt, nHardSoftHua, bIsSpecailHu );

	// check da hu for will bi xia hu 
	auto iterPing = std::find(vType.begin(), vType.end(), eFanxing_PingHu);
	auto iterMengQing = std::find(vType.begin(), vType.end(), eFanxing_MengQing);
	if (vType.size() == 1 && (iterPing != vType.end() || iterMengQing != vType.end()))
	{
	}
	else
	{
		m_isWillBiXiaHu = true;  // da hu , will be xia hu 
	}

	Json::Value jsHuTyps;
	for (auto& refHu : vType)
	{
		jsHuTyps[jsHuTyps.size()] = refHu;
	}
	jsDetail["vhuTypes"] = jsHuTyps;
	jsDetail["huardSoftHua"] = nHardSoftHua;
	jsDetail["gangKaiCoin"] = 0;
	// xiao gang kai hua 
	if (pZiMoPlayer->haveHuaGangFlag() || pZiMoPlayer->haveGangFalg())
	{
		nHuHuaCnt += 20;
		m_isWillBiXiaHu = true;
		jsDetail["gangKaiCoin"] = 20;
	}
	else if (pZiMoPlayer->haveBuHuaFlag())
	{
		nHuHuaCnt += 10;
		jsDetail["gangKaiCoin"] = 10;
		m_isWillBiXiaHu = true;
	}

	// da gang kai hua 
	auto nAllHuaCnt = nHuHuaCnt + nHardSoftHua * (isHuaZa() ? 2 : 1);
	if (isBiXiaHu())
	{
		nAllHuaCnt *= 2;
	}

	uint8_t nBaoPaiIdx = pHuPlayerCard->getSpecailHuBaoPaiKuaiZhaoIdx();
	if (false == bIsSpecailHu && (uint8_t)-1 != nBaoPaiIdx )
	{
		auto iterQinYiSe = std::find(vType.begin(), vType.end(), eFanxing_QingYiSe);
		auto iterDuiDuiHu = std::find(vType.begin(), vType.end(), eFanxing_DuiDuiHu);
		if ((iterQinYiSe != vType.end() || iterDuiDuiHu != vType.end()))
		{
			bIsSpecailHu = true;   // dang suan wai bao ;
		}
		else
		{
			nBaoPaiIdx = -1;  // should not bao pai , xiao hu ;
		}
	}

	if ((uint8_t)-1 == nBaoPaiIdx && (uint8_t)-1 != pHuPlayerCard->getSongGangIdx() ) // gang kai bao pai 
	{
		nBaoPaiIdx = pHuPlayerCard->getSongGangIdx();
	}

	jsDetail["isKuaiZhaoHu"] = bIsSpecailHu ? 1 : 0;

	auto nTotalWin = 0;
	if ( (uint8_t)-1 != nBaoPaiIdx  )
	{
		nTotalWin = nAllHuaCnt * 3; // bao pai 
		m_isBaoPaiHappend = true;
		m_isWillBiXiaHu = true;
		auto pPlayerBao = getMJPlayerByIdx(nBaoPaiIdx);
		if (isKuaiChong())
		{
			if (nTotalWin > m_nKuaiChongPool )
			{
				nTotalWin = m_nKuaiChongPool;
			}
			m_nKuaiChongPool -= nTotalWin;
		}
		else
		{
			if ( bIsSpecailHu && isEnableWaiBao() )
			{
				auto nBaseCoin = max(100, m_nInitCoin);
				nTotalWin = isBiXiaHu() ? nBaseCoin : (nBaseCoin * 0.5);
				((NJMJPlayer*)pPlayerBao)->addWaiBaoOffset(-1 * (int32_t)nTotalWin);
			}
			else
			{
				if (nTotalWin > pPlayerBao->getCoin())
				{
					nTotalWin = pPlayerBao->getCoin();
				}
				pPlayerBao->addOffsetCoin(-1 * (int32_t)nTotalWin);
			}
		}

		LOGFMTD("room id = %u uid = %u bao pai winer", getRoomID(), pPlayerBao->getUID());
		jsDetail["baoPaiIdx"] = nBaoPaiIdx;
	}
	else
	{
		Json::Value jsVLoses;
		for ( auto& pLosePlayer : m_vMJPlayers )
		{
			if (pLosePlayer == pZiMoPlayer)
			{
				continue;
			}

			auto nKouHua = nAllHuaCnt;

			if (isKuaiChong())
			{
				if ( nKouHua > m_nKuaiChongPool )
				{
					nKouHua = m_nKuaiChongPool;
				}
				m_nKuaiChongPool -= nKouHua;
			}
			else
			{
				if (nKouHua > pLosePlayer->getCoin())
				{
					nKouHua = pLosePlayer->getCoin();
				}
				pLosePlayer->addOffsetCoin(-1 * (int32_t)nKouHua);
			}


			nTotalWin += nKouHua;
			Json::Value jsLose;
			jsLose["loseCoin"] = nKouHua;
			jsLose["idx"] = pLosePlayer->getIdx();
			jsVLoses[jsVLoses.size()] = jsLose;
		}
		jsDetail["LoseIdxs"] = jsVLoses;
	}

	if (  isEnableWaiBao() && bIsSpecailHu )
	{
		((NJMJPlayer*)pZiMoPlayer)->addWaiBaoOffset(nTotalWin);
	}
	else
	{
		pZiMoPlayer->addOffsetCoin(nTotalWin);
	}
	
	jsDetail["winCoin"] = nTotalWin;

	LOGFMTD("room id = %u hu end ", getRoomID());
}

bool NJMJRoom::onPlayerApplyLeave(uint32_t nPlayerUID)
{
	auto pPlayer = getMJPlayerByUID(nPlayerUID);
	if (!pPlayer)
	{
		LOGFMTE("you are not in room id = %u , how to leave this room ? uid = %u", getRoomID(), nPlayerUID);
		return false;
	}

	Json::Value jsMsg;
	jsMsg["idx"] = pPlayer->getIdx();
	sendRoomMsg(jsMsg, MSG_ROOM_PLAYER_LEAVE); // tell other player leave ;

	auto curState = getCurRoomState()->getStateID();
	if (eRoomSate_WaitReady == curState || eRoomState_GameEnd == curState)
	{
		// direct leave just stand up ;
		//auto pXLPlayer = (XLMJPlayer*)pPlayer;
		stMsgSvrDoLeaveRoom msgdoLeave;
		msgdoLeave.nCoin = pPlayer->getCoin();
		msgdoLeave.nGameType = getRoomType();
		msgdoLeave.nRoomID = getRoomID();
		msgdoLeave.nUserUID = pPlayer->getUID();
		msgdoLeave.nGameOffset = pPlayer->getOffsetCoin();
		getRoomMgr()->sendMsg(&msgdoLeave, sizeof(msgdoLeave), pPlayer->getSessionID());
		LOGFMTD("player uid = %u , leave room id = %u", pPlayer->getUID(), getRoomID());

		if (eRoomSate_WaitReady == curState || eRoomState_GameEnd == curState)  // when game over or not start , delte player in room data ;
		{
			// tell robot dispatch player leave 
			auto ret = standup(nPlayerUID);
			return ret;
		}
		else
		{
			LOGFMTE("decide player already sync data uid = %u room id = %u", pPlayer->getUID(), getRoomID());
		}
	}
	pPlayer->doTempLeaveRoom();
	onPlayerTrusteedStateChange(pPlayer->getIdx(), true);
	return true;
}

void NJMJRoom::sendPlayersCardInfo(uint32_t nSessionID)
{
	Json::Value jsmsg;
	Json::Value vPeerCards;
	for (auto& pp : m_vMJPlayers)
	{
		if (pp == nullptr /*|| pp->haveState(eRoomPeer_CanAct) == false*/)  // lose also have card 
		{
			continue;
		}

		auto pCard = (NJMJPlayerCard*)pp->getPlayerCard();
		Json::Value jsCardInfo;
		jsCardInfo["idx"] = pp->getIdx();
		jsCardInfo["newMoCard"] = 0;
		jsCardInfo["isBaoTing"] = pCard->isHaveFlag(ePlayerFlag_TianTing) ? 1 : 0 ;
		if (getCurRoomState()->getStateID() == eRoomState_WaitPlayerAct && getCurRoomState()->getCurIdx() == pp->getIdx())
		{
			jsCardInfo["newMoCard"] = pp->getPlayerCard()->getNewestFetchedCard();
		}

		pCard->getCardInfo(jsCardInfo);
		sendMsgToPlayer(jsCardInfo, MSG_ROOM_PLAYER_CARD_INFO, nSessionID);
	}

	//jsmsg["playersCard"] = vPeerCards;
	//jsmsg["bankerIdx"] = getBankerIdx();
	//jsmsg["curActIdex"] = getCurRoomState()->getCurIdx();
	//jsmsg["leftCardCnt"] = getMJPoker()->getLeftCardCount();
	/*sendMsgToPlayer(jsmsg, MSG_ROOM_PLAYER_CARD_INFO, nSessionID);*/
	LOGFMTD("send player card infos !");
}

bool NJMJRoom::isInternalShouldClosedAll()
{
	// kuai chong chi wei 0 ;
	if ( isKuaiChong() )
	{
		return m_nKuaiChongPool <= 0 ;
	}

	// si ge ren jin yuanzi 
	uint8_t nCnt = 0;
	for (auto& player : m_vMJPlayers)
	{
		if (player->getCoin() <= 0)
		{
			++nCnt;
		}
	}

	return nCnt >= 2;
}

bool NJMJRoom::isOneCirleEnd()
{
	return ((3 == m_nBankerIdx) && (false == isLianZhuang()));
}

bool NJMJRoom::isLianZhuang()
{
	if ( isLastRoundLastBankLianZhuang() )
	{
		return true;
	}

	return ( m_isBankerHu || m_isHuangZhuang || m_isBaoPaiHappend );
}

bool NJMJRoom::isAnyPlayerRobotGang(uint8_t nInvokeIdx, uint8_t nCard)
{
	// check lou hu state
	checkLouHuState(nInvokeIdx, nCard);
	// do check is any one need the card 
	for (auto& ref : m_vMJPlayers)
	{
		if (ref == nullptr || nInvokeIdx == ref->getIdx())
		{
			continue;
		}

		auto pMJCard = (NJMJPlayerCard*)ref->getPlayerCard();
		if (((ref->isHaveLouHuFlag() == false) && pMJCard->canHuWitCard(nCard)))
		{
			return true;
		}

	}

	return false;
}

bool NJMJRoom::isAnyPlayerPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard)
{
	// check lou hu state
	checkLouHuState(nInvokeIdx,nCard);
	// do check is any one need the card 
	return IMJRoom::isAnyPlayerPengOrHuThisCard(nInvokeIdx,nCard);
	//for (auto& ref : m_vMJPlayers)
	//{
	//	if (ref == nullptr || nInvokeIdx == ref->getIdx())
	//	{
	//		continue;
	//	}

	//	auto pMJCard = (NJMJPlayerCard*)ref->getPlayerCard();
	//	if ( pMJCard->canPengWithCard(nCard) )
	//	{
	//		return true;
	//	}

	//	if ( isCanGoOnMoPai() && pMJCard->canMingGangWithCard(nCard) )
	//	{
	//		return true;
	//	}

	//	if ( ((ref->isHaveLouHuFlag() == false) && pMJCard->canHuWitCard(nCard)) )
	//	{
	//		return true;
	//	}

	//}

	//return false;
}

void NJMJRoom::checkLouHuState(uint8_t nInvokeIdx, uint8_t nCard)
{
	auto pInvoker = getMJPlayerByIdx(nInvokeIdx);
	bool bDianPaoJinYuanZi = pInvoker->getCoin() <= 0;

	// check lou hu
	for (auto& ref : m_vMJPlayers )
	{
		if (ref == nullptr || nInvokeIdx == ref->getIdx())
		{
			continue;
		}

		if (ref->isHaveLouHuFlag())
		{
			continue;
		}

		auto pMJCard = (NJMJPlayerCard*)ref->getPlayerCard();
		// hua bu gou , lou hu 
		auto bNormalHu = pMJCard->canHuWitCard(nCard);
		auto bIgnoreHua = pMJCard->canHuWitCardLocal(nCard);
		if (bNormalHu == false && bIgnoreHua)
		{
			ref->signLouHuFlag();
			continue;
		}

		if (bNormalHu == false && bIgnoreHua == false)
		{
			continue;
		}

		if ( isKuaiChong() )
		{
			continue;
		}
		// find this hu Pay coin palyer idx 
		std::vector<uint16_t> vType;
		uint16_t nHuHuaCnt = 0;
		uint16_t nHardSoftHua = 0;
		bool isSpecailHu = false;
		pMJCard->onDoHu(false, nCard, isCardByPenged(nCard), vType, nHuHuaCnt, nHardSoftHua, isSpecailHu, nInvokeIdx);
		uint8_t nBaoPaiIdx = pMJCard->getSpecailHuBaoPaiKuaiZhaoIdx();
		if (false == isSpecailHu && (uint8_t)-1 != nBaoPaiIdx)
		{
			auto iterQinYiSe = std::find(vType.begin(), vType.end(), eFanxing_QingYiSe);
			auto iterDuiDuiHu = std::find(vType.begin(), vType.end(), eFanxing_DuiDuiHu);
			if ((iterQinYiSe != vType.end() || iterDuiDuiHu != vType.end()))
			{
				isSpecailHu = true;   // dang suan wai bao ;
			}
			else
			{
				nBaoPaiIdx = -1;  // should not bao pai , xiao hu ;
			}
		}

		if ((uint8_t)-1 == nBaoPaiIdx)  // qiang gang bao pai 
		{
			nBaoPaiIdx = nInvokeIdx;
		}

		auto pPayCoinPlayer = getMJPlayerByIdx(nBaoPaiIdx);
		auto bIsPayCoinPlayerJingYuanZi = pPayCoinPlayer->getCoin() <= 0;

		// jing yuan zi 
		if ( bIsPayCoinPlayerJingYuanZi && bNormalHu)
		{
			if ( isEnableWaiBao() == false)
			{
				ref->signLouHuFlag();
				continue;
			}

			if ( !isSpecailHu )
			{
				ref->signLouHuFlag();
				continue;
			}
		}
	}
}

bool NJMJRoom::isLastRoundLastBankLianZhuang() // zui hou yi quan ,zuihou yi zhuang , te shu lian zhuang ;
{
	if ( m_pPrivateRoom == nullptr)
	{
		return false;
	}
	return  (m_pPrivateRoom->isLastCircle()) && (m_isWillBiXiaHu || m_isSiLianFengFaQian) && (3 == m_nBankerIdx);
}

void NJMJRoom::addSettle(stSettle& tSettle)
{
	m_vSettle.push_back(tSettle);
	// do send message ;
	Json::Value jsMsg;
	jsMsg["actType"] = tSettle.eSettleReason;

	Json::Value jsWin;
	for (auto& ref : tSettle.vWinIdxs)
	{
		Json::Value js;
		js["idx"] = ref.first;
		js["offset"] = ref.second;
		js["isWin"] = 1;
		jsWin[jsWin.size()] = js;
	}
	jsMsg["winers"] = jsWin;

	Json::Value jsLose;
	for (auto& ref : tSettle.vLoseIdx)
	{
		Json::Value js;
		js["idx"] = ref.first;
		js["offset"] = ref.second;
		js["isWin"] = 0;
		jsLose[jsLose.size()] = js;
	}
	jsMsg["loserIdxs"] = jsLose;

	sendRoomMsg(jsMsg, MSG_ROOM_NJ_REAL_TIME_SETTLE);
}

void NJMJRoom::onPlayerChu(uint8_t nIdx, uint8_t nCard)
{
	IMJRoom::onPlayerChu(nIdx,nCard);
	m_tChuedCards.addChuedCard(nCard, nIdx);
	m_isWillProcessChuPaiFaQian = true;
	m_nChuPaiPlayerIdx = nIdx;  // used for check chu pai fa qian 
	m_nChuedCard = nCard; // used for check chu pai fa qian 
	if ( false == isAnyPlayerRobotGang( nIdx,nCard ) )  // is any one can hu this card ?
	{
		doProcessChuPaiFanQian();
	}
}

void NJMJRoom::doProcessChuPaiFanQian()
{
	if ( false == m_isWillProcessChuPaiFaQian )
	{
		return;
	}
	m_isWillProcessChuPaiFaQian = false;

	auto nIdx = m_nChuPaiPlayerIdx;
	auto nCard = m_nChuedCard;
	// check chu 4 feng 
	{
		auto pChuPaiPlayer = getMJPlayerByIdx(nIdx);
		auto pcard = (NJMJPlayerCard*)pChuPaiPlayer->getPlayerCard();
		if (isEnableSiLianFeng() && pcard->isChued4Feng())
		{
			// do caculate ;
			stSettle st;
			st.eSettleReason = eMJAct_4Feng;
			uint16_t nWin = 0;
			uint16_t nLosePerPlayer = AN_GANG_COIN_BASE * (isBiXiaHu() ? 2 : 1);
			for (uint8_t nCheckIdx = 0; nCheckIdx < 4; ++nCheckIdx)
			{
				if (nIdx == nCheckIdx)
				{
					continue;
				}

				auto pPlayer = getMJPlayerByIdx(nCheckIdx);
				uint16_t nLose = nLosePerPlayer;

				if (isKuaiChong())
				{
					if (nLose > m_nKuaiChongPool)
					{
						nLose = m_nKuaiChongPool;
					}

					m_nKuaiChongPool -= nLose;
					st.addLose(nCheckIdx, 0);
				}
				else
				{
					if (pPlayer->getCoin() < nLose)
					{
						nLose = pPlayer->getCoin();
					}
					pPlayer->addOffsetCoin(-1 * (int32_t)nLose);
					st.addLose(nCheckIdx, nLose);
				}
				nWin += nLose;
			}

			auto pPlayerWin = getMJPlayerByIdx(nIdx);
			pPlayerWin->addOffsetCoin(nWin);
			st.addWin(nIdx, nWin);
			addSettle(st);
			m_isSiLianFengFaQian = true;
			m_isWillBiXiaHu = true;
			//return;
		}

	}

	uint8_t nFanQianTarget = -1;
	uint8_t nSettleType = 0;
	// reset songGang idx 
	auto pActPlayer = getMJPlayerByIdx(nIdx);
	auto pcard = (NJMJPlayerCard*)pActPlayer->getPlayerCard();
	pcard->setSongGangIdx(-1);

	if (m_tChuedCards.isInvokerFanQian(nFanQianTarget))
	{
		nSettleType = eMJAct_Followed;
		LOGFMTD("room id = %u gen feng fa qian , card = %u , idx = %u", getRoomID(), nCard, nIdx);
	}
	else
	{
		auto pChuPaiPlayer = getMJPlayerByIdx(nIdx);
		auto pcard = (NJMJPlayerCard*)pChuPaiPlayer->getPlayerCard();
		if (!pcard->isChued4Card(nCard))
		{
			return;
		}

		nFanQianTarget = nIdx;
		nSettleType = eMJAct_Chu;

		LOGFMTD("room id = %u chu 4 ge pai , card = %u , idx = %u", getRoomID(), nCard, nIdx);
	}

	if ((uint8_t)-1 == nFanQianTarget)
	{
		return;
	}

	m_isWillBiXiaHu = true;
	// do fanqian logic
	auto pLosePlayer = getMJPlayerByIdx(nFanQianTarget);
	uint8_t nPerPlayer = PUNISH_COIN_BASE * (isBiXiaHu() ? 2 : 1);
	uint8_t nNeedAllCoin = nPerPlayer * 3;
	uint8_t nLingTou = 0;
	if (isKuaiChong())
	{
		if (nNeedAllCoin > m_nKuaiChongPool)
		{
			nPerPlayer = m_nKuaiChongPool / 3;
			nLingTou = m_nKuaiChongPool - nPerPlayer * 3;
			nNeedAllCoin = m_nKuaiChongPool;
		}
		m_nKuaiChongPool -= nNeedAllCoin;
	}
	else
	{
		if (nNeedAllCoin > pLosePlayer->getCoin())  // if not enough all , then kong fa to all
		{
			//nPerPlayer = pLosePlayer->getCoin() / 3;
			//nLingTou = pLosePlayer->getCoin() - nPerPlayer * 3;
			//nNeedAllCoin = pLosePlayer->getCoin();

			nPerPlayer = 0;
			nLingTou = 0;
			nNeedAllCoin = 0;
		}
		pLosePlayer->addOffsetCoin(-1 * (int32_t)nNeedAllCoin);
	}

	stSettle st;
	st.eSettleReason = (eMJActType)nSettleType;
	st.addLose(nFanQianTarget, nNeedAllCoin);
	// give winner 
	for (uint8_t nIdx = 0; nIdx < 4; ++nIdx)
	{
		uint8_t nRIdx = (nFanQianTarget + nIdx) % 4;
		if (nRIdx == nFanQianTarget)
		{
			continue;
		}

		auto pWiner = getMJPlayerByIdx(nRIdx);
		uint16_t nWin = nPerPlayer;
		if (nLingTou > 0)
		{
			++nWin;
			--nLingTou;
		}
		pWiner->addOffsetCoin(nWin);
		st.addWin(nRIdx, nWin);
	}

	addSettle(st);
}

bool NJMJRoom::isCardByPenged(uint8_t nCard)
{
	for (auto& player : m_vMJPlayers)
	{
		if (!player)
		{
			continue;
		}

		auto pPlayerCard = (NJMJPlayerCard*)player->getPlayerCard();
		if (pPlayerCard->getInvokerPengIdx(nCard) != (uint8_t)-1)
		{
			return true;
		}
	}

	return false;
}

bool NJMJRoom::canPlayerCardHuaGang(uint8_t nPlayerIdx, uint8_t nHuaCard)
{
	auto pPlayer = getMJPlayerByIdx(nPlayerIdx);
	if (!pPlayer)
	{
		return false ;
	}

	auto pPlayerCard = (NJMJPlayerCard*)pPlayer->getPlayerCard();
	return pPlayerCard->canCardHuaGang(nHuaCard);
}

bool NJMJRoom::isBiXiaHu()
{
	if (!m_isEnableBixiaHu)
	{
		return false;
	}

	return m_isBiXiaHu;
}

void NJMJRoom::doAddOneRoundEntery()
{

}

std::shared_ptr<IGameRoomRecorder> NJMJRoom::createRoomRecorder()
{
	return std::make_shared<NJMJRoomRecorder>();
}