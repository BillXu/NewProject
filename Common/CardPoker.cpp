#include "CardPoker.h"
#include <assert.h>
#include <stdlib.h>
#include "LogManager.h"
// card 
CCard::CCard(  unsigned char nCompositeNum  )
	:m_eType(eCard_Heart)
	,m_nCardFaceNum(0)
{
	RsetCardByCompositeNum(nCompositeNum) ;
}

CCard::~CCard()
{

}

void CCard::RsetCardByCompositeNum( unsigned char nCompositeNum )
{
	assert(nCompositeNum > 0 && nCompositeNum <= 54 && "illegal composite Number" );
	if ( (nCompositeNum > 0 && nCompositeNum <= 54) != true )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Illegal composite Number = %d",nCompositeNum );
		return ;
	}

	if ( nCompositeNum == 53 ) 
	{
		m_eType = eCard_Joker ;
		m_nCardFaceNum = 53 ;
	}
	else if ( nCompositeNum == 54 )
	{
		m_eType = eCard_BigJoker ;
		m_nCardFaceNum = 54 ;
	}
	else
	{
		m_eType = (eCardType)((nCompositeNum - 1)/13);
		m_nCardFaceNum = nCompositeNum - m_eType * 13 ;
	}
}

unsigned char CCard::GetCardCompositeNum()
{
	return ( m_eType * 13 + m_nCardFaceNum );
}

void CCard::SetCard(eCardType etype, unsigned char nFaceNum )
{
	if ( etype < eCard_None || etype >= eCard_Max )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("unknown card type =%d", etype ) ;
		return ;
	}

	if ( nFaceNum <=0 ||nFaceNum >54 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("unlegal face number = %d",nFaceNum ) ;
	}
	m_eType = etype ;
	m_nCardFaceNum = nFaceNum ;
}

void CCard::LogCardInfo()
{
	const char* pType = NULL ;
	switch ( m_eType )
	{
	case eCard_Heart:
		pType = "红桃";
		break;
	case eCard_Sword:
		pType = "黑桃";
		break;
	case eCard_Club:
		pType = "草花";
		break;
	case eCard_Diamond:
		pType = "方块";
		break;
	default:
        pType = "unknown";
		break; 
	}
	CLogMgr::SharedLogMgr()->PrintLog("this is %s : %d . Composite Number: %d",pType,m_nCardFaceNum, GetCardCompositeNum() );
}

// Poker
CPoker::CPoker()
{
	m_vCards = NULL ;
	m_nCurIdx = 0 ;
	m_nCardCount = 0 ;
}

CPoker::~CPoker()
{
	if (m_vCards == NULL )
	{

	}
	else
	{
		delete [] m_vCards ;
	}
} 

void CPoker::InitPaiJiu()
{
	SetupCardCount(32);
	// red 
	unsigned char nRed[] = { 12,2,8,4,10,7,6,9,5} ;
	unsigned char nBlack[] = {10,6,4,7,8,11} ;
	
	CCard stcard(2) ;
	// red ;
	unsigned char ncount = sizeof(nRed) / (sizeof(unsigned char));
	for (int i = 0 ; i < ncount ; ++i )
	{
		stcard.SetCard(CCard::eCard_Heart,nRed[i]);
		AddCard(&stcard);
		stcard.SetCard(CCard::eCard_Diamond,nRed[i]);
		AddCard(&stcard);
	}

	// black 
	ncount = sizeof(nBlack) / (sizeof(unsigned char));
	for (int i = 0 ; i < ncount ; ++i )
	{
		stcard.SetCard(CCard::eCard_Club,nBlack[i]);
		AddCard(&stcard);
		stcard.SetCard(CCard::eCard_Sword,nBlack[i]);
		AddCard(&stcard);
	}

	// add hei tao A , and  3 ;
	stcard.SetCard(CCard::eCard_Sword,1);
	AddCard(&stcard);
	stcard.SetCard(CCard::eCard_Sword,3);
	AddCard(&stcard);
}

void CPoker::InitTaxasPoker()
{
	SetupCardCount(52);
	for ( int i = 1 ; i <= 52 ; ++i )
	{
		AddCard(i) ;
	}
	RestAllPoker() ;
}

void CPoker::InitBaccarat()
{
	int nCard = 8 ;
	SetupCardCount(52 * nCard );
	while ( nCard-- )
	{
		for ( int i = 1 ; i <= 52 ; ++i )
		{
			AddCard(i) ;
		}
	}

	RestAllPoker() ;
}

void CPoker::InitGolden()
{
	CLogMgr::SharedLogMgr()->ErrorLog("Implement this method please ");
}

unsigned char CPoker::GetCardWithCompositeNum()
{
	if ( m_nCardCount <= m_nCurIdx )
	{
		RestAllPoker();
	}
	return m_vCards[m_nCurIdx++] ;
}

void CPoker::ComfirmKeepCard( unsigned char nCardLeft )
{
	if ( GetLeftCard() < nCardLeft )
		RestAllPoker();
}

void CPoker::RestAllPoker()
{
	// ---
	int n = 0 ;
	for ( int i = 0 ; i < m_nCardCount ; ++i )
	{
		n = rand() % m_nCardCount  ;
		if ( n == i )
		{
			continue;
		}
		m_vCards[i] = m_vCards[n] + m_vCards[i] ;
		m_vCards[n] = m_vCards[i] - m_vCards[n] ;
		m_vCards[i] = m_vCards[i] - m_vCards[n] ;
 	}
	m_nCurIdx = 0 ;
	//LogCardInfo();
}

void CPoker::AddCard(unsigned char nCard )   // invoke when init
{
	if ( nCard < 0 || nCard > 54 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("unlegal card composite number = %d", nCard ) ;
		return ;
	}
	
	if ( m_nCurIdx >= m_nCardCount )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Poker room space is full , can not add more card ") ;
	}
	m_vCards[m_nCurIdx] = nCard ;
	++m_nCurIdx ;
}

void CPoker::AddCard(CCard* pcard)   // invoke when init
{
	AddCard(pcard->GetCardCompositeNum()) ;
}

void CPoker::SetupCardCount(unsigned short nCount )
{
	m_vCards = new unsigned char [nCount] ;
	m_nCurIdx = 0 ;
	m_nCardCount = nCount ;
}

void CPoker::LogCardInfo()
{
	CLogMgr::SharedLogMgr()->PrintLog("œ¥≈∆Ω·π˚»Áœ¬£∫   ");
	CCard stCard(2);
	for ( int i = 0 ; i < m_nCardCount ; ++i )
	{
		stCard.RsetCardByCompositeNum(m_vCards[i]) ;
		stCard.LogCardInfo() ;
	}
}