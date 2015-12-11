#pragma once
#include "tinyxml.h"
#include "TaxasPokerPeerCard.h"
#include <list>
#include <map>
enum eRobotActionType
{
	eRbt_Act_Follow,
	eRbt_Act_Add,
	eRbt_Act_Pass,
	eRbt_Act_GiveUp,
	eRbt_Act_AllIn,
	eRbt_Act_Max,
};
struct stAddType
{
	unsigned char nAddType ; // 0 times bigBlind , 1 times myself ;
	float fRate ; 
	float fTimes ;
public:
	stAddType(){nAddType = 0 ; fRate = 0 ; fTimes = 0 ;}
	void ParseNode(TiXmlElement* pNode);
};

//struct stActionType
//{
//public:
//	typedef std::list<stAddType*> LIST_ADD_TYPE ;
//public:
//	eRobotActionType eActType ;
//	float fRate ;
//	LIST_ADD_TYPE vAddType ;   // when add used ;
//public:
//	stActionType(){ eActType = eRbt_Act_Max; fRate = 0 ;}
//	static eRobotActionType ConverStringToActEnum(const char* pAct );
//	~stActionType();
//	void ParseNode(TiXmlElement* pNode);
//	bool Check();
//};

struct stBetTimesAction
{
public:
	typedef std::list<stAddType*> LIST_ADD_TYPE ;
public:
	int nBetTimes ;
	float m_ActRate[eRbt_Act_Max] ;
	LIST_ADD_TYPE m_vBetAddStrages ;
public:
	stBetTimesAction(){ memset(m_ActRate,0,sizeof(m_ActRate));} 
	void ParseNode(TiXmlElement* pNode);
	bool Check();
	eRobotActionType RandActionType()const;
	eRobotActionType RandActionWithIn(unsigned char cActA, unsigned char cActB )const;
	stAddType* RandAddStrage()const;
};

struct stCardTypeAction
{
public:
	typedef std::map<int,stBetTimesAction*> MAP_BET_TIMES_STRAGES ;
public:
	unsigned char nCard ;  // CTaxasPokerPeerCard::eCardType
	MAP_BET_TIMES_STRAGES vAllBetTimesStrages ;  // Ä¬ÈÏÊÇÉýÐò
public:
	stCardTypeAction(){ nCard = -1 ;}
	void ParseNode(TiXmlElement* pNode);
	bool Check();
	const stBetTimesAction* GetBetTimesAction( int nTimes);
};

//struct stCardControl
//{
//	float fTotalRate ;
//	stCardTypeAction m_vAllCardAction[CTaxasPokerPeerCard::eCard_Max];
//public:
//	void ParseNode(TiXmlElement* pNode);
//	stCardControl(){ fTotalRate = 0 ;}
//	bool Check();
//};

// for last player bet times me control 
//struct stTimesTypeAction
//{
//	float fTimes ;
//	float fTotalRate ;
//	stActionType m_vActionType[eRbt_Act_Max] ;
//public:
//	stTimesTypeAction(){ fTimes = 0 ;}
//	void ParseNode(TiXmlElement* pNode);
//	bool Check();
//};
//
//struct stTimesControl
//{
//	typedef std::list<stTimesTypeAction*> LIST_TIMES_ACTION ;
//	float fTotalRate ;
//	LIST_TIMES_ACTION m_vAllTimesActions ;
//public:
//	stTimesControl(){ fTotalRate = 0;}
//	void ParseNode(TiXmlElement* pNode);
//	~stTimesControl();
//	bool Check();
//	stTimesTypeAction* GetTypeAction( int nTimes );
//};

//struct stFinalAction
//{
//	float fTotal ;
//	stActionType m_vActionType[eRbt_Act_Max] ;
//	stActionType::LIST_ADD_TYPE* m_vpAddTypes ; // used when add action ;
//	stFinalAction(){ fTotal = 0 ;m_vpAddTypes = NULL ;}
//public:
//	stActionType* RandMyAction();
//	stActionType* RandMyActionWithIn(eRobotActionType eTypeA,eRobotActionType eTypeB );
//	stAddType* RandAddType();
//};

struct stRound
{
public:
	typedef std::map<unsigned char ,stCardTypeAction*> MAP_CARD_TYPE_STRAGES ;
public:
	int nRound ;
	MAP_CARD_TYPE_STRAGES  vAllCardTypeStrages ;
public:
	void ParseNode(TiXmlElement* pNode);
	stRound(){ nRound = 0 ; }
	bool Check();
	const stBetTimesAction* GetFinalAction(unsigned char nCardType, int nTimes );
};

struct stRobotAI
{
	unsigned short nRobotAIID ;
	stRound m_vRound[4] ;
public:
	void ParseNode(TiXmlElement* pNode);
	bool Check();
	const stBetTimesAction* GetFinalAction( unsigned char nRound ,unsigned char nCardType, int nTimes )
	{
		if ( nRound >= 4 )
		{
			printf(" error round = %d\n",nRound) ;
			nRound = 3 ;
		}
		return m_vRound[nRound].GetFinalAction(nCardType,nTimes) ;
	}
};

class CRobotAIManager
{
public:
	typedef std::map<unsigned short ,stRobotAI*> MAP_ROBOT_AI ;
public:
	void LoadFile(const char* pFileName );
	stRobotAI* GetRobotAIBy(unsigned short nRobotID );
protected:
	MAP_ROBOT_AI m_vAllRobotAI ;
};