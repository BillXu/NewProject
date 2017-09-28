#include "GG23Room.h"
#include "GG23Player.h"
#include "log4z.h"
#include "MJReplayFrameType.h"
#include <time.h>
#include "IMJRoomState.h"
#include "ServerMessageDefine.h"
#include "IGameRoomManager.h"
#include "GG23PlayerRecorderInfo.h"
#include "MJRoomStateWaitPlayerChu.h"
#include "MJRoomStateAskForPengOrHu.h"
#include "MJRoomStateDoPlayerAct.h"
#include "MJRoomStateGameEnd.h"
#include "RoomConfig.h"
#include "MJRoomStateWaitReady.h"
#include "MJRoomStateWaitPlayerAct.h"
#include "MJRoomStateStartGame.h"
#include "GG23RoomStateFlyUp.h"
#include "MJRoomStateAskForRobotGang.h"
#include "GG23RoomStateStartGame.h"
bool GG23Room::init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSeialNum, uint32_t nRoomID, Json::Value& vJsValue)
{
	IMJRoom::init(pRoomMgr, pConfig, nSeialNum, nRoomID, vJsValue);
	((stSitableRoomConfig*)getRoomConfig())->nMaxSeat = 3;
	m_tPoker.initAllCard(eMJ_GG23);
	m_nQiHuNeed = vJsValue["qiHuNeed"].asUInt();
	m_nLiangPai = vJsValue["liangPai"].asUInt();
	if (vJsValue["isMenQing"].isNull() == false && vJsValue["isMenQing"].isUInt())
	{
		m_isEnableMengQing = vJsValue["isMenQing"].asUInt() == 1 ? 1 : 0;
	}
	m_nLastHuIdx = -1;
	// create state and add state ;
	IMJRoomState* vState[] = {
		new CMJRoomStateWaitReady(), new MJRoomStateWaitPlayerChu(), new MJRoomStateWaitPlayerAct(), new GG23RoomStateStartGame()
		, new MJRoomStateGameEnd(), new MJRoomStateDoPlayerAct(), new MJRoomStateAskForPengOrHu(),new GG23RoomStateFlyUp(),new MJRoomStateAskForRobotGang()
	};
	for (uint8_t nIdx = 0; nIdx < sizeof(vState) / sizeof(IMJRoomState*); ++nIdx)
	{
		addRoomState(vState[nIdx]);
	}
	setInitState(vState[0]);
	return true;
}

IMJPlayer* GG23Room::doCreateMJPlayer()
{
	return new GG23Player();
}

void GG23Room::startGame()
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

	// add frame ;
	auto pFrame = getGameReplay()->createFrame(eMJFrame_StartGame, 0);
	pFrame->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(pFrame);
	LOGFMTI("room id = %u start game mengqing = %u !", getRoomID(),(uint8_t)isEnableMengQing() );

	// prepare info for replay
	Json::Value jsMsg;
	packStartGameMsg(jsMsg);
	sendRoomMsg(jsMsg, MSG_ROOM_START_GAME);

	// replay arg 
	Json::Value jsReplayInfo;
	jsReplayInfo["roomID"] = getRoomID();
	jsReplayInfo["time"] = (uint32_t)time(nullptr);
	jsReplayInfo["qiHuNeed"] = m_nQiHuNeed;
	jsReplayInfo["isMenQing"] = isEnableMengQing() ? 1 : 0;
	getGameReplay()->setReplayRoomInfo(jsReplayInfo);
}

void GG23Room::willStartGame()
{
	IMJRoom::willStartGame();
	m_nLastHuIdx = -1;
	if ((uint8_t)-1 == m_nBankerIdx)
	{
		m_nBankerIdx = rand() % getSeatCnt();;
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
			auto pPlayerCard = (GG23PlayerCard*)pPlayer->getPlayerCard();
			pPlayerCard->bindRoom(this, pPlayer->getIdx());
		}
	}
}

