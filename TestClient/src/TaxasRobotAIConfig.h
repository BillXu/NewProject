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
	/*ִ�����̣��������ṩ�����ͣ�cardType�������ƶ����͵Ĺ���ֵ����(contributeCnt)������������Ӱ����������ֵ��faceValue������ǰ���������ע����äע�Ķ��ٱ���timesBlind����
		��һ�����ҵ���Ӧ��Round���ڶ�����������Ӧ�� cardType�� ��������Ѱ������faceValue�����У���faceValue����С�����棬�������ã�Ӧ��֮�����磺������ֻ�����ˣ�2��8�� ��ô [2,8) ��ȡ2��[8,14] ��ȥ14��
		���Ĳ�������timesBlind������С���󣬴����ҵ��������ã�ѡ���Լ�����������ߵ�һ����������ͬ������˼�� 
		��ע����������͵��Ĳ���ѡ��������ֻ��һ������ôĬ����һ�������Ϊ�վͱ���
		addTimesBlind: ����С��ע��Ļ����ϣ��ϸ�addTimesBlind���Ĵ�äע�����磺��С��ע����� A�� ��ô���ռ�ע��� = A + addTimesBlind * ��äע ��
	*/
	CTaxasAINode::eAIActionType getAction(unsigned short nRound, unsigned int nCardType ,unsigned short nContributeCardCnt,unsigned short nkeyCardFaceValue,unsigned int nMostBetTimesBlind,int& addTimesBlind );
public:
	CTaxasAIRound vRound[4];
};

