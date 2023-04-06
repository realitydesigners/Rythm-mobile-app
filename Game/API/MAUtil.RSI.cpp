#include "stdafx.h"
#include "AutoPtr.h"
#include "MAUtil.h"
#include <math.h>

typedef std::list<DOUBLE> CPriceList;
static DOUBLE GetSum(const CPriceList& cList)
{
    DOUBLE dVal = 0.0;
    for (auto dPrice : cList) {
        dVal += dPrice;
    }
    return dVal;
}
DOUBLE* CMAUtil::GenerateRSI(UINT uRsiInterval, const CTradeCandleMap& cCandleMap, UINT uCandleDurationSec, UINT uVisibleCandleNum, UINT uVisibleCandleStartSec)
{
    DOUBLE* pdValues = new DOUBLE[uVisibleCandleNum];
    if (NULL == pdValues) {
        ASSERT(FALSE);
        return NULL;
    }
    memset(pdValues, 0, sizeof(DOUBLE) * uVisibleCandleNum);
    
    // try to move to the 1st available candle
    UINT uCandleTimeSec = uVisibleCandleStartSec - uRsiInterval * uCandleDurationSec;
    const UINT uLastVisibleCandleTimeSec = uVisibleCandleStartSec + (uVisibleCandleNum - 1) * uCandleDurationSec;
    CTradeCandleMap::const_iterator itFound = cCandleMap.find(uCandleTimeSec);
    while (cCandleMap.end() == itFound) { // while not found
        uCandleTimeSec += uCandleDurationSec;
        if (uLastVisibleCandleTimeSec < uCandleTimeSec) {
            // should not occur.. means no data at all
            return pdValues;
        }
        itFound = cCandleMap.find(uCandleTimeSec);
    }
    // we use closing price
    DOUBLE dPrevPrice = itFound->second.dClose;
    UINT uIndex = 0;
    CPriceList cGains;
    CPriceList cLosses;
    for ( ; uRsiInterval > uIndex; ++uIndex) {
        uCandleTimeSec += uCandleDurationSec; //move to next candle interval
        itFound = cCandleMap.find(uCandleTimeSec);
        if (cCandleMap.end() == itFound) {
            ASSERT(FALSE);
            return pdValues;
        }
        const DOUBLE dPrice = itFound->second.dClose;
        const DOUBLE dDiff = dPrice - dPrevPrice;
        if (0.0 < dDiff) {
            cGains.push_back(dDiff);
        }
        else {
            cGains.push_back(0.0);
        }
        if (0.0 > dDiff) {
            cLosses.push_back(abs(dDiff));
        }
        else {
            cLosses.push_back(0.0);
        }
        dPrevPrice = dPrice;
    }
    
    ASSERT(uCandleTimeSec >= uVisibleCandleStartSec);
    ASSERT(0 == ((uCandleTimeSec - uVisibleCandleStartSec) % uCandleDurationSec));
    uIndex = (uCandleTimeSec - uVisibleCandleStartSec) / uCandleDurationSec;
    if (uVisibleCandleNum <= uIndex) {
//        ASSERT(FALSE);
        return pdValues;
    }
    const DOUBLE dAvGain = GetSum(cGains) / uRsiInterval;
    const DOUBLE dAvLoss = GetSum(cLosses) / uRsiInterval;
    const DOUBLE dFirstRS = dAvGain / dAvLoss;
    const DOUBLE dFirstRSI = 100.0 - (100 / (1 + dFirstRS));
    pdValues[uIndex] = dFirstRSI;
    uCandleTimeSec += uCandleDurationSec;
    ++uIndex;
    while (uCandleTimeSec <= uLastVisibleCandleTimeSec) { // now iterate tru the rest
        itFound = cCandleMap.find(uCandleTimeSec);
        if (cCandleMap.end() == itFound) {
            ASSERT(FALSE);
            return pdValues;
        }
        const DOUBLE dPrice = itFound->second.dClose;
        const DOUBLE dDiff = dPrice - dPrevPrice;
        dPrevPrice = dPrice;
        const DOUBLE dGain = (0.0 < dDiff) ? dDiff : 0.0;
        const DOUBLE dLoss = (0.0 > dDiff) ? abs(dDiff) : 0.0;
        cGains.pop_front();
        cGains.push_back(dGain);
        cLosses.pop_front();
        cLosses.push_back(dLoss);
        const DOUBLE dAvGain = GetSum(cGains) / uRsiInterval;
        const DOUBLE dAvLoss = GetSum(cLosses) / uRsiInterval;
        const DOUBLE dRS = dAvGain / dAvLoss;
        const DOUBLE dRSI = 100.0 - (100.0 / (1 + dRS));
        pdValues[uIndex] = dRSI;
        uCandleTimeSec += uCandleDurationSec;
        ++uIndex;
    }
    return pdValues;
}
