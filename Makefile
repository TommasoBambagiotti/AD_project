# Copyright (C) 2017 ETH Zurich, University of Bologna and GreenWaves Technologies
# All rights reserved.
#
# This software may be modified and distributed under the terms
# of the BSD license.  See the LICENSE file for details.

DIR=$(PWD)
cores=8	#pulp_nn variable
CORES=$(cores) #pulp_fft variable

#@@@ FFT DEFINE @@@

n_seg = 7 #number of segments in Whelc's alg.
nfft_seg = 2048 #number of point for every segment
nfft = 8192 #number of total fft point
in_dyn = 5 #number of decimal digits in fixed point representation for input data
w_dyn= 16 #number of decimal digits in fixed point representation for window point 
prof_pwelch = 0 # 0=DON'T DO PWELCH PROFILING; 1=DO PWELCH PROFILING

#@@@@@ FFT_LIB SECTION @@@@@

PULP_APP = test_fft
PULP_APP_FC_SRCS =main.c FFT_Lib.c pwelch.c FFT2_Par_Lib.c barrier.c #source code of Fabric Controller
PULP_CFLAGS = -O3 -g -I$(DIR) -DNUM_CORES=${CORES} -DN_SEG=${n_seg} -DNFFT_SEG=${nfft_seg} -DNFFT=${nfft} -DIN_DYN=${in_dyn} -DWINDOW_DYN=${w_dyn} -DBUILD_LUT # -DPRINT # -DPRINTF_UART #definition and include
LIBS += -lm #library
PULP_LDFLAGS += -L$(DIR)

ifeq ($(prof_pwelch),1)
PULP_CFLAGS += -DPROFILING -DPROF_PWELCH
endif


#PARRALLEL SECTION
ifeq (${BARRIER}, sw)
PULP_CFLAGS += -DSW_BARRIER
endif

ifeq (${BARRIER}, tas)
PULP_CFLAGS += -DTAS_BARRIER
endif

#PULP_CFLAGS += -DPARALLELIZABLE_STATS -DPARFACTOR=1



#@@@@PULP NN SECTION@@@@

#PULP_APP_CL_SRCS = test_layers.c
#PULP_APP_CL_SRCS += $(wildcard ./../kernels/*.c)
PULP_APP_FC_SRCS += test_AE_mod.c  $(wildcard ./pulp-nn/kernels/*.c)# test_AE.c 

prof=1
check=0


ifndef cores
cores=1
endif


ifndef test
test= 2
else
test = $(test)  // 1:convolution, 2:fully_connected, 3:max_pooling, 4:relu (not available yet)
endif

ifndef Q
Q =8
else
Q = $(Q)
endif

ifeq ($(prof),1)
PULP_CFLAGS += -DPROFILING
endif

ifeq ($(prof),2)
PULP_CFLAGS += -DPROFILING_TOT
endif

ifeq ($(check),1)
PULP_CFLAGS += -DCHECKLAYER
endif

PULP_CFLAGS += -DTEST=$(test) -DNUM_CORES=$(cores) -DQ$(Q)  -Iinc -w 
PULP_LDFLAGS += -lm 


#PLPBRIDGE_FLAGS += -f -hyper
include $(GAP_SDK_HOME)/tools/rules/pulp_rules.mk
