#include "JJQEPlayerCard.h"
#include "json\json.h"
#include "MJCard.h"
#include "log4z.h"
#include "JJQERoom.h"
#include "IMJPoker.h"
void JJQEPlayerCard::bindRoom(JJQERoom* pRoom, uint8_t nPlayerIdx )
{
	m_pRoom = pRoom;
	m_nCurPlayerIdx = nPlayerIdx;
}

void JJQEPlayerCard::reset()
{

}

uint8_t JJQEPlayerCard::getHuaCardToBuHua()
{
	auto nNewCard = getNewestFetchedCard();
	if (0 == nNewCard)
	{

	}
	else
	{
		auto nType = card_Type(nNewCard);
		if ( eCT_Hua == nType)
		{
			return nNewCard;
		}
	}

	if (m_vCards[eCT_Hua].empty() == false)
	{
		return m_vCards[eCT_Hua].front();
	}

	return -1;
}

bool JJQEPlayerCard::getCardInfo(Json::Value& jsPeerCards)
{
	// svr: { idx : 2 , nDanDiaoKuaiZhao : 23 ,anPai : [2,3,4,34] , huaPai : [23,23,12] ,chuPai: [2,34,4] , anGangPai : [23,24],buGang : [23,45] ,pengGangInfo : [ { targetIdx : 23 , actType : 23 , card : 23 } , .... ]  }
	IMJPlayerCard::VEC_CARD vAnPai, vChuPai, vAnGangedCard;
	Json::Value jsAnPai, jsChuPai, jsAngangedPai, jsBuGang, jsHuaPai, jsFlyUp, jsJianPeng;

	getHoldCard(vAnPai);
	getChuedCard(vChuPai);
	getAnGangedCard(vAnGangedCard);

	auto toJs = [](IMJPlayerCard::VEC_CARD& vCards, Json::Value& js)
	{
		for (auto& c : vCards)
		{
			js[js.size()] = c;
		}
	};

	toJs(vAnPai, jsAnPai); toJs(vChuPai, jsChuPai); toJs(vAnGangedCard, jsAngangedPai); toJs(m_vGanged, jsBuGang); toJs(m_vBuHuaCard, jsHuaPai); toJs(m_vFlyupCard, jsFlyUp);
	toJs(m_vJianPeng,jsJianPeng);
	jsPeerCards["anPai"] = jsAnPai; jsPeerCards["chuPai"] = jsChuPai; jsPeerCards["anGangPai"] = jsAngangedPai; jsPeerCards["buGang"] = jsBuGang; jsPeerCards["huaPai"] = jsHuaPai;
	jsPeerCards["flyUp"] = jsFlyUp;
	jsPeerCards["jianPeng"] = jsJianPeng;
	return true;
}

bool JJQEPlayerCard::onFlyUp(std::vector<uint8_t>& vFlyUpCard, std::vector<uint8_t>& vNewCard )
{
	if (vFlyUpCard.empty())
	{
		LOGFMTE("why request an empty fly up ? ");
		return false;
	}

	std::vector<uint8_t> vCanQiFei;
	getHoldCardThatCanAnGang(vCanQiFei);
	if (vFlyUpCard.size() > vCanQiFei.size())
	{
		return false;
	}

	// check cantian 
	for ( auto& ref : vFlyUpCard )
	{
		auto nCheckCard = ref;
		if (card_Type(nCheckCard) == eCT_Jian && card_Value(nCheckCard) > 3)
		{
			nCheckCard -= 3;
		}

		auto ref = std::find_if(vCanQiFei.begin(), vCanQiFei.end(), [nCheckCard](uint8_t nCanFlyCard)
		{
			if ( nCanFlyCard == nCheckCard)
			{
				return true;
			}

			auto checkType = card_Type(nCheckCard);
			auto checkValue = card_Value(nCheckCard);
			bool isCheckCardXiXi = eCT_Hua == checkType && checkValue <= 2;

			auto nCanFlyType = card_Type(nCanFlyCard);
			if (nCanFlyType == eCT_Hua && isCheckCardXiXi )
			{
				return true;
			}
			return false;
		}
		);

		if ( ref == vCanQiFei.end() )
		{
			return false;
		}
	}

	// do qi fei 
	for ( auto& ref : vFlyUpCard )
	{
		auto checkType = card_Type(ref);
		if (checkType == eCT_Hua) // xixi qi fei 
		{
			do
			{
				auto iterXiXi = std::find_if(m_vCards[eCT_Hua].begin(), m_vCards[eCT_Hua].end(), [](uint8_t& nCheckCard) { return (card_Value(nCheckCard)) <= 2; });
				if ( iterXiXi == m_vCards[eCT_Hua].end())
				{
					break;
				}
				m_vCards[eCT_Hua].erase(iterXiXi);
				// bu hua 
				auto nNewCard = m_pRoom->getMJPoker()->distributeOneCard();
				onMoCard(nNewCard);
				vNewCard.push_back(nNewCard);
			
			} while (1);
			continue;
		}
		
		// not xixi qi fei 
		uint8_t nNewCard = 0;
		onQEAnGang(ref, nNewCard);
		if (nNewCard != 0)
		{
			vNewCard.push_back(nNewCard);
		}
	}

	// avoid double caculate hu cnts ;
	m_vAnGanged.clear();
	m_vFlyupCard = vFlyUpCard;
	return true;
}

