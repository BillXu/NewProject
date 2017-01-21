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
#include "MJRoomStateStartGame.h"
#include "MJRoomStateAskForRobotGang.h"
#define PUNISH_COIN_BASE 5 
#define AN_GANG_COIN_BASE 5 
#define MING_GANG_COIN_BASE 5
#define HU_GANG_COIN_BASE 10
bool NJMJRoom::init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nRoomID, Json::Value& vJsValue)
{
	IMJRoom::init(pRoomMgr,pConfig,nRoomID,vJsValue);
	m_isBiXiaHu = false;
	m_isWillBiXiaHu = false;
	m_isBankerHu = false;
	m_vSettle.clear();
	m_tChuedCards.clear();

	m_tPoker.initAllCard(eMJ_NanJing);
	// create state and add state ;
	IMJRoomState* vState[] = {
		new CMJRoomStateWaitReady(), new MJRoomStateWaitPlayerChu(), new NJRoomStateWaitPlayerAct(), new MJRoomStateStartGame()
		, new MJRoomStateGameEnd(), new NJRoomStateDoPlayerAct(), new MJRoomStateAskForPengOrHu(), new MJRoomStateAskForRobotGang()
	};
	for (uint8_t nIdx = 0; nIdx < sizeof(vState) / sizeof(IMJRoomState*); ++nIdx)
	{
		addRoomState(vState[nIdx]);
	}
	setInitState(vState[0]);

	// init banker
	m_nBankerIdx = getSeatCnt() - 1;

	return true;
}

void NJMJRoom::willStartGame()
{
	IMJRoom::willStartGame();
	m_vSettle.clear();
	m_tChuedCards.clear();
	m_isBiXiaHu = false;

	if (m_isWillBiXiaHu)
	{
		m_isBiXiaHu = true;
	}

	m_isWillBiXiaHu = false;

	if (m_isBankerHu == false)
	{
		m_nBankerIdx = (m_nBankerIdx + 1) % MAX_SEAT_CNT;
	}

	m_isBankerHu = false;
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
		pPlayerCard->bindRoom(this);

		if ( getBankerIdx() != pPlayer->getIdx() && pPlayerCard->isTingPai() )
		{
			LOGFMTI("room id = %u player uid = %u tian ting , may di hu ", getRoomID(),pPlayer->getUID() );
		}
	}

	Json::Value jsMsg;
	packStartGameMsg(jsMsg);
	sendRoomMsg(jsMsg, MSG_ROOM_START_GAME);
}

void NJMJRoom::getSubRoomInfo(Json::Value& jsSubInfo)
{
	jsSubInfo["isBiXiaHu"] = isBiXiaHu() ? 1 : 0;
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

	bool isAnyOneHu = false;
	for (auto& ref : m_vMJPlayers)
	{
		Json::Value js;
		if (ref)
		{
			js["idx"] = ref->getIdx();
			js["offset"] = ref->getOffsetCoin();
			jsDetial[jsDetial.size()] = js;
		}

		if (ref && ref->haveState(eRoomPeer_AlreadyHu))
		{
			isAnyOneHu = true;
			continue;
		}
	}

	jsMsg["isLiuJu"] = isAnyOneHu ? 0 : 1;
	jsMsg["detail"] = jsDetial;

	if (!isAnyOneHu)  // huang zhuang ;
	{
		m_isWillBiXiaHu = true;

		Json::Value jsReal;
		settleInfoToJson(jsReal);
		jsMsg["realTimeCal"] = jsReal;
	}

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
		return false;
	}

	for (auto& ref : m_vMJPlayers)
	{
		if (ref && ref->haveState(eRoomPeer_AlreadyHu))
		{
			return true;
		}
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
	// send msg ;
	Json::Value msg;
	msg["idx"] = nIdx;
	msg["actType"] = eMJAct_BuHua;
	msg["card"] = nHuaCard;
	msg["gangCard"] = nNewCard;
	sendRoomMsg(msg, MSG_ROOM_ACT);
}

