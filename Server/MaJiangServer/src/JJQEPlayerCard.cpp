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
	MJPlayerCard::reset();
	m_vBuHuaCard.clear();
	m_vFlyupCard.clear();
	m_vJianPeng.clear();

	// hu info 
	//m_isHu = false;
	m_nInvokeHuIdx = -1 ;
	m_nHuCard = 0;
	//m_vHuTypes.clear();
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
	Json::Value jsAnPai, jsChuPai, jsAngangedPai, jsBuGang, jsHuaPai, jsFlyUp, jsJianPeng, jsPeng;

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
	toJs(m_vJianPeng, jsJianPeng); toJs(m_vPenged,jsPeng);
	jsPeerCards["anPai"] = jsAnPai; jsPeerCards["chuPai"] = jsChuPai; jsPeerCards["anGangPai"] = jsAngangedPai; jsPeerCards["buGang"] = jsBuGang; jsPeerCards["huaPai"] = jsHuaPai;
	jsPeerCards["flyUp"] = jsFlyUp;
	jsPeerCards["jianPeng"] = jsJianPeng;
	jsPeerCards["peng"] = jsPeng;
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
			if ( card_Type(nCanFlyCard) == eCT_Jian && card_Value(nCanFlyCard) > 3 )
			{
				nCanFlyCard -= 3;
			}

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
	updateHuCntToClient();
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
	updateHuCntToClient();
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

bool JJQEPlayerCard::onDoHu( uint8_t nCard, uint8_t nInvokerIdx )
{
	m_nInvokeHuIdx = nInvokerIdx;
	//m_isHu = true;
	m_nHuCard = nCard;
	return true;
}

void JJQEPlayerCard::getHuInfo(uint8_t& nInvokeIdx, std::vector<uint8_t>& vHuTypes)
{
	nInvokeIdx = m_nInvokeHuIdx;
	getHuFanxingTypes(m_nHuCard,getIsZiMo(),vHuTypes);
}

uint32_t JJQEPlayerCard::getAllHuCnt( bool isHu, bool isHuZiMo, uint8_t nHuCard )
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

	auto nRet = jsHua.size() > 0;
	if (nRet)
	{
		updateHuCntToClient();
	}
	return nRet;
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
		auto Cnt = std::count_if(m_vCards[nJianType].begin(), m_vCards[nJianType].end(), [nJianZhang](uint8_t nCard) 
		{
			if (nCard == nJianZhang)
			{
				return true;
			}

			if (card_Type(nJianZhang) == eCT_Jian)
			{
				int8_t nRet = (int8_t)(card_Value(nJianZhang)) - (int8_t)(card_Value(nCard));
				return abs(nRet) == 3 ;
			}
			return false;
		}
		);
		if ( Cnt == 3 )
		{
			vGangCards.push_back(nJianZhang);
		}
	}
	
	// check wu xi qi fen 
	auto nXiXiCnt = std::count_if(m_vCards[eCT_Hua].begin(), m_vCards[eCT_Hua].end(), [](uint8_t& nCard) { auto nValue = card_Value(nCard); return nValue <= 2; });
	bool isJianXiXi = (eCT_Hua == nJianType && nJianValue <= 2);
	if (nXiXiCnt == (isJianXiXi ? 4 : 5))
	{
		vGangCards.push_back(m_vCards[eCT_Hua].front());
	}

	// check jian 
	if (m_vCards[eCT_Jian].size() > 3)
	{
		uint8_t nJ = make_Card_Num(eCT_Jian, 1);
		for (auto nValue = 0; nValue < 3; ++nValue)
		{
			auto nV = nJ + nValue;
			if ( canAnGangWithCard(nV) )
			{
				vGangCards.push_back(nV);
			}
		}
	}
	return vGangCards.empty() == false;
}

bool JJQEPlayerCard::getHoldCardThatCanBuGang(VEC_CARD& vGangCards)
{
	auto nRet = MJPlayerCard::getHoldCardThatCanBuGang(vGangCards);
	if (m_vJianPeng.empty() || m_vCards[eCT_Jian].empty() )
	{
		return nRet;
	}

	 
	VEC_CARD vJianPeng = m_vJianPeng;
	for (auto& ref : vJianPeng)
	{
		if (card_Value(ref) > 3)
		{
			ref -= 3;
		}
	}

	for (auto& ref : m_vCards[eCT_Jian] )
	{
		auto nCheck = ref;
		if (card_Value(nCheck) > 3)
		{
			nCheck -= 3;
		}

		auto iter = std::find(vJianPeng.begin(), vJianPeng.end(), nCheck);
		if (iter != vJianPeng.end())
		{
			vGangCards.push_back(ref);
		}
	}

	return !vGangCards.empty();
}

bool JJQEPlayerCard::canMingGangWithCard(uint8_t nCard)
{
	auto nCardType = card_Type(nCard);
	if (eCT_Jian == nCardType)
	{
		auto Cnt = std::count_if(m_vCards[eCT_Jian].begin(), m_vCards[eCT_Jian].end(), [nCard](uint8_t nCheckCard) { int16_t nRet = abs((int16_t)nCheckCard - (int16_t)nCard); return (0 == nRet || 3 == nRet);  });
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
		auto nXiXiCnt = std::count_if(m_vCards[eCT_Hua].begin(), m_vCards[eCT_Hua].end(), [](uint8_t& nCard) { auto nValue = card_Value(nCard); return nValue <= 2; });
		return (nXiXiCnt == (isJianXiXi ? 4 : 5));
	}
	else if ( eCT_Jian == nCardType )
	{
		if ( m_vCards[eCT_Jian].size() < 3 )
		{
			return false;
		}

		auto Cnt = std::count_if(m_vCards[eCT_Jian].begin(), m_vCards[eCT_Jian].end(), [nCard](uint8_t nCheckCard) { int16_t nRet = abs((int16_t)nCheckCard - (int16_t)nCard); return (0 == nRet || 3 == nRet);  });
		int16_t nRet = abs((int16_t)nJianZhang - (int16_t)nCard);
		bool isJianJIanZhang = (0 == nRet || 3 == nRet);
		return Cnt == (( isJianJIanZhang) ? 3 : 4 );
	}
	else if (nCard == nJianZhang && nCardType != eCT_Hua && eCT_Jian != nCardType)   // card is jiang zhang 
	{
		auto Cnt = std::count(m_vCards[nCardType].begin(), m_vCards[nCardType].end(), nCard);
		return Cnt == 3;
	}

	return MJPlayerCard::canAnGangWithCard(nCard);
}

