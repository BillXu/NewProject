#pragma once
#include "IMJRoomState.h"
#include "CommonDefine.h"
#include "NJMJRoom.h"
#include "NJMJPlayerCard.h"
#include "IMJPlayer.h"
#include "MJReplayFrameType.h"
class NJRoomStateBuHua
	:public IMJRoomState
{
public:
	uint32_t getStateID()final{ return eRoomState_NJ_Auto_Buhua; }
	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)override
	{
		IMJRoomState::enterState(pmjRoom, jsTranData);
		setStateDuringTime(0);
		m_vWaitBaoTingIdx.clear();
	}

	void onStateTimeUp()override
	{
		if (!checkBuHuaAct())
		{
			if ( checkBaoTing() )  // check is anyone need bao ting ?
			{
				setStateDuringTime(999999999); // wait forever ;
			}
			else
			{
				Json::Value jsValue;
				jsValue["idx"] = getRoom()->getBankerIdx();
				getRoom()->goToState(eRoomState_WaitPlayerAct, &jsValue);
			}
		}
		else
		{
			setStateDuringTime(0.2); // bu hua dong hua time ;
		}
	}

	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)override
	{
		if ( MSG_PLAYER_ACT == nMsgType)
		{
			auto pPlayer = getRoom()->getMJPlayerBySessionID(nSessionID);
			if (!pPlayer)
			{
				LOGFMTE(" you are not in this room can not bao ting ");
				return true;
			}

			auto iter = std::find(m_vWaitBaoTingIdx.begin(), m_vWaitBaoTingIdx.end(), pPlayer->getIdx());
			if (iter == m_vWaitBaoTingIdx.end())
			{
				LOGFMTE( "you are not ting , so can not bao ting , or you already bao ting , uid = %u",pPlayer->getUID() );
				return true;
			}

			auto actType = prealMsg["actType"].asUInt();
			if (eMJAct_Ting == actType)
			{
				// send msg ;
				Json::Value msg;
				msg["idx"] = pPlayer->getIdx();
				msg["actType"] = eMJAct_Ting;
				getRoom()->sendRoomMsg(msg, MSG_ROOM_ACT);

				// sign tian ting ;
				auto pNJCard = (NJMJPlayerCard*)pPlayer->getPlayerCard();
				pNJCard->signFlag(ePlayerFlag_TianTing);
			}
			else if ( eMJAct_Pass == actType )
			{
				
			}
			else
			{
				LOGFMTE("you should send ting or pass , can not send other act = %u",actType);
				return false;
			}
			m_vWaitBaoTingIdx.erase(iter);

			if ( m_vWaitBaoTingIdx.empty() ) // all player make chose , so wait player act state ;
			{
				Json::Value jsValue;
				jsValue["idx"] = getRoom()->getBankerIdx();
				getRoom()->goToState(eRoomState_WaitPlayerAct, &jsValue);
			}
			return true;
		}
		
		if (MSG_REQ_ACT_LIST == nMsgType)
		{
			auto pPlayer = getRoom()->getMJPlayerBySessionID(nSessionID);
			if (pPlayer == nullptr)
			{
				LOGFMTE("you are not in room  why req act list");
				return false;
			}

			auto iter = std::find(m_vWaitBaoTingIdx.begin(),m_vWaitBaoTingIdx.end(),pPlayer->getIdx() );
			if ( iter == m_vWaitBaoTingIdx.end() )
			{
				LOGFMTD("you are not cur act player , so omit you message");
				return false;
			}

			// send wait msg ;
			Json::Value jsArrayActs;
			Json::Value jsAct;
			jsAct["act"] = eMJAct_Ting;
			jsAct["cardNum"] = 0;
			jsArrayActs[jsArrayActs.size()] = jsAct;

			Json::Value jsMsg;
			jsMsg["acts"] = jsArrayActs;
			getRoom()->sendMsgToPlayer(jsMsg, MSG_PLAYER_WAIT_ACT_AFTER_RECEIVED_CARD, pPlayer->getSessionID());
			return true;
		}
			
		return false;
	}

	bool checkBuHuaAct()
	{
		auto pRoom = (NJMJRoom*)getRoom();
		bool bIsDoBuHua = false;
		for (uint8_t nIdx = 0; nIdx < getRoom()->getSeatCnt(); ++nIdx)
		{
			auto pPlayer = pRoom->getMJPlayerByIdx(nIdx);
			auto nNewCard = ((NJMJPlayerCard*)pPlayer->getPlayerCard())->getHuaCardToBuHua();
			if ((uint8_t)-1 == nNewCard)
			{
				continue;
			}

			bIsDoBuHua = true;
			if (pRoom->canPlayerCardHuaGang(nIdx, nNewCard))
			{
				pRoom->onPlayerHuaGang(nIdx, nNewCard );
			}
			else
			{
				pRoom->onPlayerBuHua(nIdx, nNewCard );
			}
		}
		return bIsDoBuHua;
	}

	bool checkBaoTing()
	{
		auto pRoom = (NJMJRoom*)getRoom();
		bool bIsDoBuHua = false;
		for (uint8_t nIdx = 0; nIdx < getRoom()->getSeatCnt(); ++nIdx)
		{
			if (nIdx == pRoom->getBankerIdx())
			{
				continue;
			}
			auto pPlayer = pRoom->getMJPlayerByIdx(nIdx);
			auto pCard = (NJMJPlayerCard*)pPlayer->getPlayerCard();
			if ( pCard->isTingPai() )
			{
				m_vWaitBaoTingIdx.push_back(nIdx);

				// send wait msg ;
				Json::Value jsArrayActs;
				Json::Value jsAct;
				jsAct["act"] = eMJAct_Ting;
				jsAct["cardNum"] = 0;
				jsArrayActs[jsArrayActs.size()] = jsAct;

				Json::Value jsMsg;
				jsMsg["acts"] = jsArrayActs;
				getRoom()->sendMsgToPlayer(jsMsg, MSG_PLAYER_WAIT_ACT_AFTER_RECEIVED_CARD, pPlayer->getSessionID());

				// add wait frame 
				Json::Value jsFrameArg, jsFrameActs;
				jsFrameActs[jsFrameActs.size()] = eMJAct_Ting;
				auto ptrReplay = getRoom()->getGameReplay()->createFrame(eMJFrame_WaitPlayerAct, 0);
				jsFrameArg["idx"] = nIdx;
				jsFrameArg["act"] = jsFrameActs;
				ptrReplay->setFrameArg(jsFrameArg);
				getRoom()->getGameReplay()->addFrame(ptrReplay);
			}
		}
		return m_vWaitBaoTingIdx.empty() == false;
	}

protected:
	std::vector<uint8_t> m_vWaitBaoTingIdx;
};