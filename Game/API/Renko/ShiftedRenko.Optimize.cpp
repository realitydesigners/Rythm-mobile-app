#include "stdafx.h"
#include "AutoPtr.h"
#include "Rythm.h"
#include "ShiftedRenko.h"
#include <math.h>

#define CHECK_HIGHEST(val, eva) \
    if (dFEvaHighest < eva) {   \
        fEvaHighest = val;      \
        dFEvaHighest = eva;     \
    }

BOOLEAN CShiftedRenko::TryOptimize(FLOAT& fOptimizedSize, UINT uBoxNum, const SRythmListener& sListener)
{
    const UINT uValueNum = (UINT)sListener.cList.size();
    if (10 > uValueNum) {
        return FALSE;   // too few numbers for optimization
    }
    DOUBLE* pdValues = new DOUBLE[uValueNum];
    if (NULL == pdValues) {
        ASSERT(FALSE);
        return FALSE;
    }
    CAutoPtrArray<DOUBLE> cValues(pdValues);
    Renko::SShiftedRenko* psRenkos =  new Renko::SShiftedRenko[SHIFTED_RANKO_HISTORY_SIZE];
    if (NULL == psRenkos) {
        ASSERT(FALSE);
        return FALSE;
    }
    CAutoPtrArray<Renko::SShiftedRenko> cRenkos(psRenkos);
    
    UINT uIndex = 0;
    for (const auto sRythm : sListener.cList) {
        pdValues[uIndex] = sRythm.dCLosePrice;
        ++uIndex;
    }
    ASSERT(uValueNum == uIndex);
    const UINT uBoxSize = uBoxNum;
    FLOAT fA = (FLOAT)pdValues[0] * 0.00001f;
    FLOAT fB = (FLOAT)pdValues[0] * 0.02f;
    DOUBLE dFA = Evaluate(psRenkos, SHIFTED_RANKO_HISTORY_SIZE, fA, uBoxSize, pdValues, uValueNum);
    DOUBLE dFB = Evaluate(psRenkos, SHIFTED_RANKO_HISTORY_SIZE, fB, uBoxSize, pdValues, uValueNum);
    
    FLOAT fEvaHighest = fA;
    FLOAT dFEvaHighest = (FLOAT)dFA;
    CHECK_HIGHEST(fB, (FLOAT)dFB);

    if (dFA > dFB) {
        // swap
        FLOAT fTmp = fA;
        fA = fB;
        fB = fTmp;
        DOUBLE dTmp = dFA;
        dFA = dFB;
        dFB = dTmp;
    }
    UINT uCount = 0;
    while (100 > uCount) {
        if (0.0 > dFA) {
            const FLOAT fC = (fA * 0.95f + fB * 0.05f);
            const DOUBLE dFC = Evaluate(psRenkos, SHIFTED_RANKO_HISTORY_SIZE, fC, uBoxSize, pdValues, uValueNum);
            CHECK_HIGHEST(fC, (FLOAT)dFC);
            fA = fC;
            dFA = dFC;
            ++uCount;
//            TRACE("0A %d: f(%f)=%f f(%f)=%f\n", uCount + 1, fA, dFA, fB, dFB);
            continue;
        }
        if (0.0 > dFB) {
            const FLOAT fC = (fA * 0.05f + fB * 0.95f);
            const DOUBLE dFC = Evaluate(psRenkos, SHIFTED_RANKO_HISTORY_SIZE, fC, uBoxSize, pdValues, uValueNum);
            CHECK_HIGHEST(fC, (FLOAT)dFC);
            fB = fC;
            dFB = dFC;
            ++uCount;
//            TRACE("0B %d: f(%f)=%f f(%f)=%f\n", uCount + 1, fA, dFA, fB, dFB);
            continue;
        }
//        TRACE("%d: f(%f)=%f f(%f)=%f\n", uCount + 1, fA, dFA, fB, dFB);
        const FLOAT fDiffAB = (fB - fA) / 11.0f;
        FLOAT fVal = fA + fDiffAB;
        FLOAT fHighestHighest = fB;
        DOUBLE dHighestHighest = dFB;
        FLOAT fHighest = fA;
        DOUBLE dHighest = dFA;
        for (UINT uIndex = 0; 10 > uIndex; ++uIndex, fVal += fDiffAB) {
            const DOUBLE dFVal = Evaluate(psRenkos, SHIFTED_RANKO_HISTORY_SIZE, fVal, uBoxSize, pdValues, uValueNum);
            CHECK_HIGHEST(fVal, (FLOAT)dFVal);
//            TRACE("Inner f(%f) = %f\n", fVal, dFVal);
            if (dHighestHighest < dFVal) {
                fHighest = fHighestHighest;
                dHighest = dHighestHighest;
                fHighestHighest = fVal;
                dHighestHighest = dFVal;
            }
            else if (dHighest < dFVal) {
                fHighest = fVal;
                dHighest = dFVal;
            }
        }
        fA = fHighest;
        dFA = dHighest;
        
        if (dHighest == dHighestHighest) { // converged
//            fOptimizedSize = fEvaHighest;
            fOptimizedSize = fHighest < fHighestHighest ? fHighest : fHighestHighest;
            TRACE("Iterated %d times\n", uCount + 1);
            return TRUE;
        }
        if (fHighest < fHighestHighest) {
            fB = fHighestHighest + fDiffAB; // move 1 step.
        }
        else {
            fB = fHighestHighest - fDiffAB; // move 1 step.
        }
        dFB = Evaluate(psRenkos, SHIFTED_RANKO_HISTORY_SIZE, fB, uBoxSize, pdValues, uValueNum);
        CHECK_HIGHEST(fB, (FLOAT)dFB);
        if (dFA > dFB) {
            // swap
            FLOAT fTmp = fA;
            fA = fB;
            fB = fTmp;
            DOUBLE dTmp = dFA;
            dFA = dFB;
            dFB = dTmp;
        }
        ++uCount;
    }
    // cannot come to a conclusion, use the average of the 2
    if (0.0 < dFEvaHighest) {
        fOptimizedSize = fEvaHighest;
        return TRUE;
    }
    // no conclusion
    return FALSE;
}

