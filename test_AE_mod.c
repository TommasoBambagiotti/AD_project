#include "rt/rt_api.h"
#include "stats.h"

//#include "pulp-nn/include/pulp_nn.h"
//#include "int8_fully_layer_parameters.h"
#include "layer1_fully.h"
#include "layer2_fully.h"
#include "layer3_fully.h"
#include "layer4_fully.h"

/*#### DATA ####*/
/*EVERY LAYER HAS ITS OWN DIMENSION*/
/*
RT_FC_SHARED_DATA int8_t out_layer1_L2[(OUT_NEURONS)];
RT_L1_DATA int8_t input_data_L1[(IFM_H_FC * IFM_H_FC * IFM_CH_FC)];
RT_L1_DATA int8_t wt_layer1_L1[(OUT_NEURONS * IFM_H_FC * IFM_H_FC * IFM_CH_FC)];
RT_L1_DATA int8_t bias_L1[OUT_NEURONS];
RT_L1_DATA int8_t out_layer1_L1[(OUT_NEURONS)];
*/

/*i files sono in L1, a cosa serve il DMA?*/
/*Only bias vectors are always in cluster L1*/
RT_L1_DATA int8_t bias_layer1_L1[OUT_NEURONS_1];
RT_L1_DATA int8_t bias_layer2_L1[OUT_NEURONS_2];
RT_L1_DATA int8_t bias_layer3_L1[OUT_NEURONS_3];
RT_L1_DATA int8_t bias_layer4_L1[OUT_NEURONS_4];
RT_L1_DATA int8_t wt_layer1_L1[OUT_NEURONS_1 * IFM_H_FC_1 * IFM_H_FC_1 * IFM_CH_FC_1];
RT_L1_DATA int8_t wt_layer2_L1[OUT_NEURONS_2 * IFM_H_FC_2 * IFM_H_FC_2 * IFM_CH_FC_2];
RT_L1_DATA int8_t wt_layer3_L1[OUT_NEURONS_3 * IFM_H_FC_3 * IFM_H_FC_3 * IFM_CH_FC_3];
RT_L1_DATA int8_t wt_layer4_L1[OUT_NEURONS_4 * IFM_H_FC_4 * IFM_H_FC_4 * IFM_CH_FC_4];
RT_L1_DATA int8_t out_layer1_L1[OUT_NEURONS_1 ];
RT_L1_DATA int8_t out_layer2_L1[OUT_NEURONS_2 ];
RT_L1_DATA int8_t out_layer3_L1[OUT_NEURONS_3 ];
RT_L1_DATA int8_t out_layer4_L1[OUT_NEURONS_4 ];


RT_FC_SHARED_DATA int8_t out_L2[OUT_NEURONS_4];
					
//RT_FC_SHARED_DATA int8_t *out_L2; 

