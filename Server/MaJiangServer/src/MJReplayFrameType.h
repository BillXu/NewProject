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
	eMJFrame_Peng,// { invokerIdx : 0 ,idx : 3, card : 23 }
	eMJFrame_MingGang, // { invokerIdx : 0 idx : 2 , gang : 23 , newCard : 22 }
	eMJFrame_Decl_Peng, // { idx : 2 }
	eMJFrame_Decl_MingGang, // { idx : 2 }
	eMJFrame_Decl_BuGang,// { idx : 2 }
	eMJFrame_HuaGang, // { idx: 2 , hua : 2 , newCard : 23 }
	eMJFrame_Player_Ting, // { idx : 2 }

	eMJFrame_Wait_ChaoZhuang, 
	eMJFrame_Player_ChaoZhuang, // { idx : 0 , isChao : 1 }
	eMJFrame_Wait_Qi_Pai, // { idx : 0 }  
	eMJFrame_Player_Qi_Pai, // { idx : 0 , isQiPai: 1 }
	eMJFrame_Wait_FlyUp, // { idx : 0 }
	eMJFrame_Player_FlyUp, // { idx : 2 , flyCards : [23,32], newCards : [23,23] }
	eMJFrame_Player_AutoBuHua, // { detail : [ { idx : 2 , hua : [2,23], card : [2,3] } , ... ] }
};