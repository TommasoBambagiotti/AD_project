#ifdef BUILD_LUT
#include <stdlib.h>
#include <math.h>
#endif

#include "pulp.h"
#include "FFT_Lib.h"

static inline v2s cplxmulsdiv2(v2s x, v2s	y)
{
   return (v2s){((signed short) ((((int) (x)[0]*(int) (y)[0]) - ((int) (x)[1]*(int) (y)[1]))>>15))>>1, ((signed short) ((((int) (x)[0]*(int) (y)[1]) + ((int) (x)[1]*(int) (y)[0]))>>15))>>1};
}

static inline v2s cplxmulsdiv4(v2s x, v2s	y)
{
   return (v2s) {((signed short) ((((int) (x)[0]*(int) (y)[0]) - ((int) (x)[1]*(int) (y)[1]))>>15))>>2, ((signed short) ((((int) (x)[0]*(int) (y)[1]) + ((int) (x)[1]*(int) (y)[0]))>>15))>>2};
}

static inline v2s cplxmuls(v2s x, v2s	y)
{
   return (v2s) {(signed short) ((((int) (x)[0]*(int) (y)[0]) - ((int) (x)[1]*(int) (y)[1]))>>15), (signed short) ((((int) (x)[0]*(int) (y)[1]) + ((int) (x)[1]*(int) (y)[0]))>>15)};
}

static inline v2s sub2rotmj(v2s x, v2s	y)
{
   return (v2s) {(x)[1]-(y)[1], (y)[0]-(x)[0]};
}
/*

  Without input/output reordering:

  Radix 2 DIT FFT on 16 points:   488     Cycles, Stall Penalty: 7.0%
  Radix 2 DIF FFT on 16 points:   480     Cycles, Stall Penalty: 7.1%

  Radix 2 DIT FFT on 32 points:   1102    Cycles, Stall Penalty: 7.8%
  Radix 2 DIF FFT on 32 points:   1057    Cycles, Stall Penalty: 8.2%

  Radix 2 DIT FFT on 64 points:   2476    Cycles, Stall Penalty: 8.4%
  Radix 2 DIF FFT on 64 points:   2338    Cycles, Stall Penalty: 8.9%

  Radix 2 DIT FFT on 128 points:  5530    Cycles, Stall Penalty: 8.8%
  Radix 2 DIF FFT on 128 points:  5171    Cycles, Stall Penalty: 9.5%

  Radix 2 DIT FFT on 256 points:  12264   Cycles, Stall Penalty: 9.1%
  Radix 2 DIF FFT on 256 points:  11396   Cycles, Stall Penalty: 9.9%

  Radix 2 DIT FFT on 512 points:  26998   Cycles, Stall Penalty: 9.3%
  Radix 2 DIF FFT on 512 points:  24981   Cycles, Stall Penalty: 10.2%

  Radix 2 DIT FFT on 1024 points: 59012   Cycles, Stall Penalty: 9.5%
  Radix 2 DIF FFT on 1024 points: 54438   Cycles, Stall Penalty: 10.4%

  Radix 4 DIT FFT on 16 points:   319     Cycles, Stall Penalty: 0.3%
  Radix 4 DIF FFT on 16 points:   314     Cycles, Stall Penalty: 0.6%

  Radix 4 DIT FFT on 64 points:   1543    Cycles, Stall Penalty: 0.1%
  Radix 4 DIF FFT on 64 points:   1495    Cycles, Stall Penalty: 0.1%

  Radix 4 DIT FFT on 256 points:  7579    Cycles, Stall Penalty: 0.0%
  Radix 4 DIF FFT on 256 points:  7344    Cycles, Stall Penalty: 0.0%

  Radix 4 DIT FFT on 1024 points: 36463   Cycles, Stall Penalty: 0.0%
  Radix 4 DIF FFT on 1024 points: 35465   Cycles, Stall Penalty: 0.0%

  Assuming 16Khz PCM.

  1/4 overlap between 2 adjacent fft:

    N FFT	Cycles/FFT	Total Cycles	Mips
  64	333.33	1495.00		498333.33	0.50
  256	83.33	7344.00		612000.00	0.61
  1024	20.83	35465.00	738854.17	0.74


  Without cplx mult:

  Radix 2 DIT FFT on 16 points:   727,    Stall Penalty: 6.8%
  Radix 2 DIF FFT on 16 points:   743,    Stall Penalty: 6.6%

  Radix 2 DIT FFT on 32 points:   1709,   Stall Penalty: 6.9%
  Radix 2 DIF FFT on 32 points:   1728,   Stall Penalty: 6.8%

  Radix 2 DIT FFT on 64 points:   3947,   Stall Penalty: 6.9%
  Radix 2 DIF FFT on 64 points:   3969,   Stall Penalty: 6.8%

  Radix 2 DIT FFT on 128 points:  8985,   Stall Penalty: 6.8%
  Radix 2 DIF FFT on 128 points:  9010,   Stall Penalty: 6.8%

  Radix 2 DIT FFT on 256 points:  20199,  Stall Penalty: 6.8%
  Radix 2 DIF FFT on 256 points:  20227,  Stall Penalty: 6.7%

  Radix 2 DIT FFT on 512 points:  44917,  Stall Penalty: 6.7%
  Radix 2 DIF FFT on 512 points:  44948,  Stall Penalty: 6.7%

  Radix 2 DIT FFT on 1024 points: 98947,  Stall Penalty: 6.6%
  Radix 2 DIF FFT on 1024 points: 98981,  Stall Penalty: 6.6%

  Radix 4 DIT FFT on 16 points:   489,    Stall Penalty: 0.0%
  Radix 4 DIF FFT on 16 points:   498,    Stall Penalty: 1.4%

  Radix 4 DIT FFT on 64 points:   2698,   Stall Penalty: 0.0%
  Radix 4 DIF FFT on 64 points:   2702,   Stall Penalty: 0.9%

  Radix 4 DIT FFT on 256 points:  13955,  Stall Penalty: 0.0%
  Radix 4 DIF FFT on 256 points:  13906,  Stall Penalty: 0.7%

  Radix 4 DIT FFT on 1024 points: 68892,  Stall Penalty: 0.0%
  Radix 4 DIF FFT on 1024 points: 68598,  Stall Penalty: 0.5%


*/


