#include "NiuNiuPeerCard.h"
CNiuNiuPeerCard::CardGroup CNiuNiuPeerCard::s_CardGroup[10] = { 
	CardGroup(0,1,2,3,4),
	CardGroup(0,2,1,3,4),
	CardGroup(0,3,1,2,4),
	CardGroup(0,4,1,2,3),

	CardGroup(1,2,0,3,4),
	CardGroup(1,3,0,2,4),
	CardGroup(1,4,0,2,3),

	CardGroup(2,3,0,1,4),
	CardGroup(2,4,0,1,3),

	CardGroup(3,4,0,1,2)
} ;
void CNiuNiuPeerCard::addCompositCardNum( uint8_t nCardCompositNum )
{
	assert( (m_nAddIdx < NIUNIU_HOLD_CARD_COUNT -1) && "too many cards" ) ;

	m_vHoldCards[m_nAddIdx].RsetCardByCompositeNum(nCardCompositNum) ;
	if ( m_nAddIdx != m_nBiggestCardIdx )
	{
		if ( m_vHoldCards[m_nAddIdx].GetCardFaceNum() > m_vHoldCards[m_nBiggestCardIdx].GetCardFaceNum() )
		{
			m_nBiggestCardIdx = m_nAddIdx ;
		}
		else if ( m_vHoldCards[m_nAddIdx].GetCardFaceNum() == m_vHoldCards[m_nBiggestCardIdx].GetCardFaceNum() )
		{
			if ( m_vHoldCards[m_nAddIdx].GetType() > m_vHoldCards[m_nBiggestCardIdx].GetType() )
			{
				m_nBiggestCardIdx = m_nAddIdx ;
			}
		}
	}
	++m_nAddIdx ;
}

const char*  CNiuNiuPeerCard::getNameString() 
{
	if ( ! isCaculated() )
	{
		caculateCards();
	}
	return "niu niu" ;
}

uint32_t CNiuNiuPeerCard::getWeight() 
{
	if ( ! isCaculated() )
	{
		caculateCards();
	}
	return m_nWeight ;
}

void CNiuNiuPeerCard::reset() 
{
	m_nAddIdx = 0 ;
	m_nBiggestCardIdx = 0 ;
	m_eType = Niu_Max ;
	m_nPoint = 0 ;
	m_nGroupIdx = 10 ;
	m_nWeight = 0 ;
}

CNiuNiuPeerCard::NiuNiuType CNiuNiuPeerCard::getType()
{
	if ( ! isCaculated() )
	{
		caculateCards();
	}
	return m_eType ;
}

uint8_t CNiuNiuPeerCard::getPoint()
{
	if ( ! isCaculated() )
	{
		caculateCards();
	}
	return m_nPoint ;
}

bool CNiuNiuPeerCard::isCaculated()
{
	return Niu_Max == m_eType ;
}

void CNiuNiuPeerCard::caculateCards()
{
	assert(m_nAddIdx == ( NIUNIU_HOLD_CARD_COUNT - 1 ) && "cards not enough" );
	m_eType = Niu_None ;
	m_nPoint = 0 ;
	if ( checkFiveSmall())
	{
		m_eType = Niu_FiveSmall ;
		m_nPoint = 10 ;
	}
	else if ( checkBoom() )
	{
		m_eType = Niu_Boom ;
		m_nPoint = 10 ;
	}
	else if ( checkNiuGolden())
	{
		 m_eType = Niu_Golden ;
		 m_nPoint = 10 ;
	}
	else if ( checkNiuSilver() )
	{
		m_eType = Niu_Silver ;
		m_nPoint = 10 ;
	}
	else
	{
		for ( uint8_t nIdx = 0 ; nIdx < 10 ; ++nIdx )
		{
			CardGroup& ref = s_CardGroup[nIdx] ; 
			if ( checkNiu( ref) )
			{
				m_nGroupIdx = nIdx ;
				m_eType = Niu_Single ;
				m_nPoint = (m_vHoldCards[ref.nTwoIdx[0]].GetCardFaceNum() + m_vHoldCards[ref.nTwoIdx[1]].GetCardFaceNum()) % 10 ;
				if ( m_nPoint == 0 )
				{
					m_eType = Niu_Niu ;
					m_nPoint = 10 ;
				}
				break;
			}
		} 
	}

	m_nWeight = 0 ;
	uint8_t nType = m_eType ;
	uint8_t nBigFaceNum = m_vHoldCards[m_nBiggestCardIdx].GetCardFaceNum() ;
	uint8_t nCardType = m_vHoldCards[m_nBiggestCardIdx].GetType();
	uint8_t nPoint = m_nPoint ;
	m_nWeight = (nType << 24) | ( nPoint << 16 ) | (nBigFaceNum << 8) | nCardType  ;
}

bool CNiuNiuPeerCard::checkNiu(CardGroup& ref )
{
	uint8_t nTotalPoint = 0 ;
	for (int i = 0; i < 3; i++)
	{
		nTotalPoint += m_vHoldCards[ref.nThreeIdx[i]].GetCardFaceNum();
	}
	return (nTotalPoint % 10) == 0 ;
}

bool CNiuNiuPeerCard::checkFiveSmall()
{
	uint8_t nTotalPoint = 0 ;
	for ( CCard& nCard : m_vHoldCards )
	{
		if ( nCard.GetCardFaceNum() >= 5 )
		{
			return false ;
		}
		nTotalPoint += nCard.GetCardFaceNum() ;
	}

	return nTotalPoint < 10 ;
}

bool CNiuNiuPeerCard::checkNiuSilver()
{
	bool bFind10 = false ;
	for ( CCard& nCard : m_vHoldCards )
	{
		if ( nCard.GetCardFaceNum() < 10 )
		{
			return false ;
		}
		
		if ( nCard.GetCardFaceNum() == 10 )
		{
			if ( bFind10 )
			{
				return false ;
			}
			bFind10 = true ;
		}
	}

	return bFind10 ;
}

bool CNiuNiuPeerCard::checkNiuGolden()
{
	for ( CCard& nCard : m_vHoldCards )
	{
		if ( nCard.GetCardFaceNum() < 10 )
		{
			return false ;
		}
	}
	return true ;
}

bool CNiuNiuPeerCard::checkBoom()
{
	uint8_t nCard1 = 0, nCard2 = 0 ;
	for ( CCard& nCard : m_vHoldCards )
	{
		if ( nCard1 == 0 )
		{
			nCard1 = nCard.GetCardFaceNum() ;
			continue;
		}

		if ( nCard1 == nCard.GetCardFaceNum() )
		{
			continue;
		}

		if ( nCard2 == 0 )
		{
			nCard2 = nCard.GetCardFaceNum() ;
			continue;
		}

		if ( nCard2 == nCard.GetCardFaceNum() )
		{
			continue;
		}
		return false ;
	}

	return true ;
}