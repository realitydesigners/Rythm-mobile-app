#include "stdafx.h"
#include "AutoPtr.h"
#include "MAUtil.h"
#include <math.h>


BOOLEAN CMAUtil::GenerateBollingerBands(SBollingerBands& sBand,
                                        const DOUBLE* pdSrcValues,
                                        UINT uSrcValuesNum,
                                        UINT uVisibleFirstIndex,
                                        UINT uVisibleValueNum)
{
    ASSERT((uVisibleFirstIndex + uVisibleValueNum) <= uSrcValuesNum);
    if (!GenerateMA(sBand.sMA, pdSrcValues, uSrcValuesNum, uVisibleFirstIndex, uVisibleValueNum)) {
        return FALSE;
    }
    if (NULL == sBand.pdDeviation) {
        ASSERT(FALSE);
        return FALSE;
    }
    memset(sBand.pdDeviation, 0, sizeof(DOUBLE) * uVisibleValueNum); // clear all to 0 value

    const UINT uBBInterval = sBand.sMA.uIntervalNum;
    UINT uIndex = 0;
    for ( ; uVisibleValueNum > uIndex; ++uIndex) {
        const DOUBLE dPrice = sBand.sMA.pdPrice[uIndex];
        const INT uActualPriceIndex = uVisibleFirstIndex + uIndex;
        if ((INT)uSrcValuesNum <= uActualPriceIndex) {
            ASSERT(FALSE);
            return FALSE;
        }
        UINT uAddedNum = 0;
        DOUBLE dTotal = 0.0;
        UINT uIndex2 = 0;
        for ( ; uBBInterval > uIndex2; ++uIndex2) {
            const INT nNowIndex = (INT)uActualPriceIndex - uIndex2;
            if (0 > nNowIndex) {
                continue;
            }
            const DOUBLE dCurrentClose = pdSrcValues[nNowIndex];
            const DOUBLE dDiff = dCurrentClose - dPrice;
            dTotal += (dDiff * dDiff);
            ++uAddedNum;
        }
        if (0 == uAddedNum) {
            ASSERT(FALSE);
        }
        else {
            sBand.pdDeviation[uIndex] = sqrt(dTotal / uAddedNum);
        }
    }
    return TRUE;
}
BOOLEAN CMAUtil::GenerateBollingerBands(SBollingerBands& sBand,
                                        const CTradeCandleMap& cCandleMap,
                                        UINT uFirstCandleTimeSec,
                                        UINT uCandleDurationSec,
                                        UINT uVisibleCandleNum,
                                        UINT uVisibleCandleStartSec)
{
    // determine number of srcvalues we need based on the interval
    const UINT uTotalValueNum = (UINT)cCandleMap.size();
    DOUBLE* pdSrcValue = new DOUBLE[uTotalValueNum];
    if (NULL == pdSrcValue) {
        ASSERT(FALSE);
        return FALSE;
    }
    CAutoPtrArray<DOUBLE> cGuardBuffer(pdSrcValue);
    UINT uTimeSec = uFirstCandleTimeSec;
    UINT uIndex = 0;
    UINT uVisibleOffset = 0xFFFFFFFF;
    for ( ; uTotalValueNum > uIndex; ++uIndex, uTimeSec += uCandleDurationSec) {
        CTradeCandleMap::const_iterator itFound = cCandleMap.find(uTimeSec);
        if (cCandleMap.end() == itFound) {
            pdSrcValue[uIndex] = 0.0f;
        }
        else {
            pdSrcValue[uIndex] = CMAUtil::GetSourceValueFromCandle(itFound->second, sBand.sMA.eSource);
        }
        if (uTimeSec == uVisibleCandleStartSec) {
            uVisibleOffset = uIndex;
        }
    }
    if (0xFFFFFFFF == uVisibleOffset) {
        return TRUE;
    }
    return GenerateBollingerBands(sBand, pdSrcValue, uTotalValueNum, uVisibleOffset, uVisibleCandleNum);
}
