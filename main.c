
//#include "test_fixed.h"
#include "rt/rt_api.h"
#include "window_2048.h"
//#include "swaptable2048.h"
#include "p20us_8192.h"
#include "arg_cluster.h" //ArgCluster definition
#include "FFT_Lib.h"
#define MOUNT           1
#define UNMOUNT         0
#define CID             0
#define S NFFT_SEG/2

/*Function prototypes*/
int autoencoder(unsigned int *input_data_L1);
void pwelch (ArgCluster_t *ArgC);
void SetupWindowLUT(unsigned short *w, int N, int Dyn);
void  SetupInput(unsigned short * In, int N, int Dyn); 
static void free_mem(void* p,int size,rt_free_req_t* req);
void cluster_init(ArgCluster_t *ArgC);


/*GLOBAL VARIABLES*/
RT_L1_DATA short Twiddles[2*NFFT_SEG];
RT_L1_DATA unsigned short In_L1[NFFT_SEG], w_ham[NFFT_SEG];
RT_L1_DATA signed short In_FFT[2*NFFT_SEG];
RT_L1_DATA unsigned int PSD[NFFT_SEG/2+1];
RT_L2_DATA unsigned short In[NFFT], w_L2[NFFT_SEG];
RT_L1_DATA unsigned short Seg_count = 0;



/*Cluster calls*/
/*
void pulp_parallel(unsigned int *input_data_L1)
{
	
  rt_team_fork(NUM_CORES, autoencoder, input_data_L1 );


}
*/

void pwelch_parallel(ArgCluster_t *ArgC)
{
	int k=0, i= 0;
	rt_dma_copy_t cp1, cp2, cp3;
	
	SetupTwiddlesLUT((ArgC->Twiddles), NFFT_SEG, 0);
        //SetupR2SwapTable((ArgC->SwapTable), NFFT_SEG);	

	//window transfer from L2 to L1
	rt_dma_memcpy(	w_L2,//ext 
			ArgC->w_ham,//int
			sizeof(short)*NFFT_SEG,//loc
			RT_DMA_DIR_EXT2LOC,
			0,
			&cp1);
	rt_dma_wait(&cp1);

	//check transfer
	//for(i=0;i<NFFT_SEG;i++)
	//{
	//	if(ArgC->w_ham[i] != w_L2[i]) printf("transfer error!\n");
	//}	
	
	for(k=0;k<N_SEG ;k++)
	{	
		//printf("segment n: %d\n",ArgC->Count);
		if((ArgC->Count == 0))
		{
		//Input transfer
		rt_dma_memcpy(  	In,//ext 
					ArgC->In,//int
					sizeof(short)*NFFT_SEG,//loc
					RT_DMA_DIR_EXT2LOC,
					0,
					&cp2);
		rt_dma_wait(&cp2);

		//check transfer
		//for(i=0;i<NFFT_SEG;i++)
		//{
		//if(ArgC->In[i+NFFT_SEG*(k-1)] != In[i+NFFT_SEG*(k-1)]) printf("Seg %d transfer error\n",ArgC->Count);
		//}
		
		
		rt_team_fork(NUM_CORES, pwelch, ArgC); //### PWELCH ###
		ArgC->Count=ArgC->Count + 1; //segment counter increment
		}
		else
		{
			if((ArgC->Count > 0))
			{
				//Input transfer
				rt_dma_memcpy(  	In+(NFFT_SEG/2)*(k-1)+NFFT_SEG,//ext 
							ArgC->In,//int
							sizeof(short)*NFFT_SEG/2,//loc
							RT_DMA_DIR_EXT2LOC,
							0,
							&cp3);
				rt_dma_wait(&cp3);
				
			//check transfer
			//for(i=0;i<NFFT_SEG/2;i++)
			//{
			//	if(ArgC->In[i+(NFFT_SEG/2)*(k-1)+NFFT_SEG] != In[i+(NFFT_SEG/2)*(k-1)+NFFT_SEG]) printf("Seg %d transfer error\n",ArgC->Count);
			//}		
				rt_team_fork(NUM_CORES, pwelch, ArgC);//### PWELCH ###
				ArgC->Count=ArgC->Count + 1; //segment counter increment
			}//if
		}//else

		
		}
	//AE
	//rt_team_fork(NUM_CORES,autoencoder,ArgC->PSD);
	
	printf("exit from cluster\n");	
	
}


