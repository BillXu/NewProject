#include "JJQERoom.h"
#include "JJQEPlayer.h"
#include "JJQEPlayerCard.h"
#include "log4z.h"
#include "JJQERoomStateWaitReady.h"
#include "JJQERoomStateWaitChaoZhuang.h"
#include "JJQERoomStateFlyUp.h"
#include "JJQERoomStateAutoBuHua.h"
#include "JJQERoomStateWaitPlayerAct.h"
#include "MJRoomStateWaitPlayerChu.h"
#include "MJRoomStateAskForPengOrHu.h"
#include "MJRoomStateDoPlayerAct.h"
#include "MJRoomStateGameEnd.h"
#include "JJQERoomStateStartGame.h"
#include "MJReplayFrameType.h"
#include <ctime>
#include "JJQEPlayerRecorderInfo.h"
#include "ServerMessageDefine.h"
#include "IGameRoomManager.h"
#include "RoomConfig.h"
bool JJQERoom::init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSeialNum, uint32_t nRoomID, Json::Value& vJsValue)
{
	IMJRoom::init(pRoomMgr, pConfig, nSeialNum, nRoomID, vJsValue);
	((stSitableRoomConfig*)getRoomConfig())->nMaxSeat = 3;
	m_tPoker.initAllCard(eMJ_JJQE);
	m_nQiHuNeed = vJsValue["qiHuNeed"].asUInt();
	m_nTopLimit = vJsValue["fengDing"].asUInt();
	m_nJianZhang = 0;
	m_nChaoZhuangLevel = vJsValue["chaoZhuangLevel"].asUInt();
	m_nQingErHuCnt = vJsValue["qingErHu"].asUInt();
	m_isHave13Hu = vJsValue["is13Hu"].asUInt() == 1;
	m_nLastHuIdx = -1;
	m_isHaveSun = vJsValue["haveSun"].asUInt() == 1;
	m_isHaveMoon = vJsValue["haveMoon"].asUInt() == 1;
	m_tPoker.initQingErExt( m_isHaveSun, m_isHaveMoon);
	// create state and add state ;
	IMJRoomState* vState[] = {
		new JJQERoomStateWaitReady(), new MJRoomStateWaitPlayerChu(), new JJQERoomStateWaitPlayerAct(), new JJQERoomStateStartGame(), new JJQERoomStateAutoBuHua()
		, new MJRoomStateGameEnd(), new MJRoomStateDoPlayerAct(), new MJRoomStateAskForPengOrHu(),new JJQERoomStateFlyUp(),new JJQERoomStateWaitChaoZhuang()
	};
	for (uint8_t nIdx = 0; nIdx < sizeof(vState) / sizeof(IMJRoomState*); ++nIdx)
	{
		addRoomState(vState[nIdx]);
	}
	setInitState(vState[0]);
	return true;
}

IMJPlayer* JJQERoom::doCreateMJPlayer()
{
	return new JJQEPlayer();
}

void JJQERoom::onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	bool isZiMo = vHuIdx.size() == 1 && vHuIdx.front() == nInvokeIdx;
	auto nHuIdx = vHuIdx.front();
	if ( !isZiMo && vHuIdx.size() > 1 )
	{
		for (uint8_t nIdxOffset = 1; nIdxOffset < getSeatCnt(); ++nIdxOffset )
		{
			auto nCheckIdx = nInvokeIdx + nIdxOffset;
			nCheckIdx = nCheckIdx % getSeatCnt();
			auto iter = std::find(vHuIdx.begin(),vHuIdx.end(),nCheckIdx );
			if (iter != vHuIdx.end())
			{
				nHuIdx = nCheckIdx;
				break;
			}
		}
	}

	auto pPlayer = getMJPlayerByIdx(nHuIdx);
	if (!pPlayer)
	{
		LOGFMTE("room id = %u playeridx = %u is null can not hu ",getRoomID(),nHuIdx);
		return;
	}
	auto pPlayerCard = (JJQEPlayerCard*)pPlayer->getPlayerCard();
	if (!pPlayerCard->onDoHu(nCard, nInvokeIdx ))
	{
		LOGFMTE("can not hu room id = %u idx = %u",getRoomID(),nHuIdx );
		return;
	}
	pPlayer->setState(eRoomPeer_AlreadyHu);
	m_nLastHuIdx = pPlayer->getIdx();

	Json::Value jsMsg;
	jsMsg["idx"] = nHuIdx;
	jsMsg["huCard"] = nCard;
	jsMsg["invokeIdx"] = nInvokeIdx;
	sendRoomMsg(jsMsg, MSG_ROOM_JJQE_PLAYER_HU);
}

