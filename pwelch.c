#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "arg_cluster.h"
//#include "test_fixed.h"
#include "pulp.h"
#include "FFT_Lib.h"
#include "FFT_Lib.h"
#include "stats.h"
#include "rt/rt_api.h"
#include "swaptable2048.h"
//#include "stats.h"
//#include "p20us_8192.h"
//#include "window_2048.h" //include windowLUT


//#define NFFT		8192 //number of input data
//#define NFFT_SEG	2048//number of point for each segment
//#define N_SEG		7 //(NFFT/S - 1) with this numbers we ensure an overlap of 50% between two adjacent segments
//#define FS
#define SQR(x)	((x)*(x))
//#define WINDOW_DYN	16 // Q0.16 for w_ham (unsigned short) 
//#define IN_DYN		5 // Q11.5 for In (unsigned short)
//#define S		NFFT_SEG/2 //number of overlapping points for each segment 
//#define FS 		50000 //Sampling frequency used to sample In 
#define INC NFFT_SEG/NUM_CORES
#define INC_PSD (NFFT_SEG/2)/NUM_CORES
#ifndef PI
#define PI 3.141592653589793238462643383279502884197169399375105820974944
#endif


void Radix2FFT_DIF_Par(signed short *__restrict__ Data, signed short *__restrict__ Twiddles, int N_FFT2);