int main ()
{
	int k=0;
	int t1,t2;
        ArgCluster_t Cluster;
	rt_event_sched_t *p_sched;
        rt_event_sched_init(p_sched);
	rt_cluster_call_t *call;		
	//void *pv[6] = {NULL};
        if (rt_event_alloc(p_sched, 8)) return -1;

//	t1=rt_time_get_us();
//	pwelch(NULL);
//	t2=rt_time_get_us();
//	printf("pwelch time: %d\n",t2-t1);

	// allocate performance counters
	rt_perf_t *perf = rt_alloc(RT_ALLOC_L2_CL_DATA, sizeof(rt_perf_t));
	if (perf == NULL) return -1;
	/*
	rt_perf_t * cluster_perf = rt_alloc(RT_ALLOC_L2_CL_DATA, sizeof(rt_perf_t));
 	if (cluster_perf == NULL) return -1;
	*/
	// PRINT FC FREQUENCY //	
	printf("Soc FC frequency: %d\n",rt_freq_get(__RT_FREQ_DOMAIN_FC));
	
	//SETUP INPUT
	printf("Setup input\n");
	SetupInput(In, NFFT, IN_DYN);
       	SetupWindowLUT(w_L2, NFFT_SEG, WINDOW_DYN);
	
	/*power to the cluster*/
        rt_cluster_mount(MOUNT, CID, 0, NULL);

	//print cluster frequency
	printf("Cluster frequency: %d\n",rt_freq_get(__RT_FREQ_DOMAIN_CL));
	
	/*#### CLUSTER CALLS #####*/
	/*PW CLUSTER CALL*/
	printf("CLUSTER CALL: Setup Pwelch Structure\n");
	rt_cluster_call(NULL, CID, (void (*)(void *))cluster_init,&Cluster, NULL, 1024, 1024, rt_nb_pe(),NULL);	
	printf("Cluster call 2\n");
	rt_cluster_call(NULL, CID, (void (*)(void *))pwelch_parallel,&Cluster, NULL, 1024, 1024, rt_nb_pe(),NULL);	
	printf("pwelch finita\n");

	/*#### PSD CHECK ####*/		
/*	printf("PSD_CHECK\n");
        for(k=0;k<NFFT_SEG/2+1;k++)
        {
       	 float  Re =( ((float)((Cluster.PSD[k])))*(1L<<(N_SEG)))/(1<<(2*(FFT2_SAMPLE_DYN)));
        
               //Re =( (float)((PSD[i])))/(1<<1); 
                printf("%f\n",Re);
        }
*/
	
/*	
	rt_free(RT_ALLOC_CL_DATA, In , NFFT_SEG*sizeof(short) );
	rt_free(RT_ALLOC_CL_DATA, In_FFT, NFFT_SEG*sizeof(short) );
	rt_free(RT_ALLOC_CL_DATA, w_ham , NFFT_SEG*sizeof(short) );
	rt_free(RT_ALLOC_CL_DATA, Twiddles, 2*NFFT_SEG*sizeof(short));
	rt_free(RT_ALLOC_CL_DATA, SwapTable, NFFT_SEG*sizeof(short));
*/
//	printf("set timer ed entro nell'AE\n");
//	t1=rt_time_get_us();	
	/*AE CLUSTER CALL*/
//	rt_cluster_call(NULL, CID, (void *)pulp_parallel,PSD , NULL,1024, 1024, rt_nb_pe(), NULL);
	
//	t2=rt_time_get_us();
//	printf("Cluster time: %d\n",t2-t1);
	
	rt_free(RT_ALLOC_L2_CL_DATA, (void *) perf, sizeof(rt_perf_t));
	
//	rt_free(RT_ALLOC_L2_CL_DATA, (void *) cluster_perf, sizeof(rt_perf_t));
	
        rt_cluster_mount(UNMOUNT, CID, 0, NULL);
	 printf("\nFC last\n",rt_core_id());
return (0);
}

void  SetupInput(unsigned short * In, int N, int Dyn) 
{
	unsigned short i;
	
	for(i=0;i<N;i++)
	{
	In[i] =(unsigned short) (data[i]*((1<<Dyn)));
	//In[2*i+1]=0;
	}	
	

}


void SetupWindowLUT(unsigned short *w, int N, int Dyn)
{
	int i=0;
	//float S2_temp = 0.0;
	//only if not using windowLUT	
	//float w_temp;

	//LUT per finestra
	for(i=0;i<N;i++)
	{
	//matlab script --> cos((2*PI*i)/(N-1))
	//python script --> cos((2*PI*i)/(N))
	
	w[i] = (unsigned short) ((w_temp[i])*(1<<Dyn)); 
	//Window check
	//printf("w[%d] --> %d\n",i,w[i]);
	
	}
}

 void cluster_init(ArgCluster_t *ArgC  )
{
	ArgC->In=In_L1; 
	if((ArgC->In) == NULL) printf("error allocating In\n");
	ArgC->In_FFT=In_FFT; 
	if((ArgC->In_FFT) == 0) printf("error allocating In_FFt\n"); 
	ArgC->w_ham=w_ham; 
	if((ArgC->w_ham) == 0) printf("error allocating w_ham\n"); 
	ArgC->PSD=PSD; 
	if((ArgC->PSD) == 0) printf("error allocating PSD\n"); 
	ArgC->Twiddles=Twiddles; 
	if((ArgC->Twiddles) == 0) printf("error allocating Twiddles\n"); 
	//ArgC->SwapTable=SwapTable; 
	//if((ArgC->SwapTable) == 0) printf("error allocating SwapTable\n"); 
	ArgC->Count = Seg_count;
	

	//printf("ArgC->In[0] %d In[0] %d \n",ArgC->In[0],In_L1[0]);
	
	
/*
	//only for performance profiling
#ifdef PROFILING
	ArgC->welch_perf = rt_alloc(RT_ALLOC_L2_CL_DATA, sizeof(rt_perf_t));
        if (ArgC->welch_perf == NULL) return -1;
	rt_perf_init(ArgC->welch_perf);
	rt_perf_conf(ArgC->welch_perf,RT_PERF_CYCLES);
#endif	
*/
}