void JJQEPlayerCard::onBuHua(uint8_t nHuaCard, uint8_t nNewCard)
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

	onMoCard(nNewCard);
	m_vBuHuaCard.push_back(nHuaCard);
}

void JJQEPlayerCard::onGetChaoZhuangHua(uint8_t nHua)
{
	auto eType = card_Type(nHua);
	if (eType != eCT_Hua)
	{
		LOGFMTE("onGetChaoZhuangHua target not hua card = %u", nHua);
		return;
	}
	m_vBuHuaCard.push_back(nHua);
}

bool JJQEPlayerCard::onDoHu( bool isZiMo, uint8_t nCard )
{
	return true;
}

void JJQEPlayerCard::getHuInfo(uint8_t& nInvokeIdx, std::vector<uint8_t>& vHuTypes)
{
	if ( m_vHuTypes.empty() )  // not hu 
	{
		return;
	}

	nInvokeIdx = m_nInvokeHuIdx;
	vHuTypes = m_vHuTypes;
}

uint32_t JJQEPlayerCard::getAllHuCnt()
{
	return 0;
}

bool JJQEPlayerCard::doAutoBuhua(Json::Value& jsHua, Json::Value& jsCard)
{
	auto pPoker = m_pRoom->getMJPoker();
	uint8_t nHua = getHuaCardToBuHua();
	while ((uint8_t)-1 != nHua)
	{
		jsHua[jsHua.size()] = nHua;
		auto nCard = pPoker->distributeOneCard();
		jsCard[jsCard.size()] = nCard;
		onBuHua(nHua, nCard);
		nHua = getHuaCardToBuHua();
	}

	return jsHua.size() > 0;
}

bool JJQEPlayerCard::getHoldCardThatCanAnGang(VEC_CARD& vGangCards)
{
	MJPlayerCard::getHoldCardThatCanAnGang(vGangCards);
	// check jian zhang an Gang 
	auto nJianZhang = m_pRoom->getJianZhang();
	if ((uint8_t)-1 == nJianZhang)
	{
		LOGFMTE("check an gang jian zhang is -1");
		return false;
	}

	auto nJianType = card_Type(nJianZhang);
	auto nJianValue = card_Value(nJianZhang);
	bool isHuaJian = nJianType == eCT_Hua;
	if (!isHuaJian)
	{
		auto Cnt = std::count(m_vCards[nJianType].begin(), m_vCards[nJianType].begin(), nJianZhang );
		if ( Cnt == 3 )
		{
			vGangCards.push_back(nJianZhang);
		}
	}
	
	// check wu xi qi fen 
	auto nXiXiCnt = std::count_if(m_vCards[eCT_Hua].begin(), m_vCards[eCT_Hua].begin(), [](uint8_t& nCard) { auto nValue = card_Value(nCard); return nValue <= 2; });
	bool isJianXiXi = (eCT_Hua == nJianType && nJianValue <= 2);
	if (nXiXiCnt == (isJianXiXi ? 4 : 5))
	{
		vGangCards.push_back(nJianZhang);
	}

	// check jian 
	if (m_vCards[eCT_Jian].size() > 3)
	{
		uint8_t nJ = make_Card_Num(eCT_Jian, 1);
		for (auto nValue = 0; nValue < 3; ++nValue)
		{
			nJ += nValue;
			if ( canAnGangWithCard(nJ) )
			{
				vGangCards.push_back(nJ);
			}
		}
	}
	return vGangCards.empty() == false;
}

