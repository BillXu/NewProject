#pragma once 
#include "MJPlayer.h"
#include "NJMJPlayerCard.h"
class NJMJPlayer
	:public MJPlayer
{
public:
	void init(stEnterRoomData* pData)override;
	void onWillStartGame()override;
	IMJPlayerCard* getPlayerCard()override;
	void signHuaGangFlag();
	void clearHuaGangFlag();
	void signBuHuaFlag();
	void clearBuHuaFlag();
	bool haveBuHuaFlag(){ return m_bHaveBuHuaFlag; }
	bool haveHuaGangFlag(){ return m_bHaveHuaGangFlag; }
	void addWaiBaoOffset( int32_t nOffsetCoin  );
	int32_t getWaiBaoOffset();
	int32_t getWaiBaoCoin();
protected:
	bool m_bHaveHuaGangFlag;
	bool m_bHaveBuHuaFlag;
	NJMJPlayerCard m_tPlayerCard;
	int32_t m_nWaitBaoOffset;
	int32_t m_nWaiBaoCoin;
};