void NJMJRoom::onPlayerHuaGang(uint8_t nIdx, uint8_t nGangCard )
{
	auto pPlayerWin = (NJMJPlayer*)getMJPlayerByIdx(nIdx);
	auto pActCard = (NJMJPlayerCard*)pPlayerWin->getPlayerCard();
	auto nNewCard = getMJPoker()->distributeOneCard();
	pActCard->onHuaGang(nGangCard, nNewCard);
	pPlayerWin->signHuaGangFlag();
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
		uint16_t nLose = nLosePerPlayer;
		if (pPlayer->getCoin() < nLose)
		{
			nLose = pPlayer->getCoin();
		}
		pPlayer->addOffsetCoin(-1 * (int32_t)nLose);
		st.addLose(nCheckIdx, nLose);
		nWin += nLose;
	}

	pPlayerWin->addOffsetCoin(nWin);
	st.addWin(nIdx, nWin);
	addSettle(st);
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
	pActCard->addActSign(nCard, nInvokeIdx, eMJAct_MingGang);

	auto pInvokerPlayer = getMJPlayerByIdx(nInvokeIdx);

	// do cacualte ;
	stSettle st;
	st.eSettleReason = eMJAct_MingGang;
	uint16_t nLose = MING_GANG_COIN_BASE;
	if (nLose > pInvokerPlayer->getCoin())
	{
		nLose = pInvokerPlayer->getCoin();
	}

	pInvokerPlayer->addOffsetCoin(-1 * (int32_t)nLose);
	st.addLose(nInvokeIdx,nLose);

	pActPlayer->addOffsetCoin(nLose);
	st.addWin(nIdx,nLose);
	addSettle(st);
}

void NJMJRoom::onPlayerAnGang(uint8_t nIdx, uint8_t nCard)
{
	IMJRoom::onPlayerAnGang(nIdx, nCard);
	
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
		uint16_t nLose = nLosePerPlayer;
		if (pPlayer->getCoin() < nLose)
		{
			nLose = pPlayer->getCoin();
		}
		pPlayer->addOffsetCoin( -1 * (int32_t)nLose);
		st.addLose(nCheckIdx,nLose);
		nWin += nLose;
	}
	
	auto pPlayerWin = getMJPlayerByIdx(nIdx);
	pPlayerWin->addOffsetCoin(nWin);
	st.addWin(nIdx, nWin);
	addSettle(st);
}

