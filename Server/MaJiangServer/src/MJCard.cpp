#include "MJCard.h"
#include <cassert>
#include "log4z.h"
uint8_t CMJCard::getCard()
{
	if ( isEmpty() )
	{
		return 0 ;
	}

	return m_vAllCards[m_nCurCardIdx++] ;
}

uint8_t CMJCard::getLeftCardCount()
{
	if ( m_vAllCards.size() <= m_nCurCardIdx )
	{
		return 0 ;
	}

	return m_vAllCards.size() - m_nCurCardIdx ;
}

void CMJCard::shuffle()
{
	uint16_t n = 0 ;
	for ( uint16_t i = 0 ; i < m_vAllCards.size() - 2 ; ++i )
	{
		n = rand() % ( m_vAllCards.size() - i - 1 ) + i + 1    ;
		m_vAllCards[i] = m_vAllCards[n] + m_vAllCards[i] ;
		m_vAllCards[n] = m_vAllCards[i] - m_vAllCards[n] ;
		m_vAllCards[i] = m_vAllCards[i] - m_vAllCards[n] ;
	}
	m_nCurCardIdx = 0 ;
}

void CMJCard::debugCardInfo()
{
	printf("card Info: \n");
	for ( uint8_t nCard : m_vAllCards )
	{
		printf("cardNumber : %u\n",nCard) ;
	}
	printf("card info end \n\n") ;
}

void CMJCard::initAllCard( eMJGameType eType )
{
	m_vAllCards.clear() ;
	m_nCurCardIdx = 0 ;

	assert(eType < eMJ_Max && eType >= eMJ_None && "invalid card type" );
	if ( eMJ_TwoBird == eType )
	{
		initTwoBirdCard() ;
		return ;
	}

	m_eMJGameType = eType ;

	// every card are 4 count 
	for ( uint8_t nCnt = 0 ; nCnt < 4 ; ++nCnt )
	{
		// add base 
		uint8_t vType[3] = { eCT_Wan,eCT_Tiao,eCT_Tong } ;
		for ( uint8_t nType : vType )
		{
			for ( uint8_t nValue = 1 ; nValue <= 9 ; ++nValue )
			{
				m_vAllCards.push_back(makeCardNumber((eMJCardType)nType,nValue)) ;
			}
		}

		if ( eMJ_COMMON == m_eMJGameType )
		{
			// add feng , add ke
			for ( uint8_t nValue = 1 ; nValue <= 4 ; ++nValue )
			{
				m_vAllCards.push_back(makeCardNumber(eCT_Feng,nValue)) ;
			}

			for ( uint8_t nValue = 1 ; nValue <= 3 ; ++nValue )
			{
				m_vAllCards.push_back(makeCardNumber(eCT_Jian,nValue)) ;
			}
		}
	}
}

void CMJCard::initTwoBirdCard()
{
	m_vAllCards.clear() ;
	m_nCurCardIdx = 0 ;
	m_eMJGameType = eMJ_TwoBird ;

	// every card are 4 count 
	for ( uint8_t nCnt = 0 ; nCnt < 4 ; ++nCnt )
	{
		// add base 
		uint8_t vType[] = { eCT_Wan } ;
		for ( uint8_t& nType : vType )
		{
			for ( uint8_t nValue = 1 ; nValue <= 9 ; ++nValue )
			{
				m_vAllCards.push_back(makeCardNumber((eMJCardType)nType,nValue)) ;
			}
		}

		// add feng , add jian
		for ( uint8_t nValue = 1 ; nValue <= 4 ; ++nValue )
		{
			m_vAllCards.push_back(makeCardNumber(eCT_Feng,nValue)) ;
		}

		for ( uint8_t nValue = 1 ; nValue <= 3 ; ++nValue )
		{
			m_vAllCards.push_back(makeCardNumber(eCT_Jian,nValue)) ;
		}
	}
}

eMJGameType CMJCard::getGameType()
{
	return m_eMJGameType ;
}

bool CMJCard::isEmpty()
{
	return getLeftCardCount() <= 0 ;
}

eMJCardType CMJCard::parseCardType(uint8_t nCardNum)
{
	uint8_t nType = nCardNum & 0xF0 ;
	nType = nType >> 4 ;
	if ( (nType < eCT_Max && nType > eCT_None) == false )
	{
		LOGFMTE("parse card type error , cardnum = %u",nCardNum) ;
	}
	assert(nType < eCT_Max && nType > eCT_None && "invalid card type" );
	return (eMJCardType)nType ;
}

uint8_t CMJCard::parseCardValue(uint8_t nCardNum )
{
	return  (nCardNum & 0xF) ;
}

uint8_t CMJCard::makeCardNumber(eMJCardType eType,uint8_t nValue )
{
	if ( ((eType < eCT_Max && eType > eCT_None) == false) || (nValue <= 9 && nValue >= 1) == false )
	{
		LOGFMTE("makeCardNumber card type error , type  = %u, value = %u ",eType,nValue) ;
	}

	assert(eType < eCT_Max && eType > eCT_None && "invalid card type" );
	assert(nValue <= 9 && nValue >= 1 && "invalid card value" );
	uint8_t nType = eType ;
	nType = nType << 4 ;
	uint8_t nNum = nType | nValue ;
	return nNum ;
}

void CMJCard::parseCardTypeValue(uint8_t nCardNum, eMJCardType& eType,uint8_t& nValue )
{
	eType = parseCardType(nCardNum) ;
	nValue = parseCardValue(nCardNum) ;

	if ( ((eType < eCT_Max && eType > eCT_None) == false) || (nValue <= 9 && nValue >= 1) == false )
	{
		LOGFMTE("parseCardTypeValue card type error , type  = %u, value = %u number = %u",eType,nValue ,nCardNum) ;
	}

	assert(eType < eCT_Max && eType > eCT_None && "invalid card type" );
	assert(nValue <= 9 && nValue >= 1 && "invalid card value" );
}

void CMJCard::debugSinglCard(uint8_t nCard )
{
	auto cardType = parseCardType(nCard) ;
	auto cardValue = parseCardValue(nCard) ;

	switch (cardType)
	{
	case eCT_None:
		break;
	case eCT_Wan:
		printf("%u 万 \n",cardValue) ;
		break;
	case eCT_Tong:
		printf("%u 筒 \n",cardValue) ;
		break;
	case eCT_Tiao:
		printf("%u 条 \n",cardValue) ;
		break;
	case eCT_Feng:
		switch ( cardValue )
		{
		case 1 :
			printf("东 风\n") ;
			break;
		case 2:
			printf("南 风\n") ;
			break;
		case 3:
			printf("西 风\n") ;
			break;
		case 4:
			printf("北 风\n") ;
			break ;
		default:
			printf("unknown 风 card = %u \n",nCard ) ;
			break;
		}
		break;
	case eCT_Jian:
		switch ( cardValue )
		{
		case 1 :
			printf("中 \n") ;
			break;
		case 2:
			printf("发 \n") ;
			break;
		case 3:
			printf("白\n") ;
			break;
		default:
			printf("unknown 箭牌 card = %u \n",nCard ) ;
			break;
		}
		break;
	case eCT_Max:
		printf("unknown card = %u \n",nCard) ;
		break;
	default:
		break;
	}
}