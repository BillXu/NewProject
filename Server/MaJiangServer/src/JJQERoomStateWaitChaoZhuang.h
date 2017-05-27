#pragma once
#include "IMJRoomState.h"
#include "CommonDefine.h"
#include "IMJRoom.h"
#include "log4z.h"
#include "JJQEPlayer.h"
#include "Timer.h"
#include "MJCard.h"
#include "JJQERoom.h"
class JJQERoomStateWaitChaoZhuang
	:public IMJRoomState
{
public:
	uint32_t getStateID()final { return eRoomState_WaitChaoZhuang; }

	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)
	{
		IMJRoomState::enterState(pmjRoom,jsTranData);
		getRoom()->willStartGame();
		m_vWaitChoseQiaoZhuangIdx = {0,1,2};
		m_isLookCardAniFinished = false;
		setStateDuringTime(9999999999);
		m_tLookCardAniTimer.reset();

		Json::Value js;
		getRoom()->sendRoomMsg(js, MSG_ROOM_WAIT_CHAO_ZHUANG );
	}

	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)override
	{
		if (MSG_REQ_ACT_LIST == nMsgType)
		{
			auto pPlayer = getRoom()->getMJPlayerBySessionID(nSessionID);
			if ( pPlayer )
			{
				auto iter = std::find(m_vWaitChoseQiaoZhuangIdx.begin(), m_vWaitChoseQiaoZhuangIdx.end(), pPlayer->getIdx());
				if ( iter != m_vWaitChoseQiaoZhuangIdx.end() )
				{
					Json::Value js;
					getRoom()->sendMsgToPlayer(js, MSG_ROOM_WAIT_CHAO_ZHUANG, nSessionID);
				}
			}
			return true;
		}

		if ( MSG_PLAYER_CHOSED_CHAO_ZHUANG != nMsgType)
		{
			return false;
		}

		auto pPlayer = (JJQEPlayer*)getRoom()->getMJPlayerBySessionID(nSessionID);
		uint8_t nRet = 0;
		do
		{
			if (pPlayer == nullptr)
			{
				LOGFMTE("you are not in this room how to chao zhuang ? session id = %u", nSessionID);
				nRet = 1;
				break;
			}

			auto iter = std::find(m_vWaitChoseQiaoZhuangIdx.begin(),m_vWaitChoseQiaoZhuangIdx.end(),pPlayer->getIdx() );
			if (iter == m_vWaitChoseQiaoZhuangIdx.end())
			{
				nRet = 2;
				LOGFMTE("you are already maked chao zhuang ? session id = %u", nSessionID);
				break;
			}

			// you can chose , then should erase from wait vec ;
			m_vWaitChoseQiaoZhuangIdx.erase(iter);
		} while (0);

		if ( nRet )  // can not do this act ;
		{
			prealMsg["ret"] = nRet;
			getRoom()->sendMsgToPlayer(prealMsg, nMsgType, nSessionID);
			return true;
		}

		bool isChaoZhuang = prealMsg["isChao"].asUInt() == 1;
		pPlayer->setIsChaoZhuang(isChaoZhuang);

		// tell other the result ;
		prealMsg["idx"] = pPlayer->getIdx();
		getRoom()->sendRoomMsg(prealMsg, MSG_ROOM_PLAYER_CHOSED_CHAO_ZHUANG);

		// check current player is last player , means qie pai player 
		if (pPlayer->getIdx() == (getRoom()->getBankerIdx() + 2) % getRoom()->getSeatCnt())
		{
			if (isChaoZhuang == false)
			{
				m_isLookCardAniFinished = true;
			}
			else
			{
				// do look card ani 
				onDoLookLastCard();
			}
			
		}
		onCheckStateEnd();
		return true;
	}

	void onCheckStateEnd()
	{
		if ( m_isLookCardAniFinished && m_vWaitChoseQiaoZhuangIdx.empty() )
		{
			getRoom()->goToState(eRoomState_StartGame);
		}
	}

	void onDoLookLastCard()
	{
		auto pPoker = getRoom()->getMJPoker();
		auto nCard = pPoker->getLastCard();
		bool isHuaCard = false;
		float fAniTime = 0.15;
		if (card_Type(nCard) == eCT_Hua)
		{
			isHuaCard = true;
			fAniTime = 0.3;
		}
		auto pRoom = (JJQERoom*)getRoom();
		pRoom->onLastChaoZhuangPlayerViewCard(nCard, isHuaCard);

		m_tLookCardAniTimer.reset();
		m_tLookCardAniTimer.setInterval(fAniTime);
		m_tLookCardAniTimer.setIsAutoRepeat(false);
		m_tLookCardAniTimer.setCallBack([this, isHuaCard](CTimer* pTime, float fDeta) { onLookCardEnd(isHuaCard); });
		m_tLookCardAniTimer.start();
	}
 
	void onLookCardEnd(bool isHuaCard)
	{
		if (isHuaCard)
		{
			onDoLookLastCard();
		}
		else
		{
			m_isLookCardAniFinished = true;
			onCheckStateEnd();
		}
	}
protected:
	std::vector<uint8_t> m_vWaitChoseQiaoZhuangIdx;
	bool m_isLookCardAniFinished;
	CTimer m_tLookCardAniTimer;
};