void NJMJRoom::onPlayerBuGang(uint8_t nIdx, uint8_t nCard)
{
	IMJRoom::onPlayerBuGang(nIdx, nCard);
	auto pActPlayer = getMJPlayerByIdx(nIdx);
	auto pActCard = (NJMJPlayerCard*)pActPlayer->getPlayerCard();

	auto nInvokeIdx = pActCard->getInvokerPengIdx(nCard);
	if ((uint8_t)-1 == nInvokeIdx)
	{
		LOGFMTE("room id = %u, bu gang do not have idx = %u peng = %u",getRoomID(),nIdx,nCard);
		return;
	}

	auto pInvokerPlayer = getMJPlayerByIdx(nInvokeIdx);

	// do cacualte ;
	stSettle st;
	st.eSettleReason = eMJAct_BuGang;
	uint16_t nLose = MING_GANG_COIN_BASE;
	if (nLose > pInvokerPlayer->getCoin())
	{
		nLose = pInvokerPlayer->getCoin();
	}

	pInvokerPlayer->addOffsetCoin(-1 * (int32_t)nLose);
	st.addLose(nInvokeIdx, nLose);

	pActPlayer->addOffsetCoin(nLose);
	st.addWin(nIdx, nLose);
	addSettle(st);
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

void NJMJRoom::onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx)
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
	if (isZiMo)
	{
		onPlayerZiMo(nInvokeIdx, nCard, jsDetail);
		jsMsg["detail"] = jsDetail;
		sendRoomMsg(jsMsg, MSG_ROOM_NJ_PLAYER_HU);
		return;
	}

	// check dian piao ;
	if (vHuIdx.size() > 1)  // yi pao duo xiang 
	{
		m_isWillBiXiaHu = true;
	}

	auto pLosePlayer = getMJPlayerByIdx(nInvokeIdx);
	if ( !pLosePlayer)
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
	for ( auto& nHuIdx : vHuIdx)
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
		pHuPlayerCard->onDoHu(false, nCard,isCardByPenged(nCard), vType, nHuHuaCnt, nHardSoftHua);
		auto nAllHuaCnt = nHuHuaCnt + nHardSoftHua * 2;
		if (isBiXiaHu())
		{
			nAllHuaCnt *= 2;
		}

		jsHuPlayer["huardSoftHua"] = nHardSoftHua;
		Json::Value jsHuTyps;
		for (auto& refHu : vType)
		{
			jsHuTyps[jsHuTyps.size()] = refHu;
		}
		jsHuPlayer["vhuTypes"] = jsHuTyps;

		// process bao pai qing kuang ;
		uint8_t nBaoPaiIdx = pHuPlayerCard->getKuaiZhaoBaoPaiIdx();
		if (nBaoPaiIdx == (uint8_t)-1)
		{
			// check 清一色、对对胡、全球独钓 last dui bao pai 
			auto iterQing = std::find(vType.begin(), vType.end(), eFanxing_QingYiSe);
			auto iterDuiDuiHu = std::find(vType.begin(), vType.end(), eFanxing_DuiDuiHu);
			auto iterQuanQiuDuDiao = std::find(vType.begin(), vType.end(), eFanxing_QuanQiuDuDiao);
			if (iterQing != vType.end() && iterDuiDuiHu != vType.end() && iterQuanQiuDuDiao != vType.end())
			{
				nBaoPaiIdx = pHuPlayerCard->getLastActSignIdx();
			}
		}

		if ( pLosePlayer->haveDecareBuGangFalg() ) // robot gang ;
		{
			nAllHuaCnt *= 3;  // robot gang means bao pai, and zi mo ; menas zi mo 
			LOGFMTD("room id = %u , ploseplayer = %u have gang dec ",getRoomID(),pLosePlayer->getUID());
			if (nBaoPaiIdx == (uint8_t)-1)
			{
				nBaoPaiIdx = pLosePlayer->getIdx();
			}
		}

		LOGFMTD("room id = %u winner = %u all huaCnt = %u lose uid =%u",getRoomID(),pHuPlayer->getUID(),nAllHuaCnt, pLosePlayer->getUID());
		if ((uint8_t)-1 != nBaoPaiIdx)
		{
			m_isWillBiXiaHu = true;
			auto pPlayerBao = getMJPlayerByIdx(nBaoPaiIdx);
			if (nAllHuaCnt > pPlayerBao->getCoin())
			{
				nAllHuaCnt = pPlayerBao->getCoin();
			}
			pPlayerBao->addOffsetCoin(-1 * (int32_t)nAllHuaCnt);
			if (nBaoPaiIdx == pLosePlayer->getIdx())
			{
				nTotalLose += nAllHuaCnt;
			}
			jsHuPlayer["baoPaiIdx"] = pPlayerBao->getIdx();
			LOGFMTD("room id = %u uid = %u bao pai winer",getRoomID(),pPlayerBao->getUID());
		}
		else
		{
			if (nAllHuaCnt > pLosePlayer->getCoin())
			{
				nAllHuaCnt = pLosePlayer->getCoin();
			}
			pLosePlayer->addOffsetCoin(-1 * (int32_t)nAllHuaCnt);
			nTotalLose += nAllHuaCnt;
		}
	
		pHuPlayer->addOffsetCoin(nAllHuaCnt);
		jsHuPlayer["win"] = nAllHuaCnt;

		jsHuPlayers[jsHuPlayers.size()] = jsHuPlayer;
		
		// check da hu for will bi xia hu 
		if (m_isWillBiXiaHu == false && vType.size() == 1)
		{
			auto iterPing = std::find(vType.begin(), vType.end(), eFanxing_QingYiSe);
			auto iterMengQing = std::find(vType.begin(), vType.end(), eFanxing_DuiDuiHu);
			if (iterPing != vType.end() || iterMengQing != vType.end())
			{

			}
			else
			{
				m_isWillBiXiaHu = true;  // da hu , will be xia hu 
			}
		}
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
	pHuPlayerCard->onDoHu(true, nCard, isCardByPenged(nCard), vType, nHuHuaCnt, nHardSoftHua);

	Json::Value jsHuTyps;
	for (auto& refHu : vType)
	{
		jsHuTyps[jsHuTyps.size()] = refHu;
	}
	jsDetail["vhuTypes"] = jsHuTyps;

	jsDetail["huardSoftHua"] = nHardSoftHua;
	jsDetail["isGangKai"] = 0;
	// xiao gang kai hua 
	if (pZiMoPlayer->haveBuHuaFlag())
	{
		nHuHuaCnt += 10;
	}
	else if (pZiMoPlayer->haveHuaGangFlag() || pZiMoPlayer->haveGangFalg())
	{
		nHuHuaCnt += 20;
		m_isWillBiXiaHu = true;
		jsDetail["isGangKai"] = 1;
	}

	// check da hu for will bi xia hu 
	if (m_isWillBiXiaHu == false && vType.size() == 1 )
	{
		auto iterPing = std::find(vType.begin(), vType.end(), eFanxing_QingYiSe);
		auto iterMengQing = std::find(vType.begin(), vType.end(), eFanxing_DuiDuiHu);
		if (iterPing != vType.end() || iterMengQing != vType.end())
		{

		}
		else
		{
			m_isWillBiXiaHu = true;  // da hu , will be xia hu 
		}
	}
	// da gang kai hua 
	auto nAllHuaCnt = nHuHuaCnt + nHardSoftHua * 2;
	if (isBiXiaHu())
	{
		nAllHuaCnt *= 2;
	}

	auto nBaoPaiIdx = pHuPlayerCard->getKuaiZhaoBaoPaiIdx();
	if ((uint8_t)-1 == nBaoPaiIdx && pZiMoPlayer->haveGangFalg()) // gang kai bao pai 
	{
		nBaoPaiIdx = pHuPlayerCard->getInvokerGangIdx(nCard);
	}

	auto nTotalWin = 0;
	if ((uint8_t)-1 != nBaoPaiIdx)
	{
		nTotalWin = nAllHuaCnt * 3; // bao pai 
		auto pPlayerBao = getMJPlayerByIdx(nBaoPaiIdx);
		if (nTotalWin > pPlayerBao->getCoin())
		{
			nTotalWin = pPlayerBao->getCoin();
		}
		pPlayerBao->addOffsetCoin( -1 * (int32_t)nTotalWin );
		LOGFMTD("room id = %u uid = %u bao pai winer", getRoomID(), pPlayerBao->getUID());
		m_isWillBiXiaHu = true;
		jsDetail["baoPaiIdx"] = nBaoPaiIdx;
	}
	else
	{
		Json::Value jsVLoses;
		for (auto& pLosePlayer : m_vMJPlayers)
		{
			auto nKouHua = nAllHuaCnt;
			if (nKouHua > pLosePlayer->getCoin())
			{
				nKouHua = pLosePlayer->getCoin();
			}

			nTotalWin += nKouHua;
			pLosePlayer->addOffsetCoin(-1 * (int32_t)nKouHua);
			Json::Value jsLose;
			jsLose["loseCoin"] = nKouHua;
			jsLose["idx"] = pLosePlayer->getIdx();
			jsVLoses[jsVLoses.size()] = jsLose;
		}
		jsDetail["LoseIdxs"] = jsVLoses;
	}
	pZiMoPlayer->addOffsetCoin(nTotalWin);
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
		jsWin[jsWin.size()] = js;
	}
	jsMsg["winers"] = jsWin;

	Json::Value jsLose;
	for (auto& ref : tSettle.vWinIdxs)
	{
		Json::Value js;
		js["idx"] = ref.first;
		js["offset"] = ref.second;
		jsLose[jsLose.size()] = js;
	}
	jsMsg["winers"] = jsLose;

	sendRoomMsg(jsMsg, MSG_ROOM_NJ_REAL_TIME_SETTLE);
}