void pwelch (ArgCluster_t *ArgC )
{
  	int i=0;
	int m=0;
	int seg_Inc;
/*		
	unsigned short  *In=ArgC->In;
	signed short *In_FFT=ArgC->In_FFT;
	unsigned short *w_ham=ArgC->w_ham;
	unsigned int *PSD=ArgC->PSD;
	short *Twiddles=ArgC->Twiddles;
	short *SwapTable=ArgC->SwapTable;
*/
	float re,im; //check variables
	float wS2_ck, wck, Inck; //check variables
	

	/*###Profiling variables###*/	
	//unsigned int cycles, instr,t1,t2;
	/*
	rt_perf_t *perf;

				
	perf = rt_alloc(RT_ALLOC_L2_CL_DATA, sizeof(rt_perf_t));
  	rt_perf_init(perf);
	rt_perf_conf(perf, (1<<RT_PERF_CYCLES) | (1<<RT_PERF_INSTR));
	
		
	rt_perf_reset(perf);
	rt_perf_start(perf);
	*/
	//t1=rt_time_get_us();
	
		//if(rt_core_id()==0)	
		//printf("pre pwelch\n");;
		//printf("Segment %d\n",k);
#ifdef INPUT_CHECK

if((rt_core_id() == 0) && (ArgC->Count == 1))
{
	for(i=0; i<NFFT_SEG; i++)
	{
	
	re = (((float) (ArgC->In[i]))/(1<<IN_DYN));
	//im = (((float) (ArgC->In[2*i + 1]))/(1<<IN_DYN));
	printf("%4.7f\n",re);
	}

}




#else //INPUT_CHECK
 
#ifdef PROF_PWELCH
		INIT_PROFILING();
		START_PROFILING();
#endif		

		//SetupR2SwapTable((ArgC->SwapTable), NFFT_SEG);	
		seg_Inc = ArgC->Count;
		if(rt_core_id()==0)
		printf("segment(inside pwelch): %d\n",seg_Inc);
		rt_team_barrier();
		for(m = 0;m < NFFT_SEG/NUM_CORES ;m++)
		{
//		if(i==0) {printf("pre-somma\n"); i++;}
		/*	x[m] ---> Q11.5 unsigned short
			w[m] ---> Q0.16 unsigned short
			x[m]*w[m] ---> Q11.21 unsigned int
			1) Shift ((x[m]*w[m]) >> 16 ) Q11.5 because the FFT input data are short
			2) Shift (x[m]*w[m]) >> 1 ) Q11.4 because the FFT input data are signed
			INC ---> cores increment 
			seg_Inc ---> increment in ArgC->In array 
*/		
			switch (rt_core_id()) {

			case 0: {
			ArgC->In_FFT[2*m] =(signed short) ((((unsigned int) ArgC->w_ham[m])*((unsigned int) ArgC->In[m]))>>17);
			ArgC->In_FFT[2*m+1] = (signed short) 0;
	//		printf("core 0 done: ID[%d]\n",rt_core_id());
				} break;
			case 1: {
			ArgC->In_FFT[2*(m+INC)] =(signed short) ((((unsigned int) ArgC->w_ham[m+INC])*((unsigned int) ArgC->In[m+INC]))>>17);
			ArgC->In_FFT[2*(m+INC)+1] = (signed short) 0;
		//	printf("core 1 done\n");
				} break;


			case 2: {
			ArgC->In_FFT[2*(m+2*INC)] =(signed short) ((((unsigned int) ArgC->w_ham[m+2*INC])*((unsigned int) ArgC->In[m+2*INC]))>>17);
			ArgC->In_FFT[2*(m+2*INC)+1] = (signed short) 0;

		//	printf("core 2 done\n");
				} break;

			case 3: {
			ArgC->In_FFT[2*(m+3*INC)] =(signed short) ((((unsigned int) ArgC->w_ham[m+3*INC])*((unsigned int) ArgC->In[m+3*INC]))>>17);
			ArgC->In_FFT[2*(m+3*INC)+1] = (signed short) 0;
		//	printf("core 3 done\n");
			
				}break;

			case 4: {
			ArgC->In_FFT[2*(m+4*INC)] =(signed short) ((((unsigned int) ArgC->w_ham[m+4*INC])*((unsigned int) ArgC->In[m+4*INC]))>>17);
			ArgC->In_FFT[2*(m+4*INC)+1] = (signed short) 0;

		//	printf("core 4 done\n");
				}break;
			case 5: {
			ArgC->In_FFT[2*(m+5*INC)] =(signed short) ((((unsigned int) ArgC->w_ham[m+5*INC])*((unsigned int) ArgC->In[m+5*INC]))>>17);
			ArgC->In_FFT[2*(m+5*INC)+1] = (signed short) 0;

		//	printf("core 5 done\n");
				}break;

			case 6: {
			ArgC->In_FFT[2*(m+6*INC)] =(signed short) ((((unsigned int) ArgC->w_ham[m+6*INC])*((unsigned int) ArgC->In[m+6*INC]))>>17);
			ArgC->In_FFT[2*(m+6*INC)+1] = (signed short) 0;

		//	printf("core 6 done\n");
				}break;

			case 7: {
			ArgC->In_FFT[2*(m+7*INC)] =(signed short) ((((unsigned int) ArgC->w_ham[m+7*INC])*((unsigned int) ArgC->In[m+7*INC]))>>17);
			ArgC->In_FFT[2*(m+7*INC)+1] = (signed short) 0;

		//	printf("core 7 done\n");
				}break;
			}//switch
	}//for

#ifdef WINDOWING_CHECK	
	if((rt_core_id() == 0) && (seg_Inc == 5))
	{
		for(m=0;m<NFFT_SEG;m++)
		{	
			//FIXED POINT TO FLOAT CONVERSION and CHECK
			re = ((float)(ArgC->In_FFT[2*m])/(1<<(FFT2_SAMPLE_DYN)));
			wck = ((float)(ArgC->w_ham[m])/(1<<(WINDOW_DYN)));
			Inck = ((float)(ArgC->In[m])/(1<<(IN_DYN)));

			printf("In_FFT[%d] = %10d | %4.7f In_FFT[%d] = w_ham[%d]*In[%d] = %10d*%10d | %4.7f*%4.7f;\n",2*m,ArgC->In_FFT[2*m],re,m,m,m,ArgC->w_ham[m],ArgC->In[m],wck,Inck);
			
		}	
	}
#endif

#ifdef FFT


/*
	if(rt_core_id()==0)
	printf("pwelch fft\n");
*/
	/*#### FFT ####*/
	
	//printf("Vectorial Radix 2 DIF FFT on %d points\n",NFFT_SEG);
	/*			FFT
	FFT output are scaled by NFFT_SEG/2 							*/
//	t1=rt_time_get_us();
//	if(i==1) {printf("pre-fft\n");i++;}

	if(rt_core_id()==0)
	printf("pre FFT\n");
	rt_team_barrier();
	Radix2FFT_DIF_Par(ArgC->In_FFT, ArgC->Twiddles, NFFT_SEG);
	if(rt_core_id()==0)
	printf("post FFT\n");
	
	
//	t2=rt_time_get_us();
//	printf("%d point window FFT time: %d\n",NFFT_SEG,t2-t1);


	/*only core 0 swaps samples*/
	if(rt_core_id() == 0)
	{
	printf("pre swap\n");
	SwapSamples  ((v2s *) ArgC->In_FFT, SwapTable, NFFT_SEG);
	printf("post swap\n");
	}


/*					
	//FFT CHECK
	if(k==N_SEG)
	{
		
		for (i=0; i<NFFT_SEG; i++) 
		{
        		 Re = (((float) In_FFT[2*i  ]))/(1<<(FFT2_SAMPLE_DYN));
                	 Im = (((float) In_FFT[2*i+1]))/(1<<(FFT2_SAMPLE_DYN));
                	//printf("%4d -> [%4.7f, %4.7f] %f\n", i, Re, Im, Re*Re+Im*Im);
			printf("%f,%f\n",Re,Im);
		}
	}
*/	
/*	if(rt_core_id()==0)
	printf("pwelch PSD\n");	
*/
	//PSD OK
//	if(i==2) {printf("pre-prodotto\n");i++;}

#endif //FFT_SECTION

#ifdef PSD
	rt_team_barrier();	
	for(i=0;i<(NFFT_SEG/2)/NUM_CORES;i++)
	{

			switch (rt_core_id()) {

			case 0: {
       		 ArgC->PSD[i]=  (ArgC->PSD[i]) +((((unsigned int)((ArgC->In_FFT[2*i])*(ArgC->In_FFT[2*i]))) >> N_SEG ) + (((unsigned int)((ArgC->In_FFT[2*i+1])*(ArgC->In_FFT[2*i+1]))) >> N_SEG)) ;
		//	printf("core 0 done\n");
				} break;
			case 1: {
       		 ArgC->PSD[i+INC_PSD]=  (ArgC->PSD[i+INC_PSD]) +((((unsigned int)((ArgC->In_FFT[2*(i+INC_PSD)])*(ArgC->In_FFT[2*(i+INC_PSD)]))) >> N_SEG ) + (((unsigned int)((ArgC->In_FFT[2*(i+INC_PSD)+1])*(ArgC->In_FFT[2*(i+INC_PSD)+1]))) >> N_SEG)) ;
		//	printf("core 1 done\n");
				} break;


			case 2: {
       		 ArgC->PSD[i+2*INC_PSD]=  (ArgC->PSD[i+2*INC_PSD]) +((((unsigned int)((ArgC->In_FFT[2*(i+2*INC_PSD)])*(ArgC->In_FFT[2*(i+2*INC_PSD)]))) >> N_SEG ) + (((unsigned int)((ArgC->In_FFT[2*(i+2*INC_PSD)+1])*(ArgC->In_FFT[2*(i+2*INC_PSD)+1]))) >> N_SEG)) ;

		//	printf("core 2 done\n");
				} break;

			case 3: {
       		 ArgC->PSD[i+3*INC_PSD]=  (ArgC->PSD[i+3*INC_PSD]) +((((unsigned int)((ArgC->In_FFT[2*(i+3*INC_PSD)])*(ArgC->In_FFT[2*(i+3*INC_PSD)]))) >> N_SEG ) + (((unsigned int)((ArgC->In_FFT[2*(i+3*INC_PSD)+1])*(ArgC->In_FFT[2*(i+3*INC_PSD)+1]))) >> N_SEG)) ;
		//	printf("core 3 done\n");
			
				}break;

			case 4: {
       		 ArgC->PSD[i+4*INC]=  (ArgC->PSD[i+4*INC_PSD]) +((((unsigned int)((ArgC->In_FFT[2*(i+4*INC_PSD)])*(ArgC->In_FFT[2*(i+4*INC_PSD)]))) >> N_SEG ) + (((unsigned int)((ArgC->In_FFT[2*(i+4*INC_PSD)+1])*(ArgC->In_FFT[2*(i+4*INC_PSD)+1]))) >> N_SEG)) ;

		//	printf("core 4 done\n");
				}break;
			case 5: {
       		 ArgC->PSD[i+5*INC_PSD]=  (ArgC->PSD[i+5*INC_PSD]) +((((unsigned int)((ArgC->In_FFT[2*(i+5*INC_PSD)])*(ArgC->In_FFT[2*(i+5*INC_PSD)]))) >> N_SEG ) + (((unsigned int)((ArgC->In_FFT[2*(i+5*INC_PSD)+1])*(ArgC->In_FFT[2*(i+5*INC_PSD)+1]))) >> N_SEG)) ;


		//	printf("core 5 done\n");
				}break;

			case 6: {
       		 ArgC->PSD[i+6*INC_PSD]=  (ArgC->PSD[i+6*INC_PSD]) +((((unsigned int)((ArgC->In_FFT[2*(i+6*INC_PSD)])*(ArgC->In_FFT[2*(i+6*INC_PSD)]))) >> N_SEG ) + (((unsigned int)((ArgC->In_FFT[2*(i+6*INC_PSD)+1])*(ArgC->In_FFT[2*(i+6*INC_PSD)+1]))) >> N_SEG)) ;


		//	printf("core 6 done\n");
				}break;

			case 7: {
       		 ArgC->PSD[i+7*INC_PSD]=  (ArgC->PSD[i+7*INC_PSD]) +((((unsigned int)((ArgC->In_FFT[2*(i+7*INC_PSD)])*(ArgC->In_FFT[2*(i+7*INC_PSD)]))) >> N_SEG ) + (((unsigned int)((ArgC->In_FFT[2*(i+7*INC_PSD)+1])*(ArgC->In_FFT[2*(i+7*INC_PSD)+1]))) >> N_SEG)) ;


		//	printf("core 7 done\n");
				}break;

			}
		if((i==(NFFT_SEG/2)/NUM_CORES) && (rt_core_id() == 0) ) 
       		 ArgC->PSD[NFFT_SEG/2]=  (ArgC->PSD[NFFT_SEG/2]) +((((unsigned int)((ArgC->In_FFT[2*(NFFT_SEG/2)])*(ArgC->In_FFT[2*(NFFT_SEG/2)]))) >> N_SEG ) + (((unsigned int)((ArgC->In_FFT[2*(NFFT_SEG/2)+1])*(ArgC->In_FFT[2*(NFFT_SEG/2)+1]))) >> N_SEG)) ;

	/*	
		//before each sum (Shift >> 1) to prevent overflow x N_SEG --> total shift of N_SEG
       		 ArgC->PSD[i]=  (ArgC->PSD[i]) +((((unsigned int)((ArgC->In_FFT[2*i])*(ArgC->In_FFT[2*i]))) >> N_SEG ) + (((unsigned int)((ArgC->In_FFT[2*i+1])*(ArgC->In_FFT[2*i+1]))) >> N_SEG)) ;
*/
 }
#endif //PSD

#ifdef PROF_PWELCH
	STOP_PROFILING();
#endif

	//	printf("post prodotto\n");
/*		
	//PSD CHECK on the number-k segment
	if (k==2)
	{
		for(i=0;i<NFFT_SEG/2+1;i++)
		{
			//PSD format is Q1
			Re =( ((float)((PSD[i])))*(1L<<N_SEG))/(1<<(2*(FFT2_SAMPLE_DYN)));                 
			printf("PSD[%d] ---> %f\n",i,Re);
		}
	}
*/	


/*			
	//PSD CHECK ORIGINAL
	printf("PSD_CHECK\n");
	for(i=0;i<NFFT_SEG/2+1;i++)
	{
		Re =( ((float)((PSD[i])))*(1L<<(N_SEG)))/(1<<(2*(FFT2_SAMPLE_DYN)));
	
               //Re =( (float)((PSD[i])))/(1<<1); 
                printf("%f\n",Re);
	}
*/
	
	/*	
	//t2=rt_time_get_us();
	rt_perf_stop(perf);
	rt_perf_save(perf);
	
	//instr = rt_perf_get(perf, RT_PERF_INSTR);
	//cycles = rt_perf_get(perf, RT_PERF_CYCLES);
	printf("core[%d] cycles,instructions: %d,%d\n",rt_core_id(),rt_perf_get(perf,RT_PERF_CYCLES),rt_perf_get(perf,RT_PERF_INSTR));
	*/

#endif //INPUT_CHECK
}