bool JJQEPlayerCard::canMingGangWithCard(uint8_t nCard)
{
	auto nCardType = card_Type(nCard);
	if (eCT_Jian == nCardType)
	{
		auto Cnt = std::count_if(m_vCards[eCT_Jian].begin(), m_vCards[eCT_Jian].begin(), [nCard](uint8_t nCheckCard) { int16_t nRet = abs((int16_t)nCheckCard - (int16_t)nCard); return (0 == nRet || 3 == nRet);  });
		return Cnt >= 3;
	}
	return MJPlayerCard::canMingGangWithCard(nCard);
}

bool JJQEPlayerCard::canAnGangWithCard(uint8_t nCard)
{
	// check is jiang zhang 
	auto nJianZhang = m_pRoom->getJianZhang();
	auto nCardType = card_Type(nCard);
	auto nCardValue = card_Value(nCard);
	if ( nCardType == eCT_Hua && nCardValue <= 2 )  // means xi xi 
	{
		// check xi xi qi fei 
		if (m_vCards[eCT_Hua].size() < 4)
		{
			return false;
		}

		auto nJianType = card_Type(nJianZhang);
		auto nJianValue = card_Value(nJianZhang);
		bool isJianXiXi = (eCT_Hua == nJianType && nJianValue <= 2);
		auto nXiXiCnt = std::count_if(m_vCards[eCT_Hua].begin(), m_vCards[eCT_Hua].begin(), [](uint8_t& nCard) { auto nValue = card_Value(nCard); return nValue <= 2; });
		return (nXiXiCnt == (isJianXiXi ? 4 : 5));
	}
	else if ( eCT_Jian == nCardType )
	{
		if ( m_vCards[eCT_Jian].size() < 3 )
		{
			return false;
		}

		auto Cnt = std::count_if(m_vCards[eCT_Jian].begin(), m_vCards[eCT_Jian].begin(), [nCard](uint8_t nCheckCard) { int16_t nRet = abs((int16_t)nCheckCard - (int16_t)nCard); return (0 == nRet || 3 == nRet);  });
		int16_t nRet = abs((int16_t)nJianZhang - (int16_t)nCard);
		bool isJianJIanZhang = (0 == nRet || 3 == nRet);
		return Cnt == (( isJianJIanZhang) ? 3 : 4 );
	}
	else if (nCard == nJianZhang && nCardType != eCT_Hua && eCT_Jian != nCardType)   // card is jiang zhang 
	{
		auto Cnt = std::count(m_vCards[nCardType].begin(), m_vCards[nCardType].begin(), nCard);
		return Cnt == 3;
	}

	return MJPlayerCard::canAnGangWithCard(nCard);
}

bool JJQEPlayerCard::canPengWithCard(uint8_t nCard)
{
	auto nCardType = card_Type(nCard);
	if ( eCT_Jian == nCardType )
	{
		auto Cnt = std::count_if(m_vCards[eCT_Jian].begin(), m_vCards[eCT_Jian].begin(), [nCard](uint8_t nCheckCard) { int16_t nRet = abs((int16_t)nCheckCard - (int16_t)nCard); return (0 == nRet || 3 == nRet);  });
		return Cnt >= 2;
	}
	return MJPlayerCard::canPengWithCard(nCard);
}

bool JJQEPlayerCard::canBuGangWithCard(uint8_t nCard)
{
	auto nCardType = card_Type(nCard);
	if (eCT_Jian == nCardType)
	{
		auto Cnt = std::count_if(m_vJianPeng.begin(), m_vJianPeng.begin(), [nCard](uint8_t nCheckCard) { int16_t nRet = abs((int16_t)nCheckCard - (int16_t)nCard); return (0 == nRet || 3 == nRet);  });
		return Cnt >= 3;
	}
	return MJPlayerCard::canBuGangWithCard(nCard);
}

