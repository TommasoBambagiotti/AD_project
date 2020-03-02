#ifndef _FFTLIB_H
#define _FFTLIB_H
#include "pulp.h"


void Radix4FFT_DIT_Scalar(signed short *__restrict__ Data, signed short *__restrict__ Twiddles, int N_FFT4, unsigned int Inverse);
void Radix4FFT_DIF_Scalar(signed short *__restrict__ Data, signed short *__restrict__ Twiddles, int N_FFT4, unsigned int Inverse);
void Radix2FFT_DIT_Scalar(signed short *__restrict__ Data, signed short *__restrict__ Twiddles, int N_FFT2);
void Radix2FFT_DIF_Scalar(signed short *__restrict__ Data, signed short *__restrict__ Twiddles, int N_FFT2);

void Radix4FFT_DIT(signed short *__restrict__ Data, signed short *__restrict__ Twiddles, int N_FFT4, unsigned int Inverse);
void Radix4FFT_DIF(signed short *__restrict__ Data, signed short *__restrict__ Twiddles, int N_FFT4, unsigned int Inverse);

void Radix2FFT_DIT(signed short *__restrict__ Data, signed short *__restrict__ Twiddles, int N_FFT2);
void Radix2FFT_DIF(signed short *__restrict__ Data, signed short *__restrict__ Twiddles, int N_FFT2);

void SwapSamples (v2s *__restrict__ Data, short *__restrict__ SwapTable, int Ni);

#ifdef BUILD_LUT
void SetupTwiddlesLUT(signed short *Twiddles, int Nfft, int Inverse);
void SetupScalarTwiddlesLUT(signed short *Twiddles, int Nfft, int Inverse);

void SetupR4SwapTable (short int *SwapTable, int Ni);
void SetupR2SwapTable (short int *SwapTable, int Ni);
#endif

#define FFT4_SAMPLE_DYN 5
#define FFT2_SAMPLE_DYN 4
#define FFT_TWIDDLE_DYN 15

#define FFT4_SCALEDOWN 2
#define FFT2_SCALEDOWN 1


#endif
