#include "GG23PlayerCard.h"
#include "log4z.h"
#include "GG23Room.h"
void GG23PlayerCard::reset()
{
	MJPlayerCard::reset();
	m_vFlyupCard.clear();
	m_v5FlyUp.clear();
	m_v5BuGang.clear();
	m_v5AnGang.clear();
	m_v5MingGang.clear();
	m_nInvokeHuIdx = -1;
	m_nHuCard = 0;
}

void GG23PlayerCard::bindRoom(GG23Room* pRoom, uint8_t nPlayerIdx)
{
	m_pRoom = pRoom;
	m_nCurPlayerIdx = nPlayerIdx;
}

bool GG23PlayerCard::getCardInfo(Json::Value& jsPeerCards)
{
	// svr: { idx : 2 , nDanDiaoKuaiZhao : 23 ,anPai : [2,3,4,34] , huaPai : [23,23,12] ,chuPai: [2,34,4] , anGangPai : [23,24],buGang : [23,45] ,pengGangInfo : [ { targetIdx : 23 , actType : 23 , card : 23 } , .... ]  }
	IMJPlayerCard::VEC_CARD vAnPai, vChuPai, vAnGangedCard;
	Json::Value jsAnPai, jsChuPai, jsAngangedPai, jsBuGang, jsFlyUp, jsPeng, js5FlyUp, js5BuGang, js5AnGang, js5MingGang;

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

	toJs(vAnPai, jsAnPai); toJs(vChuPai, jsChuPai); toJs(vAnGangedCard, jsAngangedPai); toJs(m_vGanged, jsBuGang); toJs(m_vFlyupCard, jsFlyUp);
	toJs(m_vPenged, jsPeng); toJs(m_v5FlyUp,js5FlyUp); toJs(m_v5BuGang, js5BuGang); toJs(m_v5AnGang, js5AnGang); toJs(m_v5MingGang, js5MingGang);
	jsPeerCards["anPai"] = jsAnPai; jsPeerCards["chuPai"] = jsChuPai; jsPeerCards["anGangPai"] = jsAngangedPai; jsPeerCards["buGang"] = jsBuGang; 
	jsPeerCards["flyUp"] = jsFlyUp; jsPeerCards["flyUp5"] = js5FlyUp; jsPeerCards["buGang5"] = js5BuGang; jsPeerCards["anGang5"] = js5AnGang;
	jsPeerCards["peng"] = jsPeng; jsPeerCards["mingGang5"] = js5MingGang;
	return true;
}

bool GG23PlayerCard::onFlyUp(std::vector<uint8_t>& vFlyUpCard, std::vector<uint8_t>& vNewCard )
{
	if ( vFlyUpCard.empty() )
	{
		return false;
	}
	// check all card can an gang 
	for ( auto& ref : vFlyUpCard)
	{
		if ( !canAnGangWithCard(ref) )
		{
			LOGFMTE("ref = %u can not flyup client error  room id= %u , idx = %u",ref,m_pRoom->getRoomID(),m_nCurPlayerIdx );
			return false;
		}
	}

	// do qi fei 
	for ( auto& ref : vFlyUpCard )
	{
		auto nNewCard = m_pRoom->getMJPoker()->distributeOneCard();
		onAnGang(ref, nNewCard);
		vNewCard.push_back(nNewCard);
	}
	m_vAnGanged.clear(); // in fact not an gang ;

	// sign flyUpCard ;
	for ( auto& ref : vFlyUpCard )
	{
		if ( card_Type(ref) == eCT_Feng)
		{
			if (canAnGangWithCard(ref))
			{
				auto nNewCard = m_pRoom->getMJPoker()->distributeOneCard();
				onAnGang(ref, nNewCard );
				vNewCard.push_back(nNewCard);
				m_v5FlyUp.push_back(ref);
				m_v5AnGang.clear();  // in fact not an gang ;
				continue;
			}
		}
		m_vFlyupCard.push_back(ref);
	}
	return true;
}

bool GG23PlayerCard::onDoHu(uint8_t nCard, uint8_t nInvokerIdx)
{
	m_nHuCard = nCard;
	m_nInvokeHuIdx = nInvokerIdx;
	addCardToVecAsc(m_vCards[card_Type(nCard)],nCard );
	return true;
}