bool JJQEPlayerCard::onPeng( uint8_t nCard )
{
	if ( card_Type(nCard) != eCT_Jian )
	{
		return MJPlayerCard::onPeng(nCard);
	}

	auto nValue = card_Value(nCard);
	if ( nValue > 3 )  // other is black ;
	{
		m_vPengedOtherHeiJQK.push_back(nCard);
		// remove self red two 
		auto& vCard = m_vCards[eCT_Jian];
		auto nEraseCnt = 2;
		while (nEraseCnt-- > 0)
		{
			auto iter = std::find(vCard.begin(), vCard.end(), nCard - 3 );
			vCard.erase(iter);
			// add to jian peng 
			m_vJianPeng.push_back(nCard - 3);
		}
	}
	else // other is red ;
	{
		auto& vCard = m_vCards[eCT_Jian];
		auto nSelfRedCnt = std::count( vCard.begin(),vCard.end(),nCard );
		if ( nSelfRedCnt >= 2 )  // self have two red ;
		{
			auto nEraseCnt = 2;
			while (nEraseCnt-- > 0)
			{
				auto iter = std::find(vCard.begin(), vCard.end(), nCard);
				vCard.erase(iter);
				// add to jian peng 
				m_vJianPeng.push_back(nCard);
			}
		}
		else  // self do not have two red , must a black ;
		{
			m_vJianPeng.push_back(nCard+3);
			m_vJianPeng.push_back(nCard);
			auto iter = std::find(vCard.begin(), vCard.end(), nCard);
			if (iter != vCard.end())
			{
				vCard.erase(iter);
			}
			else
			{
				LOGFMTE("erase black peng , error card = %u",nCard );
			}
			
			iter = std::find(vCard.begin(), vCard.end(), nCard + 3 ); // black 
			if (iter != vCard.end())
			{
				vCard.erase(iter);
			}
			else
			{
				LOGFMTE("erase black peng , error card = %u", nCard + 3 );
			}
		}
	}
	return true;
}

bool JJQEPlayerCard::onMingGang(uint8_t nCard, uint8_t nGangGetCard)
{
	auto nCardType = card_Type(nCard);
	if ( eCT_Jian == nCardType )
	{
		if (card_Value(nCard) > 3 )
		{
			m_vPengedOtherHeiJQK.push_back(nCard);
		}

		do
		{
			auto iter = std::find_if(m_vCards[eCT_Jian].begin(),m_vCards[eCT_Jian].end(), [nCard](uint8_t nCheckCard) { int16_t nRet = abs((int16_t)nCheckCard - (int16_t)nCard); return (0 == nRet || 3 == nRet);  });
			if (iter == m_vCards[eCT_Jian].end())
			{
				break;
			}
			m_vCards[eCT_Jian].erase(iter);

		} while (1);
		m_vGanged.push_back(nCard);
		onMoCard(nGangGetCard);
		return true;
	}
	return MJPlayerCard::onMingGang(nCard,nGangGetCard);
}

bool JJQEPlayerCard::onQEAnGang(uint8_t nCard, uint8_t& nGetNewCard)
{
	nGetNewCard = 0;
	auto nType = card_Type(nCard);
	uint8_t nDeleteCnt = 0;
	do
	{
		auto iter = std::find_if(m_vCards[nType].begin(), m_vCards[nType].end(), [nType, nCard](uint8_t& nCheckCard)
		{
			if (eCT_Jian != nType)
			{
				return nCard == nCheckCard;
			}
			int16_t nRet = abs((int16_t)nCheckCard - (int16_t)nCard);
			return (0 == nRet || 3 == nRet);
		});

		if (iter == m_vCards[nType].end())
		{
			break;
		}
		m_vCards[nType].erase(iter);
		++nDeleteCnt;

	} while (1);

	if (nDeleteCnt < 3)
	{
		LOGFMTE("an gang delte cnt = %", nDeleteCnt);
		return false;
	}

	m_vAnGanged.push_back(nCard);
	if ( nDeleteCnt == 4 ) // normal an gang 
	{
		nGetNewCard = m_pRoom->getMJPoker()->distributeOneCard();
		onMoCard(nGetNewCard);
	}
	else
	{
		LOGFMTD("jian zhang an gang %u",nCard);
	}
	// other means jian zhang an gang , so need not mo pai 
	return true;
}

