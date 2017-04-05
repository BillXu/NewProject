#pragma once
#include "NativeTypes.h"
#include "json/json.h"
#include <memory>
class IReplayFrame
{
public:
	virtual bool init(uint16_t nFrameType, uint32_t nTimeStamp ) = 0;
	virtual void setFrameArg(Json::Value& jsArg ) = 0 ;
	virtual void restore( Json::Value& jsFrame) = 0;
	virtual void toJson(Json::Value& jsFrame ) = 0;
};

class IReplayGame
{
public:
	virtual void setReplayRoomInfo( Json::Value& jsInfo ) = 0;
	virtual uint32_t getReplayID() = 0;
	virtual void addFrame( std::shared_ptr<IReplayFrame> ptrFrame ) = 0;
	virtual void restore( Json::Value& jsReplay ) = 0;
	virtual void toJson( Json::Value& jsReplay ) = 0;
	virtual std::shared_ptr<IReplayFrame> createFrame( uint16_t nFrameType , uint32_t nTimestamp ) = 0 ;
};