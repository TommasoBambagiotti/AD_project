#include "rt/rt_api.h"
#include "stats.h"

#include "pulp_nn.h"
#include "./layer_parameters/int8_fully_layer_parameters.h"

/*#### DATA ####*/
RT_FC_SHARED_DATA int8_t fully_int8_out_L2[(OUT_NEURONS)];
RT_L1_DATA int8_t input_data_fully_int8_L1[(IFM_H_FC * IFM_H_FC * IFM_CH_FC)];
RT_L1_DATA int8_t fully_wt_int8_L1[(OUT_NEURONS * IFM_H_FC * IFM_H_FC * IFM_CH_FC)];
RT_L1_DATA int8_t fully_bias_int8_L1[OUT_NEURONS];
RT_L1_DATA int8_t fully_int8_out_L1[(OUT_NEURONS)];



int test_layers(rt_perf_t *perf)
{

	if(rt_core_id()==0) //core 0 stuff, not FC
	{
		/* transfer layer weights and activations from L2 to L1 memory through DMA */
		rt_dma_copy_t cp1;
		rt_dma_copy_t cp2;

		printf("activations transfer from L2 to L1\n");
    	rt_dma_memcpy(
#ifdef Q8
			input_data_fully_int8_L2, // ext
			input_data_fully_int8_L1, // loc
			IFM_H_FC * IFM_H_FC *IFM_CH_FC, // size
#endif
			RT_DMA_DIR_EXT2LOC, // dir
			0, // merge
			&cp1 // copy
			);
    	rt_dma_wait(&cp1);

		printf("weights transfer from L2 to L1\n");
    	rt_dma_memcpy(
#ifdef Q8
			fully_wt_int8_L2, // ext
			fully_wt_int8_L1, // loc
			OUT_NEURONS * IFM_H_FC * IFM_H_FC *IFM_CH_FC, // size
#endif
			RT_DMA_DIR_EXT2LOC, // dir
			0, // merge
			&cp2 // copy
			);
    	rt_dma_wait(&cp2);

#ifdef Q8
    	//BIAS
    	for (int k= 0; k < OUT_NEURONS; k++)
		{
			fully_bias_int8_L1[k] = 0;
		}
#endif
	printf("end of transfer. Start Fully-connected kernel parallel exec \n");
	}
	rt_team_barrier();

	/* Execution of FC layer */

#ifdef PROFILING
	/* These functions allow for a complete profiling of the kernel execution */
	/* Defined in "stats.h" header. See file for complete info                */
	INIT_PROFILING();
	START_PROFILING();
#endif

	rt_team_barrier();

#ifdef Q8
	pulp_nn_linear_int8(input_data_fully_int8_L1, fully_wt_int8_L1, IFM_H_FC * IFM_H_FC * IFM_CH_FC,
									OUT_NEURONS, BIAS_SHIFT_FC, OUT_QF_FC, fully_bias_int8_L1, fully_int8_out_L1);
#endif

#ifdef PROFILING
	STOP_PROFILING();
#endif


#ifdef CHECKLAYER
	if(rt_core_id()==0)
	{
		rt_dma_copy_t cp3;
		rt_dma_memcpy(
		#ifdef Q8
			fully_int8_out_L2, // ext
			fully_int8_out_L1, // loc
			OUT_NEURONS, // size
		#endif
			RT_DMA_DIR_LOC2EXT, // dir
			0, // merge
			&cp3 // copy
			);
	    rt_dma_wait(&cp3);

	    int errors=0;
	    for (int i=0; i< OUT_NEURONS; i++)
	    {
	    	if(fully_int8_out_L2[i] != checksum_fully_int8[i])
	    	{
	    		printf("exp: %X, real: %X, index: %d\n",checksum_fully_int8[i], fully_int8_out_L2[i],i );
	    		errors ++;
	    	}
	    }

	    if(errors!=0)
	    	printf("check failed. number of errors: %d \n", errors);
	    else
	    	printf("check ok. The layer has been tested successfully. \n");
	}
#endif

return (0);
}