bool JJQEPlayerCard::onBuGang(uint8_t nCard, uint8_t nGangGetCard)
{
	auto nCardType = card_Type(nCard);
	if (eCT_Jian == nCardType)
	{
		// delete jian peng 
		do
		{
			auto iter = std::find_if(m_vJianPeng.begin(), m_vJianPeng.end(), [nCard](uint8_t nCheckCard) { int16_t nRet = abs((int16_t)nCheckCard - (int16_t)nCard); return (0 == nRet || 3 == nRet);  });
			if (iter == m_vJianPeng.end())
			{
				break;
			}
			m_vJianPeng.erase(iter);

		} while (1);
		// do bu gang 
		m_vGanged.push_back(nCard);
		onMoCard(nGangGetCard);
		return true;
	}
	return MJPlayerCard::onBuGang(nCard, nGangGetCard);
}

uint8_t JJQEPlayerCard::getBlackJQKHuCnt()
{
	// seacher peng 
	uint8_t nQJKCnt = 0;
	uint8_t nHuCnt = 0;
	uint8_t nCheck = make_Card_Num(eCT_Jian, 4 );
	for ( uint8_t nCnt = 0; nCnt < 3; ++nCnt )
	{
		// seacher peng 
		bool isFind = false;
		auto iter = std::find(m_vJianPeng.begin(), m_vJianPeng.end(), nCheck + nCnt);
		if ( iter != m_vJianPeng.end())
		{
			isFind = true;
		}

		if (!isFind && m_vAnGanged.size() > 0 )
		{
			// secher an gang 
			auto iterBig = std::find(m_vAnGanged.begin(), m_vAnGanged.end(), nCheck + nCnt);
			auto iterAn = std::find(m_vAnGanged.begin(), m_vAnGanged.end(), nCheck + nCnt - 3);
			if (iterBig != m_vAnGanged.end() || iterAn != m_vAnGanged.end())
			{
				if (m_pRoom->getJianZhang() != (nCheck + nCnt))
				{
					isFind = true;
				}
			}
		}

		if (!isFind && m_vGanged.size() > 0 )
		{
			// secher MingGang ;
			auto iterMingBig = std::find(m_vGanged.begin(), m_vGanged.end(), nCheck + nCnt);
			auto iterMing = std::find(m_vGanged.begin(), m_vGanged.end(), nCheck + nCnt - 3);
			if (iterMingBig != m_vGanged.end() || iterMing != m_vGanged.end())
			{
				auto iterOther = std::find(m_vPengedOtherHeiJQK.begin(), m_vPengedOtherHeiJQK.end(), nCheck + nCnt);
				if (iterOther == m_vPengedOtherHeiJQK.end())
				{
					isFind = true;
				}
			}
		}

		if (!isFind && m_vCards[eCT_Jian].size() > 0 )
		{
			// secher hold card 
			auto iterHold = std::find(m_vCards[eCT_Jian].begin(), m_vCards[eCT_Jian].end(), nCheck + nCnt);
			if (iterHold != m_vCards[eCT_Jian].end())
			{
				isFind = true;
			}
		}

		if (!isFind && m_vFlyupCard.size() > 0 )
		{
			// secher fly up 
			auto iterFlyBig = std::find(m_vFlyupCard.begin(), m_vFlyupCard.end(), nCheck + nCnt);
			auto iterFlyAn = std::find(m_vFlyupCard.begin(), m_vFlyupCard.end(), nCheck + nCnt - 3);
			if (iterFlyBig != m_vFlyupCard.end() || iterFlyAn != m_vFlyupCard.end())
			{
				if (m_pRoom->getJianZhang() != (nCheck + nCnt))
				{
					isFind = true;
				}
			}
		}

		if (isFind)
		{
			nHuCnt += 10;
			if (m_nCurPlayerIdx == nCnt)
			{
				nHuCnt += 10;
			}
			++nQJKCnt;
		}

	}
	if (nQJKCnt == 3)
	{
		nHuCnt += 10;
	}
	LOGFMTD( "room id = %u idx = %u jqk cnt = %u",m_pRoom->getRoomID(),m_nCurPlayerIdx,nQJKCnt );
	return nQJKCnt;
}

