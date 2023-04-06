#include "stdafx.h"
#include "ATRUtil.h"

#include <math.h>

VOID ATR::SetupAtr(ATR::SATR_Param& sParam, UINT uPeriod, UINT uDurationSeconds)
{
    sParam.uDurationSeconds = uDurationSeconds;
    sParam.uPeriod = uPeriod;
    sParam.dATR = 0.0;
    sParam.dPrevATR = 0.0;
    sParam.cTicks.clear();
}
VOID ATR::ReleaseAtr(ATR::SATR_Param& sParam)
{
    sParam.dATR = 0.0;
    sParam.dPrevATR = 0.0;
    sParam.cTicks.clear();
}

static VOID CalculateATR(ATR::SATR_Param& sParam)
{
    if (sParam.uPeriod > sParam.cTicks.size()) {
         // not enough data, skip
        return;
    }
    ATR::CTickList::reverse_iterator  itr = sParam.cTicks.rbegin();
    ASSERT(sParam.cTicks.rend() != itr);
    const ATR::SATR_Tick& sNowTick = (*itr);
    ++itr;
    ASSERT(sParam.cTicks.rend() != itr);
    const ATR::SATR_Tick& sPrevTick = *(itr);
    const DOUBLE d1 = abs(sNowTick.dHigh - sNowTick.dLow);
    const DOUBLE d2 = abs(sNowTick.dHigh - sPrevTick.dClose);
    const DOUBLE d3 = abs(sNowTick.dLow - sPrevTick.dClose);
    DOUBLE dTR = d1 > d2 ? d1 : d2;
    dTR = dTR > d3 ? dTR : d3;
    
    const BOOLEAN bNeedRemoveLastTick = sParam.uPeriod < sParam.cTicks.size();
    if (bNeedRemoveLastTick) {
        ASSERT((sParam.uPeriod+1) == sParam.cTicks.size());
        sParam.cTicks.pop_front(); // remove the oldest entries
        sParam.dPrevATR = sParam.dATR;
    }
    if (0.0 != sParam.dPrevATR) { // if prev ATR exists, calculate new ATR
        sParam.dATR = (sParam.dPrevATR * (sParam.uPeriod - 1) + dTR) / sParam.uPeriod;
    }
    else {
        // do simple SMA
        DOUBLE dTotal = 0.0;
        ATR::CTickList::iterator it = sParam.cTicks.begin();
        const ATR::SATR_Tick& sOldest = *it;
        dTotal = sOldest.dHigh - sOldest.dLow;
        ++it;
        DOUBLE dPrevClose = sOldest.dClose;
        for ( ; sParam.cTicks.end() != it; ++it) {
            const ATR::SATR_Tick& sCurrent = *it;
            const DOUBLE d1 = abs(sCurrent.dHigh - sCurrent.dLow);
            const DOUBLE d2 = abs(sCurrent.dHigh - dPrevClose);
            const DOUBLE d3 = abs(sCurrent.dLow - dPrevClose);
            DOUBLE dTR = d1 > d2 ? d1 : d2;
            dTR = dTR > d3 ? dTR : d3;
            dTotal += dTR;
            dPrevClose = sCurrent.dClose;
        }
        sParam.dATR = dTotal / sParam.uPeriod;
    }
}
VOID ATR::UpdateAtr(ATR::SATR_Param& sParam, DOUBLE dPrice, UINT uTimeSec)
{
    if (0 == sParam.cTicks.size()) {
        const ATR::SATR_Tick sTick = {
            uTimeSec,   //            UINT uTimeSec;
            dPrice,     //            DOUBLE dHigh;
            dPrice,     //            DOUBLE dLow;
            dPrice      //            DOUBLE dClose;
        };
        sParam.cTicks.push_back(sTick);
        return;
    }
    ATR::SATR_Tick& sLastTick = *(sParam.cTicks.rbegin());
    const UINT uTimeOverSec = sLastTick.uTimeSec + sParam.uDurationSeconds;
    if (uTimeSec < uTimeOverSec) { // if within the same tick
        if (sLastTick.dLow > dPrice) {
            sLastTick.dLow = dPrice;
        }
        else if (sLastTick.dHigh < dPrice) {
            sLastTick.dHigh = dPrice;
        }
        sLastTick.dClose = dPrice;
        CalculateATR(sParam);
        return;
    }
    // over the tick, create new tick
    const ATR::SATR_Tick sTick = {
        uTimeSec,   //            UINT uTimeSec;
        dPrice,     //            DOUBLE dHigh;
        dPrice,     //            DOUBLE dLow;
        dPrice      //            DOUBLE dClose;
    };
    sParam.cTicks.push_back(sTick);
    CalculateATR(sParam);
}