bool JJQEPlayerCard::canPengWithCard(uint8_t nCard)
{
	auto nCardType = card_Type(nCard);
	if ( eCT_Jian == nCardType )
	{
		auto Cnt = std::count_if(m_vCards[eCT_Jian].begin(), m_vCards[eCT_Jian].end(), [nCard](uint8_t nCheckCard) { int16_t nRet = abs((int16_t)nCheckCard - (int16_t)nCard); return (0 == nRet || 3 == nRet);  });
		return Cnt >= 2;
	}
	return MJPlayerCard::canPengWithCard(nCard);
}

bool JJQEPlayerCard::canBuGangWithCard(uint8_t nCard)
{
	auto nCardType = card_Type(nCard);
	if (eCT_Jian == nCardType)
	{
		auto Cnt = std::count_if(m_vJianPeng.begin(), m_vJianPeng.end(), [nCard](uint8_t nCheckCard) { int16_t nRet = abs((int16_t)nCheckCard - (int16_t)nCard); return (0 == nRet || 3 == nRet);  });
		return Cnt >= 1;
	}
	return MJPlayerCard::canBuGangWithCard(nCard);
}

bool JJQEPlayerCard::onPeng( uint8_t nCard )
{
	if ( card_Type(nCard) != eCT_Jian )
	{
		auto nRet = MJPlayerCard::onPeng(nCard);
		updateHuCntToClient();
		return nRet;
	}

	auto nValue = card_Value(nCard);
	bool bOtherBlack = true;
	if ( nValue <= 3 )  // other is black ;
	{
		m_vJianPeng.push_back(nCard);
		bOtherBlack = false;
	}

	// remove self red two 
	auto& vCard = m_vCards[eCT_Jian];
	auto nEraseCnt = 2;
	while (nEraseCnt-- > 0)
	{
		if ( bOtherBlack )
		{
			auto iter = std::find(vCard.begin(), vCard.end(), nCard - 3);
			vCard.erase(iter);
			// add to jian peng 
			m_vJianPeng.push_back(nCard - 3);
		}
		else
		{
			// remove red 
			auto iter = std::find(vCard.begin(), vCard.end(), nCard);
			if ( iter != vCard.end() )
			{
				vCard.erase(iter);
				// add to jian peng 
				m_vJianPeng.push_back(nCard);
				continue;
			}

			// remove self black ; no red 
			iter = std::find(vCard.begin(), vCard.end(), nCard + 3);
			if (iter != vCard.end())
			{
				vCard.erase(iter);
				// add to jian peng 
				m_vJianPeng.push_back(nCard + 3);
				continue;
			}
			LOGFMTE("you need peng ,but you do not have ta least 2 card = %u",nCard);
		}
	}
	updateHuCntToClient();
	return true;
}

bool JJQEPlayerCard::onMingGang(uint8_t nCard, uint8_t nGangGetCard)
{
	auto nCardType = card_Type(nCard);
	if ( eCT_Jian == nCardType )
	{
		do
		{
			auto iter = std::find_if(m_vCards[eCT_Jian].begin(),m_vCards[eCT_Jian].end(), [nCard](uint8_t nCheckCard) { int16_t nRet = abs((int16_t)nCheckCard - (int16_t)nCard); return (0 == nRet || 3 == nRet);  });
			if (iter == m_vCards[eCT_Jian].end())
			{
				break;
			}
			m_vCards[eCT_Jian].erase(iter);

		} while (1);

		if ( card_Value(nCard) > 3 )  // other' card is black 
		{
			m_vGanged.push_back( nCard - 3);  // self should be red ;
		}
		else  // other is red 
		{
			m_vGanged.push_back(nCard + 3);  // sefl should be black ;
		}

		onMoCard(nGangGetCard);
		updateHuCntToClient();
		return true;
	}
	auto isRet = MJPlayerCard::onMingGang(nCard, nGangGetCard);
	updateHuCntToClient();
	return isRet;
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
		LOGFMTE("an gang delte cnt = %u", nDeleteCnt);
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
	updateHuCntToClient();
	return true;
}

bool JJQEPlayerCard::onBuGang(uint8_t nCard, uint8_t nGangGetCard)
{
	auto nCardType = card_Type(nCard);
	if (eCT_Jian == nCardType)
	{
		// delete jian peng 
		uint8_t nSelfBlack = 0;
		do
		{
			auto iter = std::find_if(m_vJianPeng.begin(), m_vJianPeng.end(), [nCard](uint8_t nCheckCard) { int16_t nRet = abs((int16_t)nCheckCard - (int16_t)nCard); return (0 == nRet || 3 == nRet);  });
			if (iter == m_vJianPeng.end())
			{
				break;
			}

			if (card_Value(*iter) > 3)
			{
				nSelfBlack = *iter;
			}

			m_vJianPeng.erase(iter);

		} while (1);

		// delete hold jian pai 
		{
			auto iter = std::find_if(m_vCards[eCT_Jian].begin(), m_vCards[eCT_Jian].end(), [nCard](uint8_t nCheckCard) { int16_t nRet = abs((int16_t)nCheckCard - (int16_t)nCard); return (0 == nRet || 3 == nRet);  });
			if (iter == m_vCards[eCT_Jian].end())
			{
				LOGFMTE("why hold card do not have card = %u can not bu gang",nCard);
			}
			else
			{
				if (card_Value(*iter) > 3)
				{
					nSelfBlack = *iter;
				}

				m_vCards[eCT_Jian].erase(iter);
			}
		}
		// do bu gang 
		if (nSelfBlack != 0)
		{
			m_vGanged.push_back( nSelfBlack );
		}
		else
		{
			m_vGanged.push_back(nCard);
		}
		
		onMoCard(nGangGetCard);
		updateHuCntToClient();
		return true;
	}
	auto bRet = MJPlayerCard::onBuGang(nCard, nGangGetCard);
	updateHuCntToClient();
	return bRet;
}

