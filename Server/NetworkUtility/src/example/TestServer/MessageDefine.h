#pragma once 
#pragma pack(push)
#pragma pack(1)
struct stMsgText
{
	unsigned short nIdx  ;
	char pText[300];
	stMsgText(){ memset(pText,0,sizeof(pText)) ; }
};
#pragma pack(pop)