void GG23PlayerCard::getHuInfo(uint8_t& nInvokeIdx, std::vector<uint8_t>& vHuTypes)
{
	nInvokeIdx = m_nInvokeHuIdx;
	getHuFanxingTypes(m_nHuCard, getIsZiMo(), vHuTypes);
}

bool GG23PlayerCard::getHoldCardThatCanAnGang(VEC_CARD& vGangCards)
{
	MJPlayerCard::getHoldCardThatCanAnGang(vGangCards);

	// check flyup have 10 or  king ,  that can do 5 card an gang ;
	uint8_t nV[] = { make_Card_Num(eCT_Feng,1),make_Card_Num(eCT_Feng,2) };
	for ( auto& ref : nV )
	{
		if (std::count(vGangCards.begin(), vGangCards.end(), ref) > 0)
		{
			continue;
		}

		if (canAnGangWithCard(ref))
		{
			vGangCards.push_back(ref);
			continue;
		}
	}

	return vGangCards.size() > 0;
}

bool GG23PlayerCard::getHoldCardThatCanBuGang(VEC_CARD& vGangCards)
{
	MJPlayerCard::getHoldCardThatCanBuGang(vGangCards);

	// check flyup have 10 or  king ,  that can do 5 card bu gang ;
	uint8_t nV[] = { make_Card_Num(eCT_Feng,1),make_Card_Num(eCT_Feng,2) };
	for (auto& ref : nV)
	{
		if (std::count(vGangCards.begin(), vGangCards.end(), ref) > 0)
		{
			continue;
		}

		if (canBuGangWithCard(ref))
		{
			vGangCards.push_back(ref);
			continue;
		}
	}

	return vGangCards.size() > 0;
}

bool GG23PlayerCard::canMingGangWithCard(uint8_t nCard)
{
	if ( MJPlayerCard::canMingGangWithCard(nCard) )
	{
		return true;
	}

	if (card_Type(nCard) != eCT_Feng)
	{
		return false;
	}

	// check fly an gang or an ganged 10 0r king , go on 5 ming gang ;
	auto iter = std::find(m_vFlyupCard.begin(), m_vFlyupCard.end(), nCard );
	if (iter != m_vFlyupCard.end())
	{
		return true;
	}

	iter = std::find(m_vAnGanged.begin(), m_vAnGanged.end(), nCard );
	if (iter != m_vAnGanged.end())
	{
		return true;
	}
	return false;
}

bool GG23PlayerCard::canAnGangWithCard(uint8_t nCard)
{
	if (MJPlayerCard::canAnGangWithCard(nCard))
	{
		return true;
	}

	if (card_Type(nCard) != eCT_Feng)
	{
		return false;
	}

	// check 10 or king , 5 an gang ;
	if ( isHaveCard(nCard) == false )
	{
		return false;
	}

	auto iter = std::find(m_vFlyupCard.begin(), m_vFlyupCard.end(), nCard );
	if (iter != m_vFlyupCard.end())
	{
		return true;
	}

	iter = std::find(m_vAnGanged.begin(), m_vAnGanged.end(), nCard );
	if (iter != m_vAnGanged.end())
	{
		return true;
	}
	return false;
}

bool GG23PlayerCard::canBuGangWithCard(uint8_t nCard)
{
	if ( MJPlayerCard::canBuGangWithCard(nCard) )
	{
		return true;
	}

	if (card_Type(nCard) != eCT_Feng)
	{
		return false;
	}

	if (isHaveCard(nCard) == false)
	{
		return false;
	}

	// already bu ganged , or ming ganged , that can 5 bu gang ;
	auto iter = std::find(m_vGanged.begin(), m_vGanged.end(), nCard);
	if (iter != m_vGanged.end())
	{
		return true;
	}
	return false;
}

