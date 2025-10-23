#ifndef _MINICAR_H_
#define _MINICAR_H

#include "common.h"
#include "context.h"

void MinicarInit(SystemState* system_state);
void MinicarIter(SystemState* system_state);
void MinicarShutdown(SystemState* system_state);

#endif