FLOAT CShiftedRenko::Evaluate(Renko::SShiftedRenko* psRenkos, UINT uRenkoNum,
                              FLOAT fSize, UINT uBoxSize,
                              const DOUBLE* pdValues, UINT uValueNum)
{
    ASSERT(NULL != psRenkos);
    ASSERT(NULL != pdValues);
    memset(psRenkos, 0, sizeof(Renko::SShiftedRenko) * uRenkoNum);
    psRenkos[0].byRankoNum = uBoxSize;
    psRenkos[0].dDivisor   = 1.0 / ((DOUBLE)fSize / uBoxSize);
    UINT uCurrentIndex = 0;
    UINT uUsedNum = 0;
    for (UINT uIndex = 0; uValueNum > uIndex; ++uIndex) {
        Update(psRenkos, uRenkoNum, uCurrentIndex, uUsedNum, pdValues[uIndex], 0, FALSE, 0);
    }
    ASSERT(0 < uUsedNum);
    const FLOAT fValueRatio = (FLOAT)uValueNum/(FLOAT)uUsedNum;
    if (1.0f > fValueRatio) {
        return -1.0f;
    }
    const FLOAT fBalance = ComputeBalance(psRenkos, uCurrentIndex, uUsedNum, uRenkoNum);
    if (0.0f >= fBalance) {
        return -1.0f;
    }
    const FLOAT fResult = (FLOAT)(log(fValueRatio) * fBalance);
    return fResult;
}

FLOAT CShiftedRenko::ComputeBalance(const Renko::SShiftedRenko* psRenkos, UINT uCurrentIndex, UINT uUsedNum, UINT uTotalRenkoNum)
{
    INT nBalance = 1;
    INT nSignChanges = 0;
    const Renko::SShiftedRenko* psPrev = &psRenkos[uCurrentIndex];
    for (UINT uIndex = 1; uUsedNum > uIndex; ++uIndex) {
        UINT uActualIndex = (uCurrentIndex + uTotalRenkoNum - uIndex);
        if (uActualIndex >= uTotalRenkoNum) {
            uActualIndex -= uTotalRenkoNum;
        }
        ASSERT(uActualIndex < uTotalRenkoNum);
        const Renko::SShiftedRenko& sNow = psRenkos[uActualIndex];
        if (psPrev->byBarType == sNow.byBarType) { // if same direction
            ++nBalance;
        }
        else if (sNow.byBarType != Renko::EType_Neutral) { // different and not neutral
            nBalance -= sNow.byRankoNum;
            ++nSignChanges;
        }
        psPrev = &sNow;
    }
    if (0 > nBalance) {
        return -1.0f;
    }
    FLOAT fBalance = (FLOAT)nBalance;
    if (0 < nSignChanges) {
        fBalance /= (FLOAT)nSignChanges;
    }
    fBalance += 1.0;
    return (FLOAT)log(fBalance);
}

