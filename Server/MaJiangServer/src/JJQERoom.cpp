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
bool JJQERoom::init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSeialNum, uint32_t nRoomID, Json::Value& vJsValue)
{
	IMJRoom::init(pRoomMgr, pConfig, nSeialNum, nRoomID, vJsValue);
	m_tPoker.initAllCard(eMJ_JJQE);
	m_nQiHuNeed = vJsValue["qiHuNeed"].asUInt();
	m_nTopLimit = vJsValue["fengDing"].asUInt();
	m_nJianZhang = 0;
	m_nChaoZhuangLevel = vJsValue["chaoZhuangLevel"].asUInt();

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
}

void JJQERoom::willStartGame()
{

}

void JJQERoom::getSubRoomInfo(Json::Value& jsSubInfo)
{

}

void JJQERoom::sendPlayersCardInfo(uint32_t nSessionID)
{

}

void JJQERoom::packStartGameMsg(Json::Value& jsMsg)
{

}

bool JJQERoom::isEnableChaoZhuang()
{
	return 0 != m_nChaoZhuangLevel;
}

void JJQERoom::onLastChaoZhuangPlayerViewCard( uint8_t nCard, bool isHua )
{

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

}

void JJQERoom::onDoAllPlayersAutoBuHua()
{

}

void JJQERoom::onDoPlayerBuHua(uint8_t nIdx, uint8_t nHuaCard)
{

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