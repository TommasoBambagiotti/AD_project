#ifndef __LAYER3__
#define __LAYER3__

#include "rt/rt_api.h"

#define IFM_CH_FC_3               4
#define IFM_H_FC_3                1
#define OUT_NEURONS_3             4
#define OUT_QF_FC_3               4
#define BIAS_SHIFT_FC_3   0

RT_L2_DATA const char input_data_layer3_L2[IFM_CH_FC_3 * IFM_H_FC_3 * IFM_H_FC_3] = {0};

RT_L2_DATA const char wt_layer3_L2[OUT_NEURONS_3 * IFM_H_FC_3 * IFM_H_FC_3 * IFM_CH_FC_3] = {0};

RT_L2_DATA const int8_t checksum_layer3[OUT_NEURONS_3] = {0};


#endif
