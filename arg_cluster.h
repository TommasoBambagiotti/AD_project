#include "rt/rt_api.h"


//Structure needed for pwelch cluster call 
typedef struct ArgCluster {
        unsigned short *        In; //input samples
        signed short*           In_FFT; //FFT input samples
        unsigned short*         w_ham; //hamming window samples
        unsigned int*           PSD; //PSD array
        short*                  Twiddles; 
        short*                  SwapTable;
	rt_perf_t*		welch_perf; //performance counter
//	ArgNN_t			Cluster_NN;
}ArgCluster_t;

