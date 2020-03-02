#include <stdio.h>
#include "pulp.h"

#include "barrier.h"

#include "stats.h"

#define FFT2_SCALEDOWN	1

#define Min(x, y)	(((x)<(y))?(x):(y))

static inline v2s cplxmulsdiv2(v2s x, v2s	y)
{
   return (v2s){((signed short) ((((int) (x)[0]*(int) (y)[0]) - ((int) (x)[1]*(int) (y)[1]))>>15))>>1, ((signed short) ((((int) (x)[0]*(int) (y)[1]) + ((int) (x)[1]*(int) (y)[0]))>>15))>>1};
}

void Radix2FFT_DIF_Par(signed short *__restrict__ Data, signed short *__restrict__ Twiddles, int N_FFT2)

{
// #define Min(a, b)	(((a)<(b))?(a):(b))
  int iLog2N  = __builtin_pulp_fl1(N_FFT2);
  int iCnt1, iCnt2, iCnt3,
            iQ,    iL,    iM,
            iA,    iB;
  v2s *CoeffV = (v2s *) Twiddles;
  v2s *DataV  = (v2s *) Data;
        unsigned int CoreId;
        int First, Last, Chunk;

  int CNT = 0;
  int CNT1 = 0;

      CoreId = rt_core_id();


      iL = 1;
      iM = N_FFT2 / 2;

  //Chunk = ((iLog2N-1)/NUM_CORES); First =  CoreId*Chunk; Last = Min((First+Chunk), (iLog2N-1));

         // INIT_PAR_STATS();

      //for (iCnt1 = 0; iCnt1 < (iLog2N-1); iCnt1++) {
      for (iCnt1 = 0; iCnt1 < Min((iLog2N-1), 4); iCnt1++) {
          Chunk = (iM/NUM_CORES); First =  CoreId*Chunk; Last = First+Chunk;
          // iQ = 0;
          iQ = First*iL;

          //START_PAR_STATS();

          // for (iCnt2 = 0; iCnt2 < iM; iCnt2++) {
          for (iCnt2 = First; iCnt2 < Last; iCnt2++) {
                v2s W = CoeffV[iQ];
                //v2s A = DataV[iCnt2];
                //v2s B = DataV[iCnt2+iM];
                iA = iCnt2;
                for (iCnt3 = 0; iCnt3 < iL; iCnt3++) {
                    v2s Tmp;
                    iB = iA + iM;
                    Tmp       = (DataV[iA] - DataV[iB]);
                    DataV[iA] = (DataV[iA] + DataV[iB]) >> (v2s) {FFT2_SCALEDOWN, FFT2_SCALEDOWN};
                    //Tmp       = (A - B);
                    //DataV[iA] = (A + B) >> (v2s) {FFT2_SCALEDOWN, FFT2_SCALEDOWN};
                    //A = DataV[iA + 2*iM]; B = DataV[iA + 3*iM];
                    DataV[iB] = cplxmulsdiv2(Tmp, W);
                    iA = iA + 2 * iM;
                }
                iQ += iL;
          }
          #define BPAR(n) ((n)/((((n)+PAR_FACTOR-1)/PAR_FACTOR)))
          //STOP_PAR_STATS();
          #undef BPAR
          iL <<= 1;
          iM >>= 1;
      }

  // Synchronize all cores for current layer of the trellis
  BARRIER();

  for (iCnt1 = 4; iCnt1 < (iLog2N-1); iCnt1++) {
      iQ = 0;
      for (iCnt2 = 0; iCnt2 < iM; iCnt2++) {
        v2s W = CoeffV[iQ];
        iA = iCnt2;
        Chunk = (iL/NUM_CORES); First =  CoreId*Chunk; Last = First+Chunk;



       // START_PAR_STATS();
        // for (iCnt3 = 0; iCnt3 < iL; iCnt3++) {
        for (iCnt3 = First; iCnt3 < Last; iCnt3++) {
               v2s Tmp;
                iB = iA + iM;
                Tmp       = (DataV[iA] - DataV[iB]);
                DataV[iA] = (DataV[iA] + DataV[iB]) >> (v2s) {FFT2_SCALEDOWN, FFT2_SCALEDOWN};
                DataV[iB] = cplxmulsdiv2(Tmp, W);
                iA = iA + 2 * iM;
         }
         #define BPAR(n) ((n)/((((n)+PAR_FACTOR-1)/PAR_FACTOR)))
         //STOP_PAR_STATS();
         #undef BPAR

         iQ += iL;
      }
      iL <<= 1;
      iM >>= 1;
  }
  // Synchronize all cores for current layer of the trellis
  BARRIER();

        iA = 0;
    /* Last Layer: W = (1, 0) */
    Chunk = ((N_FFT2>>1)/NUM_CORES); First =  CoreId*Chunk; Last = Min(First+Chunk, (N_FFT2>>1));

    //START_PAR_STATS();
    //for (iCnt3 = 0; iCnt3 < (N_FFT2>>1); iCnt3++) {
    for (iCnt3 = First; iCnt3 < Last; iCnt3++) {
      v2s Tmp;
      iB = iA + 1;
      Tmp = (DataV[iA] - DataV[iB]);
      DataV[iA] = (DataV[iA] + DataV[iB]);
      DataV[iB] = Tmp;
      iA = iA + 2;
    }
   #define BPAR(n) ((n)/((((n)+PAR_FACTOR-1)/PAR_FACTOR)))
   //STOP_PAR_STATS();
   #undef BPAR

  // Synchronize all cores for current layer of the trellis
  BARRIER();

// #undef Min
}