bool GG23PlayerCard::onMingGang(uint8_t nCard, uint8_t nGangGetCard)
{
	if (MJPlayerCard::canMingGangWithCard(nCard))
	{
		if ( MJPlayerCard::onMingGang(nCard, nGangGetCard) )  // pu tong ming gang ;
		{
			return true;
		}
	}

	// feng ming gang specail , 10 or king  5 ming gang ;
	if ( card_Type(nCard) != eCT_Feng )
	{
		LOGFMTE( "terrible error can not ming gang card = %u", nCard );
		return false;
	}

	//auto& vCard = m_vCards[eCT_Feng];
	//auto nEraseCnt = std::count(vCard.begin(), vCard.end(), nCard);
	//if (nEraseCnt == 4)
	//{
	//	while (nEraseCnt-- > 0)
	//	{
	//		auto iter = std::find(vCard.begin(), vCard.end(), nCard);
	//		vCard.erase(iter);
	//	}
	//}

	m_v5MingGang.push_back(nCard);
	onMoCard(nGangGetCard);

	auto iter = std::find(m_v5FlyUp.begin(), m_v5FlyUp.end(), nCard);
	if (iter != m_v5FlyUp.end())
	{
		m_v5FlyUp.erase(iter);
	}
	else
	{
		iter = std::find(m_vAnGanged.begin(), m_vAnGanged.end(), nCard);
		if (iter != m_vAnGanged.end())
		{
			m_vAnGanged.erase(iter);
		}
	}
	return true;
}

bool GG23PlayerCard::onBuGang(uint8_t nCard, uint8_t nGangGetCard)
{
	if (MJPlayerCard::canBuGangWithCard(nCard))
	{
		if (MJPlayerCard::onBuGang(nCard, nGangGetCard))  // pu tong bu gang ;
		{
			return true;
		}
	}

	// feng bu gang specail , 10 or king  5 bu gang ;
	if (card_Type(nCard) != eCT_Feng)
	{
		LOGFMTE("terrible error can not onBuGang card = %u", nCard);
		return false;
	}

	auto iter = std::find(m_vCards[eCT_Feng].begin(),m_vCards[eCT_Feng].end(),nCard);
	if (iter == m_vCards[eCT_Feng].end())
	{
		LOGFMTE("do have have this card , how to  bu gang ? card = %u",nCard );
		return false;
	}
	m_vCards[eCT_Feng].erase(iter);

	m_v5BuGang.push_back(nCard);
	onMoCard(nGangGetCard);
	return true;
}

bool GG23PlayerCard::onAnGang(uint8_t nCard, uint8_t nGangGetCard)
{
	if (MJPlayerCard::canAnGangWithCard(nCard))
	{
		if (MJPlayerCard::onAnGang(nCard, nGangGetCard))  // pu tong bu gang ;
		{
			return true;
		}
	}

	// feng bu gang specail , 10 or king  5 bu gang ;
	if ( card_Type(nCard) != eCT_Feng )
	{
		LOGFMTE("terrible error can not onAnGang card = %u", nCard);
		return false;
	}

	auto iter = std::find(m_vCards[eCT_Feng].begin(), m_vCards[eCT_Feng].end(), nCard);
	if (iter == m_vCards[eCT_Feng].end())
	{
		LOGFMTE("do have have this card , how to  bu gang ? card = %u", nCard);
		return false;
	}
	m_vCards[eCT_Feng].erase(iter);

	m_v5AnGang.push_back(nCard);
	onMoCard(nGangGetCard);
	return true;
}

bool GG23PlayerCard::isHoldCardCanHu()
{
	auto nRet = isHoldCardCanHuNew();
	if ( false == nRet )
	{
		return false;
	}

	auto nTotalCnt = getMingPaiHuaCnt();
	if (nTotalCnt >= m_pRoom->getQiHuNeed())
	{
		return true;
	}

	// back up member 
	auto nBackHu = m_nHuCard;
	auto nInvokerIdx = m_nInvokeHuIdx;
	m_nHuCard = getNewestFetchedCard();
	m_nInvokeHuIdx = m_nCurPlayerIdx;

	nTotalCnt += getHoldAnKeCnt(true, true);
	nTotalCnt += getHoldWenQianCnt(true);

	//restore member 
	m_nHuCard = nBackHu;
	m_nInvokeHuIdx = nInvokerIdx;

	return nTotalCnt >= m_pRoom->getQiHuNeed() ;
}