uint16_t JJQEPlayerCard::getPengHuCnt()
{
	uint16_t nJianHuCnt = 0;
	// jian peng
	if (!m_vJianPeng.empty())
	{
		uint8_t nBaseJian = make_Card_Num(eCT_Jian, 1);
		for (uint8_t n = 0; n < 3; ++n)
		{
			auto nCheck = nBaseJian + n;
			auto iter = std::find(m_vJianPeng.begin(), m_vJianPeng.end(), nCheck);
			if (iter != m_vJianPeng.end())
			{
				if ( nCheck == m_pRoom->getJianZhang())
				{
					nJianHuCnt += 16;
					LOGFMTD(" jian peng room id = %u jianzha peng as ming gang npeng = %u idx = %u", m_pRoom->getRoomID(), nCheck, m_nCurPlayerIdx);
				}
				else
				{
					nJianHuCnt += 4;
				}
				
			}
		}
	}

	// pu tong peng 
	for (auto nPeng : m_vPenged)
	{
		if ( m_pRoom->isCardJianPai(nPeng))
		{
			if (nPeng == m_pRoom->getJianZhang())
			{
				nJianHuCnt += 16;
				LOGFMTD("room id = %u jianzha peng as ming gang npeng = %u idx = %u",m_pRoom->getRoomID(),nPeng,m_nCurPlayerIdx );
			}
			else
			{
				nJianHuCnt += 4;
			}
		}
		else
		{
			nJianHuCnt += 1;
		}
	}

	LOGFMTD("room id = %u idx = %u pengHu = %u",m_pRoom->getRoomID(),m_nCurPlayerIdx,nJianHuCnt );
	return nJianHuCnt;
}

uint16_t JJQEPlayerCard::getGangHuCnt()
{
	// an gang ;
	uint16_t nHuCnt = 0;
	for (auto& nGang : m_vAnGanged)
	{
		if (m_pRoom->isCardJianPai(nGang))
		{
			nHuCnt += 24;
		}
		else
		{
			nHuCnt += 6;
		}
	}

	// ming gang 
	for (auto& nGang : m_vGanged )
	{
		if (m_pRoom->isCardJianPai(nGang))
		{
			nHuCnt += 16;
		}
		else
		{
			nHuCnt += 4;
		}
	}

	LOGFMTD("room id = %u player idx = %u gangHuCnt = %u",m_pRoom->getRoomID(),m_nCurPlayerIdx,nHuCnt );
	return nHuCnt;
}

