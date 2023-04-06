#include "stdafx.h"
#include "TickDef.h"
#include "MAUtil.h"
#include "PlayerData.h"

BOOLEAN CTickDef::GenerateMACDSignal(STickListener& sTickListener)
{
    sTickListener.sProcessed.eMACD = EMACD_SIG_NEUTRAL; // safety
    if (20 > sTickListener.sProcessed.uNum) {
        return TRUE;
    }
    const UINT uMaxSize = sTickListener.sProcessed.uMax;
    DOUBLE* pdPrices = sTickListener.sProcessed.pdWorkBuffer1;
    if (NULL == pdPrices) {
        ASSERT(FALSE);
        return FALSE;
    }
    UINT uIndex = 0;
    for (auto& sTick : sTickListener.cMinuteList) {
        if (uMaxSize <= uIndex) {
            ASSERT(FALSE);
            return FALSE;
        }
        pdPrices[uIndex] = sTick.dCLosePrice;
        ++uIndex;
    }
    SMACD sMACD(CPlayerData::GetMACDParam());
    sMACD.pdMACD = sTickListener.sProcessed.pdMACD;
    sMACD.pdSignal = sTickListener.sProcessed.pdSignal;
    sMACD.pdMA1 = sTickListener.sProcessed.pdWorkBuffer2;
    sMACD.pdMA2 = sTickListener.sProcessed.pdWorkBuffer3;
    sMACD.uNum = sTickListener.sProcessed.uMax;
    if (NULL == sMACD.pdMACD || NULL == sMACD.pdSignal || NULL == sMACD.pdMA1 || NULL == sMACD.pdMA2) {
        ASSERT(FALSE);
        return FALSE;
    }
    
    if (!CMAUtil::GenerateMACD(sMACD, pdPrices, uIndex, 0, uIndex)) {
        ASSERT(FALSE);
        return FALSE;
    }
    if (0.0 > sMACD.pdMACD[uIndex-1]) {
        sTickListener.sProcessed.eMACD = EMACD_SIG_SELL;
        return TRUE;
    }
    // check if its within 5 ticks that it switched to +ve
    UINT uI2 = 1;
    for ( ; 5 > uI2; ++uI2) {
        const UINT uActualIndex = uIndex - 1 - uI2;
        if (0.0 >= sMACD.pdMACD[uActualIndex]) {
            sTickListener.sProcessed.eMACD = EMACD_SIG_BUY;
            return TRUE;
        }
    }
    return TRUE;
}