//#define EVALUATION_THRESHOLD (0.000001)
//BOOLEAN CShiftedRenko::BrendsMethod(FLOAT fA, FLOAT fB, Renko::SShiftedRenko* psRenkos, UINT uRenkoNum, UINT uBoxSize, const DOUBLE* pdValues, UINT uValueNum)
//{
//    TRACE("Determine Size: %f %f\n", fA, fB);
//    DOUBLE dA = fA;
//    DOUBLE dB = fB;
//    DOUBLE dFA = -Evaluate(psRenkos, SHIFTED_RANKO_HISTORY_SIZE, fA, uBoxSize, pdValues, uValueNum);
//    DOUBLE dFB = -Evaluate(psRenkos, SHIFTED_RANKO_HISTORY_SIZE, fB, uBoxSize, pdValues, uValueNum);
//    if (dFA * dFB >= 0.0) { // cannot determine result
//        TRACE("No Result (dFA * dFB >= 0.0)\n");
//        return FALSE;
//    }
//
//    DOUBLE dAbs_FA = abs(dFA);
//    DOUBLE dAbs_FB = abs(dFB);
//    if (dAbs_FA < dAbs_FB) { // if magnitude of f(lower_bound) is less than magnitude of f(upper_bound)
//        DOUBLE dTmp = dA;    // do swap
//        dA = dB;
//        dB = dTmp;
//        dTmp = dFA;
//        dFA = dFB;
//        dFB = dTmp;
//    }
//
//    DOUBLE  dC      = dA;           // c now equals the largest magnitude of the lower and upper bounds
//    DOUBLE  dFC     = dFA;          // precompute function evalutation for point c by assigning it the same value as fa
//    BOOLEAN bFlag   = TRUE;         // boolean flag used to evaluate if statement later on
//    DOUBLE  dS      = 0.0;          // Our Root that will be returned
//    DOUBLE  dFS     = 0.0;
//    DOUBLE  dD      = 0.0;          // Only used if mflag is unset (mflag == false)
//
//    for (UINT uIndex = 0; 1000 > uIndex; ++uIndex) {
//        if (abs(dA - dB) < EVALUATION_THRESHOLD) {
//            TRACE("Root Is %f\n", dS);
//            return TRUE;
//        }
//        if (dFA != dFC && dFB != dFC) {
//            // use inverse quadratic interopolation
//            dS =    ( dA * dFB * dFC / ((dFA - dFB) * (dFA - dFC) )
//                  + ( dB * dFA * dFC / ((dFB - dFA) * (dFB - dFC)) )
//                  + ( dC * dFA * dFB / ((dFC - dFA) * (dFC - dFB)) ));
//        }
//        else {
//            // secant method
//            dS = dB - dFB * (dB - dA) / (dFB - dFA);
//        }
//        /*
//            Crazy condition statement!:
//            -------------------------------------------------------
//            (condition 1) s is not between  (3a+b)/4  and b or
//            (condition 2) (mflag is true and |s−b| ≥ |b−c|/2) or
//            (condition 3) (mflag is false and |s−b| ≥ |c−d|/2) or
//            (condition 4) (mflag is set and |b−c| < |TOL|) or
//            (condition 5) (mflag is false and |c−d| < |TOL|)
//        */
//        if (    ( (dS < (3 * dA + dB) * 0.25) || (dS > dB) ) ||
//                ( bFlag && (abs(dS-dB) >= (abs(dB-dC) * 0.5)) ) ||
//                ( !bFlag && (abs(dS-dB) >= (abs(dC-dD) * 0.5)) ) ||
//                ( bFlag && (abs(dB-dC) < EVALUATION_THRESHOLD) ) ||
//                ( !bFlag && (abs(dC-dD) < EVALUATION_THRESHOLD))  )
//        {
//            // bisection method
//            dS = (dA+dB)*0.5;
//            bFlag = TRUE;
//        }
//        else
//        {
//            bFlag = FALSE;
//        }
//        dFS = -Evaluate(psRenkos, SHIFTED_RANKO_HISTORY_SIZE, dS, uBoxSize, pdValues, uValueNum);
//        dD = dC;      // first time d is being used (wasnt used on first iteration because mflag was set)
//        dC = dB;      // set c equal to upper bound
//        dFC = dFB;    // set f(c) = f(b)
//        if (dFA * dFS < 0) { // fa and fs have opposite signs
//            dB = dS;
//            dFB = dFS;    // set f(b) = f(s)
//        }
//        else {
//            dA = dS;
//            dFA = dFS;    // set f(a) = f(s)
//        }
//
//        if (abs(dFA) < abs(dFB)) { // if magnitude of fa is less than magnitude of fb
//            DOUBLE dTmp = dA;    // do swap
//            dA = dB;
//            dB = dTmp;
//            dTmp = dFA;
//            dFA = dFB;
//            dFB = dTmp;
//        }
//    }
//    TRACE("No Solution...");
//    return FALSE;
//}
//