void JJQERoom::startGame()
{
	for (auto& pPlayer : m_vMJPlayers)
	{
		if (pPlayer)
		{
			pPlayer->onStartGame();
		}
	}

	// distribute card 
	auto pPoker = getMJPoker();
	//LOGFMTD("room id = %u start game shuffle card ",getRoomID());
	//LOGFMTD("room id = %u shuffle end", getRoomID());
	Json::Value jsFrameArg, jsPlayers;
	jsFrameArg["bankIdx"] = getBankerIdx();
	for (auto& pPlayer : m_vMJPlayers)
	{
		if (!pPlayer)
		{
			continue;
		}

		//LOGFMTD("distribute card for player idx = %u and decrease desk fee = %u",pPlayer->getIdx(),getRoomConfig()->nDeskFee );
		Json::Value jsPlayer;
		jsPlayer["idx"] = pPlayer->getIdx();
		Json::Value jsHoldCard;
		for (uint8_t nIdx = 0; nIdx < 22; ++nIdx)
		{
			auto nCard = pPoker->distributeOneCard();
			pPlayer->getPlayerCard()->addDistributeCard(nCard);
			jsHoldCard[jsHoldCard.size()] = nCard;
			//LOGFMTD("card idx = %u card number = %u", nIdx,nCard);
		}

		if (getBankerIdx() == pPlayer->getIdx())
		{
			auto nCard = pPoker->distributeOneCard();
			pPlayer->getPlayerCard()->onMoCard(nCard);
			jsHoldCard[jsHoldCard.size()] = nCard;
		}

		jsPlayer["cards"] = jsHoldCard;
		jsPlayer["coin"] = pPlayer->getCoin();
		jsPlayer["uid"] = pPlayer->getUID();
		jsPlayers[jsPlayers.size()] = jsPlayer;
	}
	jsFrameArg["players"] = jsPlayers;

	// jian zhang ;
	m_nJianZhang = pPoker->distributeOneCard();
	jsFrameArg["jianZhang"] = m_nJianZhang;
	// add frame ;
	auto pFrame = getGameReplay()->createFrame(eMJFrame_StartGame, 0);
	pFrame->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(pFrame);
	LOGFMTI("room id = %u start game !", getRoomID());

	// prepare info for replay
	Json::Value jsMsg;
	packStartGameMsg(jsMsg);
	sendRoomMsg(jsMsg, MSG_ROOM_START_GAME);

	// replay arg 
	Json::Value jsReplayInfo;
	jsReplayInfo["roomID"] = getRoomID();
	jsReplayInfo["time"] = (uint32_t)time(nullptr);
	jsReplayInfo["qiHuNeed"] = m_nQiHuNeed;
	jsReplayInfo["fengDing"] = m_nTopLimit;
	jsReplayInfo["chaoZhuangLevel"] = m_nChaoZhuangLevel;
	jsReplayInfo["jianZhang"] = getJianZhang();
 
	getGameReplay()->setReplayRoomInfo(jsReplayInfo);
}

