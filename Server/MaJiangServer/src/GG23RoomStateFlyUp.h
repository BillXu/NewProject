#pragma once
#pragma once
#include "IMJRoomState.h"
#include "CommonDefine.h"
#include "log4z.h"
#include "GG23Room.h"
#include "IMJPlayer.h"
class GG23RoomStateFlyUp
	:public IMJRoomState
{
public:
	uint32_t getStateID()final { return eRoomState_WaitPlayerFlyUp; }
	uint8_t getCurIdx()override { return m_nCurWaitIdx; };
	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)override
	{
		IMJRoomState::enterState(pmjRoom, jsTranData);
		setStateDuringTime(99999999);
		m_vAlreadyCheckedPlayerFlyUpIdxs.clear();
		m_nCurWaitIdx = -1;
		doInformPlayerFlyUp();
	}

	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)override
	{
		if (MSG_REQ_ACT_LIST == nMsgType)
		{
			auto pPlayer = getRoom()->getMJPlayerBySessionID(nSessionID);
			if (pPlayer && m_nCurWaitIdx == pPlayer->getIdx())
			{
				auto pRoom = (GG23Room*)getRoom();
				pRoom->informPlayerFlyUp(m_nCurWaitIdx);
			}
			return true;
		}

		if (MSG_PLAYER_FLY_UP != nMsgType)
		{
			return false;
		}

		auto pPlayer = getRoom()->getMJPlayerBySessionID(nSessionID);
		std::vector<uint8_t> vFlyCards;
		uint8_t nRet = 0;
		do
		{
			if (pPlayer == nullptr)
			{
				LOGFMTE("you are not in this room how to fly up ? session id = %u", nSessionID);
				nRet = 1;
				break;
			}

			if (m_nCurWaitIdx != pPlayer->getIdx())
			{
				nRet = 2;
				LOGFMTE("you are not cur wait fly up ? session id = %u", nSessionID);
				break;
			}

			// parse vFlyCards;
			auto jsCards = prealMsg["flyCards"];
			for (uint8_t ncardidx = 0; ncardidx < jsCards.size(); ++ncardidx)
			{
				vFlyCards.push_back(jsCards[ncardidx].asUInt());
			}

			if (vFlyCards.empty())  // do not fly up , give up flyup
			{
				//nRet = 3;  //not error 
				break;
			}

			// do flyup 
			auto pRoom = (GG23Room*)getRoom();
			auto isValid = pRoom->onPlayerDoFlyUp(m_nCurWaitIdx, vFlyCards);
			if (!isValid)
			{
				nRet = 4;
				break;
			}
		} while (0);

		//if (nRet)  // can not do this act ;
		{
			prealMsg["ret"] = nRet;
			getRoom()->sendMsgToPlayer(prealMsg, nMsgType, nSessionID);
			//return true;
		}

		//if ( nRet ) // go on wait
		//{
		//	return;
		//}

		// inform next 
		if (nRet == 0)
		{
			doInformPlayerFlyUp();
		}
		return true;
	}

	void doInformPlayerFlyUp()
	{
		m_nCurWaitIdx = -1;
		if (m_vAlreadyCheckedPlayerFlyUpIdxs.size() == getRoom()->getSeatCnt())
		{
			// delay finish this state 
			setStateDuringTime(0.1);
			return;
		}

		auto pRoom = (GG23Room*)getRoom();
		uint8_t nCheckIdx = getRoom()->getBankerIdx() + m_vAlreadyCheckedPlayerFlyUpIdxs.size();
		nCheckIdx = nCheckIdx % getRoom()->getSeatCnt();
		m_nCurWaitIdx = nCheckIdx;
		m_vAlreadyCheckedPlayerFlyUpIdxs.push_back(nCheckIdx);
		if (false == pRoom->informPlayerFlyUp(nCheckIdx))
		{
			doInformPlayerFlyUp();
			return;
		}

		// do timer wait chose fly up 
		setStateDuringTime(9999999999);
	}

	void onStateTimeUp()override
	{
		Json::Value jsValue;
		jsValue["idx"] = getRoom()->getBankerIdx();
		getRoom()->goToState(eRoomState_WaitPlayerAct, &jsValue);
	}
protected:
	std::vector<uint8_t> m_vAlreadyCheckedPlayerFlyUpIdxs;
	uint8_t m_nCurWaitIdx;
};