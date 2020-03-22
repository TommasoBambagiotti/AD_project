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

RT_L1_DATA int8_t *input_data_L1, *wt_L1, *out_L1,  *bias_L1;
RT_FC_SHARED_DATA int8_t *out_L2; 

int test_layers_1(rt_perf_t *perf)
{
	input_data_L1 = rt_alloc(RT_ALLOC_CL_DATA, (sizeof(char)*(IFM_H_FC_1 * IFM_H_FC_1 * IFM_CH_FC_1)));
        wt_L1 = rt_alloc(RT_ALLOC_CL_DATA, (sizeof(char)*(OUT_NEURONS_1 * IFM_H_FC_1 * IFM_H_FC_1 * IFM_CH_FC_1)));
        bias_L1 = rt_alloc(RT_ALLOC_CL_DATA, (sizeof(char)*(OUT_NEURONS_1)));
        out_L1 = rt_alloc(RT_ALLOC_CL_DATA, (sizeof(char)*(OUT_NEURONS_1)));
        out_L2 = rt_alloc(RT_ALLOC_FC_DATA, (sizeof(char)*(OUT_NEURONS_1)));


#ifdef PRINT
	printf("PULP PARALLEL 1st LAYER\n");
#endif
	if(rt_core_id()==0) //core 0 stuff, not FC
	{
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

#ifdef PRINT
	printf("end of transfer. Start Fully-connected kernel parallel exec \n");
#endif
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
	pulp_nn_linear_int8(input_data_L1, wt_L1, IFM_H_FC_1 * IFM_H_FC_1* IFM_CH_FC_1,
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

#ifdef PRINT
	    		printf("exp: %X, real: %X, index: %d\n",checksum_layer1[i], out_L2[i],i );
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
	}
#endif

        rt_free(RT_ALLOC_CL_DATA,input_data_L1,(sizeof(char)*(IFM_H_FC_1 * IFM_H_FC_1 * IFM_CH_FC_1)));
        rt_free(RT_ALLOC_CL_DATA,wt_L1,(sizeof(char)*(OUT_NEURONS_1*IFM_H_FC_1 * IFM_H_FC_1 * IFM_CH_FC_1)));
        rt_free(RT_ALLOC_CL_DATA,bias_L1,(sizeof(char)*(OUT_NEURONS_1)));
        rt_free(RT_ALLOC_CL_DATA,out_L1,(sizeof(char)*(OUT_NEURONS_1)));
        rt_free(RT_ALLOC_FC_DATA,out_L2,(sizeof(char)*(OUT_NEURONS_1)));
return (0);
}

int test_layers_2(rt_perf_t *perf)
{

	input_data_L1 = rt_alloc(RT_ALLOC_CL_DATA, (sizeof(char)*(IFM_H_FC_2 * IFM_H_FC_2 * IFM_CH_FC_2)));
        wt_L1 = rt_alloc(RT_ALLOC_CL_DATA, (sizeof(char)*(OUT_NEURONS_2 * IFM_H_FC_2 * IFM_H_FC_2 * IFM_CH_FC_2)));
        bias_L1 = rt_alloc(RT_ALLOC_CL_DATA, (sizeof(char)*(OUT_NEURONS_2)));
        out_L1 = rt_alloc(RT_ALLOC_CL_DATA, (sizeof(char)*(OUT_NEURONS_2)));
        out_L2 = rt_alloc(RT_ALLOC_FC_DATA, (sizeof(char)*(OUT_NEURONS_2)));



#ifdef PRINT
	printf("PULP PARALLEL 2nd LAYER\n");
#endif
	if(rt_core_id()==0) //core 0 stuff, not FC
	{
		/* transfer layer weights and activations from L2 to L1 memory through DMA */
		rt_dma_copy_t cp1;
		rt_dma_copy_t cp2;

#ifdef PRINT
		printf("activations transfer from L2 to L1\n");
#endif
    	rt_dma_memcpy(
#ifdef Q8
			input_data_layer2_L2, // ext
			input_data_L1, // loc
			IFM_H_FC_2 * IFM_H_FC_2 *IFM_CH_FC_2, // size
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
			wt_layer2_L2, // ext
			wt_L1, // loc
			OUT_NEURONS_2 * IFM_H_FC_2 * IFM_H_FC_2 *IFM_CH_FC_2, // size
#endif
			RT_DMA_DIR_EXT2LOC, // dir
			0, // merge
			&cp2 // copy
			);
    	rt_dma_wait(&cp2);

#ifdef Q8
    	//BIAS
    	for (int k= 0; k < OUT_NEURONS_2; k++)
		{
			bias_L1[k] = 0;
		}
#endif

#ifdef PRINT
	printf("end of transfer. Start Fully-connected kernel parallel exec \n");
#endif
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
	pulp_nn_linear_int8(input_data_L1, wt_L1, IFM_H_FC_2 * IFM_H_FC_2 * IFM_CH_FC_2,
									OUT_NEURONS_2, BIAS_SHIFT_FC_2, OUT_QF_FC_2, bias_L1, out_L1);
	pulp_nn_relu_int8(out_L1,1,OUT_NEURONS_2);


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
			OUT_NEURONS_2, // size
		#endif
			RT_DMA_DIR_LOC2EXT, // dir
			0, // merge
			&cp3 // copy
			);
	    rt_dma_wait(&cp3);

	    int errors=0;
	    for (int i=0; i< OUT_NEURONS_2; i++)
	    {
	    	if(out_L2[i] != checksum_layer2[i])
	    	{

#ifdef PRINT
	    		printf("exp: %X, real: %X, index: %d\n",checksum_layer2[i], out_L2[i],i );
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
	}
#endif




	rt_free(RT_ALLOC_CL_DATA,input_data_L1,(sizeof(char)*(IFM_H_FC_2 * IFM_H_FC_2 * IFM_CH_FC_2)));
        rt_free(RT_ALLOC_CL_DATA,wt_L1,(sizeof(char)*(OUT_NEURONS_2*IFM_H_FC_2 * IFM_H_FC_2 * IFM_CH_FC_2)));
        rt_free(RT_ALLOC_CL_DATA,bias_L1,(sizeof(char)*(OUT_NEURONS_2)));
        rt_free(RT_ALLOC_CL_DATA,out_L1,(sizeof(char)*(OUT_NEURONS_2)));
        rt_free(RT_ALLOC_FC_DATA,out_L2,(sizeof(char)*(OUT_NEURONS_2)));

}

int test_layers_3(rt_perf_t *perf)
{
	input_data_L1 = rt_alloc(RT_ALLOC_CL_DATA, (sizeof(char)*(IFM_H_FC_3 * IFM_H_FC_3 * IFM_CH_FC_3)));
        wt_L1 = rt_alloc(RT_ALLOC_CL_DATA, (sizeof(char)*(OUT_NEURONS_3 * IFM_H_FC_3 * IFM_H_FC_3 * IFM_CH_FC_3)));
        bias_L1 = rt_alloc(RT_ALLOC_CL_DATA, (sizeof(char)*(OUT_NEURONS_3)));
        out_L1 = rt_alloc(RT_ALLOC_CL_DATA, (sizeof(char)*(OUT_NEURONS_3)));
        out_L2 = rt_alloc(RT_ALLOC_FC_DATA, (sizeof(char)*(OUT_NEURONS_3)));


	
#ifdef PRINT
	printf("PULP PARALLEL 3rd LAYER\n");
#endif
	if(rt_core_id()==0) //core 0 stuff, not FC
	{
		/* transfer layer weights and activations from L2 to L1 memory through DMA */
		rt_dma_copy_t cp1;
		rt_dma_copy_t cp2;

#ifdef PRINT
		printf("activations transfer from L2 to L1\n");
#endif
    	rt_dma_memcpy(
#ifdef Q8
			input_data_layer3_L2, // ext
			input_data_L1, // loc
			IFM_H_FC_3 * IFM_H_FC_3 *IFM_CH_FC_3, // size
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
			wt_layer3_L2, // ext
			wt_L1, // loc
			OUT_NEURONS_3 * IFM_H_FC_3 * IFM_H_FC_3 *IFM_CH_FC_3, // size
#endif
			RT_DMA_DIR_EXT2LOC, // dir
			0, // merge
			&cp2 // copy
			);
    	rt_dma_wait(&cp2);

#ifdef Q8
    	//BIAS
    	for (int k= 0; k < OUT_NEURONS_3; k++)
		{
			bias_L1[k] = 0;
		}
#endif

#ifdef PRINT
	printf("end of transfer. Start Fully-connected kernel parallel exec \n");
#endif
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
	pulp_nn_linear_int8(input_data_L1, wt_L1, IFM_H_FC_3 * IFM_H_FC_3 * IFM_CH_FC_3,
									OUT_NEURONS_3, BIAS_SHIFT_FC_3, OUT_QF_FC_3, bias_L1, out_L1);
	pulp_nn_relu_int8(out_L1,1,OUT_NEURONS_3);
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
			OUT_NEURONS_3, // size
		#endif
			RT_DMA_DIR_LOC2EXT, // dir
			0, // merge
			&cp3 // copy
			);
	    rt_dma_wait(&cp3);

	    int errors=0;
	    for (int i=0; i< OUT_NEURONS_3; i++)
	    {
	    	if(out_L2[i] != checksum_layer3[i])
	    	{

#ifdef PRINT
	    		printf("exp: %X, real: %X, index: %d\n",checksum_layer3[i], out_L2[i],i );
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
	}

#endif

        rt_free(RT_ALLOC_CL_DATA,input_data_L1,(sizeof(char)*(IFM_H_FC_3 * IFM_H_FC_3 * IFM_CH_FC_3)));
        rt_free(RT_ALLOC_CL_DATA,wt_L1,(sizeof(char)*(OUT_NEURONS_3*IFM_H_FC_3 * IFM_H_FC_3 * IFM_CH_FC_3)));
        rt_free(RT_ALLOC_CL_DATA,bias_L1,(sizeof(char)*(OUT_NEURONS_3)));
        rt_free(RT_ALLOC_CL_DATA,out_L1,(sizeof(char)*(OUT_NEURONS_3)));
        rt_free(RT_ALLOC_FC_DATA,out_L2,(sizeof(char)*(OUT_NEURONS_3)));







return (0);
}

int test_layers_4(rt_perf_t *perf)
{
	input_data_L1 = rt_alloc(RT_ALLOC_CL_DATA, (sizeof(char)*(IFM_H_FC_4 * IFM_H_FC_4 * IFM_CH_FC_4)));
        wt_L1 = rt_alloc(RT_ALLOC_CL_DATA, (sizeof(char)*(OUT_NEURONS_4 * IFM_H_FC_4 * IFM_H_FC_4 * IFM_CH_FC_4)));
        bias_L1 = rt_alloc(RT_ALLOC_CL_DATA, (sizeof(char)*(OUT_NEURONS_4)));
        out_L1 = rt_alloc(RT_ALLOC_CL_DATA, (sizeof(char)*(OUT_NEURONS_4)));
        out_L2 = rt_alloc(RT_ALLOC_FC_DATA, (sizeof(char)*(OUT_NEURONS_4)));



#ifdef PRINT
	printf("PULP PARALLEL 1st LAYER\n");
#endif
	if(rt_core_id()==0) //core 0 stuff, not FC
	{
		/* transfer layer weights and activations from L2 to L1 memory through DMA */
		rt_dma_copy_t cp1;
		rt_dma_copy_t cp2;
#ifdef PRINT
		printf("activations transfer from L2 to L1\n");
#endif
    	rt_dma_memcpy(
#ifdef Q8
			input_data_layer4_L2, // ext
			input_data_L1, // loc
			IFM_H_FC_4 * IFM_H_FC_4 *IFM_CH_FC_4, // size
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
			wt_layer4_L2, // ext
			wt_L1, // loc
			OUT_NEURONS_4 * IFM_H_FC_4 * IFM_H_FC_4 *IFM_CH_FC_4, // size
#endif
			RT_DMA_DIR_EXT2LOC, // dir
			0, // merge
			&cp2 // copy
			);
    	rt_dma_wait(&cp2);

#ifdef Q8
    	//BIAS
    	for (int k= 0; k < OUT_NEURONS_4; k++)
		{
			bias_L1[k] = 0;
		}
#endif

#ifdef PRINT
	printf("end of transfer. Start Fully-connected kernel parallel exec \n");
#endif
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
	pulp_nn_linear_int8(input_data_L1, wt_L1, IFM_H_FC_4 * IFM_H_FC_4* IFM_CH_FC_4,
									OUT_NEURONS_4, BIAS_SHIFT_FC_4, OUT_QF_FC_4, bias_L1, out_L1);

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
			OUT_NEURONS_4, // size
		#endif
			RT_DMA_DIR_LOC2EXT, // dir
			0, // merge
			&cp3 // copy
			);
	    rt_dma_wait(&cp3);

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
	}
#endif

        rt_free(RT_ALLOC_CL_DATA,input_data_L1,(sizeof(char)*(IFM_H_FC_4 * IFM_H_FC_4 * IFM_CH_FC_4)));
        rt_free(RT_ALLOC_CL_DATA,wt_L1,(sizeof(char)*(OUT_NEURONS_4*IFM_H_FC_4 * IFM_H_FC_4 * IFM_CH_FC_4)));
        rt_free(RT_ALLOC_CL_DATA,bias_L1,(sizeof(char)*(OUT_NEURONS_4)));
        rt_free(RT_ALLOC_CL_DATA,out_L1,(sizeof(char)*(OUT_NEURONS_4)));
        rt_free(RT_ALLOC_FC_DATA,out_L2,(sizeof(char)*(OUT_NEURONS_4)));

}
return (0);
}









