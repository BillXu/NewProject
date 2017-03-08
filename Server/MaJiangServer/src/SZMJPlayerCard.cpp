#include "SZMJPlayerCard.h"
#include "log4z.h"
#include "MJCard.h"
void SZMJPlayerCard::reset()
{
	MJPlayerCard::reset();
	m_vBuHuaCard.clear();
}

void SZMJPlayerCard::onBuHua(uint8_t nHuaCard, uint8_t nCard)
{
	auto eType = card_Type(nHuaCard);
	if (eType >= eCT_Max)
	{
		LOGFMTE("onBuHua parse card type error so do not have this card = %u", nHuaCard);
		return;
	}

	auto& vCard = m_vCards[eType];
	auto iter = std::find(vCard.begin(), vCard.end(), nHuaCard);
	vCard.erase(iter);

	onMoCard(nCard);
	m_vBuHuaCard.push_back(nHuaCard);
}

bool SZMJPlayerCard::getCardInfo(Json::Value& jsPeerCards)
{

}

bool SZMJPlayerCard::onDoHu(bool isZiMo, uint8_t nCard, std::vector<uint16_t>& vHuTypes, uint16_t& nHuHuaCnt, uint16_t& nHardAndSoftHua)
{

}

uint8_t SZMJPlayerCard::getSongGangIdx()
{

}

uint8_t SZMJPlayerCard::getHuaCardToBuHua()
{
	auto nNewCard = getNewestFetchedCard();
	if (0 == nNewCard)
	{

	}
	else
	{
		auto nType = card_Type(nNewCard);
		if (eCT_Jian == nType || eCT_Hua == nType)
		{
			return nNewCard;
		}
	}


	if (m_vCards[eCT_Jian].empty() == false)
	{
		return m_vCards[eCT_Jian].front();
	}

	if (m_vCards[eCT_Hua].empty() == false)
	{
		return m_vCards[eCT_Hua].front();
	}

	return -1;
}

bool SZMJPlayerCard::canHuWitCard( uint8_t nCard )
{
	// xiao hu yao hua ;
	// da hu ji ke 
}

bool SZMJPlayerCard::checkDaMenQing()
{
	VEC_CARD vAllCard;
	getHoldCard(vAllCard);
	if (vAllCard.size() != 14)
	{
		return false;
	}

	auto n = getHuaCntWithoutHuTypeHuaCnt();
	return n == 0;
}

bool SZMJPlayerCard::checkXiaoMenQing()
{
	VEC_CARD vCard;
	getMingGangedCard(vCard);
	if (vCard.empty() == false)
	{
		return false;
	}

	getPengedCard(vCard);
	if (vCard.empty() == false)
	{
		return false;
	}
	return true;
}

bool SZMJPlayerCard::checkHunYiSe()
{
	VEC_CARD vAllCard;
	getHoldCard(vAllCard);

	VEC_CARD vTemp;
	getAnGangedCard(vTemp);
	vAllCard.insert(vAllCard.end(), vTemp.begin(), vTemp.end());

	vTemp.clear();
	getMingGangedCard(vTemp);
	vAllCard.insert(vAllCard.end(), vTemp.begin(), vTemp.end());

	vTemp.clear();
	getPengedCard(vTemp);
	vAllCard.insert(vAllCard.end(), vTemp.begin(), vTemp.end());

	bool bFindFeng = false;
	do
	{
		auto iter = std::find_if(vAllCard.begin(), vAllCard.end(), [](uint8_t n) { return card_Type(n) == eCT_Feng; });
		if (iter != vAllCard.end() )
		{
			bFindFeng = true;
			vAllCard.erase(iter);
		}
		else
		{
			break;
		}
	} while ( 1 );

	if (bFindFeng == false || vAllCard.empty() )
	{
		return false;
	}

	auto nType = card_Type(vAllCard.front());
	for (auto& ref : vAllCard)
	{
		auto tt = card_Type(ref);
		if (nType != tt)
		{
			return false;
		}
	}
	return true;
}

