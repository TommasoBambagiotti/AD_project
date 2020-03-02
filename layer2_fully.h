#ifndef __LAYER2__
#define __LAYER2__

#include "rt/rt_api.h"

#define IFM_CH_FC_2               8
#define IFM_H_FC_2                1
#define OUT_NEURONS_2             4
#define OUT_QF_FC_2               4
#define BIAS_SHIFT_FC_2   0

RT_L2_DATA const char input_data_layer2_L2[IFM_CH_FC_2 * IFM_H_FC_2 * IFM_H_FC_2] = {0};

RT_L2_DATA const char wt_layer2_L2[OUT_NEURONS_2 * IFM_H_FC_2 * IFM_H_FC_2 * IFM_CH_FC_2] = {0};

RT_L2_DATA const int8_t checksum_layer2[OUT_NEURONS_2] = {0};


#endif
