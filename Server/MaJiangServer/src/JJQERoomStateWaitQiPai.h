#pragma once
#include "IMJRoomState.h"
#include "CommonDefine.h"
#include "log4z.h"
#include "JJQERoom.h"
#include "IMJPlayer.h"
class JJQERoomStateWaitQiPai
	:public IMJRoomState
{
public:
	uint32_t getStateID()final { return eRoomState_WaitPlayerQiPai; }

	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)override
	{
		IMJRoomState::enterState(pmjRoom, jsTranData);
		setStateDuringTime(9999999999);
		m_vAlreadyCheckedPlayerQiPaiIdxs.clear();
		m_nCurWaitIdx = -1;
		doInformPlayerQiPai();
	}

	uint8_t getCurIdx()override { return m_nCurWaitIdx; };
	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)override
	{
		if (MSG_REQ_ACT_LIST == nMsgType)
		{
			auto pPlayer = getRoom()->getMJPlayerBySessionID(nSessionID);
			if (pPlayer && m_nCurWaitIdx == pPlayer->getIdx())
			{
				auto pRoom = (JJQERoom*)getRoom();
				pRoom->informPlayerQiPai(m_nCurWaitIdx);
			}
			return true;
		}

		if ( MSG_PLAYER_QI_PAI != nMsgType)
		{
			return false;
		}

		auto pPlayer = getRoom()->getMJPlayerBySessionID(nSessionID);
		uint8_t nRet = 0;
		uint8_t nQipPai = 0;
		do
		{
			if (pPlayer == nullptr)
			{
				LOGFMTE("you are not in this room how to qi pai ? session id = %u", nSessionID);
				nRet = 1;
				break;
			}

			if (m_nCurWaitIdx != pPlayer->getIdx())
			{
				nRet = 2;
				LOGFMTE("you are not cur wait qi pai ? session id = %u", nSessionID);
				break;
			}

			if ( prealMsg["isQiPai"].isNull() )
			{
				nRet = 3;
				LOGFMTE("argument error qi pai  ? session id = %u", nSessionID);
				break;
			}
			nQipPai = prealMsg["isQiPai"].asUInt();
		} while (0);

		if (nRet)  // can not do this act ;
		{
			prealMsg["ret"] = nRet;
			getRoom()->sendMsgToPlayer(prealMsg, nMsgType, nSessionID);
			return true;
		}

		prealMsg["idx"] = pPlayer->getIdx();
		getRoom()->sendRoomMsg(prealMsg, MSG_ROOM_QI_PAI);
		((JJQERoom*)getRoom())->onPlayerQiPai(pPlayer->getIdx(), nQipPai == 1);
		if ( nQipPai == 1 ) // qi pai 
		{
			onDoQiPai(); // go on play game 
			return true;
		}
 
		// inform next 
		doInformPlayerQiPai();
		return true;
	}

	void doInformPlayerQiPai()
	{
		m_nCurWaitIdx = -1;
		if (m_vAlreadyCheckedPlayerQiPaiIdxs.size() == getRoom()->getSeatCnt())
		{
			// delay finish this state 
			setStateDuringTime(0.1);
			return;
		}

		auto pRoom = (JJQERoom*)getRoom();
		uint8_t nCheckIdx = getRoom()->getBankerIdx() + m_vAlreadyCheckedPlayerQiPaiIdxs.size();
		nCheckIdx = nCheckIdx % getRoom()->getSeatCnt();
		m_nCurWaitIdx = nCheckIdx;
		m_vAlreadyCheckedPlayerQiPaiIdxs.push_back(nCheckIdx);
		if (false == pRoom->informPlayerQiPai(nCheckIdx))
		{
			doInformPlayerQiPai();
			return;
		}

		// do timer wait chose fly up 
		setStateDuringTime(9999999999);
	}

	void onStateTimeUp()override  // restart game 
	{
		auto pRoom = (JJQERoom*)getRoom();
		pRoom->onAllNotQiPai();

		if (pRoom && pRoom->isEnableChaoZhuang())
		{
			getRoom()->goToState(eRoomState_WaitChaoZhuang);
		}
		else
		{
			getRoom()->goToState(eRoomState_StartGame);
		}
	}

	void onDoQiPai()
	{
		auto pRoom = (JJQERoom*)getRoom();
		if (pRoom->isAnyPlayerFlyUp())
		{
			getRoom()->goToState(eRoomState_WaitPlayerFlyUp);
		}
		else if (pRoom->isAnyPlayerAutoBuHua())
		{
			getRoom()->goToState(eRoomState_NJ_Auto_Buhua);
		}
		else
		{
			Json::Value jsValue;
			jsValue["idx"] = getRoom()->getBankerIdx();
			getRoom()->goToState(eRoomState_WaitPlayerAct, &jsValue);
		}
	}
protected:
	std::vector<uint8_t> m_vAlreadyCheckedPlayerQiPaiIdxs;
	uint8_t m_nCurWaitIdx;
};



