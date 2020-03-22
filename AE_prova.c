#include "rt/rt_api.h"
#include "stats.h"

#include "pulp-nn/include/pulp_nn.h"
#include "int8_fully_layer_parameters.h"

/*#### DATA ####*/
/*
RT_FC_SHARED_DATA int8_t out_L2[(OUT_NEURONS_1)];
RT_L1_DATA int8_t input_data_L1[(IFM_H_FC_1 * IFM_H_FC_1 * IFM_CH_FC_1)];
RT_L1_DATA int8_t wt_L1[(OUT_NEURONS_1 * IFM_H_FC_1 * IFM_H_FC_1 * IFM_CH_FC_1)];
RT_L1_DATA int8_t bias_L1[OUT_NEURONS_1];
RT_L1_DATA int8_t out_L1[(OUT_NEURONS_1)];
*/

RT_L1_DATA int8_t *input_data_L1, *wt_L1, *out_L1, *out_L2, *bias_L1;


int test_layers(rt_perf_t *perf)
{	
	/*
	 rt_alloc_req_t *input_alloc, *wt_alloc, *bias_alloc, *out_L1_alloc, *out_L2_alloc;
	 rt_free_req_t *input_free, *wt_free, *bias_free, *out_L1_free, *out_L2_free;


	printf("1\n");	
	rt_alloc_cluster(RT_ALLOC_CL_DATA, (sizeof(char)*(IFM_H_FC_1 * IFM_H_FC_1 * IFM_CH_FC_1)), input_alloc);
	if ((input_data_L1 = rt_alloc_cluster_wait 	( 	input_alloc	)) == NULL ) return(-1); 	

	printf("2\n");	
	rt_alloc_cluster(RT_ALLOC_CL_DATA, sizeof(char)*(OUT_NEURONS_1 * IFM_H_FC_1 * IFM_H_FC_1 * IFM_CH_FC_1), wt_alloc);
	if ((wt_L1 = rt_alloc_cluster_wait 	( 	wt_alloc	))==NULL) return(-1); 	

	printf("3\n");	
	rt_alloc_cluster(RT_ALLOC_CL_DATA, sizeof(char)*(OUT_NEURONS_1), bias_alloc);
	if ((bias_L1 = rt_alloc_cluster_wait 	( 	bias_alloc	)) == NULL) return(-1); 	

	printf("4\n");	
	rt_alloc_cluster(RT_ALLOC_CL_DATA, sizeof(char)*(OUT_NEURONS_1), out_L1_alloc);
	if ((out_L1 = rt_alloc_cluster_wait 	( 	out_L1_alloc	)) == NULL) return(-1); 	

	printf("5\n");	
	rt_alloc_cluster(RT_ALLOC_FC_DATA, sizeof(char)*(OUT_NEURONS_1), out_L2_alloc);
	if ((out_L2 = rt_alloc_cluster_wait 	( 	out_L2_alloc	)) == NULL) return(-1); 	
	*/

	input_data_L1 = rt_alloc(RT_ALLOC_CL_DATA, (sizeof(char)*(IFM_H_FC_1 * IFM_H_FC_1 * IFM_CH_FC_1)));
	wt_L1 = rt_alloc(RT_ALLOC_CL_DATA, (sizeof(char)*(OUT_NEURONS_1 * IFM_H_FC_1 * IFM_H_FC_1 * IFM_CH_FC_1)));
	bias_L1 = rt_alloc(RT_ALLOC_CL_DATA, (sizeof(char)*(OUT_NEURONS_1)));
	out_L1 = rt_alloc(RT_ALLOC_CL_DATA, (sizeof(char)*(OUT_NEURONS_1)));
	out_L2 = rt_alloc(RT_ALLOC_FC_DATA, (sizeof(char)*(OUT_NEURONS_1)));	



	if(rt_core_id()==0) //core 0 stuff, not FC
	{
		/* transfer layer weights and activations from L2 to L1 memory through DMA */
		rt_dma_copy_t cp1;
		rt_dma_copy_t cp2;

		printf("activations transfer from L2 to L1\n");
    	rt_dma_memcpy(
#ifdef Q8
			input_data_fully_int8_L2, // ext
			input_data_L1, // loc
			IFM_H_FC_1 * IFM_H_FC_1 *IFM_CH_FC_1, // size
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
			wt_L1, // loc
			OUT_NEURONS_1 * IFM_H_FC_1 * IFM_H_FC_1 *IFM_CH_FC_1, // size
#endif
			RT_DMA_DIR_EXT2LOC, // dir
			0, // merge
			&cp2 // copy
			);
    	rt_dma_wait(&cp2);

#ifdef Q8
    	//BIAS
    	for (int k= 0; k < OUT_NEURONS_1; k++)
		{
			bias_L1[k] = 0;
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
	pulp_nn_linear_int8(input_data_L1, wt_L1, IFM_H_FC_1 * IFM_H_FC_1 * IFM_CH_FC_1,
									OUT_NEURONS_1, BIAS_SHIFT_FC_1, OUT_QF_FC_1, bias_L1, out_L1);
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
			out_L2, // ext
			out_L1, // loc
			OUT_NEURONS_1, // size
		#endif
			RT_DMA_DIR_LOC2EXT, // dir
			0, // merge
			&cp3 // copy
			);
	    rt_dma_wait(&cp3);

	    int errors=0;
	    for (int i=0; i< OUT_NEURONS_1; i++)
	    {
	    	if(out_L2[i] != checksum_layer1[i])
	    	{
	    		printf("exp: %X, real: %X, index: %d\n",checksum_layer1[i], out_L2[i],i );
	    		errors ++;
	    	}
	    }

	    if(errors!=0)
	    	printf("check failed. number of errors: %d \n", errors);
	    else
	    	printf("check ok. The layer has been tested successfully. \n");
	}
#endif

	rt_free(RT_ALLOC_CL_DATA,input_data_L1,(sizeof(char)*(IFM_H_FC_1 * IFM_H_FC_1 * IFM_CH_FC_1)));
	rt_free(RT_ALLOC_CL_DATA,wt_L1,(sizeof(char)*(OUT_NEURONS_1*IFM_H_FC_1 * IFM_H_FC_1 * IFM_CH_FC_1)));
	rt_free(RT_ALLOC_CL_DATA,bias_L1,(sizeof(char)*(OUT_NEURONS_1)));
	rt_free(RT_ALLOC_CL_DATA,out_L1,(sizeof(char)*(OUT_NEURONS_1)));
	rt_free(RT_ALLOC_FC_DATA,out_L2,(sizeof(char)*(OUT_NEURONS_1)));

/*
	rt_free_cluster(RT_ALLOC_CL_DATA, input_data_L1, sizeof(char)*(IFM_H_FC_1 * IFM_H_FC_1 * IFM_CH_FC_1), input_free);
        rt_free_cluster_wait   (       input_free      );
        rt_free_cluster(RT_ALLOC_CL_DATA, wt_L1, sizeof(char)*(OUT_NEURONS_1 * IFM_H_FC_1 * IFM_H_FC_1 * IFM_CH_FC_1), wt_free);
        rt_free_cluster_wait   (       wt_free      );      
        rt_free_cluster(RT_ALLOC_CL_DATA, bias_L1, sizeof(char)*(OUT_NEURONS_1), bias_free);
        rt_free_cluster_wait   (       bias_free    );      
        rt_free_cluster(RT_ALLOC_CL_DATA, out_L1, sizeof(char)*(OUT_NEURONS_1), out_L1_free);
        rt_free_cluster_wait   (       out_L1_free     );
	rt_free_cluster(RT_ALLOC_FC_DATA, out_L2, sizeof(char)*(OUT_NEURONS_1), out_L2_free);
        rt_free_cluster_wait   (       out_L2_free     ); 
*/
      
return (0);
}
