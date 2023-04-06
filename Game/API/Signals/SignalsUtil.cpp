#include "stdafx.h"
#include "AutoPtr.h"
#include "MAUtil.h"
#include "PlayerData.h"
#include "SignalsUtil.h"

VOID CSignalsUtil::GenerateSignals(CSignalList& cList,
                                   DOUBLE dAmountAllocated,
                                   const CTradeCandleMap& cCandles,
                                   UINT uCandleStartTimeSec,
                                   UINT uCandleEndTimeSec,
                                   UINT uCandleDurationSec)
{
    cList.clear(); // safety
    ASSERT(0 == ((uCandleEndTimeSec - uCandleStartTimeSec) % uCandleDurationSec));
    const UINT uCandleNum = (uCandleEndTimeSec - uCandleStartTimeSec) / uCandleDurationSec;
    if (0 == uCandleNum) {
        return;
    }
    DOUBLE* pdPrices = new DOUBLE[uCandleNum];
    DOUBLE* pdVolume = new DOUBLE[uCandleNum];
    if (NULL == pdPrices || NULL == pdVolume) {
        ASSERT(FALSE);
        return;
    }
    CAutoPtrArray<DOUBLE> cGuardArray1(pdPrices);
    CAutoPtrArray<DOUBLE> cGuardArray2(pdVolume);
    UINT uNowCandleTimeSec = uCandleStartTimeSec;
    UINT uIndex = 0;
    for ( ; uCandleNum > uIndex; ++uIndex, uNowCandleTimeSec += uCandleDurationSec) {
        CTradeCandleMap::const_iterator itFound = cCandles.find(uNowCandleTimeSec);
        if (cCandles.end() == itFound) {
            ASSERT(FALSE);
            return;
        }
        const STradeCandle& sCandle = itFound->second;
        pdPrices[uIndex] = sCandle.dClose;
        pdVolume[uIndex] = sCandle.dVolume;
    }
    
    // try special strategy
    GenerateSpecial(cList, dAmountAllocated, pdPrices, pdVolume, uCandleNum, uCandleStartTimeSec, uCandleDurationSec, cCandles);
}