uint8_t JJQEPlayerCard::getBlackJQKHuCnt( bool bSkipHold )
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

		// secher chu 
		if ( !isFind && m_vChuedCard.size() > 0 )
		{
			auto iter = std::find(m_vChuedCard.begin(), m_vChuedCard.end(), nCheck + nCnt);
			if (iter != m_vChuedCard.end())
			{
				isFind = true;
			}
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
			if (iterMingBig != m_vGanged.end())
			{
				isFind = true;
			}
		}

		if ( !isFind && !bSkipHold && m_vCards[eCT_Jian].size() > 0  && ( (nCheck + nCnt) != m_nHuCard ) )
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
			if ( m_nCurPlayerIdx == ( 2 - nCnt + m_pRoom->getBankerIdx() ) % m_pRoom->getSeatCnt() )
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
	return nHuCnt;
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
				if ( nCheck == m_pRoom->getJianZhang() || ( nCheck + 3 ) == m_pRoom->getJianZhang() )
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

uint16_t JJQEPlayerCard::getHoldAnKeCnt( bool isHu, bool isHuZiMo)
{
	bool isNeedAdd = (isHu && (isHuZiMo == false));
	if (isNeedAdd)
	{
		addCardToVecAsc(m_vCards[card_Type(m_nHuCard)], m_nHuCard);
	}
	bool isThisHu = isHu;
	uint16_t nHoldHuCnt = 0;
	uint8_t nJianZhang = m_pRoom->getJianZhang();
	if ( 0 != nJianZhang && card_Type(nJianZhang) == eCT_Jian && card_Value(nJianZhang) > 3)
	{
		nJianZhang -= 3;
	}

	auto nHuCard = 0;
	if (isThisHu)
	{
		nHuCard = m_nHuCard;
		if (card_Type(nHuCard) == eCT_Jian && card_Value(nHuCard) > 3)
		{
			nHuCard -= 3;
		}
	}

	// back up jian 
	VEC_CARD vJian = m_vCards[eCT_Jian];
	for (auto& ref : m_vCards[eCT_Jian])
	{
		if (card_Value(ref) > 3)
		{
			ref -= 3;  // must sort jian 
		}
	}
	std::sort(m_vCards[eCT_Jian].begin(), m_vCards[eCT_Jian].end());
	// back up tong 
	VEC_CARD vTong = m_vCards[eCT_Tong];
	auto nWenQianCnt = getHoldWenQianCnt(isHu, isNeedAdd );
	while (nWenQianCnt--)
	{
		m_vCards[eCT_Tong].erase(std::find(m_vCards[eCT_Tong].begin(), m_vCards[eCT_Tong].end(), make_Card_Num(eCT_Tong, 1)));
		m_vCards[eCT_Tong].erase(std::find(m_vCards[eCT_Tong].begin(), m_vCards[eCT_Tong].end(), make_Card_Num(eCT_Tong, 2)));
		m_vCards[eCT_Tong].erase(std::find(m_vCards[eCT_Tong].begin(), m_vCards[eCT_Tong].end(), make_Card_Num(eCT_Tong, 3)));
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
				if ( (!isHuZiMo) && vCards[nIdx] == nHuCard) // suan ming gang 
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
				if (isThisHu && (!isHuZiMo) && vCards[nIdx] == nHuCard)  // when hu other card , should regart as peng ;
				{
					nThisHu = 1;
				}

				if ( isThisHu && nJianZhang == vCards[nIdx]) // should regarts as gang ;
				{
					nThisHu = 6;
					if ((!isHuZiMo) && vCards[nIdx] == nHuCard)
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
	// restor tong 
	m_vCards[eCT_Tong] = vTong;
	LOGFMTD("room id = %u , player idx = %u hold hua cnt = %u ", m_pRoom->getRoomID(),m_nCurPlayerIdx ,nHoldHuCnt );
	if (isNeedAdd)
	{
		auto iter = std::find(m_vCards[card_Type(m_nHuCard)].begin(), m_vCards[card_Type(m_nHuCard)].end(),m_nHuCard );
		m_vCards[card_Type(m_nHuCard)].erase(iter);
	}
	return nHoldHuCnt;
}

uint16_t JJQEPlayerCard::getHoldWenQianCnt( bool isHu, bool AlreadyAdd )
{
	if ( false == isHu )
	{
		return 0;
	}

	bool isNeedAdd = false == AlreadyAdd && ( getIsZiMo() == false) && (card_Type(m_nHuCard) == eCT_Tong) ;
	if (isNeedAdd )
	{
		addCardToVecAsc(m_vCards[eCT_Tong], m_nHuCard);
	}

	auto pRemoveHuCard = [this]()
	{
		auto iter = std::find(m_vCards[eCT_Tong].begin(), m_vCards[eCT_Tong].end(), m_nHuCard);
		m_vCards[eCT_Tong].erase(iter);
	};

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
			if (isNeedAdd)
			{
				pRemoveHuCard();
			}
			return 0;
		}
	}
	LOGFMTD( "room id = %u idx = %u wenqian cnt = %u",m_pRoom->getRoomID(),m_nCurPlayerIdx , nWenQianCnt );
	// check valid wen qian 
	if ( m_vCards[eCT_Tong].size() % 3 == 2 )  // may a , 2 , 3 regart as jiang 
	{
		auto vTong = m_vCards[eCT_Tong];
		auto nRealWenQianCnt = 0;
		while ( nWenQianCnt-- )
		{
			// remove 1 , 2 , 3 ;
			//auto iterErase = std::remove_if(vTong.begin(), vTong.end(), [](uint8_t nCard) { return (nCard >= make_Card_Num(eCT_Tong, 1) && nCard <= make_Card_Num(eCT_Tong, 3)); });
			vTong.erase(std::find(vTong.begin(), vTong.end(), make_Card_Num(eCT_Tong, 1)));
			vTong.erase(std::find(vTong.begin(), vTong.end(), make_Card_Num(eCT_Tong, 2)));
			vTong.erase(std::find(vTong.begin(), vTong.end(), make_Card_Num(eCT_Tong, 3)));

			SET_NOT_SHUN vNotShun;
			getNotShuns(vTong, vNotShun, false);

			bool isOk = false;
			auto iter = vNotShun.begin();
			for (; iter != vNotShun.end(); ++iter)
			{
				if ((*iter).vCards.size() == 2 && (*iter).vCards[0] == (*iter).vCards[1])
				{
					isOk = true;
					break;
				}
			}

			if (isOk == false)
			{
				break;
			}
			++nRealWenQianCnt;
		}
		nWenQianCnt = nRealWenQianCnt;
	}

	if (isNeedAdd)
	{
		pRemoveHuCard();
	}
	return nWenQianCnt;
}

