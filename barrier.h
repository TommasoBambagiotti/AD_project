#ifndef _BARRIER_
#define _BARRIER_

#include "rt/rt_api.h"
#include "pulp.h"

void sw_barrier();
void init_tas_barrier();
void tas_barrier();

#if defined(SW_BARRIER)
#define BARRIER() sw_barrier();
#elif defined(TAS_BARRIER)
#define BARRIER() tas_barrier();
#else
#define BARRIER() rt_team_barrier();
#endif

#endif