/* Number of points for the Radix4 FFT */

/*
  Input samples are in Q<FFT4_SAMPLE_DYN>
  Coeff are in Q<FFT_TWIDDLE_DYN>

  Out samples in Q<FFT4_SAMPLE_DYN>
*/

static void Radix4FFTKernel_Twiddle0(v2s *InOutA, v2s *InOutB, v2s *InOutC, v2s *InOutD, unsigned int Inverse)

{
  v2s A = *InOutA, B = *InOutB, C = *InOutC, D = *InOutD;

  /* Used for IFFT here.
     IFFT uses (1,  1,  1,  1)  FFT uses (1,  1,  1,  1)
         (1,  j, -1, -j)	       (1, -j, -1,  j)
         (1, -1,  1, -1)	       (1, -1,  1, -1)
         (1, -j, -1,  j)	       (1,  j, -1, -j)
     To use this code for FFT:
    *InOutA = ((A + C) +	           (B + D ));
    *InOutB = ((A - C) + sub2rotmj(B,  D));
    *InOutC = ((A + C) -	           (B + D ));
    *InOutD = ((A - C) - sub2rotmj(B,  D));
  */

  if (Inverse) {
    *InOutA = ((A + C) +	           (B + D));
    *InOutB = ((A - C) - sub2rotmj(B,  D));
    *InOutC = ((A + C) -	           (B + D));
    *InOutD = ((A - C) + sub2rotmj(B,  D));
  } else {
    *InOutA = ((A + C) +	           (B + D));
    *InOutB = ((A - C) + sub2rotmj(B,  D));
    *InOutC = ((A + C) -	           (B + D));
    *InOutD = ((A - C) - sub2rotmj(B,  D));
  }
}

static void Radix4FFTKernel_Twiddle0_Scaled(v2s *InOutA, v2s *InOutB, v2s *InOutC, v2s *InOutD, unsigned int Inverse)