bool GG23PlayerCard::canHuWitCard(uint8_t nCard)
{
	// do add 
	addCardToVecAsc(m_vCards[card_Type(nCard)], nCard);

	auto nRet = isHoldCardCanHuNew();
	uint16_t nTotalCnt = 0;
	do
	{
		if ( false == nRet)
		{
			break;
		}

		nTotalCnt = getMingPaiHuaCnt();
		if (nTotalCnt >= m_pRoom->getQiHuNeed())
		{
			break;
		}

		// back up member 
		auto nBackHu = m_nHuCard;
		auto nInvokerIdx = m_nInvokeHuIdx;
		m_nHuCard = getNewestFetchedCard();
		m_nInvokeHuIdx = m_nCurPlayerIdx;

		nTotalCnt += getHoldAnKeCnt(true, false);
		nTotalCnt += getHoldWenQianCnt(true);

		//restore member 
		m_nHuCard = nBackHu;
		m_nInvokeHuIdx = nInvokerIdx;

	} while ( 0 );

	// restore 
	auto iter = std::find(m_vCards[card_Type(nCard)].begin(), m_vCards[card_Type(nCard)].end(), nCard);
	m_vCards[card_Type(nCard)].erase(iter);

	return (nRet && nTotalCnt >= m_pRoom->getQiHuNeed());
}

uint16_t GG23PlayerCard::getMingPaiHuaCnt()
{
	auto nCnt = getPengHuCnt();
	nCnt += getGangHuCnt();
	nCnt += getFlyUpHuCnt();
	return nCnt;
}

uint16_t GG23PlayerCard::getFinalHuCnt( bool isHu )
{
	auto nHuCnt = getMingPaiHuaCnt();
	nHuCnt += getHoldWenQianCnt(isHu);
	nHuCnt += getHoldAnKeCnt(isHu, getIsZiMo());
	if (isHu)
	{
		nHuCnt += 20;
	}
	else
	{
		return nHuCnt;
	}

	// consider hu types 
	std::vector<uint8_t> vHuTypes;
	getHuFanxingTypes(m_nHuCard, getIsZiMo(), vHuTypes);
	if (vHuTypes.empty())
	{
		LOGFMTE("hu pai ? but hutypes is empty ? ");
		return nHuCnt;
	}
	// check qing su 
	switch ( vHuTypes.front() )
	{
	case eFanxing_23_PiaoHu:
	{
		nHuCnt *= 4;
	}
	break;
	case eFanxing_QingEr:
	{
		nHuCnt += 30;
	}
	break;
	case eFanxing_DuiDuiHu:
	{
		nHuCnt *= 2;
	}
	break;
	default:
		break;
	}
	 
	return nHuCnt;
}

void GG23PlayerCard::updateHuCntToClient()
{
	Json::Value jsMsg;
	jsMsg["idx"] = m_nCurPlayerIdx;
	jsMsg["huCnt"] = getMingPaiHuaCnt();
	if (m_pRoom)
	{
		m_pRoom->sendRoomMsg(jsMsg, MSG_REQUEST_CUR_HU_CNT);
	}
}

uint16_t GG23PlayerCard::getPengHuCnt()
{
	uint16_t nCnt = 0;
	for (auto& ref : m_vPenged)
	{
		if (m_pRoom->isCardJianPai(ref))
		{
			nCnt += 2;
		}
		else
		{
			nCnt += 1;
		}
	}
	return nCnt;
}

uint16_t GG23PlayerCard::getGangHuCnt()
{
	uint16_t nCnt = 0;

	// check ming gang 
	for (auto& ref : m_vGanged)
	{
		if (m_pRoom->isCardJianPai(ref))
		{
			nCnt += 8;
		}
		else
		{
			nCnt += 4;
		}
	}

	// check an gang 
	for (auto& ref : m_vAnGanged )
	{
		if (m_pRoom->isCardJianPai(ref))
		{
			nCnt += 12;
		}
		else
		{
			nCnt += 6;
		}
	}

	// check five gang ;
	nCnt += (m_v5AnGang.size() * 12);
	nCnt += ( (m_v5MingGang.size() + m_v5BuGang.size() ) * 4 );
	return nCnt;
}

