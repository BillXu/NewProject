#pragma once 
#include "CommonDefine.h"
#include "CardPoker.h"
#include <vector>
class CPeerCard
{
public:
	enum ePeerCardType
	{
		ePeerCard_None,  // common 
		ePeerCard_Pair,  // dui zi 
		ePeerCard_Sequence, // shun zi
		ePeerCard_SameColor,  // jin hua
		ePeerCard_SameColorSequence, // shun jin
		ePeerCard_Bomb,  // bao zi 
		ePeerCard_Max,
	};
	typedef std::vector<unsigned char> VEC_CARD_COMPSITION_NUM ;
public:
	CPeerCard();
	~CPeerCard();
	void SetPeerCardByNumber( unsigned char nNumber , unsigned char nNum, unsigned char nNum2 );
	bool PKPeerCard(CPeerCard* pPeerCard);
	void ReplaceCardByNumber( unsigned char nOld , unsigned char nNew );
	void ShowCardByNumber(unsigned char nCardNum);
	ePeerCardType GetType(){ return m_eCardType ;}
	void LogInfo();
	void Reset();
    CCard* GetCardByIdx( char idx ){ return m_vCard[idx] ;}
	void GetCompositeCardRepresent(char* vCard ); // 3 vector must ;
protected:
	void ArrangeCard();
protected:
	ePeerCardType m_eCardType ;
	CCard* m_vCard[PEER_CARD_COUNT] ;
	VEC_CARD_COMPSITION_NUM m_vRepacedCard ;
	VEC_CARD_COMPSITION_NUM m_vShowedCard;
};