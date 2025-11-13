#ifndef _MINICAR_H_
#define _MINICAR_H_

#include "common.h"
#include "context.h"
#include "can.h"
#include "motor.h"

#define HEARTBEAT_TIMEOUT_MS 5000

void MinicarInit(SystemState* state);
void MinicarIter(SystemState* state);
void MinicarShutdown(SystemState* state);

#endif
