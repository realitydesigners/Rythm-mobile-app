#include "stdafx.h"
#include "AutoPtr.h"
#include "MAUtil.h"
#include <math.h>

DOUBLE CMAUtil::GetSourceValueFromCandle(const STradeCandle& sTradeCandle, EMA_VALUE_SOURCE eSource)
{
    switch (eSource) {
        case EMA_VALUE_CLOSE:
            return sTradeCandle.dClose;
        case EMA_VALUE_OPEN:
            return sTradeCandle.dOpen;
        case EMA_VALUE_AVG:
            return sTradeCandle.dVWAP;
        default:
            ASSERT(FALSE);
            break;
    }
    return sTradeCandle.dClose;
}

static BOOLEAN SetupMA(SMovingAverage& sMA, UINT uVisibleCandleNum)
{
    if (NULL == sMA.pdPrice) {
        ASSERT(FALSE);
        return FALSE;
    }
    memset(sMA.pdPrice, 0, sizeof(DOUBLE) * uVisibleCandleNum); // clear all to 0 value
    return TRUE;
}

static BOOLEAN GenerateSMA(SMovingAverage& sMA, const DOUBLE* pdSrcValues, UINT uSrcValuesNum, UINT uVisibleFirstIndex, UINT uVisibleValueNum)
{
    UINT uIndex = 0;
    for ( ; uVisibleValueNum > uIndex; ++uIndex) {
        const UINT uEndValueIndex = uVisibleFirstIndex + uIndex + 1;
        const UINT uStartValueIndex = (uEndValueIndex > sMA.uIntervalNum) ? (uEndValueIndex - sMA.uIntervalNum) : 0;
        DOUBLE dTotalPrice = 0.0;
        UINT uIndex2 = uStartValueIndex;
        UINT uAddedNum = 0;
        for ( ; uEndValueIndex > uIndex2; ++uIndex2) {
            if (uSrcValuesNum <= uIndex2) {
                ASSERT(FALSE);
                return FALSE;
            }
            if (0.0 != pdSrcValues[uIndex2]) {
                dTotalPrice += pdSrcValues[uIndex2];
                ++uAddedNum;
            }
        }
        if (0 < uAddedNum) {
            sMA.pdPrice[uIndex] = dTotalPrice / uAddedNum;
        }
        else {
            sMA.pdPrice[uIndex] = 0.0f;
        }
    }
    return TRUE;
}
static BOOLEAN GenerateEMA(SMovingAverage& sMA, const DOUBLE* pdSrcValues, UINT uSrcValuesNum, UINT uVisibleFirstIndex, UINT uVisibleValueNum)
{
    ASSERT(uSrcValuesNum > (sMA.uIntervalNum));
    // generate 1st entry sma
    if (!GenerateSMA(sMA, pdSrcValues, sMA.uIntervalNum, sMA.uIntervalNum-1, 1)) {
        return FALSE;
    }
    
    const DOUBLE dMultiplier = 2.0 / (1 + sMA.uIntervalNum);
    const DOUBLE dInverse = 1.0 - dMultiplier;
    DOUBLE dPrevPrice = sMA.pdPrice[0];
    UINT uIndex = 1;
    const UINT uEndVisibleIndex = uVisibleFirstIndex + uVisibleValueNum;
    for ( ; uEndVisibleIndex > uIndex; ++uIndex) { // iterate thru all the source values
        if (uSrcValuesNum <= uIndex) {
            ASSERT(FALSE);
            return FALSE;
        }
        const DOUBLE dNowPrice = pdSrcValues[uIndex];
        const DOUBLE dNewPrice = dNowPrice * dMultiplier + dPrevPrice * dInverse;
        if (uIndex >= uVisibleFirstIndex) {
            ASSERT(uVisibleValueNum > (uIndex - uVisibleFirstIndex));
            sMA.pdPrice[uIndex - uVisibleFirstIndex] = dNewPrice;
        }
        dPrevPrice = dNewPrice;
    }
    return TRUE;
}
BOOLEAN CMAUtil::GenerateMA(SMovingAverage& sMA, const DOUBLE* pdSrcValues, UINT uSrcValuesNum, UINT uVisibleFirstIndex, UINT uVisibleValueNum)
{
    ASSERT((uVisibleFirstIndex + uVisibleValueNum) <= uSrcValuesNum);
    if (!SetupMA(sMA, uVisibleValueNum)) {
        return FALSE;
    }
    switch (sMA.eType) {
        case EMA_SMA:
            return GenerateSMA(sMA, pdSrcValues, uSrcValuesNum, uVisibleFirstIndex, uVisibleValueNum);
        case EMA_EMA:
            return GenerateEMA(sMA, pdSrcValues, uSrcValuesNum, uVisibleFirstIndex, uVisibleValueNum);
        default:
            ASSERT(FALSE);
            break;
    }
    return FALSE;
}

