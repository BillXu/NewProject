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
		if (jsTranData["idx"].isNull() == false && jsTranData["idx"].isUInt())
		{
			m_nIdx = jsTranData["idx"].asUInt();
			if (!checkPlayerBuHua())
			{
				m_isAutoBuHuaOrHuaGang = false;
				setStateDuringTime(100000000);
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
		if ( m_isAutoBuHuaOrHuaGang )
		{
			if (!checkPlayerBuHua())
			{
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
};