{
  v2s A = *InOutA, B = *InOutB, C = *InOutC, D = *InOutD;

  /* Used for IFFT here.
     IFFT uses (1,  1,  1,  1)  FFT uses (1,  1,  1,  1)
         (1,  j, -1, -j)	       (1, -j, -1,  j)
         (1, -1,  1, -1)	       (1, -1,  1, -1)
         (1, -j, -1,  j)	       (1,  j, -1, -j)
     To use this code for FFT:
    *InOutA = ((A + C) +	           (B + D ));
    *InOutB = ((A - C) + sub2rotmj(B,  D));
    *InOutC = ((A + C) -	           (B + D ));
    *InOutD = ((A - C) - sub2rotmj(B,  D));
  */

  if (Inverse) {
    *InOutA = ((A + C) +	           (B + D)) >> (v2s) {FFT4_SCALEDOWN, FFT4_SCALEDOWN};
    *InOutB = ((A - C) - sub2rotmj(B,  D)) >> (v2s) {FFT4_SCALEDOWN, FFT4_SCALEDOWN};
    *InOutC = ((A + C) -	           (B + D)) >> (v2s) {FFT4_SCALEDOWN, FFT4_SCALEDOWN};
    *InOutD = ((A - C) + sub2rotmj(B,  D)) >> (v2s) {FFT4_SCALEDOWN, FFT4_SCALEDOWN};
  } else {
    *InOutA = ((A + C) +	           (B + D)) >> (v2s) {FFT4_SCALEDOWN, FFT4_SCALEDOWN};
    *InOutB = ((A - C) + sub2rotmj(B,  D)) >> (v2s) {FFT4_SCALEDOWN, FFT4_SCALEDOWN};
    *InOutC = ((A + C) -	           (B + D)) >> (v2s) {FFT4_SCALEDOWN, FFT4_SCALEDOWN};
    *InOutD = ((A - C) - sub2rotmj(B,  D)) >> (v2s) {FFT4_SCALEDOWN, FFT4_SCALEDOWN};
  }
}

/*
   Inputs are in Q<FFT4_SAMPLE_DYN>, Twidlle factors are in Q<FFT_TWIDDLE_DYN>,
   Outputs are in Q<FFT4_SAMPLE_DYN> therefore we need to shift by <FFT_TWIDDLE_DYN> to get a Q<FFT4_SAMPLE_DYN> number
   Note that output could be > 1.0 and go up to 8.0
*/

static void Radix4FFTKernelDIT(v2s *InOutA, v2s *InOutB, v2s *InOutC, v2s *InOutD, v2s W1, v2s W2, v2s W3, unsigned int Inverse)

{
        v2s B1, C1, D1;
  v2s A = *InOutA, B = *InOutB, C = *InOutC, D = *InOutD;

  B1 = cplxmuls(B, W1); C1 = cplxmuls(C, W2); D1 = cplxmuls(D, W3);

  if (Inverse) {
          *InOutA = ((A + C1) +               (B1 + D1)) >> (v2s) {FFT4_SCALEDOWN, FFT4_SCALEDOWN};
          *InOutB = ((A - C1) - sub2rotmj(B1,  D1)) >> (v2s) {FFT4_SCALEDOWN, FFT4_SCALEDOWN};
          *InOutC = ((A + C1) -               (B1 + D1)) >> (v2s) {FFT4_SCALEDOWN, FFT4_SCALEDOWN};
          *InOutD = ((A - C1) + sub2rotmj(B1,  D1)) >> (v2s) {FFT4_SCALEDOWN, FFT4_SCALEDOWN};
  } else {
          *InOutA = ((A + C1) +               (B1 + D1)) >> (v2s) {FFT4_SCALEDOWN, FFT4_SCALEDOWN};
          *InOutB = ((A - C1) + sub2rotmj(B1,  D1)) >> (v2s) {FFT4_SCALEDOWN, FFT4_SCALEDOWN};
          *InOutC = ((A + C1) -               (B1 + D1)) >> (v2s) {FFT4_SCALEDOWN, FFT4_SCALEDOWN};
          *InOutD = ((A - C1) - sub2rotmj(B1,  D1)) >> (v2s) {FFT4_SCALEDOWN, FFT4_SCALEDOWN};
  }
}

static void Radix4FFTKernelDIF(v2s *InOutA, v2s *InOutB, v2s *InOutC, v2s *InOutD,
                  v2s W1, v2s W2, v2s W3, unsigned int Inverse)

