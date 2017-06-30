#pragma once
#include "MJPlayerCard.h"
#include "CommonDefine.h"
namespace Json
{
	class Value;
}

class GG23Room;
class GG23PlayerCard
	:public MJPlayerCard
{
public:
	bool canEatCard(uint8_t nCard, uint8_t& nWithA, uint8_t& withB) override { return false; }
	void reset() override;
	void bindRoom(GG23Room* pRoom, uint8_t nPlayerIdx);
	bool getCardInfo(Json::Value& jsPeerCards);
	bool onFlyUp(std::vector<uint8_t>& vFlyUpCard, std::vector<uint8_t>& vNewCard);
	bool onDoHu(uint8_t nCard, uint8_t nInvokerIdx);
	void getHuInfo(uint8_t& nInvokeIdx, std::vector<uint8_t>& vHuTypes);
	bool getHoldCardThatCanAnGang(VEC_CARD& vGangCards)override;
	bool getHoldCardThatCanBuGang(VEC_CARD& vGangCards)override;
	// specail process , cao hua j q k , and xi xi qi fei 
	bool canMingGangWithCard(uint8_t nCard) override;
	bool canAnGangWithCard(uint8_t nCard)override;
	bool canBuGangWithCard(uint8_t nCard)override;
	bool onMingGang(uint8_t nCard, uint8_t nGangGetCard) override;
	bool onBuGang(uint8_t nCard, uint8_t nGangGetCard) override;
	bool onAnGang(uint8_t nCard, uint8_t nGangGetCard) override;
	bool isHoldCardCanHu() override;
	bool canHuWitCard(uint8_t nCard) override;
	uint16_t getMingPaiHuaCnt();
	bool getIsZiMo() { return (m_nInvokeHuIdx == m_nCurPlayerIdx); }
	uint8_t getHuCard() { return m_nHuCard; }
	uint16_t getFinalHuCnt( bool isHu );
	bool onPeng(uint8_t nCard)override;
protected:
	void updateHuCntToClient();
	//bool isZiMo() { return m_isHu && (m_nInvokeHuIdx == m_nCurPlayerIdx); }
	uint16_t getPengHuCnt();
	uint16_t getGangHuCnt();
	uint16_t getHoldAnKeCnt(bool isHu, bool isZiMo);
	uint16_t getHoldWenQianCnt(bool isHu);
	uint16_t getFlyUpHuCnt();
	// check hu , shaizi , quan hun , qing su , piao hu 
	bool checkQuanHun();
	bool checkQingSu();
	bool checkPiaoHu( bool isZiMo );

	bool getHuFanxingTypes(uint8_t nHuCard, bool isZiMo, std::vector<uint8_t>& vHuTypes);
protected:
	VEC_CARD m_vFlyupCard;
	GG23Room* m_pRoom;
	uint8_t m_nCurPlayerIdx;

	VEC_CARD m_v5FlyUp;
	VEC_CARD m_v5BuGang;
	VEC_CARD m_v5AnGang;
	VEC_CARD m_v5MingGang;
	// hu info 
	//bool m_isHu;
	uint8_t m_nInvokeHuIdx;
	uint8_t m_nHuCard;
	//std::vector<uint8_t> m_vHuTypes;
};