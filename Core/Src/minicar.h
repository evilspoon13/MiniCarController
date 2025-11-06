#ifndef _MINICAR_H_
#define _MINICAR_H

#include "common.h"
#include "context.h"
#include "can.h"

#define HEARTBEAT_TIMEOUT_MS 5000

void MinicarInit(SystemState* system_state);
void MinicarIter(SystemState* system_state);
int MinicarWakeup(SystemState* system_state);
void MinicarShutdown(SystemState* system_state);

#endif