void JJQERoom::willStartGame()
{
	IMJRoom::willStartGame();
	m_nJianZhang = -1;
	m_nLastHuIdx = -1;
	if ((uint8_t)-1 == m_nBankerIdx)
	{
		m_nBankerIdx = 0;
	}
	else
	{
		m_nBankerIdx = (m_nBankerIdx + 1) % getSeatCnt();
	}

	// bind room ;
	for (auto& pPlayer : m_vMJPlayers)
	{
		if (pPlayer)
		{
			auto pPlayerCard = (JJQEPlayerCard*)pPlayer->getPlayerCard();
			pPlayerCard->bindRoom(this,pPlayer->getIdx());
		}
	}
}

void JJQERoom::getSubRoomInfo(Json::Value& jsSubInfo)
{
	jsSubInfo["qiHuNeed"] = m_nQiHuNeed;
	jsSubInfo["fengDing"] = m_nTopLimit;
	jsSubInfo["chaoZhuangLevel"] = m_nChaoZhuangLevel;
	jsSubInfo["jianZhang"] = getJianZhang();
	jsSubInfo["is13Hu"] = isEnable13Hu() ? 1 : 0;
	jsSubInfo["haveSun"] = m_isHaveSun ? 1 : 0;
	jsSubInfo["haveMoon"] = m_isHaveSun ? 1 : 0;
	jsSubInfo["qingErHu"] = m_nQingErHuCnt;
}

