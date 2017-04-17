#pragma once 
#include "IMJRoom.h"
#include "CommonDefine.h"
#include "MJCard.h"
class MJPrivateRoom;
class NJMJRoom
	:public IMJRoom
{
public:
	struct stSettle
	{
		std::map<uint8_t,uint16_t> vWinIdxs;
		std::map<uint8_t, uint16_t> vLoseIdx;
		eMJActType eSettleReason;
		void addWin(uint8_t nIdx, uint16_t nWinCoin)
		{
			vWinIdxs[nIdx] = nWinCoin;
		}

		void addLose(uint8_t nIdx, uint16_t nLoseCoin)
		{
			auto iter = vLoseIdx.find(nIdx);
			if (iter != vLoseIdx.end())
			{
				iter->second += nLoseCoin;
				return;
			}
			vLoseIdx[nIdx] = nLoseCoin;
		}
	};

	struct stChuedCards
	{
		uint8_t nCard;
		std::vector<uint8_t> vFollowedIdxs;
		void clear()
		{
			vFollowedIdxs.clear();
			nCard = 0;
		}

		void addChuedCard(uint8_t nChuCard, uint8_t nIdx)
		{
			if (nChuCard != nCard)
			{
				clear();
			}

			nCard = nChuCard;
			vFollowedIdxs.push_back(nIdx);
		}

		bool isInvokerFanQian(uint8_t& vTargetIdx)
		{
			if (vFollowedIdxs.size() == 4)
			{
				vTargetIdx = vFollowedIdxs.front();
			}

			return vFollowedIdxs.size() == 4;
		}
	};
public:
	bool init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSeialNum, uint32_t nRoomID, Json::Value& vJsValue) override;
	void willStartGame()override;
	void startGame()override;
	void onGameDidEnd()override;
	void onGameEnd()override;
	void bindPrivateRoom(MJPrivateRoom* pPrivateRoom) { m_pPrivateRoom = pPrivateRoom; };
	IMJPlayer* doCreateMJPlayer()override;
	IMJPoker* getMJPoker()override;
	bool isGameOver()override;
	void onPlayerMo(uint8_t nIdx)override;
	void onPlayerBuHua(uint8_t nIdx, uint8_t nHuaCard );
	void onPlayerHuaGang(uint8_t nIdx, uint8_t nGangCard );
	void onPlayerPeng(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx) override;
	void onPlayerMingGang(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)override;
	void onPlayerAnGang(uint8_t nIdx, uint8_t nCard)override;
	void onPlayerBuGang(uint8_t nIdx, uint8_t nCard)override;
	void onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx)override;
	void onPlayerChu(uint8_t nIdx, uint8_t nCard)override;
	bool isBiXiaHu();
	bool isCardByPenged( uint8_t nCard );
	bool canPlayerCardHuaGang( uint8_t nPlayerIdx , uint8_t nHuaCard );
	bool onPlayerApplyLeave(uint32_t nPlayerUID)override;
	uint8_t getRoomType()override { return eRoom_MJ_NanJing; }
	void sendPlayersCardInfo(uint32_t nSessionID)override;
	bool isInternalShouldClosedAll()override;
	bool isOneCirleEnd()override;
	bool isAnyPlayerRobotGang(uint8_t nInvokeIdx, uint8_t nCard)override;
	bool isAnyPlayerPengOrHuThisCard(uint8_t nInvokeIdx, uint8_t nCard)override;
	bool isJingYuanZi() { return m_isJingYuanZiMode; }
	bool isEnableWaiBao() { if (isJingYuanZi() == false) return false; return m_isEnableWaiBao; }
	bool isHaveLouPeng()override{ return true; }
	bool isEnableSiLianFeng() { return m_isEnableSiLianFeng; }
	bool isLastRoundLastBankLianZhuang();
	void doProcessChuPaiFanQian();
protected:
	void checkLouHuState(uint8_t nInvokeIdx, uint8_t nCard);
	bool isLianZhuang();
	void getSubRoomInfo(Json::Value& jsSubInfo)override;
	void addSettle(stSettle& tSettle );
	void onPlayerZiMo(uint8_t nIdx , uint8_t nCard , Json::Value& jsDetail );
	void settleInfoToJson(Json::Value& jsInfo);
	void packStartGameMsg(Json::Value& jsMsg)override;
	bool isHuaZa(){ return m_isEnableHuaZa; }
	bool isKuaiChong(){ return m_isKuaiChong; }
	void doAddOneRoundEntery();
	std::shared_ptr<IGameRoomRecorder> createRoomRecorder()override;
protected:
	MJPrivateRoom* m_pPrivateRoom;
	stChuedCards m_tChuedCards;
	std::vector<stSettle> m_vSettle;
	bool m_isBiXiaHu;
	bool m_isWillBiXiaHu;
	bool m_isSiLianFengFaQian;
	bool m_isBankerHu;
	bool m_isHuangZhuang;
	bool m_isEnableBixiaHu;
	bool m_isEnableHuaZa;
	bool m_isJingYuanZiMode;
	bool m_isEnableWaiBao;
	bool m_isEnableSiLianFeng;
	bool m_isBaoPaiHappend;
	bool m_isLianZhuang;

	CMJCard m_tPoker;

	uint32_t m_nInitCoin;
	bool m_isKuaiChong;
	uint32_t m_nInitKuaiChongPool;
	uint32_t m_nKuaiChongPool;

	bool m_isWillProcessChuPaiFaQian;
	uint8_t m_nChuPaiPlayerIdx;  // used for check chu pai fa qian 
	uint8_t m_nChuedCard; // used for check chu pai fa qian 
};