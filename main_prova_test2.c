
#include "test_fixed.h"
#include "rt/rt_api.h"

#define MOUNT           1
#define UNMOUNT         0
#define CID             0
#define PROFILING
int test_layers_1(rt_perf_t * perf);
int test_layers_2(rt_perf_t * perf);
int test_layers_3(rt_perf_t * perf);
int test_layers_4(rt_perf_t * perf);

/*GLOBAL VARIABLES*/
RT_L2_DATA int t1,t2;


void pulp_parallel(rt_perf_t * perf)
{

  rt_team_fork(NUM_CORES, test_layers_1, perf);
}

void pulp_parallel_2(rt_perf_t * perf)
{
  rt_team_fork(NUM_CORES, test_layers_2, perf);
}

void pulp_parallel_3(rt_perf_t * perf)
{
  rt_team_fork(NUM_CORES, test_layers_3, perf);
}


void pulp_parallel_4(rt_perf_t * perf)
{
  rt_team_fork(NUM_CORES, test_layers_4, perf);
}

static void end_of_call(void *arg)
{
 
	t2=rt_time_get_us();
	printf("Cluster tasks time: %d\n",t2-t1);
}



/*
static void pwelch_entry(void *arg)
{
  printf("Entering cluster on core %d\n", rt_core_id());
  printf("There are %d cores available here.\n", rt_nb_pe());
  rt_team_fork(1, pwelch, (void *)0x0);
  printf("Leaving cluster on core %d\n", rt_core_id());
}
*/

int main ()
{

        rt_event_sched_t *p_sched;
        rt_event_sched_init(p_sched);
	rt_cluster_call_t *call;		

        if (rt_event_alloc(p_sched, 8)) return -1;

//	printf("pwelch entry point\n");
	pwelch(NULL);
//	printf("pwelch exit point\n");

 	rt_event_t *p_event = rt_event_get(p_sched, end_of_call, (void *) CID);
	 // allocate performance counters
	rt_perf_t *perf = rt_alloc(RT_ALLOC_L2_CL_DATA, sizeof(rt_perf_t));
	if (perf == NULL) return -1;

	rt_perf_t * cluster_perf = rt_alloc(RT_ALLOC_L2_CL_DATA, sizeof(rt_perf_t));
 	if (cluster_perf == NULL) return -1;

        rt_cluster_mount(MOUNT, CID, 0, NULL);
	/*CLUSTER TASKS PERFORMANCE*/
	printf("start timer\n");
	t1= rt_time_get_us();
	rt_cluster_call(call, CID, (void *)pulp_parallel, cluster_perf, NULL,1024, 1024, rt_nb_pe(), NULL);
	rt_cluster_call(call, CID, (void *)pulp_parallel_2, cluster_perf, NULL,1024, 1024, rt_nb_pe(), NULL);
	rt_cluster_call(call, CID, (void *)pulp_parallel_3, cluster_perf, NULL,1024, 1024, rt_nb_pe(), NULL);
	t2=rt_time_get_us();
	printf("all-layer4 time: %d\n",t2-t1);
	rt_cluster_call(call, CID, (void *)pulp_parallel_4, cluster_perf, NULL,1024, 1024, rt_nb_pe(), p_event);
	rt_event_execute(p_sched, 1);
	rt_free(RT_ALLOC_L2_CL_DATA, (void *) perf, sizeof(rt_perf_t));
	rt_free(RT_ALLOC_L2_CL_DATA, (void *) cluster_perf, sizeof(rt_perf_t));
	
        rt_cluster_mount(UNMOUNT, CID, 0, NULL);
	 printf("\nFC last\n",rt_core_id());
return (0);
}


