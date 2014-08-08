#pragma once
#include "CardPoker.h"
#include "RoomBaseData.h"
class CGoldenRoomData
	:public CRoomBaseData
{
public:
	virtual void Init();
	stRoomGoldenDataOnly* GetDataOnly();
	unsigned char GetReadyPeerCnt();
	unsigned char GetNextActIdx();
	void OnStartGame();
	void OnEndGame();
	char GameOverCheckAndProcess() ; // return > 0 means game over ret value is winner idx ;
	void DistributeCard();
protected:
	CPoker tPoker ;
};