{
        v2s A1, B1, C1, D1;
  v2s A = *InOutA, B = *InOutB, C = *InOutC, D = *InOutD;

  if (Inverse) {
    A1 = ((A + C) +               (B + D)) >> (v2s) {FFT4_SCALEDOWN, FFT4_SCALEDOWN};
    B1 = ((A - C) - sub2rotmj(B,  D));
    C1 = ((A + C) -               (B + D));
    D1 = ((A - C) + sub2rotmj(B,  D));
  } else {
    A1 = ((A + C) +               (B + D)) >> (v2s) {FFT4_SCALEDOWN, FFT4_SCALEDOWN};
    B1 = ((A - C) + sub2rotmj(B,  D));
    C1 = ((A + C) -               (B + D));
    D1 = ((A - C) - sub2rotmj(B,  D));
  }

  B1 = cplxmulsdiv4(B1, W1); C1 = cplxmulsdiv4(C1, W2); D1 = cplxmulsdiv4(D1, W3);
        *InOutA = A1; *InOutB = B1; *InOutC = C1; *InOutD = D1;
}



/*
   Radix-4 Decimated in Time FFT. Input have to be digitally-reversed, output is naturally ordered.
   First stage uses the fact that twiddles are all (1, 0)
*/

static void __attribute__ ((__always_inline__)) Radix4FFT_DIT_Internal(signed short *__restrict__ Data,
                       signed short *__restrict__ Twiddles,
                       int N_FFT4,
                       unsigned int Inverse)

{
      int iCnt1, iCnt2, iCnt3;
        int iL,    iM,    iQ;
        int iA,    iB,    iC,     iD;
  int iLog4N  = __builtin_pulp_fl1(N_FFT4)>>1;
  v2s *DataV  = (v2s *) Data;
  v2s *CoeffV = (v2s *) Twiddles;

  iL = N_FFT4 / 4; iM = 1;
  iA = 0;
  for (iCnt3 = 0; iCnt3 < iL; ++iCnt3) {
    Radix4FFTKernel_Twiddle0_Scaled((v2s *) (DataV + iA       ), (v2s *) (DataV + iA +   iM),
                        (v2s *) (DataV + iA + 2*iM), (v2s *) (DataV + iA + 3*iM), Inverse);
    iA =  iA + 4 * iM;
  }
  iL = iL >> 2;
  iM = iM << 2;
      for (iCnt1 = 1; iCnt1 < iLog4N; ++iCnt1) {
          iQ = 0;
          for (iCnt2 = 0; iCnt2 < iM; ++iCnt2) {
      iA = iCnt2;
      v2s W1 = CoeffV[  iQ], W2 = CoeffV[2*iQ], W3 = CoeffV[3*iQ];
      for (iCnt3 = 0; iCnt3 < iL; ++iCnt3) {
        Radix4FFTKernelDIT((v2s *) (DataV + iA       ), (v2s *) (DataV + iA +   iM),
               (v2s *) (DataV + iA + 2*iM), (v2s *) (DataV + iA + 3*iM),
               W1, W2, W3, Inverse);

        iA =  iA + 4 * iM;
      }
      iQ += iL;
    }
    iL = iL >> 2;
    iM = iM << 2;
  }
}

void Radix4FFT_DIT(signed short *__restrict__ Data, signed short *__restrict__ Twiddles, int N_FFT4, unsigned int Inverse)

{
  if (Inverse) Radix4FFT_DIT_Internal(Data, Twiddles, N_FFT4, 1);
  else 	     Radix4FFT_DIT_Internal(Data, Twiddles, N_FFT4, 0);
}

/*
  Radix 4, Decimated in Frequency, fft.
  Input are natural order, output is digitally-reversed.
  The last stage is handled differently since twidlles are (1, 0) leading to a some cycle count reduction
*/

static void __attribute__ ((__always_inline__)) Radix4FFT_DIF_Internal(signed short *__restrict__ Data,
                       signed short *__restrict__ Twiddles,
                       int N_FFT4,
                       unsigned int Inverse)

{
      int iCnt1, iCnt2, iCnt3,
            iL,    iM,    iQ,
            iA,    iB,    iC,     iD;
  int iLog4N  = __builtin_pulp_fl1(N_FFT4)>>1;
  v2s *DataV  = (v2s *) Data;
  v2s *CoeffV = (v2s *) Twiddles;

      iL = 1;
      iM = N_FFT4 / 4;

      for (iCnt1 = 0; iCnt1 < (iLog4N-1); ++iCnt1) {
          iQ = 0;
          for (iCnt2 = 0; iCnt2 < iM; ++iCnt2) {
                iA = iCnt2;
      v2s W1 = CoeffV[  iQ], W2 = CoeffV[2*iQ], W3 = CoeffV[3*iQ];
                for (iCnt3 = 0; iCnt3 < iL; ++iCnt3) {
        Radix4FFTKernelDIF((v2s *) (DataV + iA       ), (v2s *) (DataV + iA + iM),
                                                   (v2s *) (DataV + iA + 2*iM), (v2s *) (DataV + iA + 3*iM),
                                                   W1, W2, W3, Inverse);

        iA = iA + 4 * iM;
                }
                iQ += iL;
          }
          iL <<= 2;
          iM >>= 2;
      }
  iA = 0; iL = (N_FFT4>>2);
  for (iCnt3 = 0; iCnt3 < iL; ++iCnt3) {
    Radix4FFTKernel_Twiddle0((v2s *) (DataV + iA       ), (v2s *) (DataV + iA +   iM),
                 (v2s *) (DataV + iA + 2*iM), (v2s *) (DataV + iA + 3*iM), Inverse);
    iA =  iA + 4 * iM;
  }
}

