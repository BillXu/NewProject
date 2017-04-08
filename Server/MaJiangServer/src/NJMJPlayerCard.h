#pragma once 
#include "MJPlayerCard.h"
#include "CommonDefine.h"
#include "json/json.h"
class NJMJRoom;
class NJMJPlayerCard
	:public MJPlayerCard
{
public:
	struct stActCardSign
	{
		uint8_t nCard;
		uint8_t InvokerIdx;
		eMJActType eAct;
	};

	enum eDanDiaoState
	{
		eDanDiao_Not_Set,
		eDanDiao_Do_Set,
		eDanDiao_GiveUp,
		eDanDiao_Max,
	};

public:
	void reset() override;
	void bindRoom(NJMJRoom* pRoom , uint8_t nThisPlayerIdx );
	bool canEatCard(uint8_t nCard, uint8_t& nWithA, uint8_t& withB) override;
	void addActSign(uint8_t nCard, uint8_t nInvokerIdx, eMJActType eAct );
	bool isChued4Card(uint8_t nCard );
	bool isChued4Feng();
	uint8_t getInvokerPengIdx( uint8_t nCard );  // -1 means no target ;
	uint8_t getInvokerGangIdx(uint8_t nCard);  // -1 means no target ;
	void onHuaGang( uint8_t nCard ,uint8_t nNewCard );
	void onBuHua( uint8_t nHuaCard ,uint8_t nCard );
	bool onDoHu(bool isZiMo,uint8_t nCard, bool isBePenged, std::vector<uint16_t>& vHuTypes, uint16_t& nHuHuaCnt, uint16_t& nHardAndSoftHua,bool& isSpecailHuPai, uint8_t nInvokerIdx = -1 );
	bool canHuWitCard(uint8_t nCard)override;
	bool getCanHuCards(std::set<uint8_t>& vCanHuCards)override;
	bool isHoldCardCanHu() override;
	uint8_t getKuaiZhaoBaoPaiIdx();  // -1 means no target 
	uint8_t getLastActSignIdx(); // -1 menas not target ;
	bool canCardHuaGang( uint8_t nCard );
	bool getCardInfo(Json::Value& jsPeerCards);
	uint8_t getHoldCardHuaCnt();
	uint8_t getHuaCardToBuHua(); // -1 means no target ;
	void setSongGangIdx(uint8_t nSongGangIdx );
	uint8_t getSongGangIdx();
	bool getIsSpecailHu( uint8_t nTargetCard, std::vector<uint16_t>& vHuTypes, uint16_t& nHuHuaCnt, uint8_t nInvokerIdx);
	bool getIsZiMoSpecailHu( std::vector<uint16_t>& vHuTypes, uint16_t& nHuHuaCnt);
	//bool getIsQingYiSeKuaiZhaoHu( uint8_t nTargetCard );
	bool getIsDanDiaoHu( uint8_t nTargetCard , std::vector<uint16_t>& vHuTypes, uint16_t& nHuaCnt );
	bool onChuCard(uint8_t nChuCard)override;
	bool onMingGang(uint8_t nCard, uint8_t nGangGetCard) override;
	bool onAnGang(uint8_t nCard, uint8_t nGangGetCard) override;
	uint8_t getSpecailHuBaoPaiKuaiZhaoIdx();
	bool canMingGangWithCard(uint8_t nCard) override;
	bool getHoldCardThatCanAnGang(VEC_CARD& vGangCards)override;
	bool canPengWithCard(uint8_t nCard) override;
	bool canHuWitCardLocal(uint8_t nCard);  // hu lue hua de xian zhi 
	uint8_t eraseRandNotHuaCard( bool isMustNewFeatch ); // return erased card ;
	bool isLocalCardCanHu() { return MJPlayerCard::isHoldCardCanHu(); }
protected:
	// check pai xing 
	bool checkHunYiSe(std::vector<uint16_t>& vHuTypes, uint16_t& nHuaCnt );
	bool checkMenQing(std::vector<uint16_t>& vHuTypes, uint16_t& nHuaCnt);
	bool checkQingYiSe(std::vector<uint16_t>& vHuTypes, uint16_t& nHuaCnt);
	bool checkDuiDuiHu(std::vector<uint16_t>& vHuTypes, uint16_t& nHuaCnt);
	bool checkQiDui(uint8_t nCard, std::vector<uint16_t>& vHuTypes, uint16_t& nHuaCnt);
	bool checkQuanQiuDuDiao(uint8_t nCard, std::vector<uint16_t>& vHuTypes, uint16_t& nHuaCnt, uint8_t nInvokerIdx , bool isZiMo );
	bool checkYaJue(uint8_t nCard, bool isBePenged, std::vector<uint16_t>& vHuTypes, uint16_t& nHuaCnt);
	bool checkWuHuaGuo(std::vector<uint16_t>& vHuTypes, uint16_t& nHuaCnt);

	uint16_t getAllHuaCnt( uint8_t nHuCard );
	bool checkYaDang(uint8_t nCard);
	bool checkDuZhan(uint8_t nCard);
	bool checkBianZhi( uint8_t nCard );
	bool checkQueYi( uint8_t nCard);
protected:
	VEC_CARD m_vBuHuaCard;
	//VEC_CARD m_vHuaGang;
	std::vector<stActCardSign> m_vActCardSign;
	std::vector<stActCardSign> m_vAllActCardSign;
	NJMJRoom* m_pCurRoom;
	uint8_t m_nThisPlayerIdx;

	uint8_t m_nSongGangIdx;
	bool m_isHaveZhiGangFlag;
	bool m_isHaveAnGangFlag;

	VEC_CARD m_vFirse4Card;

	uint8_t m_nDanDiaoKuaiZhaoState;  // 0 not seted , 1 seted , 2 gived up ;
	uint8_t m_nDanDiaoHoldCard;
	uint8_t m_nDanDiaoChuedCard;
};