void JJQERoom::sendPlayersCardInfo(uint32_t nSessionID)
{
	Json::Value jsmsg;
	Json::Value vPeerCards;
	for (auto& pp : m_vMJPlayers)
	{
		if (pp == nullptr /*|| pp->haveState(eRoomPeer_CanAct) == false*/)  // lose also have card 
		{
			continue;
		}

		auto pCard = (JJQEPlayerCard*)pp->getPlayerCard();
		Json::Value jsCardInfo;
		jsCardInfo["idx"] = pp->getIdx();
		jsCardInfo["curHuCnt"] = pCard->getMingPaiHuaCnt();
		jsCardInfo["newMoCard"] = 0;
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

void JJQERoom::packStartGameMsg(Json::Value& jsMsg)
{
	IMJRoom::packStartGameMsg(jsMsg);
	jsMsg["jianZhang"] = getJianZhang();
}

bool JJQERoom::isEnableChaoZhuang()
{
	return 0 != m_nChaoZhuangLevel;
}

void JJQERoom::onLastChaoZhuangPlayerViewCard( uint8_t nCard, bool isHua )
{
	auto nTemieralIdx = (getBankerIdx() + getSeatCnt() - 1) % getSeatCnt();
	auto player = getMJPlayerByIdx(nTemieralIdx);
	if (player == nullptr)
	{
		LOGFMTE("room id = %u chao zhuang player is null banker idx = %u, seat cnt = %u", getRoomID(), getBankerIdx(), getSeatCnt());
		return;
	}

	if ( isHua )
	{
		auto pActCard = (JJQEPlayerCard*)player->getPlayerCard();
		pActCard->onGetChaoZhuangHua(nCard);
	}

	Json::Value jsMsg;
	jsMsg["card"] = nCard;
	sendMsgToPlayer(jsMsg, MSG_ROOM_CHAO_ZHUANG_VIEW_LAST_CARD, player->getSessionID());
}

bool JJQERoom::isAnyPlayerFlyUp()
{
	MJPlayerCard::VEC_CARD vCard;
	for (auto& player : m_vMJPlayers)
	{
		if ( nullptr == player )
		{
			continue;
		}

		auto pCard = player->getPlayerCard();
		vCard.clear();
		if ( pCard->getHoldCardThatCanAnGang(vCard) )
		{
			return true;
		}
	}
	return false;
}

bool JJQERoom::isAnyPlayerAutoBuHua()
{
	for (auto& player : m_vMJPlayers)
	{
		if (nullptr == player)
		{
			continue;
		}

		auto pCard = (JJQEPlayerCard*)player->getPlayerCard();
		if ( (uint8_t)-1 != pCard->getHuaCardToBuHua() )
		{
			return true;
		}
	}
	return false;
}

bool JJQERoom::informPlayerFlyUp(uint8_t nPlayerIdx)
{
	auto pPlayer = getMJPlayerByIdx(nPlayerIdx);
	if (nullptr == pPlayer)
	{
		LOGFMTE("room id = %u player idx = %u player is null , can not fly up",getRoomID(),nPlayerIdx);
		return false;
	}

	MJPlayerCard::VEC_CARD vCard;
	auto pCard = pPlayer->getPlayerCard();
	vCard.clear();
	if (!pCard->getHoldCardThatCanAnGang(vCard))
	{
		return false;
	}

	Json::Value jsMsg;
	sendMsgToPlayer(jsMsg, MSG_ROOM_WAIT_PLAYER_FLY_UP, pPlayer->getSessionID());
	return true;
}

bool JJQERoom::onPlayerDoFlyUp( uint8_t nIdx, std::vector<uint8_t>& vFlyUpCard )
{
	if (vFlyUpCard.empty())
	{
		LOGFMTE("room id = %u idx = %u flycard is empty",getRoomID(),nIdx);
		return false;
	}

	auto pPlayer = getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("room id = %u player idx = %u player is null can not flyup",getRoomID(),nIdx);
		return false;
	}

	auto pPlayerCard = (JJQEPlayerCard*)pPlayer->getPlayerCard();
	std::vector<uint8_t> vNewCards;
	if (!pPlayerCard->onFlyUp(vFlyUpCard, vNewCards))
	{
		LOGFMTE("card is invalid can not flyup room id = %u , idx = %u",getRoomID(),nIdx);
		return false;
	}

	Json::Value jsMsg;
	jsMsg["idx"] = nIdx;

	Json::Value jsArrayCard, jsNewCards;
	for (auto& ref : vFlyUpCard)
	{
		jsArrayCard[jsArrayCard.size()] = ref;
	}

	for (auto& ref : vNewCards)
	{
		jsNewCards[jsNewCards.size()] = ref;
	}
	jsMsg["flyCards"] = jsArrayCard;
	jsMsg["newCards"] = jsNewCards;
	sendRoomMsg(jsMsg, MSG_ROOM_PLAYER_FLY_UP);
	return true;
}

void JJQERoom::onDoAllPlayersAutoBuHua()
{
	Json::Value jsMsg;
	Json::Value jsDetail;
	for (auto& ref : m_vMJPlayers)
	{
		if (!ref)
		{
			continue;
		}
		auto pPlayerCard = (JJQEPlayerCard*)ref->getPlayerCard();

		Json::Value jsHua;
		Json::Value jsCard;
		if ( !pPlayerCard->doAutoBuhua(jsHua, jsCard) )
		{
			continue;
		}

		Json::Value jsPlayerItem;
		jsPlayerItem["idx"] = ref->getIdx();
		jsPlayerItem["hua"] = jsHua;
		jsPlayerItem["card"] = jsCard;
		jsDetail[jsDetail.size()] = jsPlayerItem;
	}
	jsMsg["detail"] = jsDetail;
	sendRoomMsg(jsMsg, MSG_ROOM_JJQE_AUTO_BU_HUA );
}

void JJQERoom::onGameDidEnd()
{
	IMJRoom::onGameDidEnd();
	m_nJianZhang = -1;
	if (getDelegate())
	{
		getDelegate()->onDidGameOver(this);
		return;
	}
}

void JJQERoom::onGameEnd()
{
	// svr: { isLiuJu : 0 , detail : [ {idx : 0 , offset : 23 }, ...  ], realTimeCal : [ { actType : 23, detial : [ {idx : 2, offset : -23 } ]  } , ... ] } 
	Json::Value jsMsg;
	Json::Value playerResult;

	auto ptrSingleRecorder = getRoomRecorder()->createSingleRoundRecorder();
	ptrSingleRecorder->init(getRoomRecorder()->getRoundRecorderCnt(), (uint32_t)time(nullptr), getGameReplay()->getReplayID());
	getRoomRecorder()->addSingleRoundRecorder(ptrSingleRecorder);

	// get hu info 
	if ((uint8_t)-1 != m_nLastHuIdx) // have some one hu 
	{
		auto pPlayer = getMJPlayerByIdx(m_nLastHuIdx);
		if ( pPlayer == nullptr)
		{
			LOGFMTE("room id = %u hu idx = %u is nullptr",getRoomID(),m_nLastHuIdx);
		}
		else
		{
			auto pPlayerCard = (JJQEPlayerCard*)pPlayer->getPlayerCard();
			std::vector<uint8_t> vHuTypes;
			uint8_t nInvokerIdx = -1 ;
			pPlayerCard->getHuInfo(nInvokerIdx, vHuTypes);
			jsMsg["huIdx"] = m_nLastHuIdx;
			jsMsg["invokeIdx"] = nInvokerIdx;

			Json::Value jsHuTypes;
			for (auto& ref : vHuTypes)
			{
				jsHuTypes[jsHuTypes.size()] = ref;
			}
			jsMsg["huTypes"] = jsHuTypes;
		}
	}

	jsMsg["isHu"] = (uint8_t)-1 != m_nLastHuIdx ? 1 : 0;

	// get player huDetail ;
	for (uint8_t nIdx = 0; nIdx < getSeatCnt(); ++nIdx)
	{
		auto pCurPlayer = (JJQEPlayer*)getMJPlayerByIdx(nIdx);
		if (pCurPlayer == nullptr)
		{
			LOGFMTE("why seat player is null idx = %u",nIdx);
			continue;
		}
		auto pPlayerCard = (JJQEPlayerCard*)pCurPlayer->getPlayerCard();
		//int16_t nThisHuCnt = pPlayerCard->getAllHuCnt(pPlayerCard->getIsPlayerHu(),pPlayerCard->getIsZiMo(),pPlayerCard->getHuCard());
		bool b3Red = false;
		int16_t nThisHuCnt = pPlayerCard->getFinalHuCnt(pCurPlayer->haveState(eRoomPeer_AlreadyHu), b3Red);//pPlayerCard->getAllHuCnt(pPlayerCard->getIsPlayerHu(), pPlayerCard->getIsZiMo(), pPlayerCard->getHuCard());
		int16_t nMyOffset = 0;
		for (uint8_t nCheckIdx = 0; nCheckIdx < getSeatCnt(); ++nCheckIdx )
		{
			if (nCheckIdx == nIdx)
			{
				continue;
			}

			auto pCheckPlayer = (JJQEPlayer*)getMJPlayerByIdx(nCheckIdx);
			if ( pCheckPlayer == nullptr)
			{
				LOGFMTE("why seat check player is null idx = %u", nIdx);
				continue;
			}
			auto pCheckPlayerCard = (JJQEPlayerCard*)pCheckPlayer->getPlayerCard();
			bool bCheck3Red = false;
			int16_t nCheckHuCnt = pCheckPlayerCard->getFinalHuCnt(pCheckPlayer->haveState(eRoomPeer_AlreadyHu), bCheck3Red);
			nMyOffset += ( nThisHuCnt - nCheckHuCnt) * getChaoZhuangRate(pCurPlayer->isChaoZhuang(),pCheckPlayer->isChaoZhuang());
		}
		
		// do caculate offset 
		pCurPlayer->addOffsetCoin(nMyOffset);

		Json::Value jsPlayerItem;
		jsPlayerItem["idx"] = nIdx;
		jsPlayerItem["huCnt"] = nThisHuCnt;
		jsPlayerItem["offset"] = nMyOffset;
		jsPlayerItem["is3Red"] = b3Red ? 1 : 0;
		playerResult[playerResult.size()] = jsPlayerItem;

		// do add recorder 
		auto pPlayerRecorderInfo = std::make_shared<JJQEPlayerRecorderInfo>();
		pPlayerRecorderInfo->init(pCurPlayer->getUID(), pCurPlayer->getOffsetCoin());
		ptrSingleRecorder->addPlayerRecorderInfo(pPlayerRecorderInfo);
	}
	jsMsg["playerResult"] = playerResult;

	Json::Value jsFoldCards;
	while (getMJPoker()->getLeftCardCount() > 0)
	{
		jsFoldCards[jsFoldCards.size()] = getMJPoker()->distributeOneCard();
	}
	jsMsg["foldCards"] = jsFoldCards;
  
	//for (auto& ref : m_vMJPlayers)
	//{
	//	Json::Value js;
	//	if (ref)
	//	{
	//		js["idx"] = ref->getIdx();
	//		js["offset"] = ref->getOffsetCoin();
	//		js["final"] = ref->getCoin();
	//		jsDetial[jsDetial.size()] = js;

	//		auto pPlayerRecorderInfo = std::make_shared<NJMJPlayerRecorderInfo>();
	//		pPlayerRecorderInfo->init(ref->getUID(), ref->getOffsetCoin(), ((NJMJPlayer*)ref)->getWaiBaoOffset());
	//		ptrSingleRecorder->addPlayerRecorderInfo(pPlayerRecorderInfo);
	//	}

	//	if (ref && ref->haveState(eRoomPeer_AlreadyHu))
	//	{
	//		isAnyOneHu = true;
	//		if (ref->getIdx() == (getBankerIdx() + 1) % getSeatCnt())
	//		{
	//			isBankerNextHu = true;
	//		}
	//		continue;
	//	}
	//}

	//jsMsg["isLiuJu"] = isAnyOneHu ? 0 : 1;
	//jsMsg["detail"] = jsDetial;



	sendRoomMsg(jsMsg, MSG_ROOM_JJQE_RESULT);
	// send msg to player ;
	IMJRoom::onGameEnd();
}

void JJQERoom::onDoPlayerBuHua(uint8_t nIdx, uint8_t nHuaCard)
{
	auto player = getMJPlayerByIdx(nIdx);

	auto pActCard = (JJQEPlayerCard*)player->getPlayerCard();
	auto nNewCard = getMJPoker()->distributeOneCard();
	pActCard->onBuHua(nHuaCard, nNewCard);

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

bool JJQERoom::isCardJianPai(uint8_t nCheckCard)
{
	auto nType = card_Type(nCheckCard);
	if (eCT_Jian == nType)
	{
		return true;
	}

	auto nValue = card_Value(nCheckCard);
	if (1 == nValue || 9 == nValue)
	{
		return true;
	}

	auto nJianType = card_Type(m_nJianZhang);
	if (eCT_Hua == nJianType)
	{
		return true;
	}

	auto nJianValue = card_Value(m_nJianZhang);
	if ( (nJianValue % 3) == (nValue % 3) )
	{
		return true;
	}
	return false;
}

uint8_t JJQERoom::getJianZhang()
{
	return m_nJianZhang;
}

float JJQERoom::getSingleChaoZhuangRate()
{
	if (isEnableChaoZhuang() == false)
	{
		return 1;
	}
	LOGFMTE("player decide ho to chao zhuang rate");
	return 1;
}

float JJQERoom::getDoubleChaoZhuangRate()
{
	if (isEnableChaoZhuang() == false)
	{
		return 1;
	}
	LOGFMTE("player decide ho to chao zhuang rate");
	return 1;
}

uint16_t JJQERoom::getQiHuNeed()
{
	return m_nQiHuNeed;
}

uint16_t JJQERoom::getTopHuLimit()
{
	return m_nTopLimit;
}

void JJQERoom::onPlayerAnGang(uint8_t nIdx, uint8_t nCard)
{
	auto pPlayer = getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("why this player is null idx = %u , can not an gang", nIdx);
		return;
	}
	pPlayer->signGangFlag();
	pPlayer->addAnGangCnt();
	uint8_t nGangGetCard = 0;
	if ( ((JJQEPlayerCard*)pPlayer->getPlayerCard())->onQEAnGang(nCard, nGangGetCard) == false)
	{
		LOGFMTE("nidx = %u an gang card = %u error,", nIdx, nCard);
	}

	// send msg ;
	Json::Value msg;
	msg["idx"] = nIdx;
	msg["actType"] = eMJAct_AnGang;
	msg["card"] = nCard;
	msg["gangCard"] = nGangGetCard;
	sendRoomMsg(msg, MSG_ROOM_ACT);

	Json::Value jsFrameArg;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_AnGang, (uint32_t)time(nullptr));
	jsFrameArg["idx"] = nIdx;
	jsFrameArg["gang"] = nCard;
	jsFrameArg["newCard"] = nGangGetCard;
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);
}