int autoencoder(unsigned int *input_data_L1)
{

	if(rt_core_id()==0) //core 0 stuff, not FC
	{
	
/*### PROFILING SECTION ###*/
#ifdef PROFILING_TOT
	INIT_PROFILING();
	START_PROFILING();
	rt_team_barrier();
#endif

#ifdef PRINT
	printf("PULP PARALLEL 1st LAYER\n");
#endif

	/* transfer layer weights and activations from L2 to L1 memory through DMA */
	rt_dma_copy_t cp1;
	rt_dma_copy_t cp2;
		
#ifdef PRINT
	printf("activations transfer from L2 to L1\n");
#endif    
	rt_dma_memcpy(
	#ifdef Q8
			input_data_L2, // ext
			input_data_L1, // loc
			IFM_H_FC_1 * IFM_H_FC_1 *IFM_CH_FC_1, // size
	#endif
			RT_DMA_DIR_EXT2LOC, // dir
			0, // merge
			&cp1 // copy
			);
    	rt_dma_wait(&cp1);

#ifdef PRINT
	printf("weights transfer from L2 to L1\n");
#endif
    	rt_dma_memcpy(
	#ifdef Q8
			wt_layer1_L2, // ext
			wt_layer1_L1, // loc
			OUT_NEURONS_1 * IFM_H_FC_1 * IFM_H_FC_1 *IFM_CH_FC_1, // size
	#endif
			RT_DMA_DIR_EXT2LOC, // dir
			0, // merge
			&cp2 // copy
			);
    	rt_dma_wait(&cp2);

#ifdef Q8
    	//BIAS
    	for (int z= 0; z < OUT_NEURONS_1; z++)
	{
		bias_layer1_L1[z] = 0;
	}
#endif

#ifdef PRINT
	printf("end of transfer. Start Fully-connected kernel parallel exec \n");
#endif

	}
	rt_team_barrier();

	/* Execution of LAYER 1 */

	/*### PROFILING SECTION ###*/

#ifdef PROFILING
	/* These functions allow for a complete profiling of the kernel execution */
	/* Defined in "stats.h" header. See the file for complete info                */
	printf("Layer 1 profiling\n");
	INIT_PROFILING();
	START_PROFILING();
	rt_team_barrier();

#endif
	/*### LAYER 1 ###*/
#ifdef Q8
	pulp_nn_linear_int8(input_data_L1, wt_layer1_L1, IFM_H_FC_1 * IFM_H_FC_1* IFM_CH_FC_1,
							OUT_NEURONS_1, BIAS_SHIFT_FC_1, OUT_QF_FC_1, bias_layer1_L1, out_layer1_L1);
#endif

#ifdef PROFILING
	STOP_PROFILING();
#endif

	/*#### LAYER 2 ####*/
	/*load the second weight layer*/
	if(rt_core_id()==0)
	{
		rt_dma_copy_t cp3;
		
		rt_dma_memcpy(
#ifdef Q8
			wt_layer2_L2, // ext
			wt_layer2_L1, // loc
			OUT_NEURONS_2 * IFM_H_FC_2 * IFM_H_FC_2 *IFM_CH_FC_2, // size
#endif
			RT_DMA_DIR_EXT2LOC, // dir
			0, // merge
			&cp3 // copy
			);
    	rt_dma_wait(&cp3);
	
	/*BIAS LAYER 2*/	
	for (int z= 0; z < OUT_NEURONS_2; z++)
		{
			bias_layer2_L1[z] = 0;
		}

	}

	/*EXECUTION OF LAYER 2*/
	rt_team_barrier();

#ifdef PROFILING
	printf("Layer 2 profiling\n");
	//INIT_PROFILING();
	START_PROFILING();
	/*Sync*/
	rt_team_barrier();

#endif
	/*LAYER 2*/
#ifdef Q8
	pulp_nn_linear_int8(out_layer1_L1, wt_layer2_L1, IFM_H_FC_2 * IFM_H_FC_2* IFM_CH_FC_2,
								OUT_NEURONS_2, BIAS_SHIFT_FC_2, OUT_QF_FC_2, bias_layer2_L1, out_layer2_L1);
	pulp_nn_relu_int8(out_layer2_L1,1,OUT_NEURONS_2);
#endif

#ifdef PROFILING
	STOP_PROFILING();
#endif
	/*#### LAYER 3 ####*/
	/*charge the second weight layer*/
		if(rt_core_id()==0)
		{
		rt_dma_copy_t cp4;
			
		rt_dma_memcpy(
#ifdef Q8
			wt_layer3_L2, // ext
			wt_layer3_L1, // loc
			OUT_NEURONS_3 * IFM_H_FC_3 * IFM_H_FC_3 *IFM_CH_FC_3, // size
#endif
			RT_DMA_DIR_EXT2LOC, // dir
			0, // merge
			&cp4 // copy
			);
    	rt_dma_wait(&cp4);
	
	/*BIAS LAYER 3*/	
	for (int z= 0; z < OUT_NEURONS_3; z++)
		{
			bias_layer3_L1[z] = 0;
		}
	
	}
	/*EXECUTION OF LAYER 3*/
	rt_team_barrier();

#ifdef PROFILING
	printf("layer 3 profiling\n");
	//INIT_PROFILING();
	START_PROFILING();
	/*Sync*/
	rt_team_barrier();

#endif
	/*LAYER 3*/
#ifdef Q8
	pulp_nn_linear_int8(out_layer2_L1, wt_layer3_L1, IFM_H_FC_3 * IFM_H_FC_3* IFM_CH_FC_3,
								OUT_NEURONS_3, BIAS_SHIFT_FC_3, OUT_QF_FC_3, bias_layer3_L1, out_layer3_L1);
	pulp_nn_relu_int8(out_layer3_L1,1,OUT_NEURONS_3);
#endif

#ifdef PROFILING
	STOP_PROFILING();
#endif
	/*LAYER 4*/
	/*charge the second weight layer*/
		if(rt_core_id()==0)
		{	
		rt_dma_copy_t cp5;
		
		rt_dma_memcpy(
#ifdef Q8
			wt_layer4_L2, // ext
			wt_layer4_L1, // loc
			OUT_NEURONS_4 * IFM_H_FC_4 * IFM_H_FC_4 *IFM_CH_FC_4, // size
#endif
			RT_DMA_DIR_EXT2LOC, // dir
			0, // merge
			&cp5 // copy
			);
    	rt_dma_wait(&cp5);
	
	/*BIAS LAYER 4*/	
	for (int z= 0; z < OUT_NEURONS_4; z++)
		{
			bias_layer4_L1[z] = 0;
		}
	
	}
	/*EXECUTION OF LAYER 4*/
	rt_team_barrier();

#ifdef PROFILING
	//INIT_PROFILING();i
	printf("layer 4 profiling\n");
	START_PROFILING();

	/*Sync*/
	rt_team_barrier();
	/*LAYER 4*/
#endif
#ifdef Q8
	pulp_nn_linear_int8(out_layer3_L1, wt_layer4_L1, IFM_H_FC_4 * IFM_H_FC_4* IFM_CH_FC_4,
								OUT_NEURONS_4, BIAS_SHIFT_FC_4, OUT_QF_FC_4, bias_layer4_L1, out_layer4_L1);
#endif

#ifdef PROFILING
	STOP_PROFILING();
#endif
	
#ifdef PROFILING_TOT
	STOP_PROFILING();
#endif


	if(rt_core_id()==0)
	{

		rt_dma_copy_t cp6;
		rt_dma_memcpy(
		#ifdef Q8
			out_L2, // ext
			out_layer4_L1, // loc
			OUT_NEURONS_4, // size
		#endif
			RT_DMA_DIR_LOC2EXT, // dir
			0, // merge
			&cp6 // copy
			);
	//    rt_dma_wait(&cp6); //we don't have to wait because out_L2 are statically allocated in L2 --> no FREE problem
	}
#ifdef CHECKLAYER
	    int errors=0;
	    for (int i=0; i< OUT_NEURONS_4; i++)
	    {
	    	if(out_L2[i] != checksum_layer4[i])
	    	{

#ifdef PRINT
	    		printf("exp: %X, real: %X, index: %d\n",checksum_layer4[i], out_L2[i],i );
#endif
	    		errors ++;
	    	}
	    }

#ifdef PRINT
	    if(errors!=0)
	    	printf("check failed. number of errors: %d \n", errors);
	    else
	    	printf("check ok. The layer has been tested successfully. \n");
#endif
				
#endif

	//exit from core0
	printf("post-free\n");
return (0);

}