void Radix2FFT_DIF_Par_Fast(signed short *__restrict__ Data, signed short *__restrict__ Twiddles, int N_FFT2)

{
// #define Min(a, b)	(((a)<(b))?(a):(b))
  int iLog2N  = __builtin_pulp_fl1(N_FFT2);
  int iCnt1, iCnt2, iCnt3,
            iQ,    iL,    iM,
            iA,    iB;
  v2s *CoeffV = (v2s *) Twiddles;
  v2s *DataV  = (v2s *) Data;
        unsigned int CoreId;
        int First, Last, Chunk;

  int CNT = 0;
  int CNT1 = 0;

        CoreId = rt_core_id();


      iL = 1;
      iM = N_FFT2 / 2;

  // Layer 0
  Chunk = (iM/NUM_CORES); First =  CoreId*Chunk; Last = First+Chunk; iQ = First*iL;
        for (iCnt2 = First; iCnt2 < Last; iCnt2++) {
    v2s W = CoeffV[iQ], A = DataV[iCnt2], B = DataV[iCnt2+iM];
              iA = iCnt2;
              for (iCnt3 = 0; iCnt3 < iL; iCnt3++) { // 1
      v2s Tmp   = (A - B); DataV[iA] = (A + B) >> (v2s) {FFT2_SCALEDOWN, FFT2_SCALEDOWN};
      A = DataV[iA + 2*iM]; B = DataV[iA + 3*iM];
      DataV[iB] = cplxmulsdiv2(Tmp, W);
                  iA = iA + 2 * iM;
              }
          iQ += iL;
        }
        iL <<= 1; iM >>= 1;
  BARRIER();
  // Layer 1
  Chunk = (iM/NUM_CORES); First =  CoreId*Chunk; Last = First+Chunk; iQ = First*iL;
        for (iCnt2 = First; iCnt2 < Last; iCnt2++) {
    v2s W = CoeffV[iQ], A = DataV[iCnt2], B = DataV[iCnt2+iM];
              iA = iCnt2;
              for (iCnt3 = 0; iCnt3 < iL; iCnt3++) { // 2
      v2s Tmp   = (A - B); DataV[iA] = (A + B) >> (v2s) {FFT2_SCALEDOWN, FFT2_SCALEDOWN};
      A = DataV[iA + 2*iM]; B = DataV[iA + 3*iM];
      DataV[iB] = cplxmulsdiv2(Tmp, W);
                  iA = iA + 2 * iM;
              }
          iQ += iL;
        }
        iL <<= 1; iM >>= 1;
  BARRIER();
  // Layer 2
  Chunk = (iM/NUM_CORES); First =  CoreId*Chunk; Last = First+Chunk; iQ = First*iL;
        for (iCnt2 = First; iCnt2 < Last; iCnt2++) {
    v2s W = CoeffV[iQ], A = DataV[iCnt2], B = DataV[iCnt2+iM];
              iA = iCnt2;
              for (iCnt3 = 0; iCnt3 < iL; iCnt3++) { // 4
      v2s Tmp   = (A - B); DataV[iA] = (A + B) >> (v2s) {FFT2_SCALEDOWN, FFT2_SCALEDOWN};
      A = DataV[iA + 2*iM]; B = DataV[iA + 3*iM];
      DataV[iB] = cplxmulsdiv2(Tmp, W);
                  iA = iA + 2 * iM;
              }
          iQ += iL;
        }
        iL <<= 1; iM >>= 1;
  BARRIER();
  // Layer 3
  Chunk = (iM/NUM_CORES); First =  CoreId*Chunk; Last = First+Chunk; iQ = First*iL;
        for (iCnt2 = First; iCnt2 < Last; iCnt2++) {
    v2s W = CoeffV[iQ], A = DataV[iCnt2], B = DataV[iCnt2+iM];
              iA = iCnt2;
              for (iCnt3 = 0; iCnt3 < iL; iCnt3++) { // 8
      v2s Tmp   = (A - B); DataV[iA] = (A + B) >> (v2s) {FFT2_SCALEDOWN, FFT2_SCALEDOWN};
      A = DataV[iA + 2*iM]; B = DataV[iA + 3*iM];
      DataV[iB] = cplxmulsdiv2(Tmp, W);
                  iA = iA + 2 * iM;
              }
          iQ += iL;
        }
        iL <<= 1; iM >>= 1;
  BARRIER();

      for (iCnt1 = 4; iCnt1 < (iLog2N-1); iCnt1++) {
          iQ = 0;
          for (iCnt2 = 0; iCnt2 < iM; iCnt2++) {
      v2s W = CoeffV[iQ];
                iA = iCnt2;
      Chunk = (iL/NUM_CORES); First =  CoreId*Chunk; Last = First+Chunk;

                // for (iCnt3 = 0; iCnt3 < iL; iCnt3++) {
                for (iCnt3 = First; iCnt3 < Last; iCnt3++) {
        v2s Tmp;
                    iB = iA + iM;
        Tmp       = (DataV[iA] - DataV[iB]);
        DataV[iA] = (DataV[iA] + DataV[iB]) >> (v2s) {FFT2_SCALEDOWN, FFT2_SCALEDOWN};
        DataV[iB] = cplxmulsdiv2(Tmp, W);
                    iA = iA + 2 * iM;
                }
                iQ += iL;
          }
          iL <<= 1;
          iM >>= 1;
  }
  // Synchronize all cores for current layer of the trellis
  BARRIER();
        iA = 0;
  /* Last Layer: W = (1, 0) */
  Chunk = ((N_FFT2>>1)/NUM_CORES); First =  CoreId*Chunk; Last = Min(First+Chunk, (N_FFT2>>1));
        // for (iCnt3 = 0; iCnt3 < (N_FFT2>>1); iCnt3++) {
/*
        for (iCnt3 = First; iCnt3 < Last; iCnt3++) {
    v2s Tmp     = (DataV[iA] - DataV[iA+1]);
    DataV[iA]   = (DataV[iA] + DataV[iA+1]);
    DataV[iA+1] = Tmp;
                 iA = iA + 2;
  }
*/
  {
    v2s A = DataV[iA  ];
    v2s B = DataV[iA+1];
          for (iCnt3 = First; iCnt3 < Last; iCnt3+=2) {
      v2s A1 = DataV[iA+2];
      v2s B1 = DataV[iA+3];
      DataV[iA  ] = A + B;
      DataV[iA+1] = A - B;
      DataV[iA+3] = A1 + B1;
      DataV[iA+4] = A1 - B1;
                   iA = iA + 4;
      A = DataV[iA  ]; B = DataV[iA+1];
    }
  }
  // Synchronize all cores for current layer of the trellis
  BARRIER();

// #undef Min
}

static void SwapSamples_Par(v2s *__restrict__ Data, short *__restrict__ SwapTable, int Ni)

{
  int i;
        unsigned int CoreId;
        int First, Last, Chunk;

        CoreId = rt_core_id();

  Chunk = (Ni/NUM_CORES); First =  CoreId*Chunk; Last = Min(First+Chunk, Ni);
  // for (i = 0; i < Ni; i++) {
  for (i = First; i < Last; i++) {
    int SwapIndex = SwapTable[i];
    if (i < SwapIndex) {
      v2s S = Data[i];
      Data[i] = Data[SwapIndex]; Data[SwapIndex] = S;
    }
  }
  // Synchronize all cores for current layer of the trellis
  BARRIER();
}

void Radix2FFT_DIF_Parallel(signed short *__restrict__ Data, signed short *__restrict__ Twiddles, int N_FFT2, short *__restrict__ SwapTable, int Fast)

{
  if (Fast) Radix2FFT_DIF_Par_Fast(Data, Twiddles, N_FFT2);
  else Radix2FFT_DIF_Par(Data, Twiddles, N_FFT2);

  if (SwapTable) SwapSamples_Par(Data, SwapTable, N_FFT2);
}
