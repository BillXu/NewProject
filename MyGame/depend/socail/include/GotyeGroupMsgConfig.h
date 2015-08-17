//
//  GotyeGroupMsgConfig.h
//  GotyeAPI
//
//  Created by ouyang on 15/4/17.
//  Copyright (c) 2015年 Ailiao Technologies. All rights reserved.
//

#ifndef GotyeAPI_GotyeGroupMsgConfig_h
#define GotyeAPI_GotyeGroupMsgConfig_h

#include "Gotye.h"

NS_GOTYEAPI_BEGIN

typedef enum
{
    ShieldingGroupMsg = 0,
    ReceivingGroupMsg = 1 << 0,
    NotifyingGroupMsg = 1 << 1
}GotyeGroupMsgConfig;

NS_GOTYEAPI_END

#endif
