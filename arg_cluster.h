#include "rt/rt_api.h"

/*
typedef struct ArgNN {

	rt_perf_t*		perf;
	unsigned int*		PSD_L1;


}ArgNN_t
*/


typedef struct ArgCluster {
        unsigned short *        In;
        signed short*           In_FFT;
        unsigned short*         w_ham;
        unsigned int*           PSD;
        short*                  Twiddles;
        short*                  SwapTable;
	rt_perf_t*		welch_perf;
//	ArgNN_t			Cluster_NN;
}ArgCluster_t;

