#pragma once
#include "MJRoomStateDoPlayerAct.h"
#include "log4z.h"
#include "NJMJRoom.h"
#include "CommonDefine.h"
#include "MJCard.h"
class NJRoomStateDoPlayerAct
	:public MJRoomStateDoPlayerAct
{
public:
	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)override
	{
		MJRoomStateDoPlayerAct::enterState(pmjRoom, jsTranData);
		if (eMJAct_Hu != m_eActType && eMJAct_Chu != m_eActType )
		{
			auto pRoom = (NJMJRoom*)getRoom();
			pRoom->doProcessChuPaiFanQian();
		}
	}

	void doAct()override
	{
		switch (m_eActType)
		{
		case eMJAct_BuHua:
		{
			auto pRoom = (NJMJRoom*)getRoom();
			pRoom->onPlayerBuHua(m_nActIdx, m_nCard);
		}
		break;
		case eMJAct_HuaGang:
		{
			auto pRoom = (NJMJRoom*)getRoom();
			pRoom->onPlayerHuaGang(m_nActIdx, m_nCard);
		}
		break;
		default:
			MJRoomStateDoPlayerAct::doAct();
			break;
		}
	}

	void onStateTimeUp()override
	{
		switch (m_eActType)
		{
		case eMJAct_HuaGang:
		case eMJAct_BuGang:
		case eMJAct_BuGang_Declare:
		case eMJAct_AnGang:
		case eMJAct_MingGang:
		case eMJAct_Chu:
		{
			if ( getRoom()->isInternalShouldClosedAll() )
			{
				getRoom()->goToState(eRoomState_GameEnd);
				return;
			}
		}
		break;
		default:
			break;
		}

		switch (m_eActType)
		{
		case eMJAct_BuHua:
		case eMJAct_HuaGang:
		{
			Json::Value jsValue;
			jsValue["idx"] = m_nActIdx;
			getRoom()->goToState(eRoomState_WaitPlayerAct, &jsValue);
		}
		break;
		default:
			MJRoomStateDoPlayerAct::onStateTimeUp();
			break;
		}
	}

	float getActTime()override 
	{
		switch (m_eActType)
		{
		case eMJAct_Mo:
			return eTime_DoPlayerMoPai;
		case eMJAct_Peng:
		case eMJAct_Chi:
			return eTime_DoPlayerAct_Peng;
		case eMJAct_MingGang:
		case eMJAct_BuGang:
		case eMJAct_AnGang:
		case eMJAct_BuHua:
		case eMJAct_HuaGang:
		{
			if (getRoom()->isGameOver())
			{
				return 1.0f;
			}
			return eTime_DoPlayerAct_Gang;
		}
		case eMJAct_Hu:
			return eTime_DoPlayerAct_Hu;
			break;
		case eMJAct_Chu:
			return eTime_DoPlayerActChuPai;
		default:
			LOGFMTE("unknown act type = %u can not return act time", m_eActType);
			return 0;
		}
		return 0;
	}
};