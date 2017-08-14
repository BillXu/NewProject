#include "MJPlayerCard.h"
#include "MJCard.h"
#include "XLMJPlayerCard.h"
#include "NJMJPlayerCard.h"
#include "JJQEPlayerCard.h"
void tempTest()
{
	//CMJHuPaiInfo tInfo ;
	//	std::vector<uint8_t> vCards ;
	//	uint8_t nValue ;
	//	
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,2);
	//	vCards.push_back(nValue);
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,2);
	//	vCards.push_back(nValue);
	//	
	//	nValue = CMJCard::makeCardNumber(eCT_Wan, 2);
	//	vCards.push_back(nValue);
	//	
	//	nValue = CMJCard::makeCardNumber(eCT_Wan, 3);
	//	vCards.push_back(nValue);
	//	
	//	nValue = CMJCard::makeCardNumber(eCT_Wan, 4);
	//	vCards.push_back(nValue);
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,3);
	//	vCards.push_back(nValue);
	//	
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,6);
	//	vCards.push_back(nValue);
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,6);
	//	vCards.push_back(nValue);
	//	
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,4);
	//	vCards.push_back(nValue);
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,4);
	//	vCards.push_back(nValue);
	//	
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,5);
	//	vCards.push_back(nValue);
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,5);
	//	vCards.push_back(nValue);
	//	
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,6);
	//	vCards.push_back(nValue);
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,7);
	//	vCards.push_back(nValue);
	//		
	//	bool b = tInfo.parseHuPaiInfo(vCards);
	//	if ( b )
	//	{
	//		printf("real hu \n") ;
	//	}
		JJQEPlayerCard tPeer ;
		tPeer.bindRoom(nullptr, 0);
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tiao, 1));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tiao, 2));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tiao, 3));

		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 4));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 5));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 6));

		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 7));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 8));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tiao, 7));


		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 1));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 1));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 2));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 2));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 2));
		//tPeer.onPeng(CMJCard::makeCardNumber(eCT_Tong, 9));
		//--------------
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 3));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 3));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 3));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 4));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 5));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 6));

		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 7));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 7));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 7));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 8));

		auto isHu = tPeer.onDoHu(CMJCard::makeCardNumber(eCT_Wan, 6), 1);
		bool is3Red = false;
		auto nCnt = tPeer.getFinalHuCnt(true, is3Red);
		if ( isHu )
		{
			printf("right") ;
		}
}

#include "MJServer.h"
#include "Application.h"
int main()
{
	//tempTest();
	CApplication theAplication(CMJServerApp::getInstance());
	theAplication.startApp();
	return 0;
}