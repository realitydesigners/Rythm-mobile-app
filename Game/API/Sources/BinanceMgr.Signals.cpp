#include "stdafx.h"
#include "AutoPtr.h"
#include "BinanceMgr.h"
#include "MAUtil.h"
#include "PlayerData.h"
#include <math.h>

#define OBV_DIFFERENCE_NEGLIGIBLE_PERCENT (5)
/*
struct SProcessedTick {
    UINT                uNum;           // number of entries
    UINT                uMax;           // maximum accepted number of entries
    DOUBLE*             pdMACD;         // MACD
    DOUBLE*             pdSignal;       // MACD signal
    DOUBLE*             pdWorkBuffer;   // Work Buffer;
    
    EOBV_SIG            aeUpperSignalOBV[EOBV_TYPE_NUM];
    UINT                aauUpperIndexOBV[EOBV_TYPE_NUM][2];
    
    EOBV_SIG            aeLowerSignalOBV[EOBV_TYPE_NUM];
    UINT                aauLowerIndexOBV[EOBV_TYPE_NUM][2];
    
    EMACD_SIG           eMACD;
    ESECOND_TICK_SIG    eSecondTick;
};*/

static VOID ClearAllSignals(SProcessedTick& sProcessed)
{
    memset(sProcessed.aeUpperSignalOBV, 0, sizeof(sProcessed.aeUpperSignalOBV));
    memset(sProcessed.aauUpperIndexOBV, 0, sizeof(sProcessed.aauUpperIndexOBV));
    memset(sProcessed.aeLowerSignalOBV, 0, sizeof(sProcessed.aauUpperIndexOBV));
    memset(sProcessed.aauLowerIndexOBV, 0, sizeof(sProcessed.aauUpperIndexOBV));
    sProcessed.eMACD = EMACD_SIG_NEUTRAL;
    sProcessed.eSecondTick = ESECOND_TICK_NEUTRAL;
}
static BOOLEAN SetupMemory(SProcessedTick& sProcessed)
{
    if (sProcessed.uMax < sProcessed.uNum) {
        SAFE_DELETE_ARRAY(sProcessed.pdMACD);
        SAFE_DELETE_ARRAY(sProcessed.pdSignal);
        SAFE_DELETE_ARRAY(sProcessed.pdWorkBuffer1);
        SAFE_DELETE_ARRAY(sProcessed.pdWorkBuffer2);
        SAFE_DELETE_ARRAY(sProcessed.pdWorkBuffer3);
        sProcessed.uMax = sProcessed.uNum;
    }
    if (NULL == sProcessed.pdMACD) {
        sProcessed.pdMACD = new DOUBLE[sProcessed.uMax];
        if (NULL == sProcessed.pdMACD) {
            ASSERT(FALSE);
            return FALSE;
        }
    }
    if (NULL == sProcessed.pdSignal) {
        sProcessed.pdSignal = new DOUBLE[sProcessed.uMax];
        if (NULL == sProcessed.pdSignal) {
            ASSERT(FALSE);
            return FALSE;
        }
    }
    if (NULL == sProcessed.pdWorkBuffer1) {
        sProcessed.pdWorkBuffer1 = new DOUBLE[sProcessed.uMax];
        if (NULL == sProcessed.pdWorkBuffer1) {
            ASSERT(FALSE);
            return FALSE;
        }
    }
    if (NULL == sProcessed.pdWorkBuffer2) {
        sProcessed.pdWorkBuffer2 = new DOUBLE[sProcessed.uMax];
        if (NULL == sProcessed.pdWorkBuffer2) {
            ASSERT(FALSE);
            return FALSE;
        }
    }
    if (NULL == sProcessed.pdWorkBuffer3) {
        sProcessed.pdWorkBuffer3 = new DOUBLE[sProcessed.uMax];
        if (NULL == sProcessed.pdWorkBuffer3) {
            ASSERT(FALSE);
            return FALSE;
        }
    }
    return TRUE;
}
static BOOLEAN GenerateLineSignals(STickListener& sTickListener, BOOLEAN bPrice)
{
    const UINT auInterval[EOBV_TYPE_NUM] = { 240, 120, 60, 30 };
    SProcessedTick& sProcessed = sTickListener.sProcessed;
    INT nIndex = EOBV_TYPE_NUM - 1;
    for ( ; 0 <= nIndex; --nIndex) {    // for each OBV Type
        const UINT uIterationNum = auInterval[nIndex];
        CTickList& cList = sTickListener.cMinuteList;
        if (cList.size() < uIterationNum) {
            ASSERT(FALSE);
            return FALSE;
        }
        const UINT uHalfIterationNum = uIterationNum / 2;
        CTickList::reverse_iterator itrTick = cList.rbegin();
        const STick* psTick = &(*(itrTick));
        UINT uHigh2Index = 0;
        UINT uLow2Index = 0;
        DOUBLE dHigh2 = bPrice ? psTick->dCLosePrice : psTick->dOBV;
        DOUBLE dLow2 = dHigh2;
        ++itrTick;
        UINT uIndex2 = 1;   // already iterated 1 time (see above code)
        
        UINT uHigh1Index = uIterationNum - 1;
        UINT uLow1Index = uIterationNum - 1;
        DOUBLE dHigh1 = 0.0f;
        DOUBLE dLow1 = 0.0;
        BOOLEAN bCanStartFindHigh1 = FALSE;
        BOOLEAN bHasHighCurveReversed = FALSE;
        BOOLEAN bCanStartFindLow1 = FALSE;
        BOOLEAN bHasLowCurveReversed = FALSE;
        DOUBLE dPrev = dHigh2;
        for ( ; uIterationNum > uIndex2; ++uIndex2, ++itrTick) { // for the entire duration
            const STick& sTick = *(itrTick);
            const DOUBLE dValue = bPrice ? sTick.dCLosePrice : sTick.dOBV;

            if (uHalfIterationNum > uIndex2) {
                if (dValue > dHigh2) {
                    dHigh2 = dValue;
                    uHigh2Index = uIndex2;
                    bCanStartFindHigh1 = FALSE;
                    bHasHighCurveReversed = FALSE;
                }
                else if (dValue < dLow2) {
                    dLow2 = dValue;
                    uLow2Index = uIndex2;
                    bCanStartFindLow1 = FALSE;
                    bHasLowCurveReversed = FALSE;
                }
                if (!bCanStartFindHigh1) {
                    bCanStartFindHigh1 = dValue < dPrev;
                }
                else {
                    if (!bHasHighCurveReversed) { // curve havent reverse back up
                        bHasHighCurveReversed = dValue > dPrev;
                        if (bHasHighCurveReversed) {
                            dHigh1 = dValue;
                            uHigh1Index = uIndex2;
                        }
                    }
                }
                if (!bCanStartFindLow1) {
                    bCanStartFindLow1 = dValue > dPrev;
                }
                else {
                    if (!bHasLowCurveReversed) {
                        bHasLowCurveReversed = dValue < dPrev;
                        if (bHasLowCurveReversed) {
                            dLow1 = dValue;
                            uLow1Index = uIndex2;
                        }
                    }
                }
                dPrev = dValue;
                continue;
            }
            // 2nd half
            if (!bCanStartFindHigh1) {
                bCanStartFindHigh1 = dValue < dPrev;
            }
            else {
                if (!bHasHighCurveReversed) { // curve havent reverse back up
                    bHasHighCurveReversed = dValue > dPrev;
                    if (bHasHighCurveReversed) {
                        dHigh1 = dValue;
                        uHigh1Index = uIndex2;
                    }
                }
            }
            if (!bCanStartFindLow1) {
                bCanStartFindLow1 = dValue > dPrev;
            }
            else {
                if (!bHasLowCurveReversed) {
                    bHasLowCurveReversed = dValue < dPrev;
                    if (bHasLowCurveReversed) {
                        dLow1 = dValue;
                        uLow1Index = uIndex2;
                    }
                }
            }
            if (bCanStartFindHigh1 && bHasHighCurveReversed) {
                if (dValue > dHigh1 || uHigh1Index < uHalfIterationNum) {
                    dHigh1 = dValue;
                    uHigh1Index = uIndex2;
                }
            }
            if (bCanStartFindLow1 && bHasLowCurveReversed) {
                if (dValue < dLow1 || uLow1Index < uHalfIterationNum) {
                    dLow1 = dValue;
                    uLow1Index = uIndex2;
                }
            }
            dPrev = dValue;
        }
        ASSERT(0 <= nIndex && EOBV_TYPE_NUM > nIndex);

        if (bPrice) {
            sProcessed.aauUpperIndexPrice[nIndex][0] = sProcessed.uNum - uHigh1Index - 1;
            sProcessed.aauUpperIndexPrice[nIndex][1] = sProcessed.uNum - uHigh2Index - 1;
            sProcessed.aauLowerIndexPrice[nIndex][0] = sProcessed.uNum - uLow1Index - 1;
            sProcessed.aauLowerIndexPrice[nIndex][1] = sProcessed.uNum - uLow2Index - 1;
        }
        else {
            sProcessed.aauUpperIndexOBV[nIndex][0] = sProcessed.uNum - uHigh1Index - 1;
            sProcessed.aauUpperIndexOBV[nIndex][1] = sProcessed.uNum - uHigh2Index - 1;
            sProcessed.aauLowerIndexOBV[nIndex][0] = sProcessed.uNum - uLow1Index - 1;
            sProcessed.aauLowerIndexOBV[nIndex][1] = sProcessed.uNum - uLow2Index - 1;
        }
        const DOUBLE dHighest = dHigh1 > dHigh2 ? dHigh1 : dHigh2;
        const DOUBLE dLowest = dLow1 < dLow2 ? dLow1 : dLow2;
        const DOUBLE dDifference = dHighest - dLowest;
        const DOUBLE dHighNeglible = abs((OBV_DIFFERENCE_NEGLIGIBLE_PERCENT * dDifference) * 0.01f);
        const DOUBLE dDiffHigh = dHigh1 - dHigh2;
        EOBV_SIG* psUpperSignal = bPrice ? sProcessed.aeUpperSignalPrice : sProcessed.aeUpperSignalOBV;
        // check if considered neutral
        if (dHighNeglible >= abs(dDiffHigh)) {
            psUpperSignal[nIndex] = EOBV_SIG_NEUTRAL;
        }
        else if (0.0f < dDiffHigh) {
            psUpperSignal[nIndex] = EOBV_SIG_DOWN;
        }
        else {
            psUpperSignal[nIndex] = EOBV_SIG_UP;
        }
        const DOUBLE dLowNeglible = abs((OBV_DIFFERENCE_NEGLIGIBLE_PERCENT * dLow1) * 0.01f);
        const DOUBLE dDiffLow = dLow1 - dLow2;
        EOBV_SIG* psLowerSignal = bPrice ? sProcessed.aeLowerSignalPrice : sProcessed.aeLowerSignalOBV;
        // check if considered neutral
        if (dLowNeglible >= abs(dDiffLow)) {
            psLowerSignal[nIndex] = EOBV_SIG_NEUTRAL;
        }
        else if (0.0f < dDiffLow) {
            psLowerSignal[nIndex] = EOBV_SIG_DOWN;
        }
        else {
            psLowerSignal[nIndex] = EOBV_SIG_UP;
        }
    }
    return TRUE;
}

VOID CBinanceMgr::GenerateSignals(STickListener& sTickListener)
{
    SProcessedTick& sProcessed = sTickListener.sProcessed;
    sProcessed.uNum = (UINT)sTickListener.cMinuteList.size();
    if (240 > sProcessed.uNum) {            // need at least 240 seconds
        ClearAllSignals(sProcessed);
        return;
    }
    if (!SetupMemory(sProcessed)) {         // setup proper memory
        return;
    }
    if (!GenerateLineSignals(sTickListener, TRUE)) {  // generate price signals
        return;
    }
    if (!GenerateLineSignals(sTickListener, FALSE)) {  // generate OBV signals
        return;
    }
    if (!CTickDef::GenerateMACDSignal(sTickListener)) { // generate MACD signal
        return;
    }
}