void NJMJRoom::onPlayerChu(uint8_t nIdx, uint8_t nCard)
{
	IMJRoom::onPlayerChu(nIdx,nCard);

	m_tChuedCards.addChuedCard(nCard,nIdx);
	uint8_t nFanQianTarget = -1;
	uint8_t nSettleType = 0;
	if ( m_tChuedCards.isInvokerFanQian(nFanQianTarget) )
	{
		nSettleType = eMJAct_Followed;
	}
	else
	{
		auto pChuPaiPlayer = getMJPlayerByIdx(nIdx);
		auto pcard = (NJMJPlayerCard*)pChuPaiPlayer->getPlayerCard();
		if (!pcard->isChued4Card(nIdx))
		{
			return;
		}

		nFanQianTarget = nIdx;
		nSettleType = eMJAct_Chu;
	}

	if ((uint8_t)-1 == nFanQianTarget)
	{
		return;
	}
	
	m_isWillBiXiaHu = true;
	// do fanqian logic
	auto pLosePlayer = getMJPlayerByIdx(nFanQianTarget);
	uint8_t nNeedAllCoin = PUNISH_COIN_BASE * 3 * (isBiXiaHu() ? 2 : 1 );
	uint8_t nPerPlayer = PUNISH_COIN_BASE * (isBiXiaHu() ? 2 : 1 );
	uint8_t nLingTou = 0;
	if (nNeedAllCoin > pLosePlayer->getCoin())
	{
		nPerPlayer = pLosePlayer->getCoin()  / 3;
		nLingTou = pLosePlayer->getCoin() - nPerPlayer * 3;
		nNeedAllCoin = pLosePlayer->getCoin();
	}
	pLosePlayer->addOffsetCoin(-1 * (int32_t)nNeedAllCoin);

	stSettle st;
	st.eSettleReason = (eMJActType)nSettleType;
	st.addLose(nFanQianTarget,nNeedAllCoin);
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
		if ( nLingTou > 0 )
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
	return m_isBiXiaHu;
}
