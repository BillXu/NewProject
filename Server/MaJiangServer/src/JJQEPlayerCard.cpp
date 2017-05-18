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
	return vGangCards.empty() != false;
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
		auto iter = std::find(m_vJianPeng.begin(), m_vJianPeng.end(), nCheck + nCnt);
		if ( iter != m_vJianPeng.end())
		{
			nHuCnt += 10;
			if (m_nCurPlayerIdx == nCnt)
			{
				nHuCnt += 10;
			}
			++nQJKCnt;
			continue;
		}

		// secher an gang 
		auto iterBig = std::find(m_vAnGanged.begin(), m_vAnGanged.end(), nCheck + nCnt);
		auto iterAn = std::find(m_vAnGanged.begin(), m_vAnGanged.end(), nCheck + nCnt - 3 );
		if (iterBig != m_vAnGanged.end() || iterAn != m_vAnGanged.end())
		{
			if (m_pRoom->getJianZhang() != (nCheck + nCnt))
			{
				nHuCnt += 10;
				if (m_nCurPlayerIdx == nCnt)
				{
					nHuCnt += 10;
				}
				++nQJKCnt;
				continue;
			}
		}

		// secher MingGang ;
		auto iterMingBig = std::find(m_vGanged.begin(), m_vGanged.end(), nCheck + nCnt);
		auto iterMing = std::find(m_vGanged.begin(), m_vGanged.end(), nCheck + nCnt - 3);
		if ( iterMingBig != m_vGanged.end() || iterMing != m_vGanged.end() )
		{
			auto iterOther = std::find( m_vPengedOtherHeiJQK.begin(),m_vPengedOtherHeiJQK.end(),nCheck + nCnt );
			if (iterOther == m_vPengedOtherHeiJQK.end() )
			{
				nHuCnt += 10;
				if (m_nCurPlayerIdx == nCnt)
				{
					nHuCnt += 10;
				}
				++nQJKCnt;
				continue;
			}
		}

		// secher hold card 
		auto iterHold = std::find( m_vCards[eCT_Jian].begin(), m_vCards[eCT_Jian].end(), nCheck + nCnt);
		if (iterHold != m_vCards[eCT_Jian].end() )
		{
			nHuCnt += 10;
			if (m_nCurPlayerIdx == nCnt)
			{
				nHuCnt += 10;
			}
			++nQJKCnt;
			continue;
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

}

uint16_t JJQEPlayerCard::getGangHuCnt()
{

}

uint16_t JJQEPlayerCard::getHoldAnKeCnt()
{

}

uint16_t JJQEPlayerCard::getHoldWenQianCnt()
{

}

uint16_t JJQEPlayerCard::getFlyUpHuCnt()
{

}

uint16_t JJQEPlayerCard::getHuaHuCnt()
{
	// jie hun ;
}