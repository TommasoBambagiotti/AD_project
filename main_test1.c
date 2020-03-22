
#include "test_fixed.h"
#include "rt/rt_api.h"

#define MOUNT           1
#define UNMOUNT         0
#define CID             0
#define PROFILING
int test_layers(rt_perf_t * perf);


void pulp_parallel(rt_perf_t * perf)
{

  rt_team_fork(NUM_CORES, test_layers, perf);
}



static void end_of_call(void *arg)
{
  printf("[clusterID: 0x%x] Hello from core %d\n", rt_cluster_id(), rt_core_id());
}

static void pwelch_entry(void *arg)
{
  printf("Entering cluster on core %d\n", rt_core_id());
  printf("There are %d cores available here.\n", rt_nb_pe());
  rt_team_fork(1, pwelch, (void *)0x0);
  printf("Leaving cluster on core %d\n", rt_core_id());
}

int main ()
{

        rt_event_sched_t *p_sched;
        rt_event_sched_init(p_sched);
        if (rt_event_alloc(p_sched, 8)) return -1;

        rt_event_t *p_event = rt_event_get(p_sched, end_of_call, (void *) CID);

	pwelch(NULL);

        rt_cluster_mount(MOUNT, CID, 0, NULL);

       // rt_cluster_call(NULL, CID, pwelch_entry, NULL, NULL,NULL, NULL, 1,p_event);
       // rt_event_execute(p_sched, 1);

	 // allocate performance counters
	rt_perf_t *perf = rt_alloc(RT_ALLOC_L2_CL_DATA, sizeof(rt_perf_t));
	if (perf == NULL) return -1;

	rt_perf_t * cluster_perf = rt_alloc(RT_ALLOC_L2_CL_DATA, sizeof(rt_perf_t));
 	if (cluster_perf == NULL) return -1;


	rt_cluster_call(NULL, CID, (void *)pulp_parallel, cluster_perf, NULL,1024, 1024, rt_nb_pe(), NULL);
	
	//rt_event_execute(p_sched,1);
	rt_free(RT_ALLOC_L2_CL_DATA, (void *) perf, sizeof(rt_perf_t));
	rt_free(RT_ALLOC_L2_CL_DATA, (void *) cluster_perf, sizeof(rt_perf_t));
	
        rt_cluster_mount(UNMOUNT, CID, 0, NULL);
//	 printf("\nFC last\n",rt_core_id());
return (0);
}

