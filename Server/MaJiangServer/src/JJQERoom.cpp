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
bool JJQERoom::init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSeialNum, uint32_t nRoomID, Json::Value& vJsValue)
{
	IMJRoom::init(pRoomMgr, pConfig, nSeialNum, nRoomID, vJsValue);
	m_tPoker.initAllCard(eMJ_JJQE);
	m_nQiHuNeed = vJsValue["qiHuNeed"].asUInt();
	m_nTopLimit = vJsValue["fengDing"].asUInt();
	m_nJianZhang = 0;
	m_nChaoZhuangLevel = vJsValue["chaoZhuangLevel"].asUInt();
	m_nLastHuIdx = -1;

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
	if (!pPlayerCard->onDoHu(isZiMo, nCard))
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
			LOGFMTE("why player is null jjqe  must all player is not null");
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
}

void JJQERoom::getSubRoomInfo(Json::Value& jsSubInfo)
{
	jsSubInfo["qiHuNeed"] = m_nQiHuNeed;
	jsSubInfo["fengDing"] = m_nTopLimit;
	jsSubInfo["chaoZhuangLevel"] = m_nChaoZhuangLevel;
	jsSubInfo["jianZhang"] = getJianZhang();
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
	if (!pPlayerCard->onFlyUp(vFlyUpCard))
	{
		LOGFMTE("card is invalid can not flyup room id = %u , idx = %u",getRoomID(),nIdx);
		return false;
	}

	Json::Value jsMsg;
	jsMsg["idx"] = nIdx;

	Json::Value jsArrayCard;
	for (auto& ref : vFlyUpCard)
	{
		jsArrayCard[jsArrayCard.size()] = ref;
	}
	jsMsg["flyCards"] = jsArrayCard;
	sendRoomMsg(jsMsg, MSG_ROOM_PLAYER_FLY_UP);
	return true;
}

void JJQERoom::onDoAllPlayersAutoBuHua()
{

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