void Radix4FFT_DIF(signed short *__restrict__ Data, signed short *__restrict__ Twiddles, int N_FFT4, unsigned int Inverse)

{
  if (Inverse) Radix4FFT_DIF_Internal(Data, Twiddles, N_FFT4, 1);
  else 	     Radix4FFT_DIF_Internal(Data, Twiddles, N_FFT4, 0);
}

/*
  Radix 2, Decimated in Frequency, fft.
  Input are natural order, output is digitally-reversed.
  The last stage is handled differently since twidlles are (1, 0) leading to a some cycle count reduction
*/

void Radix2FFT_DIF(signed short *__restrict__ Data, signed short *__restrict__ Twiddles, int N_FFT2)

{
  int iLog2N  = __builtin_pulp_fl1(N_FFT2);
  int iCnt1, iCnt2, iCnt3,
            iQ,    iL,    iM,
            iA,    iB;
  v2s *CoeffV = (v2s *) Twiddles;
  v2s *DataV  = (v2s *) Data;

  iL = 1;
  iM = N_FFT2 / 2;

    for (iCnt1 = 0; iCnt1 < (iLog2N-1); iCnt1++) {
      iQ = 0;
      for (iCnt2 = 0; iCnt2 < iM; iCnt2++) {
        v2s W = CoeffV[iQ];
        iA = iCnt2;
        for (iCnt3 = 0; iCnt3 < iL; iCnt3++) {
          v2s Tmp;
          iB = iA + iM;
          Tmp = DataV[iA] - DataV[iB];
          DataV[iA] = (DataV[iA] + DataV[iB]) >> (v2s) {FFT2_SCALEDOWN, FFT2_SCALEDOWN};
          DataV[iB] = cplxmulsdiv2(Tmp, W);
          iA = iA + 2 * iM;
        }
        iQ += iL;
      }
      iL <<= 1;
      iM >>= 1;
    }
    iA = 0;

    /* Last Layer: W = (1, 0) */
    for (iCnt3 = 0; iCnt3 < (N_FFT2>>1); iCnt3++) {
      v2s Tmp;
      iB = iA + 1;
      Tmp = (DataV[iA] - DataV[iB]);
      DataV[iA] = (DataV[iA] + DataV[iB]);
      DataV[iB] = Tmp;
      iA = iA + 2;
    }
}

/*
   Radix-2 Decimated in Time FFT. Input have to be digitally-reversed, output is naturally ordered.
   First stage uses the fact that twiddles are all (1, 0)
*/
void Radix2FFT_DIT(signed short *__restrict__ Data, signed short *__restrict__ Twiddles, int N_FFT2)