BOOLEAN CMAUtil::GenerateMA(SMovingAverage& sMA,
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
            pdSrcValue[uIndex] = GetSourceValueFromCandle(itFound->second, sMA.eSource);
        }
        if (uTimeSec == uVisibleCandleStartSec) {
            uVisibleOffset = uIndex;
        }
    }
    if (0xFFFFFFFF == uVisibleOffset) {
        return TRUE;
    }
    return GenerateMA(sMA, pdSrcValue, uTotalValueNum, uVisibleOffset, uVisibleCandleNum);
}

BOOLEAN CMAUtil::GenerateMACD(SMACD& sMACD, const DOUBLE* pdSrcValues, UINT uSrcValuesNum, UINT uVisibleFirstIndex, UINT uVisibleValueNum)
{
    ASSERT(sMACD.uNum >= uSrcValuesNum);
    // generate MA1
    SMovingAverage sMA1((EMA_TYPE)sMACD.sParams.sMA1.byType, (EMA_VALUE_SOURCE)sMACD.sParams.sMA1.bySource, sMACD.sParams.sMA1.ushInterval);
    sMA1.pdPrice = sMACD.pdMA1;
    if (!GenerateMA(sMA1, pdSrcValues, uSrcValuesNum, 0, uSrcValuesNum)) {
        return FALSE;
    }
    // now generate MA2
    SMovingAverage sMA2((EMA_TYPE)sMACD.sParams.sMA2.byType, (EMA_VALUE_SOURCE)sMACD.sParams.sMA2.bySource, sMACD.sParams.sMA2.ushInterval);
    sMA2.pdPrice = sMACD.pdMA2;
    if (!GenerateMA(sMA2, pdSrcValues, uSrcValuesNum, 0, uSrcValuesNum)) {
        return FALSE;
    }
    UINT uIndex = 0;
    for ( ; uSrcValuesNum > uIndex; ++uIndex) {
        sMA1.pdPrice[uIndex] -= sMA2.pdPrice[uIndex];   // compute MACD (MA1 - MA2)
    }
    SMovingAverage sSignal((EMA_TYPE)sMACD.sParams.sSignal.byType, (EMA_VALUE_SOURCE)sMACD.sParams.sSignal.bySource, sMACD.sParams.sSignal.ushInterval);
    sSignal.pdPrice = sMACD.pdSignal;
    if (!GenerateMA(sSignal, sMA1.pdPrice, uSrcValuesNum, uVisibleFirstIndex, uVisibleValueNum)) {
        return FALSE;
    }
    
    DOUBLE* pdMACD = sMACD.pdMACD;
    if (NULL == pdMACD) {
        ASSERT(FALSE);
        return FALSE;
    }
    uIndex = 0;
    for ( ; uVisibleValueNum > uIndex; ++uIndex) {
        pdMACD[uIndex] = sMA1.pdPrice[uIndex + uVisibleFirstIndex];
    }
    return TRUE;

}
BOOLEAN CMAUtil::GenerateMACD(SMACD& sMACD, const CTradeCandleMap& cCandleMap, UINT uFirstCandleTimeSec,
                              UINT uCandleDurationSec, UINT uVisibleCandleNum, UINT uVisibleCandleStartSec)
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
            ASSERT(FALSE);
            pdSrcValue[uIndex] = 0.0f;
        }
        else {
            pdSrcValue[uIndex] = GetSourceValueFromCandle(itFound->second, (EMA_VALUE_SOURCE)sMACD.sParams.sMA1.bySource);
        }
        if (uTimeSec == uVisibleCandleStartSec) {
            uVisibleOffset = uIndex;
        }
    }
    ASSERT(0xFFFFFFFF != uVisibleOffset);

    return GenerateMACD(sMACD, pdSrcValue, uTotalValueNum, uVisibleOffset, uVisibleCandleNum);
}