uint16_t GG23PlayerCard::getHoldAnKeCnt( bool isHu, bool isHuZiMo )
{
	uint16_t nHoldHuCnt = 0;
	// find ke zi ;
	VEC_CARD vKeZi;
	for (auto vCards : m_vCards)
	{
		if ( vCards.size() < 3 )
		{
			continue;
		}

		for ( uint8_t nIdx = 0; ( nIdx + 2 ) < vCards.size(); )
		{
			if ( vCards[nIdx] == vCards[nIdx + 2])
			{
				vKeZi.push_back(vCards[nIdx] );
				nIdx += 3;
				continue;
			}
			++nIdx;
		}
	}

	if ( vKeZi.empty() )
	{
		return 0;
	}

	if ( isHu == false )
	{
		for ( auto& ref : vKeZi )
		{
			if ( m_pRoom->isCardJianPai(ref) )
			{
				nHoldHuCnt += 4;
			}
			else
			{
				nHoldHuCnt += 2;
			}
		}
		return nHoldHuCnt;
	}
	// check valid ke zi ;
	for (auto& ref : vKeZi)
	{
		// remove 3 ;
		uint8_t nCnt = 3;
		auto nType = card_Type( ref );
		do
		{
			auto iter = std::find(m_vCards[nType].begin(),m_vCards[nType].end(),ref);
			m_vCards[nType].erase(iter);
			--nCnt;
			if (nCnt == 0)
			{
				break;
			}
		} while (0);

		bool isRealKeZi = isHoldCardCanHuNew();
		if ( isRealKeZi )
		{
			uint8_t ntemp = 1;
			if (m_pRoom->isCardJianPai(ref))
			{
				ntemp = 2;
			}

			if ( isHuZiMo || m_nHuCard != ref )
			{
				ntemp *= 2;
			}
			nHoldHuCnt += ntemp;
		}
		// add back ;
		do
		{
			addCardToVecAsc(m_vCards[nType],ref );
			++nCnt;
			if (nCnt == 3)
			{
				break;
			}
		} while (0);
	}
	return nHoldHuCnt;
}

uint16_t GG23PlayerCard::getHoldWenQianCnt( bool isHu )
{
	if ( isHu == false )
	{
		return 0;
	}

	uint8_t nMayBeWenQianCnt = 100;
	for (uint8_t nIdx = 1; nIdx <= 3; ++nIdx)
	{
		auto nRedACnt = std::count(m_vCards[eCT_Wan].begin(), m_vCards[eCT_Wan].end(), make_Card_Num(eCT_Wan, nIdx));
		if ( nRedACnt < nMayBeWenQianCnt)
		{
			nMayBeWenQianCnt = nRedACnt ;
		}

		if ( 0 == nMayBeWenQianCnt)
		{
			return 0;
		}
	}

	uint8_t nRemovedWen = 0;
	uint8_t nRealWenQian = 0;
	while ( nMayBeWenQianCnt-- > 0 )
	{
		for ( uint8_t nIdx = 1; nIdx <= 3; ++nIdx )
		{
			auto nRedIter = std::find(m_vCards[eCT_Wan].begin(), m_vCards[eCT_Wan].end(), make_Card_Num(eCT_Wan, nIdx));
			if ( nRedIter == m_vCards[eCT_Wan].end() )
			{
				LOGFMTE("why this have removed more than we need");
				break;
			}
			m_vCards[eCT_Wan].erase(nRedIter);
		}

		++nRemovedWen;
		if ( isHoldCardCanHuNew() )
		{
			++nRealWenQian;
		}
		else
		{
			break;
		}
	}

	// add back wenqian card
	while ( nRemovedWen-- > 0 )
	{
		for ( uint8_t nIdx = 1; nIdx <= 3; ++nIdx )
		{
			addCardToVecAsc(m_vCards[eCT_Wan], make_Card_Num(eCT_Wan, nIdx));
		}
	}

	return (nRealWenQian * 10);
}

