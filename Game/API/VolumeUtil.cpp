#include "stdafx.h"
#include "AutoPtr.h"
#include "VolumeUtil.h"

DOUBLE* CVolumeUtil::GenerateOBV(const DOUBLE* pdSrcPrice, const DOUBLE* pdSrcVolume, UINT uSrcValuesNum, INT nVisibleFirstIndex, UINT uVisibleValueNum, BOOLEAN bUseVWAPVolume)
{
    DOUBLE* pdOBV = new DOUBLE[uVisibleValueNum];
    if (NULL == pdOBV) {
        ASSERT(FALSE);
        return NULL;
    }
    memset(pdOBV, 0, sizeof(DOUBLE) * uVisibleValueNum); // clear 0
    UINT uIndex = 0;
    DOUBLE dPrevVolume = 0.0;
    for ( ; uVisibleValueNum > uIndex; ++uIndex) {
        const INT nActualIndex = nVisibleFirstIndex + uIndex;
        if (0 >= nActualIndex) {
            continue;
        }
        if ((INT)uSrcValuesNum <= nActualIndex) {
            ASSERT(FALSE);
            continue;
        }
        ASSERT(1 <= nActualIndex);
        const DOUBLE dNowPrice = pdSrcPrice[nActualIndex];
        const DOUBLE dPrevPrice = pdSrcPrice[nActualIndex - 1];
        const DOUBLE dVolume = pdSrcVolume[nActualIndex];
        const DOUBLE dPriceDiff = dNowPrice - dPrevPrice;
        pdOBV[uIndex] = dPrevVolume;
        if (0.0 < dPriceDiff) {
            if (bUseVWAPVolume) {
                pdOBV[uIndex] += dVolume * dPriceDiff;
            }
            else {
                pdOBV[uIndex] += dVolume;
            }
        }
        else if (0.0 > dPriceDiff) {
            if (bUseVWAPVolume) {
                pdOBV[uIndex] += dVolume * dPriceDiff;
            }
            else {
                pdOBV[uIndex] -= dVolume;
            }
        }
        dPrevVolume = pdOBV[uIndex];
    }
    return pdOBV;
}

DOUBLE* CVolumeUtil::GenerateOBV(UINT& uNumGenerated, const CTradeCandleMap& cCandleMap, UINT uCandleDurationSec, BOOLEAN bUseVWAPVolume)
{
    // determine number of srcvalues we need based on the interval
    const UINT uTotalValueNum = (UINT)cCandleMap.size();
    DOUBLE* pdSrcPrice = new DOUBLE[uTotalValueNum];
    if (NULL == pdSrcPrice) {
        ASSERT(FALSE);
        return NULL;
    }
    CAutoPtrArray<DOUBLE> cGuardBuffer(pdSrcPrice);
    DOUBLE* pdSrcVol = new DOUBLE[uTotalValueNum];
    if (NULL == pdSrcVol) {
        ASSERT(FALSE);
        return NULL;
    }
    CAutoPtrArray<DOUBLE> cGuardBuffer2(pdSrcVol);
    const UINT uFirstCandleTimeSec = CCryptoUtil::FindFirstCandleTimeSec(cCandleMap);
    UINT uTimeSec = uFirstCandleTimeSec;
    UINT uIndex = 0;
    for ( ; uTotalValueNum > uIndex; ++uIndex, uTimeSec += uCandleDurationSec) {
        CTradeCandleMap::const_iterator itFound = cCandleMap.find(uTimeSec);
        if (cCandleMap.end() == itFound) {
            ASSERT(FALSE);
            return NULL;
        }
        const STradeCandle& sCandle = itFound->second;
        if (bUseVWAPVolume) {
            pdSrcPrice[uIndex] = sCandle.dVWAP; // we try use VWAP to be more accurate than closing price
        }
        else {
            pdSrcPrice[uIndex] = sCandle.dClose;
        }
        pdSrcVol[uIndex] = sCandle.dVolume;
    }
    DOUBLE* pdOBV = GenerateOBV(pdSrcPrice, pdSrcVol, uTotalValueNum, 0, uTotalValueNum, bUseVWAPVolume);
    if (NULL != pdOBV) {
        uNumGenerated = uTotalValueNum;
    }
    return pdOBV;
}