bool SZMJPlayerCard::checkQingYiSe()
{
	VEC_CARD vAllCard;
	getHoldCard(vAllCard);

	VEC_CARD vTemp;
	getAnGangedCard(vTemp);
	vAllCard.insert(vAllCard.end(),vTemp.begin(),vTemp.end());

	vTemp.clear();
	getMingGangedCard(vTemp);
	vAllCard.insert(vAllCard.end(), vTemp.begin(), vTemp.end());

	vTemp.clear();
	getPengedCard(vTemp);
	vAllCard.insert(vAllCard.end(), vTemp.begin(), vTemp.end());
	
	auto nType = card_Type(vAllCard.front());
	for (auto& ref : vAllCard)
	{
		auto tt = card_Type(ref);
		if (nType != tt)
		{
			return false;
		}
	}
	return true;
}

bool SZMJPlayerCard::checkDuiDuiHu()
{
	VEC_CARD vAllCard;
	getHoldCard(vAllCard);
	std::sort(vAllCard.begin(),vAllCard.end());
	bool bFindJiang = false;
	for ( uint8_t nIdx = 0; ( nIdx + 1 ) < vAllCard.size(); )
	{
		auto nThirdIdx = nIdx + 2;
		if ( nThirdIdx < vAllCard.size() && vAllCard[nIdx] == vAllCard[nThirdIdx] )
		{
			nThirdIdx += 3;
			continue;
		}

		if (false == bFindJiang && vAllCard[nIdx] == vAllCard[nIdx + 1])
		{
			nIdx += 2;
			bFindJiang = true;
			continue;
		}
		return false;
	}
	return bFindJiang;
}

bool SZMJPlayerCard::checkQiDui()
{
	if (canHoldCard7PairHu())
	{
		return true;
	}
	return false;
}

bool SZMJPlayerCard::checkHaoHuaQiDui()
{
	if (checkQiDui() == false)
	{
		return false;
	}

	VEC_CARD v;
	getHoldCard(v);
	for ( uint8_t nIdx = 0; (nIdx + 3) < v.size(); ++nIdx )
	{
		if (v[nIdx] == v[nIdx + 3])
		{
			return true;
		}
	}

	return false;
}

bool SZMJPlayerCard::checkDaDiaoChe()
{
	VEC_CARD v;
	getHoldCard(v);
	return (v.size() == 2) && (v[0] == v[1]);
}

uint8_t SZMJPlayerCard::getHuaCntWithoutHuTypeHuaCnt()
{
	VEC_CARD vCard;
	uint8_t nHuaCnt = m_vBuHuaCard.size();
	// check ming gang 
	getMingGangedCard(vCard);
	for ( auto& ref :vCard )
	{
		if (card_Type(ref) == eCT_Feng)
		{
			nHuaCnt += 3;
		}
		else
		{
			nHuaCnt += 1;
		}
	}
	// check an gang 
	vCard.clear();
	getAnGangedCard(vCard);
	for (auto& ref : vCard)
	{
		if ( card_Type(ref) == eCT_Feng )
		{
			nHuaCnt += 4;
		}
		else
		{
			nHuaCnt += 2;
		}
	}
	// check feng peng 
	vCard.clear();
	getPengedCard(vCard);
	for (auto& ref : vCard)
	{
		if ( card_Type(ref) == eCT_Feng )
		{
			nHuaCnt += 1;
		}
	}
	// check feng an ke 
	auto& vFeng = m_vCards[eCT_Feng];
	for (uint8_t nidx = 0; (nidx + 2) < vFeng.size(); )
	{
		if (vFeng[nidx] == vFeng[nidx + 2])
		{
			nHuaCnt += 1;
			nidx += 3;
			continue;
		}
		++nidx;
	}

	return nHuaCnt;
}