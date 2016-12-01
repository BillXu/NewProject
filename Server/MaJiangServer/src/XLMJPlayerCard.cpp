#include "XLMJPlayerCard.h"
#include "log4z.h"
#include "MJCard.h"
XLFanXing XLMJPlayerCard::m_tFanXingChecker;
void XLMJPlayerCard::reset()
{
	MJPlayerCard::reset();
	m_nQueType = eCT_Max;
}

bool XLMJPlayerCard::canEatCard(uint8_t nCard, uint8_t& nWithA, uint8_t& withB)
{
	return false;
}

bool XLMJPlayerCard::onDoHu(bool isZiMo, uint8_t nCard, uint32_t& nHuType, uint8_t& nBeiShu, uint8_t& genCnt)
{
	// if not zi mo , must add to fo check hu ;
	if (!isZiMo)
	{
		auto type = card_Type(nCard);
		if (type >= eCT_Max)
		{
			LOGFMTE("invalid card type for card = %u",nCard);
			return false;
		}
		addCardToVecAsc(m_vCards[type], nCard);
	}

	auto funRemoveAddToCard = [this](uint8_t nCard)
	{
		auto type = card_Type(nCard);
		auto iter = std::find(m_vCards[type].begin(), m_vCards[type].end(), nCard);
		if (iter == m_vCards[type].end())
		{
			LOGFMTE("hu this card should already addto hold card , but can not remove  whay card = %u", nCard);
			return;
		}
		m_vCards[type].erase(iter);
	};

	if (isHoldCardCanHu() == false)
	{
		LOGFMTE("do hu act , but can not hu ? why ? bug card = %u ",nCard );
		debugCardInfo();
		if (!isZiMo)
		{
			funRemoveAddToCard(nCard);
		}
		return false;
	}

	auto ret = getFanxingChecker()->doCheckFanxing(this,nBeiShu,nHuType);
	if ( ret == false)
	{
		LOGFMTE("why no fanxing type matches this card ? must hu here , bug bug !!!!");
		if (!isZiMo)
		{
			funRemoveAddToCard(nCard);
		}
		return false;
	}

	m_vecAlreadyHu.push_back(nCard);

	genCnt = getGenShu();
	// some hu type must jian shao gen Shu ;
	if (nHuType == eFanxing_LongQiDui || nHuType == eFanxing_QingLongQiDui)
	{
		genCnt -= 1;
	}

	funRemoveAddToCard(nCard);
	return true;
}

bool XLMJPlayerCard::isHuaZhu()
{
	if (m_nQueType == eCT_Max)
	{
		LOGFMTE("not set que type ? so you should be hua zhu ");
		return true;
	}
	return m_vCards[getQueType()].empty() == false;
}

uint32_t XLMJPlayerCard::getMaxPossibleBeiShu()
{
	std::set<uint8_t> vCanHu;
	getCanHuCards(vCanHu);
	uint8_t nMaxBeiShu = 0;
	uint32_t nHuType = 0;
	for (auto iter = vCanHu.begin(); iter != vCanHu.end(); ++iter)
	{
		// add to hold card for check ;
		auto nCard = *iter;
		auto type = card_Type(nCard);
		auto& refVcards = m_vCards[type];
		addCardToVecAsc(refVcards, nCard);

		// do check ;
		uint8_t nBeiShu = 0;
		auto ret = getFanxingChecker()->doCheckFanxing(this, nBeiShu, nHuType);

		// remove from hold card after check ;
		auto iterDel = std::find(refVcards.begin(), refVcards.end(), nCard);
		refVcards.erase(iterDel);

		if (ret == false )
		{
			LOGFMTE("get can hu card , but can not check fanxing ? why bug ");
			continue;
		}

		if (nBeiShu > nMaxBeiShu)
		{
			nMaxBeiShu = nBeiShu;
		}
	}
	return nMaxBeiShu;
}

void XLMJPlayerCard::setQueType(uint8_t nType)
{
	if (nType > eCT_Tiao || 0 == nType)
	{
		LOGFMTE("invalid que type = %u ", nType );
		m_nQueType = getAutoQueType();
		return;
	}
	m_nQueType = nType;
}

uint8_t XLMJPlayerCard::getAutoQueType()
{
	auto pfuncGetWeight = [](VEC_CARD& vCards)
	{
		uint16_t nWeiht = vCards.size() * 100;
		std::vector<uint8_t> vecCards;
		vecCards.assign(vCards.begin(),vCards.end());
		std::sort(vecCards.begin(), vecCards.end());

		for (uint8_t nIdx = 0; nIdx < vecCards.size();)
		{
			uint8_t nValue = vecCards[nIdx];
			uint8_t nPiarValue = 0, nKeValue = 0, nGenValue = 0;
			if ((uint8_t)(nIdx + 3) < vecCards.size())
			{
				nGenValue = vecCards[nIdx + 3];
				nKeValue = vecCards[nIdx + 2];
				nPiarValue = vecCards[nIdx + 1];
			}
			else if (nIdx + 2u < vecCards.size())
			{
				nKeValue = vecCards[nIdx + 2];
				nPiarValue = vecCards[nIdx + 1];
			}
			else if (nIdx + 1u < vecCards.size())
			{
				nPiarValue = vecCards[nIdx + 1];
			}

			if (nGenValue == nValue)
			{
				nWeiht += 80;
				nIdx += 4;
				continue;
			}

			if (nKeValue == nValue)
			{
				nWeiht += 60;
				nIdx += 3;
				continue;
			}

			if (nPiarValue == nValue)
			{
				nWeiht += 40;
				nIdx += 2;
				continue;
			}
			++nIdx;
		}
		return nWeiht;
	};

	auto nTiao = pfuncGetWeight(m_vCards[eCT_Tiao]);
	auto nWan = pfuncGetWeight(m_vCards[eCT_Wan]);
	auto nTong = pfuncGetWeight(m_vCards[eCT_Tong]);
	
	if (nWan <= nTiao && nWan <= nTong )
	{
		return eCT_Wan;
	}
	else if (nTiao <= nWan && nTiao <= nTong)
	{
		return eCT_Tiao;
	}
	else
	{
		return eCT_Tong;
	}
	return 0;
}