uint16_t GG23PlayerCard::getFlyUpHuCnt()
{
	uint16_t nFlyHuCnt = 0;
	for (auto& ref : m_vFlyupCard)
	{
		if (m_pRoom->isCardJianPai(ref))
		{
			nFlyHuCnt += 16;
		}
		else
		{
			nFlyHuCnt += 8;
		}
	}

	nFlyHuCnt += (m_v5FlyUp.size() * 32);
	LOGFMTD("room id = %u idx = %u flyhu cnt = %u", m_pRoom->getRoomID(), m_nCurPlayerIdx, nFlyHuCnt);
	return nFlyHuCnt;
}

// check hu 
bool GG23PlayerCard::checkQuanHun()  // dui dui hu 
{
	bool bFindJiang = false;
	VEC_CARD vAllCard;
	getHoldCard(vAllCard);
 
	// erase wen qian ;
	int8_t nSmall = getHoldWenQianCnt(true) / 10;
	while (nSmall--)
	{
		for (uint8_t nValue = 1; nValue <= 3; ++nValue)
		{
			auto iter = std::find(vAllCard.begin(), vAllCard.end(), make_Card_Num(eCT_Wan, nValue));
			if (iter == vAllCard.end())
			{
				LOGFMTE("why do not have this card ?");
				continue;
			}
			vAllCard.erase(iter);
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

bool GG23PlayerCard::checkQingSu()
{
	if ( m_vCards[eCT_Jian].size() > 2 || m_vCards[eCT_Feng].size() > 2 )
	{
		return false;
	}

	if ( m_vPenged.size() > 0 || m_vFlyupCard.size() > 0 || m_vAnGanged.size() > 0 || m_vGanged.size() > 0 || m_v5FlyUp.size() > 0 )
	{
		return false;
	}
	
	if ( false == isHoldCardCanHuNew() )  // most for cacualte jiang ;
	{
		LOGFMTE("can not hu how to check qing su ?");
		return false;
	}
	// hold can not have ke zi 
	for ( auto& vCards : m_vCards )
	{
		if (vCards.size() < 3)
		{
			continue;
		}

		auto nCardType = card_Type(vCards.front());

		VEC_CARD vCheck = vCards;
		// remove jiang 
		if (card_Type(m_nJIang) == nCardType)
		{
			auto iter = std::find(vCheck.begin(),vCheck.end(),m_nJIang);
			vCheck.erase(iter);

			iter = std::find(vCheck.begin(), vCheck.end(), m_nJIang);
			vCheck.erase(iter);
		}

		if (vCheck.size() % 3 != 0)
		{
			LOGFMTE("check qinger you must all shun zi = %u, type = %u", vCheck.size(), nCardType);
			return false;
		}

		while (vCheck.empty() == false)
		{
			uint8_t vShunZi[3] = { 0 };
			vShunZi[0] = vCheck.front();
			vShunZi[1] = vShunZi[0] + 1;
			vShunZi[2] = vShunZi[1] + 1;
			for (auto& ref : vShunZi)
			{
				auto iter = std::find(vCheck.begin(), vCheck.end(), ref);
				if (iter != vCheck.end())
				{
					(*iter) = 0;
					continue;
				}
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
	}
	return true;
}

bool GG23PlayerCard::checkPiaoHu( bool isZiMo )
{
	if ( m_vFlyupCard.empty() == false || m_vAnGanged.empty() == false || m_vGanged.empty() == false || m_v5FlyUp.empty() == false )
	{
		return false;
	}

	VEC_CARD vHold;
	getHoldCard(vHold);

	if ( isZiMo )
	{
		return vHold.size() == 2;
	}

	if ( vHold.size() > 5 )
	{
		return false;
	}

	return checkQuanHun();
}

bool GG23PlayerCard::getHuFanxingTypes(uint8_t nHuCard, bool isZiMo, std::vector<uint8_t>& vHuTypes)
{
	// if not zi mo , must add to fo check hu 
	if (checkPiaoHu(isZiMo))
	{
		vHuTypes.push_back(eFanxing_23_PiaoHu);
	}
	else if ( checkQuanHun() )
	{
		vHuTypes.push_back(eFanxing_DuiDuiHu);
	}
	else if ( checkQingSu() )
	{
		vHuTypes.push_back(eFanxing_QingEr);
	}
	else
	{
		vHuTypes.push_back(eFanxing_PingHu);
	}
	return true;
}