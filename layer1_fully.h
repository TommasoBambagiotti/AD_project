#ifndef __LAYER1__
#define __LAYER1__

#include "rt/rt_api.h"

#define IFM_CH_FC_1               1025
#define IFM_H_FC_1                1
#define OUT_NEURONS_1             8
#define OUT_QF_FC_1               4
#define BIAS_SHIFT_FC_1   0

RT_L2_DATA const char input_data_L2[IFM_CH_FC_1 * IFM_H_FC_1 * IFM_H_FC_1] = {0};

RT_L2_DATA const char wt_layer1_L2[OUT_NEURONS_1 * IFM_H_FC_1 * IFM_H_FC_1 * IFM_CH_FC_1] = {0};

RT_L2_DATA const int8_t checksum_layer1[OUT_NEURONS_1] = {0};


#endif
