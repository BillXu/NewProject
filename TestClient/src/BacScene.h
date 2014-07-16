#include "IScene.h"
class CBacScene
	:public IScene
{
public:
	CBacScene(CClient* pClient);
	~CBacScene();
	virtual bool OnMessage( RakNet::Packet* pPacket ) ;
	virtual void OnUpdate(float fDeltaTime );
};