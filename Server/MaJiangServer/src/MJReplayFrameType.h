#pragma once
enum eMJFrameType
{
	eMJFrame_StartGame,// { bankIdx : 2, players : [ { uid : 23,idx : 0, coin : 23 , cards : [1,3,4,5] }, .... ] }
	eMJFrame_BuHua, //{ idx : 2, hua : 23, newCard : 23 }
	eMJFrame_WaitPlayerAct,  // { idx : 2, act: [ eAct_Pass, ... ] }
	eMJFrame_BuGang,  // { idx : 2 , gang : 2 , newCard : 23 }
	eMJFrame_AnGang,  // { idx : 2 , gang : 2 , newCard : 23 }
	eMJFrame_Hu, // { idx : 2 } 
	eMJFrame_Chu, // { idx : 2 , card: 23 }
	eMJFrame_Pass, // { idx : 2 }
	eMJFrame_Mo, // { idx : 2, card : 23 }
	eMJFrame_WaitPlayerActAboutCard, //  [ { idx : 2 , acts: [eAct_peng, ...] }, ... ] 
	eMJFrame_WaitRobotGang, // [2,3,1] 
	eMJFrame_Peng,// { idx : 3, card : 23 }
	eMJFrame_MingGang, // { idx : 2 , gang : 23 , newCard : 22 }

};