IMJPoker* JJQERoom::getMJPoker()
{
	return &m_tPoker;
}

bool JJQERoom::onPlayerApplyLeave(uint32_t nPlayerUID)
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

std::shared_ptr<IGameRoomRecorder> JJQERoom::createRoomRecorder()
{
	return std::make_shared<JJQERoomRecorder>();
}

bool JJQERoom::isGameOver()
{
	if (IMJRoom::isGameOver())
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
	return false;
}

int8_t JJQERoom::getChaoZhuangRate(bool isAChao, bool isBChao)
{
	if (0 == m_nChaoZhuangLevel)  // not enable chao zhuang ;
	{
		return 1;
	}

	uint8_t nRate = 0;
	if (isAChao && isBChao)
	{
		nRate = 2;
	}
	else if (isAChao || isBChao)
	{
		nRate = 1;
	}

	int8_t vRate[] = { 1,1,1 };
	if (m_nChaoZhuangLevel == 1)
	{
		vRate[0] = 1;
		vRate[1] = 2;
		vRate[2] = 3;
	}
	else if (2 == m_nChaoZhuangLevel)
	{
		vRate[0] = 2;
		vRate[1] = 3;
		vRate[2] = 4;
	}
	return vRate[nRate];
}

void JJQERoom::onPlayerPeng(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	IMJRoom::onPlayerPeng(nIdx, nCard, nInvokeIdx);
	if (card_Type(nCard) == eCT_Jian && card_Value(nCard) > 3)
	{
		auto pInvoker = getMJPlayerByIdx(nInvokeIdx);
		if (!pInvoker)
		{
			LOGFMTE("why invoker is null ?");
			return;
		}
		pInvoker->getPlayerCard()->onMoCard(nCard);
		pInvoker->getPlayerCard()->onChuCard(nCard);
	}
}

void JJQERoom::onPlayerMingGang(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	IMJRoom::onPlayerMingGang(nIdx, nCard, nInvokeIdx);
	if (card_Type(nCard) == eCT_Jian && card_Value(nCard) > 3)
	{
		auto pInvoker = getMJPlayerByIdx(nInvokeIdx);
		if (!pInvoker)
		{
			LOGFMTE("why invoker is null ?");
			return;
		}
		pInvoker->getPlayerCard()->onMoCard(nCard);
		pInvoker->getPlayerCard()->onChuCard(nCard);
	}
}

void JJQERoom::visitPlayerInfoForRoomInfo(IMJPlayer* pPlayer, Json::Value& jsPlayerInfo)
{
	IMJRoom::visitPlayerInfoForRoomInfo(pPlayer,jsPlayerInfo);
	jsPlayerInfo["isChaoZhuang"] = ((JJQEPlayer*)pPlayer)->isChaoZhuang() ? 1 : 0;
}