uint16_t JJQEPlayerCard::getHoldAnKeCnt()
{
	bool isThisHu = isThisPlayerHu();
	uint16_t nHoldHuCnt = 0;
	uint8_t nJianZhang = m_pRoom->getJianZhang();
	if (card_Type(nJianZhang) == eCT_Jian && card_Value(nJianZhang) > 3)
	{
		nJianZhang - 3;
	}

	auto nHuCard = 0;
	if (isThisHu)
	{
		nHuCard = m_nHuCard;
		if (card_Type(nHuCard) == eCT_Jian && card_Value(nHuCard) > 3)
		{
			nHuCard - 3;
		}
	}

	// back up jian 
	VEC_CARD vJian = m_vCards[eCT_Jian];
	for (auto& ref : m_vCards[eCT_Jian])
	{
		if (card_Value(ref) > 3)
		{
			ref -= 3;
		}
	}

	for (auto& vCards : m_vCards)
	{
		if (vCards.size() < 3)
		{
			continue;
		}

		for (uint8_t nIdx = 0; nIdx + 2 < vCards.size(); )
		{
			// check 4 
			if (isThisHu && (nIdx + 3) < vCards.size() && (vCards[nIdx] == vCards[nIdx + 3]) )  // look as an gang ;
			{
				uint8_t nThisHu = 6;
				if ( (!isZiMo()) && vCards[nIdx] == nHuCard) // suan ming gang 
				{
					nThisHu = 4;
				}

				if (m_pRoom->isCardJianPai(vCards[nIdx]))
				{
					nThisHu *= 4;
				}
				nHoldHuCnt += nThisHu;
				nIdx += 4;
				continue;
			}

			// check 3 ;
			if ( vCards[nIdx] == vCards[nIdx + 2])  // ke zi ;
			{
				uint8_t nThisHu = 2;
				if (isThisHu && (!isZiMo()) && vCards[nIdx] == nHuCard)  // when hu other card , should regart as peng ;
				{
					nThisHu = 1;
				}

				if ( isThisHu && nJianZhang == vCards[nIdx]) // should regarts as gang ;
				{
					nThisHu = 6;
					if ((!isZiMo()) && vCards[nIdx] == nHuCard)
					{
						nThisHu = 4;
					}
				}

				if (m_pRoom->isCardJianPai(vCards[nIdx]))
				{
					nThisHu *= 4;
				}
				nHoldHuCnt += nThisHu;
				nIdx += 3;
				continue;
			}
			++nIdx;
		}
	}

	if ( isThisHu ) // level up peng to ming gang ;
	{
		for (auto& nPeng : m_vPenged)
		{
			auto nType = card_Type(nPeng);
			auto iter = std::find(m_vCards[nType].begin(), m_vCards[nType].end(),nPeng );
			if ( iter == m_vCards[nType].end() )
			{
				continue;
			}
			LOGFMTD("level up peng to ming gang = %u", nPeng );
			nHoldHuCnt += (m_pRoom->isCardJianPai(nPeng) ? 12 : 3 );  // level peng to ming gang   + 3 , jian zhang  3 * 4 ;
		}

		if (m_vJianPeng.size() > 1)
		{
			// find jian peng , omit duplicate cards ;
			std::map<uint8_t, uint8_t> vMp;
			for (auto nJian : m_vJianPeng)
			{
				if (card_Value(nJian) > 3)
				{
					nJian -= 3;
				}
				vMp[nJian] = 1;
			}

			// do check ;
			for (auto& ref : vMp)
			{
				auto iter = std::find(m_vCards[eCT_Jian].begin(), m_vCards[eCT_Jian].end(), ref.first );
				if (iter == m_vCards[eCT_Jian].end())
				{
					continue;
				}

				nHoldHuCnt += 12;  // level peng to ming gang   + 3 , jian zhang  3 * 4 , j q k must jian zhang  ;
				LOGFMTD( "level up jian peng to ming gang = %u",ref.first );
			}

		}
	}
	// restore jian 
	m_vCards[eCT_Jian] = vJian;
	LOGFMTD("room id = %u , player idx = %u hold hua cnt = %u ", m_pRoom->getRoomID(),m_nCurPlayerIdx ,nHoldHuCnt );
	return nHoldHuCnt;
}

uint16_t JJQEPlayerCard::getHoldWenQianCnt()
{
	if ( false == isThisPlayerHu())
	{
		return 0;
	}

	uint8_t nWenQianCnt = 100;
	for (uint8_t nIdx = 1; nIdx <= 3; ++nIdx)
	{
		auto nBlackACnt = std::count(m_vCards[eCT_Tong].begin(), m_vCards[eCT_Tong].end(), make_Card_Num(eCT_Tong, nIdx ));
		if (nBlackACnt < nWenQianCnt)
		{
			nWenQianCnt = nBlackACnt;
		}

		if ( 0 == nWenQianCnt )
		{
			return 0;
		}
	}

	LOGFMTD( "room id = %u idx = %u wenqian cnt = %u",m_pRoom->getRoomID(),m_nCurPlayerIdx , nWenQianCnt );
 
	switch ( nWenQianCnt )
	{
	case 1 :
		return 20;
	case 2 :
		return 50;
	case 3 :
		return 100;
	case 4 : 
		return 200;
	default:
		LOGFMTD("invalid wen qian cnt = %u roomid = %u idx = %u",nWenQianCnt,m_pRoom->getRoomID(),m_nCurPlayerIdx );
		return 0;
	}
	return 0;
}