void GG23Room::onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx)
{
	bool isZiMo = vHuIdx.size() == 1 && vHuIdx.front() == nInvokeIdx;
	auto nHuIdx = vHuIdx.front();
	if (!isZiMo && vHuIdx.size() > 1)
	{
		for (uint8_t nIdxOffset = 1; nIdxOffset < getSeatCnt(); ++nIdxOffset)
		{
			auto nCheckIdx = nInvokeIdx + nIdxOffset;
			nCheckIdx = nCheckIdx % getSeatCnt();
			auto iter = std::find(vHuIdx.begin(), vHuIdx.end(), nCheckIdx);
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
		LOGFMTE("room id = %u playeridx = %u is null can not hu ", getRoomID(), nHuIdx);
		return;
	}
	auto pPlayerCard = (GG23PlayerCard*)pPlayer->getPlayerCard();
	if (!pPlayerCard->onDoHu(nCard, nInvokeIdx))
	{
		LOGFMTE("can not hu room id = %u idx = %u", getRoomID(), nHuIdx);
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

void GG23Room::getSubRoomInfo(Json::Value& jsSubInfo)
{
	jsSubInfo["qiHuNeed"] = m_nQiHuNeed;
	jsSubInfo["liangPai"] = m_nLiangPai;
	jsSubInfo["isMenQing"] = isEnableMengQing() ? 1 : 0;
}

void GG23Room::sendPlayersCardInfo(uint32_t nSessionID)
{
	Json::Value jsmsg;
	Json::Value vPeerCards;
	for (auto& pp : m_vMJPlayers)
	{
		if (pp == nullptr /*|| pp->haveState(eRoomPeer_CanAct) == false*/)  // lose also have card 
		{
			continue;
		}

		auto pCard = (GG23PlayerCard*)pp->getPlayerCard();
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

bool GG23Room::isAnyPlayerFlyUp()
{
	MJPlayerCard::VEC_CARD vCard;
	for (auto& player : m_vMJPlayers)
	{
		if (nullptr == player)
		{
			continue;
		}

		auto pCard = player->getPlayerCard();
		vCard.clear();
		if (pCard->getHoldCardThatCanAnGang(vCard))
		{
			return true;
		}
	}
	return false;
}

bool GG23Room::informPlayerFlyUp(uint8_t nPlayerIdx)
{
	auto pPlayer = getMJPlayerByIdx(nPlayerIdx);
	if (nullptr == pPlayer)
	{
		LOGFMTE("room id = %u player idx = %u player is null , can not fly up", getRoomID(), nPlayerIdx);
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
	jsMsg["idx"] = nPlayerIdx;
	sendRoomMsg(jsMsg, MSG_ROOM_WAIT_PLAYER_FLY_UP);

	// add frame js 
	Json::Value jsFrameArg;
	jsFrameArg["idx"] = nPlayerIdx;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_Wait_FlyUp, (uint32_t)time(nullptr));
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);
	return true;
}

bool GG23Room::onPlayerDoFlyUp(uint8_t nIdx, std::vector<uint8_t>& vFlyUpCard)
{
	if (vFlyUpCard.empty())
	{
		LOGFMTE("room id = %u idx = %u flycard is empty", getRoomID(), nIdx);
		return false;
	}

	auto pPlayer = getMJPlayerByIdx(nIdx);
	if (!pPlayer)
	{
		LOGFMTE("room id = %u player idx = %u player is null can not flyup", getRoomID(), nIdx);
		return false;
	}

	auto pPlayerCard = (GG23PlayerCard*)pPlayer->getPlayerCard();
	std::vector<uint8_t> vNewCards;
	if (!pPlayerCard->onFlyUp(vFlyUpCard, vNewCards))
	{
		LOGFMTE("card is invalid can not flyup room id = %u , idx = %u", getRoomID(), nIdx);
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

	// add frame 
	Json::Value jsFrameArg;
	jsFrameArg["idx"] = pPlayer->getIdx();
	jsFrameArg["flyCards"] = jsArrayCard;
	jsFrameArg["newCards"] = jsNewCards;
	auto ptrReplay = getGameReplay()->createFrame(eMJFrame_Player_FlyUp, (uint32_t)time(nullptr));
	ptrReplay->setFrameArg(jsFrameArg);
	getGameReplay()->addFrame(ptrReplay);
	return true;
}

bool GG23Room::isCardJianPai(uint8_t nCheckCard)
{
	auto nType = card_Type(nCheckCard);
	auto nValue = card_Value(nCheckCard);
	if ( nType != eCT_Wan && eCT_Tong != nType && eCT_Tiao != nType )
	{
		return true;
	}

	if ( nValue != 3 && 4 != nValue && 6 != nValue && 7 != nValue )
	{
		return true;
	}

	return false;
}

uint8_t GG23Room::getTenJQKkingRate( uint8_t nCheckCard )
{
	if (getQiHuNeed() != 15)
	{
		return 1;
	}

	auto nType = card_Type(nCheckCard);
	return (nType == eCT_Jian || eCT_Feng == nType) ? 2 : 1 ;
}

uint16_t GG23Room::getQiHuNeed()
{
	return m_nQiHuNeed;
}

void GG23Room::onGameDidEnd()
{
	IMJRoom::onGameDidEnd();
	if (getDelegate())
	{
		getDelegate()->onDidGameOver(this);
		return;
	}
}

void GG23Room::onGameEnd()
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
		if (pPlayer == nullptr)
		{
			LOGFMTE("room id = %u hu idx = %u is nullptr", getRoomID(), m_nLastHuIdx);
		}
		else
		{
			auto pPlayerCard = (GG23PlayerCard*)pPlayer->getPlayerCard();
			std::vector<uint8_t> vHuTypes;
			uint8_t nInvokerIdx = -1;
			pPlayerCard->getHuInfo(nInvokerIdx, vHuTypes);
			jsMsg["huIdx"] = m_nLastHuIdx;
			jsMsg["invokeIdx"] = nInvokerIdx;

			Json::Value jsHuTypes;
			for (auto& ref : vHuTypes)
			{
				jsHuTypes[jsHuTypes.size()] = ref;
			}
			jsMsg["huTypes"] = jsHuTypes;
			jsMsg["wenCnt"] = pPlayerCard->getWenQianCnt(true);
		}
	}

	jsMsg["isHu"] = (uint8_t)-1 != m_nLastHuIdx ? 1 : 0;

	// get player huDetail ;
	for (uint8_t nIdx = 0; nIdx < getSeatCnt(); ++nIdx)
	{
		auto pCurPlayer = (GG23Player*)getMJPlayerByIdx(nIdx);
		if (pCurPlayer == nullptr)
		{
			LOGFMTE("why seat player is null idx = %u", nIdx);
			continue;
		}
		auto pPlayerCard = (GG23PlayerCard*)pCurPlayer->getPlayerCard();
		//int16_t nThisHuCnt = pPlayerCard->getAllHuCnt(pPlayerCard->getIsPlayerHu(),pPlayerCard->getIsZiMo(),pPlayerCard->getHuCard());
		bool b3Red = false;
		int16_t nThisHuCnt = pPlayerCard->getFinalHuCnt(pCurPlayer->haveState(eRoomPeer_AlreadyHu));//pPlayerCard->getAllHuCnt(pPlayerCard->getIsPlayerHu(), pPlayerCard->getIsZiMo(), pPlayerCard->getHuCard());
		int16_t nMyOffset = 0;
		for (uint8_t nCheckIdx = 0; nCheckIdx < getSeatCnt(); ++nCheckIdx)
		{
			if (nCheckIdx == nIdx)
			{
				continue;
			}

			auto pCheckPlayer = (GG23Player*)getMJPlayerByIdx(nCheckIdx);
			if (pCheckPlayer == nullptr)
			{
				LOGFMTE("why seat check player is null idx = %u", nIdx);
				continue;
			}
			auto pCheckPlayerCard = (GG23PlayerCard*)pCheckPlayer->getPlayerCard();
			bool bCheck3Red = false;
			int16_t nCheckHuCnt = pCheckPlayerCard->getFinalHuCnt(pCheckPlayer->haveState(eRoomPeer_AlreadyHu));
			nMyOffset += (nThisHuCnt - nCheckHuCnt);
		}

		// do caculate offset 
		pCurPlayer->addOffsetCoin(nMyOffset);

		Json::Value jsPlayerItem;
		jsPlayerItem["idx"] = nIdx;
		jsPlayerItem["huCnt"] = nThisHuCnt;
		jsPlayerItem["offset"] = nMyOffset;
		playerResult[playerResult.size()] = jsPlayerItem;

		// do add recorder 
		auto pPlayerRecorderInfo = std::make_shared<GG23PlayerRecorderInfo>();
		pPlayerRecorderInfo->init(pCurPlayer->getUID(), pCurPlayer->getOffsetCoin());
		pPlayerRecorderInfo->setHuCnts(nThisHuCnt);
		ptrSingleRecorder->addPlayerRecorderInfo(pPlayerRecorderInfo);
	}
	jsMsg["playerResult"] = playerResult;

	Json::Value jsFoldCards;
	while (getMJPoker()->getLeftCardCount() > 0)
	{
		jsFoldCards[jsFoldCards.size()] = getMJPoker()->distributeOneCard();
	}
	jsMsg["foldCards"] = jsFoldCards;

	sendRoomMsg(jsMsg, MSG_ROOM_JJQE_RESULT);
	// send msg to player ;
	IMJRoom::onGameEnd();
}

IMJPoker* GG23Room::getMJPoker()
{
	return &m_tPoker;
}

bool GG23Room::onPlayerApplyLeave(uint32_t nPlayerUID)
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

std::shared_ptr<IGameRoomRecorder> GG23Room::createRoomRecorder()
{
	return std::make_shared<GG23RoomRecorder>();
}

bool GG23Room::isGameOver()
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