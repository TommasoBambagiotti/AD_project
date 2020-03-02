PULP_APP = main_test

PULP_APP_FC_SRCS  = main_test.c #test_layers.c
#PULP_APP_CL_SRCS = test_layers.c
#PULP_APP_CL_SRCS += $(wildcard ./../kernels/*.c)
PULP_APP_FC_SRCS += test_AE.c $(wildcard ./../kernels/*.c)

prof=1
check=1

ifndef cores
cores=1
else
cores = $(cores)
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
PULP_CFLAGS = -DPROFILING
endif

ifeq ($(check),1)
PULP_CFLAGS += -DCHECKLAYER
endif

PULP_CFLAGS += -DTEST=$(test) -DNUM_CORES=$(cores) -DQ$(Q) -O3  -Iinc -w -I../include
PULP_LDFLAGS = -lm -L$(PWD) 



include $(GAP_SDK_HOME)/tools/rules/pulp_rules.mk
