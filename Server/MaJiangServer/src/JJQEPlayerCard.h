#pragma once
#include "MJPlayerCard.h"
#include "CommonDefine.h"
namespace Json
{
	class Value;
}

class JJQERoom;
class JJQEPlayerCard
	:public MJPlayerCard
{
public:
	void reset() override;
	void bindRoom(JJQERoom* pRoom, uint8_t nPlayerIdx );
	uint8_t getHuaCardToBuHua();
	bool getCardInfo(Json::Value& jsPeerCards);
	bool onFlyUp(std::vector<uint8_t>& vFlyUpCard, std::vector<uint8_t>& vNewCard );
	void onBuHua( uint8_t nHuaCard , uint8_t nNewCard );
	void onGetChaoZhuangHua( uint8_t nHua );
	bool onDoHu(bool isZiMo, uint8_t nCard);
	void getHuInfo( uint8_t& nInvokeIdx , std::vector<uint8_t>& vHuTypes );
	uint32_t getAllHuCnt();
	bool doAutoBuhua(Json::Value& jsHua, Json::Value& jsCard);
	bool getHoldCardThatCanAnGang(VEC_CARD& vGangCards)override;
	bool canEatCard(uint8_t nCard, uint8_t& nWithA, uint8_t& withB) override { return false; }
	// specail process , cao hua j q k , and xi xi qi fei 
	bool canMingGangWithCard(uint8_t nCard) override;
	bool canAnGangWithCard(uint8_t nCard)override;
	bool canPengWithCard(uint8_t nCard) override;
	bool canBuGangWithCard(uint8_t nCard)override;
	bool onPeng(uint8_t nCard)override;
	bool onMingGang(uint8_t nCard, uint8_t nGangGetCard) override;
	bool onQEAnGang(uint8_t nCard, uint8_t& nGetNewCard );
	bool onBuGang(uint8_t nCard, uint8_t nGangGetCard) override;
protected:
	bool isThisPlayerHu() { return m_vHuTypes.size() > 0; }
	bool isZiMo() { return m_nInvokeHuIdx == m_nCurPlayerIdx; }
	uint8_t getBlackJQKHuCnt();
	uint16_t getPengHuCnt();
	uint16_t getGangHuCnt();
	uint16_t getHoldAnKeCnt();
	uint16_t getHoldWenQianCnt();
	uint16_t getFlyUpHuCnt();
	uint16_t getHuaHuCnt();
	bool checkQiongHen();
	bool check3Red();
	bool checkDuiDuiHu();
	bool checkQingErHu();
	bool check13Hu();
	bool checkSoftHua();
protected:
	VEC_CARD m_vBuHuaCard;
	VEC_CARD m_vFlyupCard;
	VEC_CARD m_vJianPeng;
	VEC_CARD m_vPengedOtherHeiJQK;
	JJQERoom* m_pRoom;
	uint8_t m_nCurPlayerIdx;

	// hu info 
	uint8_t m_nInvokeHuIdx;
	uint8_t m_nHuCard;
	std::vector<uint8_t> m_vHuTypes;
};