uint16_t JJQEPlayerCard::getHoldWenQianHuCnt(bool isHu)
{
	auto nWenQianCnt = getHoldWenQianCnt(isHu);
	if (nWenQianCnt == 0)
	{
		return 0;
	}

	uint16_t nHucnt = 0;
	switch (nWenQianCnt)
	{
	case 1:
		nHucnt = 20;
		break;
	case 2:
		nHucnt = 50;
		break;
	case 3:
		nHucnt = 100;
		break;
	case 4:
		nHucnt = 200;
		break;
	default:
		LOGFMTD("invalid wen qian cnt = %u roomid = %u idx = %u", nWenQianCnt, m_pRoom->getRoomID(), m_nCurPlayerIdx);
		return 0;
	}
	return nHucnt;
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
	if (nSun >= 1)
	{
		nHuCnt += 50;
	}

	if ( nMoon >= 1 )
	{
		nHuCnt += 50;
	}
	
	if (nSun + nMoon == 2)
	{
		nHuCnt += 50;
	}
	
	if ( card_Type(m_pRoom->getJianZhang()) == eCT_Hua )
	{
		nHuCnt *= 2;
	}

	LOGFMTD( "room id = %u , player idx = %u , huaHu = %u",m_pRoom->getRoomID(),m_nCurPlayerIdx,nHuCnt );
	return nHuCnt;
}

// check hu xiang guan
bool JJQEPlayerCard::checkQiongHen( bool isHu )
{
	if ( m_vBuHuaCard.size() > 0 )  // have hua 
	{
		return false;
	}

	if (getBlackJQKHuCnt(false) > 0)  // have balc j q k 
	{
		return false;
	}

	// check qi fei contain hua an qiong hen 
	auto iter = std::find_if(m_vFlyupCard.begin(), m_vFlyupCard.end(), [this](uint8_t nCard) 
	{
		auto nType = card_Type(nCard);
		if (eCT_Hua == nType)
		{
			return true;
		}

		if ( eCT_Jian == nType)
		{
			auto nJiangType = card_Type(m_pRoom->getJianZhang());
			auto nJiangValue = card_Value(m_pRoom->getJianZhang());
			if ( eCT_Jian != nJiangType || nJiangValue <= 3 )  // jiang is not j q k , then must 4 card fly up , if jiang is jiang and value is not black , then hold must have black one 
			{
				return true;
			}
		}
		
		return false;

	} );

	if (iter != m_vFlyupCard.end()) // have black j q k
	{
		return false;
	}
	return true;
}

bool JJQEPlayerCard::checkQiongQiongHen( bool isHu )
{
	if (checkQiongHen(isHu))
	{
		auto nJiangType = card_Type(m_pRoom->getJianZhang());
		if (eCT_Hua == nJiangType)
		{
			return true;
		}
	}
	return false;
}

