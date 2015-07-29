#pragma once
#include <stdint.h>

#ifdef WIN32
#pragma warning(disable:4200)
#pragma warning(disable:4819)
#endif

#define PROTOCOL_LENGTH_BYTES                	2

#define MAX_USER_NAME_SIZE						32
#define MAX_PASSWORD_SIZE						16
#define MAX_MATCH_NAME_SIZE						64
#define MAX_TEAM_NAME_SIZE						64

#define MAX_NETWORK_PACKAGE_SIZE                65535
#define NEW_MSG(STRUCTURE, ...) \
char __msg_buffer__[MAX_NETWORK_PACKAGE_SIZE];\
STRUCTURE* newmsg = Constructor((STRUCTURE*)(__msg_buffer__), ##__VA_ARGS__);
struct nullmsg
{
    uint16_t 				                size;
    uint8_t 				                cmd;
    nullmsg(uint8_t id)
    {
        this->size = sizeof(*this);
        this->cmd = id;
    }
};
