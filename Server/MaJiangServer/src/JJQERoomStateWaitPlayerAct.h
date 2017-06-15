#pragma once
#include "MJRoomStateWaitPlayerAct.h"
#include "CommonDefine.h"
#include "JJQERoom.h"
#include "JJQEPlayerCard.h"
#define TIME_DURATION_BU_HUA 0.3
class JJQERoomStateWaitPlayerAct
	:public MJRoomStateWaitPlayerAct
{
public:
	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)override
	{
		IMJRoomState::enterState(pmjRoom, jsTranData);
		m_isAutoBuHuaOrHuaGang = false;
		m_nHuaCard = -1;
		m_isMustGameOver = false;
		setStateDuringTime(pmjRoom->isWaitPlayerActForever() ? 100000000 : eTime_WaitPlayerAct);
		if (jsTranData["idx"].isNull() == false && jsTranData["idx"].isUInt())
		{
			m_nIdx = jsTranData["idx"].asUInt();
			if (!checkPlayerBuHua())
			{
				auto pPlayer = getRoom()->getMJPlayerByIdx(m_nIdx);
				if (!pPlayer)
				{
					LOGFMTE("why cur player idx = %u is null room id = %u",m_nIdx,getRoom()->getRoomID());
					m_isMustGameOver = true;
					setStateDuringTime(0.0001);
					return;
				}
				auto pPeerCard = (JJQEPlayerCard*)pPlayer->getPlayerCard();
				auto nNewCard = pPeerCard->getHuaCardToBuHua();
				if (nNewCard != (uint8_t)-1) // can not go on mo pai , lead to can not bu hua , so direct game over 
				{
					LOGFMTE("final card is hua , but can not bu hua , so must game over room id = %u, idx = %u",getRoom()->getRoomID(),m_nIdx);
					m_isMustGameOver = true;
					setStateDuringTime(0.0001);
					return;
				}

				m_isAutoBuHuaOrHuaGang = false;
				setStateDuringTime(pmjRoom->isWaitPlayerActForever() ? 100000000 : eTime_WaitPlayerAct);
				getRoom()->onWaitPlayerAct(m_nIdx, m_isCanPass); // normal ask do act 
			}
			return;
		}
		assert(0 && "invalid argument");
	}

	bool checkPlayerBuHua()
	{
		auto pPlayer = getRoom()->getMJPlayerByIdx(m_nIdx);
		if (pPlayer == nullptr)
		{
			LOGFMTE("room id = %u , player idx = %u  can not check hua bu ", getRoom()->getRoomID(), m_nIdx);
			return false;
		}

		auto pPeerCard = (JJQEPlayerCard*)pPlayer->getPlayerCard();
		auto nNewCard = pPeerCard->getHuaCardToBuHua();
		if ((uint8_t)-1 != nNewCard && getRoom()->isCanGoOnMoPai() )
		{
			m_nHuaCard = nNewCard;
			m_isAutoBuHuaOrHuaGang = true;
			setStateDuringTime(TIME_DURATION_BU_HUA); // one seconds later auto bu hua or hua gang 
			auto pRoom = (JJQERoom*)getRoom();
			pRoom->onDoPlayerBuHua(m_nIdx, m_nHuaCard);
			LOGFMTD("room id = %u , player idx = %u wait hua bu or gang card = %u", getRoom()->getRoomID(), m_nIdx, m_nHuaCard);
			return true;
		}
		m_isAutoBuHuaOrHuaGang = false; 

		return false;
	}

	void onStateTimeUp()override
	{
		if ( m_isMustGameOver)
		{
			getRoom()->goToState(eRoomState_GameEnd);
			return;
		}

		if ( m_isAutoBuHuaOrHuaGang )
		{
			if (!checkPlayerBuHua())
			{
				auto pPlayer = getRoom()->getMJPlayerByIdx(m_nIdx);
				if (!pPlayer)
				{
					LOGFMTE("why cur player idx = %u is null room id = %u", m_nIdx, getRoom()->getRoomID());
					m_isMustGameOver = true;
					setStateDuringTime(0.0001);
					return;
				}
				auto pPeerCard = (JJQEPlayerCard*)pPlayer->getPlayerCard();
				auto nNewCard = pPeerCard->getHuaCardToBuHua();
				if (nNewCard != (uint8_t)-1) // can not go on mo pai , lead to can not bu hua , so direct game over 
				{
					LOGFMTE("final card is hua , but can not bu hua , so must game over room id = %u, idx = %u", getRoom()->getRoomID(), m_nIdx);
					m_isMustGameOver = true;
					setStateDuringTime(0.0001);
					return;
				}

				m_isAutoBuHuaOrHuaGang = false;
				setStateDuringTime(100000000 );
				getRoom()->onWaitPlayerAct(m_nIdx, m_isCanPass); // normal ask do act 
			}
			return;
		}

		MJRoomStateWaitPlayerAct::onStateTimeUp();
		return;
	}

	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)override
	{
		if (MSG_PLAYER_ACT == nMsgType)
		{
			if (m_isAutoBuHuaOrHuaGang)
			{
				LOGFMTE("doing auto bu hua , can not process other msg = %u",nMsgType);
				return true;
			}
		}
		return MJRoomStateWaitPlayerAct::onMsg(prealMsg, nMsgType, eSenderPort, nSessionID);
	}

protected:
	bool m_isAutoBuHuaOrHuaGang;
	uint8_t m_nHuaCard;
	bool m_isMustGameOver;
};