bool JJQEPlayerCard::check3Red( bool bSkipHold, bool isHu )
{
	bool isNeedAdd = (isHu && (!bSkipHold) && (card_Type(m_nHuCard) == eCT_Jian) );
	if (isNeedAdd)
	{
		addCardToVecAsc(m_vCards[eCT_Jian],m_nHuCard);
	}

	auto pRemoveHuCard = [this]()
	{
		auto iter = std::find(m_vCards[eCT_Jian].begin(), m_vCards[eCT_Jian].end(),m_nHuCard );
		m_vCards[eCT_Jian].erase(iter);
	};
	uint8_t vJqkCnt[3] = { 0 };
	for (uint8_t nValue = 1; nValue <= 3; ++nValue)
	{
		auto pfnJian = [nValue](uint8_t nCheck)
		{
			if (eCT_Jian != card_Type(nCheck))
			{
				return false;
			}

			auto nCheckValue = card_Value(nCheck);
			if (nCheckValue == nValue || (nCheckValue == nValue + 3))
			{
				return true;
			}
			return false;
		};
		// check peng 
		auto iterP = std::find_if(m_vJianPeng.begin(),m_vJianPeng.end(),pfnJian);
		if ( iterP != m_vJianPeng.end())
		{
			vJqkCnt[nValue - 1] = 3;
			continue;
		}
		// check ming gang ,
		auto iterM = std::find_if(m_vGanged.begin(), m_vGanged.end(), pfnJian);
		if (iterM != m_vGanged.end())
		{
			vJqkCnt[nValue - 1] = 3;
			continue;
		}
		// check an gang ;
		auto iterA = std::find_if(m_vAnGanged.begin(), m_vAnGanged.end(), pfnJian);
		if (iterA != m_vAnGanged.end())
		{
			vJqkCnt[nValue - 1] = 3;
			continue;
		}
		// check flyup 
		auto iterF = std::find_if(m_vFlyupCard.begin(), m_vFlyupCard.end(), pfnJian);
		if (iterF != m_vFlyupCard.end())
		{
			vJqkCnt[nValue - 1] = 3;
			continue;
		}
		// check hold card ;
		if ( bSkipHold == false )
		{
			vJqkCnt[nValue - 1] = std::count_if(m_vCards[eCT_Jian].begin(), m_vCards[eCT_Jian].end(), pfnJian);
			if (vJqkCnt[nValue - 1] < 2)
			{
				if (isNeedAdd)
				{
					pRemoveHuCard();
				}
				return false;
			}
		}


		if ( vJqkCnt[nValue - 1] < 3 && ( false == isHu))  // if you not hu , j q k must have 3 ;
		{
			if (isNeedAdd)
			{
				pRemoveHuCard();
			}
			return false;
		}
	}

	// not hu player , already checked in above loop ;
	if (isHu)
	{
		uint8_t nLackCnt = 0;;
		for (auto& nCnt : vJqkCnt)
		{
			if ( nCnt < 3 )
			{
				++nLackCnt;
			}
		}

		if ( nLackCnt > 1 )
		{
			if (isNeedAdd)
			{
				pRemoveHuCard();
			}
			return false;
		}
	}
	if (isNeedAdd)
	{
		pRemoveHuCard();
	}
	return true;
}

