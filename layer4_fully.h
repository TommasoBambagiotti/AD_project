#ifndef __LAYER4__
#define __LAYER4__

#include "rt/rt_api.h"

#define IFM_CH_FC_4               4
#define IFM_H_FC_4                1
#define OUT_NEURONS_4             1025
#define OUT_QF_FC_4               4
#define BIAS_SHIFT_FC_4   0

RT_L2_DATA const char input_data_layer4_L2[IFM_CH_FC_4 * IFM_H_FC_4 * IFM_H_FC_4] = {0};

RT_L2_DATA const char wt_layer4_L2[OUT_NEURONS_4 * IFM_H_FC_4 * IFM_H_FC_4 * IFM_CH_FC_4] = {0};

RT_L2_DATA const int8_t checksum_layer4[OUT_NEURONS_4] = {0};


#endif