uint8_t XLMJPlayerCard::getQueType()
{
	return m_nQueType;
}

bool XLMJPlayerCard::canHuWitCard(uint8_t nCard)
{
	auto type = card_Type(nCard);
	if (getQueType() == type)
	{
		return false;
	}

	if (isHuaZhu())
	{
		return false;
	}

	auto iter = std::find(m_vecAlreadyHu.begin(),m_vecAlreadyHu.end(),nCard );
	if (iter != m_vecAlreadyHu.end())
	{
		return true;
	}

	return MJPlayerCard::canHuWitCard(nCard);
}

bool XLMJPlayerCard::isHoldCardCanHu()
{
	if (isHuaZhu())
	{
		return false;
	}
	return MJPlayerCard::isHoldCardCanHu();
}

bool XLMJPlayerCard::isTingPai()
{
	if (isHuaZhu())
	{
		return false;
	}
	return MJPlayerCard::isTingPai();
}

bool XLMJPlayerCard::canMingGangWithCard(uint8_t nCard)
{
	auto type = card_Type(nCard);
	if (getQueType() == type)
	{
		return false;
	}

	return MJPlayerCard::canMingGangWithCard(nCard);
}

bool XLMJPlayerCard::canPengWithCard(uint8_t nCard)
{
	auto type = card_Type(nCard);
	if (getQueType() == type)
	{
		return false;
	}

	return MJPlayerCard::canPengWithCard(nCard);
}

bool XLMJPlayerCard::canAnGangWithCard(uint8_t nCard)
{
	auto type = card_Type(nCard);
	if (getQueType() == type)
	{
		return false;
	}

	return MJPlayerCard::canAnGangWithCard(nCard);
}

bool XLMJPlayerCard::getHoldCardThatCanAnGang(VEC_CARD& vGangCards)
{
	MJPlayerCard::getHoldCardThatCanAnGang(vGangCards);
	for (auto& ref : vGangCards)
	{
		auto type = card_Type(ref);
		if (getQueType() == type)
		{
			ref = 0;
		}
	}

	auto iter = std::find(vGangCards.begin(),vGangCards.end(),0);
	while ( iter != vGangCards.end() )
	{
		vGangCards.erase(iter);
		iter = std::find(vGangCards.begin(), vGangCards.end(), 0);
	}
	return !vGangCards.empty();
}

uint8_t XLMJPlayerCard::getGenShu()
{
	uint8_t nGenCnt = 0; 
	for (auto& refCards : m_vCards)
	{
		if (refCards.size() < 4)
		{
			continue;
		}

		for ( uint8_t nIdx = 0; (nIdx + 3u) < refCards.size();)
		{
			if (refCards[nIdx] == refCards[nIdx + 3])
			{
				++nGenCnt;
				nIdx += 4;
			}
			else
			{
				++nIdx;
			}
		}
	}

	// add gang cnt ;
	nGenCnt += m_vGanged.size();

	// add not not Bu gang card ;
	for (auto ref : m_vPenged)
	{
		auto type = card_Type(ref);
		auto c = std::count(m_vCards[type].begin(),m_vCards[type].end(),ref) ;
		if (c > 0)
		{
			++nGenCnt;
		}
	}
	return nGenCnt;
}

bool XLMJPlayerCard::getHuedCard(VEC_CARD& vhuedCard)
{
	vhuedCard.insert(vhuedCard.begin(), m_vecAlreadyHu.begin(), m_vecAlreadyHu.end());
	return m_vecAlreadyHu.empty() == false;
}

// help fanxing
void XLMJPlayerCard::helpGetHoldCardByType(MJPlayerCard::VEC_CARD& vCards, uint8_t nType)
{
	if (nType >= eCT_Max)
	{
		LOGFMTE("invalid type for fanxing type get hold card");
		return;
	}
	vCards.insert(vCards.begin(), m_vCards[nType].begin(), m_vCards[nType].end() );
}

void XLMJPlayerCard::helpGetPengedCard(MJPlayerCard::VEC_CARD& vCards)
{
	vCards = m_vPenged;
}

void XLMJPlayerCard::helpGetGangCard(MJPlayerCard::VEC_CARD& vCards)
{
	vCards = m_vGanged;
}

bool XLMJPlayerCard::helpGetIs7PairHu()
{
	return canHoldCard7PairHu();
}

uint8_t XLMJPlayerCard::helpGetJiang()
{
	if ( !m_nJIang)
	{
		LOGFMTE("why jiang is 0 , forget invoker canHoldCardHu ? ");
		debugCardInfo();
	}

	return m_nJIang;
}