bool JJQEPlayerCard::checkDuiDuiHu()
{
	bool bFindJiang = false;
	VEC_CARD vAllCard;
	getHoldCard(vAllCard);
	// erse wen qian , 
	auto nACnt = std::count(vAllCard.begin(), vAllCard.end(), make_Card_Num(eCT_Tong,1)) ;
	auto n2Cnt = std::count(vAllCard.begin(), vAllCard.end(), make_Card_Num(eCT_Tong, 2)) ;
	auto n3Cnt = std::count(vAllCard.begin(), vAllCard.end(), make_Card_Num(eCT_Tong, 3));
	std::vector<int32_t> vMayWenCnt = { nACnt, n2Cnt , n3Cnt };
	std::sort(vMayWenCnt.begin(), vMayWenCnt.end());
	int8_t nSmall = 0;
	if (vMayWenCnt[0] == vMayWenCnt[2])
	{
		nSmall = vMayWenCnt[0];
	}
	else if (vMayWenCnt[0] == 1 || vMayWenCnt[2] == 4)
	{
		nSmall = 1;
	}
 
	while ( nSmall-- )
	{
		for (uint8_t nValue = 1; nValue <= 3; ++nValue)
		{
			auto iter = std::find(vAllCard.begin(), vAllCard.end(), make_Card_Num(eCT_Tong,nValue));
			if ( iter == vAllCard.end() )
			{
				LOGFMTE("why do not have this card ?");
				continue;
			}
			vAllCard.erase(iter);
		}
	}
	// modify jian card, j q k 
	for (uint8_t nV = 4; nV <= 6; ++nV)
	{
		auto iter = std::find(vAllCard.begin(),vAllCard.end(),make_Card_Num(eCT_Jian,nV));
		if (iter != vAllCard.end())
		{
			(*iter) -= 3;
		}
	}

	// do sort 
	std::sort(vAllCard.begin(), vAllCard.end());
	for (uint8_t nIdx = 0; (nIdx + 1) < vAllCard.size(); )
	{
		auto nThirdIdx = nIdx + 2;
		if (nThirdIdx < vAllCard.size() && vAllCard[nIdx] == vAllCard[nThirdIdx])
		{
			nIdx += 3;
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

	return true;
}

bool JJQEPlayerCard::checkQingErHu()
{
	if (m_vPenged.size() > 0 )
	{
		return false;
	}
	return checkOld13Hu();
}

bool JJQEPlayerCard::checkOld13Hu()
{
	if ( m_vCards[eCT_Jian].empty() == false && m_vCards[eCT_Jian].size() != 2 )
	{
		return false;
	}

	if ( m_vPenged.size() > 1 || m_vJianPeng.size() > 0 || m_vAnGanged.size() > 0 || m_vGanged.size() > 0)
	{
		return false;
	}

	if ( 1 == m_vPenged.size() && m_pRoom->isCardJianPai(m_vPenged.front()))
	{
		return false;
	}

	auto iter = std::find_if(m_vFlyupCard.begin(), m_vFlyupCard.end(), [](uint8_t nCheckCard) { return card_Type(nCheckCard) != eCT_Hua; });
	if (iter != m_vFlyupCard.end())
	{
		return false;
	}

	// hold can not have ke zi 
	auto pfCheckShun = [](VEC_CARD& vCheck)
	{
		while (vCheck.empty() == false)
		{
			auto nCurValue = vCheck.front();
			auto iterA = vCheck.begin();
			auto iterB = std::find(vCheck.begin(), vCheck.end(), nCurValue + 1);
			auto iterC = std::find(vCheck.begin(), vCheck.end(), nCurValue + 2);
			if (iterA != vCheck.end() && iterB != vCheck.end() && iterC != vCheck.end())
			{
				*iterA = *iterB = *iterC = 0;
			}
			else
			{
				return false;
			}

			while (true)
			{
				auto iter = std::find(vCheck.begin(), vCheck.end(), 0);
				if (iter != vCheck.end())
				{
					vCheck.erase(iter);
					continue;
				}
				break;
			}
		}
		return true;
	};

	uint8_t nJiangType = eCT_Max;
	for (auto& vCards : m_vCards)
	{
		if ( vCards.empty() )
		{
			continue;
		}

		auto nCardType = card_Type(vCards.front());
		if ( nCardType == eCT_Jian )
		{
			continue;
		}

		if ( (vCards.size() % 3 == 2 ) )
		{
			nJiangType = nCardType;
			continue;
		}
		
		VEC_CARD vCheck = vCards;
		if (!pfCheckShun(vCheck))
		{
			return false;
		}
 
	}

	if ( eCT_Max != nJiangType && eCT_Jian != nJiangType )
	{
		// find may be jiang ;
		auto vCard = m_vCards[nJiangType];
		std::set<uint8_t> vJiang;
		for ( uint8_t nIdx = 0; (nIdx + 1) < vCard.size();  )
		{
			if ( vCard[nIdx] == vCard[nIdx + 1] )
			{
				vJiang.insert(vCard[nIdx]);
				nIdx += 2;
				continue;
			}
			++nIdx;
		}

		for ( auto& ref : vJiang )
		{
			auto vCheck = vCard;
			auto iter = std::find(vCheck.begin(),vCheck.end(),ref);
			vCheck.erase(iter);
			iter = std::find(vCheck.begin(), vCheck.end(), ref);
			vCheck.erase(iter);
			if (pfCheckShun(vCheck))
			{
				return true;
			}
		}
	}
	return true;
}

bool JJQEPlayerCard::check13Hu()
{
	if (m_vCards[eCT_Jian].empty() == false && m_vCards[eCT_Jian].size() != 2)
	{
		return false;
	}

	if (m_vPenged.size() > 1 || m_vJianPeng.size() > 0 || m_vAnGanged.size() > 0 || m_vGanged.size() > 0)
	{
		return false;
	}

	if (1 == m_vPenged.size() && m_pRoom->isCardJianPai(m_vPenged.front()))
	{
		return false;
	}

	auto iter = std::find_if(m_vFlyupCard.begin(), m_vFlyupCard.end(), [](uint8_t nCheckCard) { return card_Type(nCheckCard) != eCT_Hua; });
	if (iter != m_vFlyupCard.end())
	{
		return false;
	}

	VEC_CARD vHold;
	getHoldCard(vHold);
	std::sort(vHold.begin(), vHold.end());
	for ( uint8_t nIdx = 0; ( nIdx + 2 ) < vHold.size(); ++nIdx )
	{
		if (vHold[nIdx] == vHold[nIdx + 2])
		{
			return false;
		}
	}

	if (m_vPenged.size() == 1)
	{
		if ( std::count(vHold.begin(), vHold.end(), m_vPenged[0]) > 0 )
		{
			return false;
		}
	}

	return checkOld13Hu();
}

uint16_t JJQEPlayerCard::getMingPaiHuaCnt()
{
	if (!m_pRoom)
	{
		LOGFMTE("why room is nullptr get ming pai hu cnt");
		return 0;
	}
	// is start game ?
	uint8_t nJianZhang = m_pRoom->getJianZhang();
	if ((uint8_t)-1 == nJianZhang || 0 == nJianZhang)
	{
		return 0;
	}

	auto nHuaCnt = 0;
	nHuaCnt += getBlackJQKHuCnt(true);
	nHuaCnt += getFlyUpHuCnt();
	nHuaCnt += getHuaHuCnt();
	nHuaCnt += getPengHuCnt();
	nHuaCnt += getGangHuCnt();
	if ( check3Red(true,false) ) // do this for client ;  bSkipHold = true , means for client ;
	{
		nHuaCnt *= 2;
	}

	if ( nHuaCnt > m_pRoom->getTopHuLimit() )
	{
		nHuaCnt = m_pRoom->getTopHuLimit();
	}
	return nHuaCnt;
}

bool JJQEPlayerCard::isHoldCardCanHu()
{
	VEC_CARD vBanckUpJian = m_vCards[eCT_Jian];
	for ( auto& ref : m_vCards[eCT_Jian] )
	{
		if (card_Value(ref) > 3)
		{
			ref -= 3;
		}
	}
	std::sort(m_vCards[eCT_Jian].begin(), m_vCards[eCT_Jian].end());

	auto b = MJPlayerCard::isHoldCardCanHu();
	m_vCards[eCT_Jian] = vBanckUpJian;
	if (!b)
	{
		return false;
	}
	auto nHuCardBack = m_nHuCard;
	auto nInvok = m_nInvokeHuIdx;
	onDoHu(getNewestFetchedCard(), m_nCurPlayerIdx);
	bool b3Red = false;
	auto nHuCnt = getFinalHuCnt(true, b3Red);
	m_nHuCard = nHuCardBack;
	m_nInvokeHuIdx = nInvok;
	return nHuCnt >= m_pRoom->getQiHuNeed();
}

bool JJQEPlayerCard::canHuWitCard(uint8_t nCard)
{
	auto eType = card_Type(nCard);
	if (eType >= eCT_Max)
	{
		LOGFMTE("parse card type error ,canHuWitCard have this card = %u", nCard);
		return false;
	}

	addCardToVecAsc(m_vCards[eType], nCard);

	// reset jian pai 
	VEC_CARD vBanckUpJian = m_vCards[eCT_Jian];
	for (auto& ref : m_vCards[eCT_Jian])
	{
		if (card_Value(ref) > 3)
		{
			ref -= 3;
		}
	}
	std::sort(m_vCards[eCT_Jian].begin(), m_vCards[eCT_Jian].end());

	auto bSelfHu = MJPlayerCard::isHoldCardCanHu();
	m_vCards[eCT_Jian] = vBanckUpJian;  // restore jian pai 

	auto iter = std::find(m_vCards[eType].begin(), m_vCards[eType].end(), nCard);
	m_vCards[eType].erase(iter);
	if (bSelfHu == false)
	{
		return false;
	}
	auto nHuCardBack = m_nHuCard;
	auto nInvok = m_nInvokeHuIdx;
	onDoHu(nCard, (m_nCurPlayerIdx + 1 ) % m_pRoom->getSeatCnt() );
	bool b3Red = false;
	auto nHuCnt = getFinalHuCnt(true, b3Red);
	m_nHuCard = nHuCardBack;
	m_nInvokeHuIdx = nInvok;
	return nHuCnt >= m_pRoom->getQiHuNeed() ;
}

bool JJQEPlayerCard::checkBianZhi()
{
	auto nhuCardType = card_Type(m_nHuCard);
	if ( nhuCardType == eCT_Jian)
	{
		return false;
	}

	// bian zhi 
	auto nValue = card_Value(m_nHuCard);
	if (nValue != 7 && 3 != nValue)
	{
		return false;
	}

	auto nPre = m_nHuCard;
	auto nNext = m_nHuCard;
	if (nValue == 7)
	{
		nPre = m_nHuCard + 1;
		nNext = m_nHuCard + 2;
	}
	else if (3 == nValue)
	{
		nPre = m_nHuCard - 1;
		nNext = m_nHuCard - 2;
	}
	else
	{
		return false;
	}

	if (isHaveCard(nPre) == false || false == isHaveCard(nNext))
	{
		return false;
	}

	VEC_CARD vBackUp = m_vCards[nhuCardType];
	std::vector<uint8_t> vErase = { m_nHuCard,nPre,nNext };
	for (auto& ref : vErase)
	{
		auto iter = std::find(m_vCards[nhuCardType].begin(), m_vCards[nhuCardType].end(), ref);
		m_vCards[nhuCardType].erase(iter);
	}

	auto nRet = MJPlayerCard::isHoldCardCanHu();
	// resotre
	m_vCards[nhuCardType] = vBackUp;
	return nRet;
}

bool JJQEPlayerCard::checkDuDiao()
{
	// start new 
	auto nHuCardType = card_Type(m_nHuCard);
	VEC_CARD vCard = m_vCards[nHuCardType];
	auto nCnt = std::count(vCard.begin(),vCard.end(),m_nHuCard);
	if ( nHuCardType == eCT_Jian )
	{
		nCnt = std::count_if(vCard.begin(), vCard.end(), [this](uint8_t nCard) { int8_t nRet = (int8_t)(card_Value(m_nHuCard)) - (int8_t)(card_Value(nCard));
		return abs(nRet) == 3 || 0 == nRet;  } );
	}

	if (nCnt < 2)
	{
		return false;
	}

	nCnt = 2;
	while (nCnt--)
	{
		auto iter = std::find(vCard.begin(), vCard.end(), m_nHuCard);
		if ( iter == vCard.end() && nHuCardType == eCT_Jian )
		{
			iter = std::find_if(vCard.begin(), vCard.end(), [this](uint8_t nCard) { int8_t nRet = (int8_t)(card_Value(m_nHuCard)) - (int8_t)(card_Value(nCard));
			return abs(nRet) == 3 || 0 == nRet;;  });
		}
		vCard.erase(iter);
	}

	SET_NOT_SHUN vNotShun;
	getNotShuns(vCard, vNotShun, eCT_Feng == nHuCardType || eCT_Jian == nHuCardType );
	return vNotShun.empty() == true;
}

bool JJQEPlayerCard::checkKaZhang()
{
	auto nhuCardType = card_Type(m_nHuCard);
	if (nhuCardType == eCT_Jian)
	{
		return false;
	}
	// ka zhang ,
	uint8_t nPre = m_nHuCard - 1;
	uint8_t nNext = m_nHuCard + 1;
	if (isHaveCard(m_nHuCard - 1) == false || false == isHaveCard(m_nHuCard + 1))
	{
		return false;
	}


	VEC_CARD vBackUp = m_vCards[nhuCardType];
	std::vector<uint8_t> vErase = { m_nHuCard,nPre,nNext };
	for (auto& ref : vErase)
	{
		auto iter = std::find(m_vCards[nhuCardType].begin(), m_vCards[nhuCardType].end(), ref);
		m_vCards[nhuCardType].erase(iter);
	}

	auto nRet = MJPlayerCard::isHoldCardCanHu();
	// resotre
	m_vCards[nhuCardType] = vBackUp;
	return nRet;
}

bool JJQEPlayerCard::onChuCard(uint8_t nChuCard)
{
	MJPlayerCard::onChuCard(nChuCard);
	if (card_Type(nChuCard) == eCT_Jian && card_Value(nChuCard) > 3)
	{
		updateHuCntToClient();
	}
	return true;
}

void JJQEPlayerCard::updateHuCntToClient()
{
	Json::Value jsMsg;
	jsMsg["idx"] = m_nCurPlayerIdx;
	jsMsg["huCnt"] = getMingPaiHuaCnt();
	if (m_pRoom)
	{
		m_pRoom->sendRoomMsg(jsMsg, MSG_REQUEST_CUR_HU_CNT);
	}
}

bool JJQEPlayerCard::getHuFanxingTypes( uint8_t nHuCard, bool isZiMo, std::vector<uint8_t>& vHuTypes )
{
	// if not zi mo , must add to fo check hu ;
	if (!isZiMo)
	{
		auto type = card_Type(nHuCard);
		if (type >= eCT_Max)
		{
			LOGFMTE("invalid card type for card = %u", nHuCard);
			return false;
		}
		addCardToVecAsc(m_vCards[type], nHuCard);
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

	// replace black j q k 
	VEC_CARD vBanckUpJian = m_vCards[eCT_Jian];
	auto nHuCardBack = m_nHuCard;
	for (auto& ref : m_vCards[eCT_Jian])
	{
		if (card_Value(ref) > 3)
		{
			ref -= 3;
		}
	}
	std::sort(m_vCards[eCT_Jian].begin(), m_vCards[eCT_Jian].end());

	if ( !MJPlayerCard::isHoldCardCanHu())
	{
		LOGFMTE("why you can not hu , but you say can hu ? ");
		m_vCards[eCT_Jian] = vBanckUpJian;
		m_nHuCard = nHuCardBack;
		if (!isZiMo)
		{
			funRemoveAddToCard(nHuCard);
		}
		return false ;
	}

	if (checkDuiDuiHu())
	{
		vHuTypes.push_back(eFanxing_DuiDuiHu);
	}
	else if (checkQingErHu())
	{
		vHuTypes.push_back(eFanxing_QingEr);
		if (isZiMo)
		{
			vHuTypes.push_back(eFanxing_ZiMo);
		}
	}
	else if (m_pRoom->isEnable13Hu() && check13Hu())
	{
		vHuTypes.push_back(eFanxing_13Hu);
	}
	else
	{
		vHuTypes.push_back(eFanxing_PingHu);
	}

	//if (!isZiMo)
	{
		if (checkBianZhi())
		{
			vHuTypes.push_back(eFanxing_BianZhi);
		}
		else if (checkDuDiao())
		{
			vHuTypes.push_back(eFanxing_DanDiao);
		}
		else if (checkKaZhang())
		{
			vHuTypes.push_back(eFanxing_KaZhang);
		}
	}

	// restore ;
	m_vCards[eCT_Jian] = vBanckUpJian;

	if (checkQiongQiongHen(true))
	{
		vHuTypes.push_back(eFanxing_QiongQiongHen);
	}
	else if (checkQiongHen(true))
	{
		vHuTypes.push_back(eFanxing_QiongHen);
	}

	m_nHuCard = nHuCardBack;
	if (!isZiMo)
	{
		funRemoveAddToCard(nHuCard);
	}
	return true;
}

uint16_t JJQEPlayerCard::getFinalHuCnt( bool isHu, bool& is3Red )
{
	// is start game ?
	uint8_t nJianZhang = m_pRoom->getJianZhang();
	if ((uint8_t)-1 == nJianZhang || 0 == nJianZhang)
	{
		return 0;
	}

	auto nAllCard = 0;
	nAllCard += getBlackJQKHuCnt(false);
	nAllCard += getFlyUpHuCnt();
	nAllCard += getHuaHuCnt();
	nAllCard += getPengHuCnt();
	nAllCard += getGangHuCnt();

	auto isHuZiMo = getIsZiMo();
	// do add huPai 
	if (isHu)
	{
		if ( m_nHuCard == 0)
		{
			LOGFMTE("why hu card is zero ?");
			isHu = false;
		}
	}

	nAllCard += getHoldAnKeCnt(isHu, isHuZiMo);
	nAllCard += 9;
	nAllCard /= 10;
	nAllCard *= 10;
	if (isHu)
	{
		nAllCard += getHoldWenQianHuCnt(isHu); // check wen qian ;

		nAllCard += 20;   // add base hu 20 ;

		std::vector<uint8_t> vHuTypes;
		getHuFanxingTypes(m_nHuCard, isHuZiMo, vHuTypes);

		// check bian zhi  du zhan 
		auto iter = std::find_if(vHuTypes.begin(), vHuTypes.end(), [](uint8_t nType) { return (eFanxing_BianZhi == nType) || (eFanxing_DanDiao == nType) || (eFanxing_KaZhang == nType); });
		if (iter != vHuTypes.end())
		{
			nAllCard += 10;
		}

		// qing er 
		iter = std::find(vHuTypes.begin(), vHuTypes.end(), eFanxing_QingEr);
		if (iter != vHuTypes.end())
		{
			nAllCard -= 20;  // omit base 20 
			nAllCard += m_pRoom->getQingErHuCnt();

			// check zi mo 
			iter = std::find(vHuTypes.begin(), vHuTypes.end(), eFanxing_ZiMo);
			if (iter != vHuTypes.end())
			{
				nAllCard += 10;
			}
		}

		// check x 2 type 
		iter = std::find_if(vHuTypes.begin(), vHuTypes.end(), [](uint8_t nType) { return (eFanxing_DuiDuiHu == nType) || (eFanxing_13Hu == nType)  ; });
		if (iter != vHuTypes.end())
		{
			nAllCard *= 2;
		}

		iter = std::find_if(vHuTypes.begin(), vHuTypes.end(), [](uint8_t nType) { return (eFanxing_QiongHen == nType); });
		if (iter != vHuTypes.end())
		{
			nAllCard *= 2;
		}

		// check X 4 
		iter = std::find(vHuTypes.begin(), vHuTypes.end(), eFanxing_QiongQiongHen );
		if (iter != vHuTypes.end())
		{
			nAllCard *= 4;
		}
		 
	}

	// check 3 hong 
	is3Red = false;
	if (check3Red(false, isHu))
	{
		nAllCard *= 2;
		is3Red = true;
	}

	if ( nAllCard > m_pRoom->getTopHuLimit())
	{
		nAllCard = m_pRoom->getTopHuLimit();
	}
	return nAllCard;
}