{
  int iLog2N  = __builtin_pulp_fl1(N_FFT2);
  int iCnt1, iCnt2, iCnt3,
            iQ,    iL,    iM,
            iA,    iB;
  v2s *CoeffV = (v2s *) Twiddles;
  v2s *DataV  = (v2s *) Data;

      iL = N_FFT2 >> 1; iM = 1; iA = 0;
  /* First Layer: W = (1, 0) */
        for (iCnt3 = 0; iCnt3 < (N_FFT2>>1); iCnt3++) {
    v2s Tmp;
    iB = iA + iM;
    Tmp = DataV[iB];
    DataV[iB] = (DataV[iA] - Tmp)  >> (v2s) {FFT2_SCALEDOWN, FFT2_SCALEDOWN};
    DataV[iA] = (DataV[iA] + Tmp)  >> (v2s) {FFT2_SCALEDOWN, FFT2_SCALEDOWN};
                 iA = iA + 2;
  }
        iQ += iL; iL >>= 1; iM <<= 1;

      for (iCnt1 = 1; iCnt1 < iLog2N; ++iCnt1) {
          iQ = 0;
          for (iCnt2 = 0; iCnt2 < iM; ++iCnt2) {
      v2s W = CoeffV[iQ];
                iA = iCnt2;
                for (iCnt3 = 0; iCnt3 < iL; iCnt3++) {
        v2s Tmp, Tmp1;
                    iB = iA + iM;
        Tmp = cplxmuls(DataV[iB], W);
        Tmp1 = DataV[iA];

        DataV[iB] = (Tmp1 - Tmp) >> (v2s) {FFT2_SCALEDOWN, FFT2_SCALEDOWN};
        DataV[iA] = (Tmp1 + Tmp) >> (v2s) {FFT2_SCALEDOWN, FFT2_SCALEDOWN};
                    iA = iA + 2 * iM;
                }
                iQ += iL;
          }
          iL >>= 1;
          iM <<= 1;
      }
}

/* Reorder from natural indexes to digitally-reversed one. Uses a pre computed LUT */

void SwapSamples (v2s *__restrict__ Data, short *__restrict__ SwapTable, int Ni)

{
  int i;
	
  for (i = 0; i < Ni; i++) {
  
	  v2s S = Data[i];
	   
 int SwapIndex = SwapTable[i];
  
	
      Data[i] = Data[SwapIndex]; 
Data[SwapIndex] = S;
    	}
  }
	
}

#ifdef BUILD_LUT
/* Setup twiddles factors */

void SetupTwiddlesLUT(signed short *Twiddles, int Nfft, int Inverse)

{
  int i;
  v2s *P_Twid = (v2s *) Twiddles;

  /* Radix 4: 3/4 of the twiddles
     Radix 2: 1/2 of the twiddles
  */

  if (Inverse) {
    float Theta = (2*M_PI)/Nfft;
    for (i=0; i<Nfft; i++) {
      float Phi = Theta*i;
      P_Twid[i] = (v2s) {(short int) (cos(Phi)*((1<<FFT_TWIDDLE_DYN)-1)),
             (short int) (sin(Phi)*((1<<FFT_TWIDDLE_DYN)-1))};
      // Twiddles[2*i  ] = (short int) (cos(Phi)*((1<<FFT_TWIDDLE_DYN)-1));
      // Twiddles[2*i+1] = (short int) (sin(Phi)*((1<<FFT_TWIDDLE_DYN)-1));
    }
  } else {
    float Theta = (2*M_PI)/Nfft;
    for (i=0; i<Nfft; i++) {
      float Phi = Theta*i;
      P_Twid[i] = (v2s) {(short int) (cos(-Phi)*((1<<FFT_TWIDDLE_DYN)-1)),
             (short int) (sin(-Phi)*((1<<FFT_TWIDDLE_DYN)-1))};
      // Twiddles[2*i  ] = (short int) (cos(-Phi)*((1<<FFT_TWIDDLE_DYN)-1));
      // Twiddles[2*i+1] = (short int) (sin(-Phi)*((1<<FFT_TWIDDLE_DYN)-1));
    }
  }
}

/* Setup a LUT for digitally reversed indexed, base is 4 */

void SetupR4SwapTable (short int *SwapTable, int Ni)

{
  int iL, iM, i, j;
  int Log4N  = __builtin_pulp_fl1(Ni)>>1;

  iL = Ni / 4; iM = 1;
  SwapTable[0] = 0;

  for (i = 0; i < Log4N; ++i) {
    for (j = 0; j < iM; ++j) {
      SwapTable[    iM + j] = SwapTable[j] +     iL;
      SwapTable[2 * iM + j] = SwapTable[j] + 2 * iL;
      SwapTable[3 * iM + j] = SwapTable[j] + 3 * iL;
    }
    iL >>= 2; iM <<= 2;
  }
}

void SetupR2SwapTable (short int *SwapTable, int Ni)

{
  int i, j, iL, iM;
  int Log2N  = __builtin_pulp_fl1(Ni);

      iL = Ni / 2;
      iM = 1;
      SwapTable[0] = 0;

  for (i = 0; i < Log2N; ++i) {
          for (j = 0; j < iM; ++j) SwapTable[j + iM] = SwapTable[j] + iL;
          iL >>= 1; iM <<= 1;
      }
}
#endif
