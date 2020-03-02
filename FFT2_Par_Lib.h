#ifndef __FFT_LIB_PAR_H
#define __FFT_LIB_PAR_H

#define FFT2_SAMPLE_DYN 4

void Radix2FFT_DIF_Parallel(signed short *__restrict__ Data, signed short *__restrict__ Twiddles, int N_FFT2, short *__restrict__ SwapTable, int Fast);

#endif

