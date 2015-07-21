#pragma once 
#include "tinyxml.h"
#include <map>
struct CTaxasAINode
{
public:
	typedef std::map<int,CTaxasAINode*> MAP_TAXASAINODE;
	enum  eAIActionType
	{
		eAIAct_Follow,
		eAIAct_Pass,
		eAIAct_Add,
		eAIAct_AllIn,
		eAIAct_Max,
	};

	virtual ~CTaxasAINode(){}
	virtual void parseNode(TiXmlElement* pNode) = 0;
	virtual int getKeyValue() =0;
	void clearMapNode(MAP_TAXASAINODE& vNodes);
	bool addNodeToMap(MAP_TAXASAINODE& vMap,CTaxasAINode* pNode );
	CTaxasAINode* getNodeFromMap(MAP_TAXASAINODE& vMap, int nKey );
};

struct CTaxasAIBetTimesForAction
	:public CTaxasAINode
{
public:
	void parseNode(TiXmlElement* pNode);
	int getKeyValue(){ return nBetTimes ;}
	CTaxasAINode::eAIActionType getAction(int& addTimesBlind );
public:
	int nBetTimes;  // curMostBetCoin times bigBlind of the room ;
	float vAIActionRate[eAIAct_Max];
	unsigned int nAddLowLimit;  // when do add action , times bigBlind to add ;
	unsigned int nAddTopLimit ;
};

struct CTaxasAIHoldCardContribute
	:public CTaxasAINode
{
public:
	~CTaxasAIHoldCardContribute(){clearMapNode(vAllBetTimes);}
	void parseNode(TiXmlElement* pNode);
	int getKeyValue(){ return nContributeCardCnt ;}
	CTaxasAINode::eAIActionType getAction(unsigned int nMostBetTimesBlind,int& addTimesBlind );
public:
	int nContributeCardCnt;  // how many card contributed for the final card , maybe 0, 1 ,2 
	MAP_TAXASAINODE vAllBetTimes;
};

struct CTaxasAIKeyCard
	:public CTaxasAINode
{
public:
	~CTaxasAIKeyCard(){clearMapNode(vHoldCardContributes);}
	void parseNode(TiXmlElement* pNode);
	int getKeyValue(){ return nFaceValue ;}
	CTaxasAINode::eAIActionType getAction(unsigned short nContributeCardCnt,unsigned int nMostBetTimesBlind,int& addTimesBlind );
public:
	int nFaceValue ;
	MAP_TAXASAINODE vHoldCardContributes;  // most 3 cnt ;
};

struct CTaxasAICardType
	:public CTaxasAINode
{
public:
	~CTaxasAICardType(){clearMapNode(vKeyCards);}
	void parseNode(TiXmlElement* pNode);
	int getKeyValue(){ return nCardType ;}
	CTaxasAINode::eAIActionType getAction(unsigned short nContributeCardCnt,unsigned short nkeyCardFaceValue,unsigned int nMostBetTimesBlind,int& addTimesBlind );
public:
	int nCardType ;  
	MAP_TAXASAINODE vKeyCards;  // most 14 cunt ;  
};

struct CTaxasAIRound
	:public CTaxasAINode
{
public:
	~CTaxasAIRound(){clearMapNode(vCardTypes);}
	void parseNode(TiXmlElement* pNode);
	int getKeyValue(){ return nRound ;}
	CTaxasAINode::eAIActionType getAction(unsigned int nCardType ,unsigned short nContributeCardCnt,unsigned short nkeyCardFaceValue,unsigned int nMostBetTimesBlind,int& addTimesBlind );
public:
	int nRound ; // maybe 0,1,2,3
	MAP_TAXASAINODE vCardTypes ;
};

class CTaxasRobotAI
{
public:
	bool init(const char* pAiConfigFile );
	/*执行流程：调用者提供，牌型（cardType），手牌对牌型的贡献值张数(contributeCnt)，对最终牌型影响最大的牌面值（faceValue），当前本轮最大下注额是盲注的多少倍（timesBlind）。
		第一步：找到相应的Round，第二步：锁定相应的 cardType。 第三步：寻找所有faceValue配置中，比faceValue参数小的里面，最大的配置，应用之。（如：配置里只配置了，2，8。 那么 [2,8) 都取2，[8,14] 都去14）
		第四步：根据timesBlind参数从小到大，从左到右的排序配置，选择自己或者其最左边第一个。第三步同样的意思。 
		备注：如果第三和第四不的选择配置里只有一个，那么默认那一样。如果为空就报错。
		addTimesBlind: 在最小加注额的基础上，上浮addTimesBlind倍的大盲注，例如：最小加注额度是 A， 那么最终加注额度 = A + addTimesBlind * 大盲注 ；
	*/
	CTaxasAINode::eAIActionType getAction(unsigned short nRound, unsigned int nCardType ,unsigned short nContributeCardCnt,unsigned short nkeyCardFaceValue,unsigned int nMostBetTimesBlind,int& addTimesBlind );
public:
	CTaxasAIRound vRound[4];
};