uint16_t JJQEPlayerCard::getFlyUpHuCnt()
{
	uint16_t nFlyHuCnt = 0;
	for (auto& ref : m_vFlyupCard)
	{
		if ( card_Type(ref) == eCT_Hua )  // xixi qi fei ;
		{
			if (card_Type(m_pRoom->getJianZhang()) == eCT_Hua)  // xixi jian zhang 
			{
				if ( card_Value(m_pRoom->getJianZhang()) <= 2 )  // da xiao king jian 
				{
					nFlyHuCnt += 560;
				}
				else
				{
					nFlyHuCnt += 800;
				}
			}
			else
			{
				nFlyHuCnt += 400;
			}
			continue;
		}

		if ( m_pRoom->isCardJianPai(ref) )
		{
			nFlyHuCnt += 32;
		}
		else
		{
			nFlyHuCnt += 8;
		}
	}

	LOGFMTD("room id = %u idx = %u wenqian cnt = %u", m_pRoom->getRoomID(), m_nCurPlayerIdx, nFlyHuCnt );
	return nFlyHuCnt;
}

uint16_t JJQEPlayerCard::getHuaHuCnt()
{
	// jie hun ;
	if (m_vBuHuaCard.empty())
	{
		return 0;
	}

	// add hold hua to buhua card 
	VEC_CARD vHuaCard = m_vBuHuaCard;
	for ( auto ref : m_vCards[eCT_Hua] )
	{
		vHuaCard.push_back(ref);
		LOGFMTD("room id = %u player idx = %u add hold hua to bu hua = %u ",m_pRoom->getRoomID(),m_nCurPlayerIdx,ref );
	}

	uint16_t nHuCnt = 0;
	auto nXiaoKing = std::count(vHuaCard.begin(), vHuaCard.end(),make_Card_Num(eCT_Hua,1));  // 2 zhang  20 hu mei ge 
	nHuCnt += nXiaoKing * 20;
	if ( nXiaoKing == 2 ) // jie hun 
	{
		nHuCnt += 10;
	}

	auto nDaKing = std::count(vHuaCard.begin(), vHuaCard.end(), make_Card_Num(eCT_Hua, 2));  // 3 zhang  10 hu mei ge 
	nHuCnt += nDaKing * 10;
	if ( nDaKing == 3 ) // jie hun 
	{
		nHuCnt += 20;
	}

	// da jie hun 
	if ( (nXiaoKing + nDaKing) == 5 || ( (nXiaoKing + nDaKing) == 4 && card_Type(m_pRoom->getJianZhang()) == eCT_Hua && card_Value(m_pRoom->getJianZhang() ) <= 2 ) )
	{
		nHuCnt *= 2;
	}

	auto nHuang = std::count(vHuaCard.begin(), vHuaCard.end(), make_Card_Num(eCT_Hua, 3));
	auto nHou = std::count(vHuaCard.begin(), vHuaCard.end(), make_Card_Num(eCT_Hua, 4));
	nHuCnt += ( nHuang + nHou ) * 30;
	if ( nHuang + nHou == 2)
	{
		nHuCnt += 20;
	}
	
	// sun and moon ;
	auto nSun = std::count(m_vBuHuaCard.begin(), m_vBuHuaCard.end(), make_Card_Num(eCT_Hua, 5));
	auto nMoon = std::count(vHuaCard.begin(), vHuaCard.end(), make_Card_Num(eCT_Hua, 6));
	if (nSun > 1)
	{
		nHuCnt += 50;
	}

	if ( nMoon > 1 )
	{
		nHuCnt += 100;
	}
	
	if (nSun + nMoon == 2)
	{
		nHuCnt + 50;
	}
	
	if ( card_Type(m_pRoom->getJianZhang()) == eCT_Hua )
	{
		nHuCnt *= 2;
	}

	LOGFMTD( "room id = %u , player idx = %u , huaHu = %u",m_pRoom->getRoomID(),m_nCurPlayerIdx,nHuCnt );
	return nHuCnt;
}