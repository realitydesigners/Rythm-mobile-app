#include "stdafx.h"
#include "AutoPtr.h"
#include "PlayerData.h"
#include "ZZUtil.h"
#include <math.h>

static BOOLEAN ProcessSR_RythmZZ(ZZUtil::SRythmZZ* psZZs, const Renko::SSyncedRenkoFrame* psFrames, UINT uNum, UINT uMax, FLOAT fDeviation, BYTE bySetValue)
{
    if (5 > uMax) { // skip if less than 5 points..
        return TRUE;
    }
    ZZUtil::SRythmZZ* psPrev = &psZZs[0];
    ASSERT(uMax > psPrev->uActualIndex);
    psPrev->byZZType = Renko::EType_Neutral;
    const BYTE byPrevStartIndicator = psPrev->byIndicator;
    psPrev->byIndicator = bySetValue;
    BYTE byPrevValue = 0;
    ZZUtil::SRythmZZ* psCurrent = NULL;
    UINT uSetCount = 1;
    for (UINT uIndex = 1; uNum > uIndex; ++uIndex) {
        psCurrent = &psZZs[uIndex];
        ASSERT(uMax > psCurrent->uActualIndex);
        const Renko::SSyncedRenkoFrame& sFrame = psFrames[psCurrent->uActualIndex];
        const INT nDiff = psCurrent->nPriceIndex - psPrev->nPriceIndex;
        switch (psPrev->byZZType) {
            case Renko::EType_Neutral:
                if (abs(nDiff) >= fDeviation) { // if more than deviation, accept new point
                    const BOOLEAN bUpwards = (0 < nDiff);
                    psPrev->byZZType = bUpwards ? Renko::EType_Down : Renko::EType_Up;
                    psCurrent->byZZType = bUpwards ? Renko::EType_Up : Renko::EType_Down;
                    byPrevValue = psCurrent->byIndicator;
                    psCurrent->byIndicator = bySetValue;
                    psPrev = psCurrent;
                    ++uSetCount;
                }
                break;
            case Renko::EType_Down: // prev was a down,
                {
                    if (0 == nDiff) { // if no diff.
                        const Renko::SSyncedRenkoFrame& sPrevFrame = psFrames[psPrev->uActualIndex];
                        if ((sPrevFrame.byType != Renko::EType_Down) && (sFrame.byType == Renko::EType_Down)) {
                            // replace previous down back to its older indicator value
                            psPrev->byIndicator = byPrevValue;
                            // replace previous with current
                            byPrevValue = psCurrent->byIndicator;
                            psCurrent->byIndicator = bySetValue;
                            psCurrent->byZZType = Renko::EType_Down;
                            psPrev = psCurrent;
                        }
                    }
                    else if (0 > nDiff) { // if current is still a down
                        // replace previous down back to its older indicator value
                        psPrev->byIndicator = byPrevValue;
                        // replace previous with current
                        byPrevValue = psCurrent->byIndicator;
                        psCurrent->byIndicator = bySetValue;
                        psCurrent->byZZType = Renko::EType_Down;
                        psPrev = psCurrent;
                    }
                    else {
                        if (fDeviation <= nDiff) { // if more than deviation, accept new point
                            psCurrent->byZZType = Renko::EType_Up;
                            byPrevValue = psCurrent->byIndicator;
                            psCurrent->byIndicator = bySetValue;
                            psPrev = psCurrent;
                            ++uSetCount;
                        }
                    }
                }
                break;
            case Renko::EType_Up: // prev was a up
                {
                    if (0 == nDiff) { // if no diff.
                        const Renko::SSyncedRenkoFrame& sPrevFrame = psFrames[psPrev->uActualIndex];
                        if ((sPrevFrame.byType != Renko::EType_Up) && (sFrame.byType == Renko::EType_Up)) {
                            // replace previous down back to its older indicator value
                            psPrev->byIndicator = byPrevValue;
                            // replace previous with current
                            byPrevValue = psCurrent->byIndicator;
                            psCurrent->byIndicator = bySetValue;
                            psCurrent->byZZType = Renko::EType_Up;
                            psPrev = psCurrent;
                        }
                    }
                    else if (0 < nDiff) { // if current is still a up
                        // replace previous down back to its older indicator value
                        psPrev->byIndicator = byPrevValue;
                        // replace previous with current
                        byPrevValue = psCurrent->byIndicator;
                        psCurrent->byIndicator = bySetValue;
                        psCurrent->byZZType = Renko::EType_Up;
                        psPrev = psCurrent;
                    }
                    else {
                        if (-fDeviation >= nDiff) { // if more than deviation, accept new point
                            psCurrent->byZZType = Renko::EType_Down;
                            byPrevValue = psCurrent->byIndicator;
                            psCurrent->byIndicator = bySetValue;
                            psPrev = psCurrent;
                            ++uSetCount;
                        }
                    }
                }
                break;
            default:
                ASSERT(FALSE);
                break;
        }
    }
    if (1 == uSetCount) { // if only 1st entry, reset back
        psZZs[0].byIndicator = byPrevStartIndicator;
    }
    return TRUE;
}
static BOOLEAN ProcessRythmZZ(ZZUtil::SRythmZZ* psZZs, const Renko::SShiftedRenkoInnerFrame* psFrames, UINT uNum, UINT uMax, FLOAT fDeviation, BYTE bySetValue)
{
    if (5 > uMax) { // skip if less than 5 points..
        return TRUE;
    }
    ZZUtil::SRythmZZ* psPrev = &psZZs[0];
    ASSERT(uMax > psPrev->uActualIndex);
    psPrev->byZZType = Renko::EType_Neutral;
    const BYTE byPrevStartIndicator = psPrev->byIndicator;
    psPrev->byIndicator = bySetValue;
    BYTE byPrevValue = 0;
    ZZUtil::SRythmZZ* psCurrent = NULL;
    UINT uSetCount = 1;
    for (UINT uIndex = 1; uNum > uIndex; ++uIndex) {
        psCurrent = &psZZs[uIndex];
        ASSERT(uMax > psCurrent->uActualIndex);
        const Renko::SShiftedRenkoInnerFrame& sFrame = psFrames[psCurrent->uActualIndex];
        const INT nDiff = psCurrent->nPriceIndex - psPrev->nPriceIndex;
        switch (psPrev->byZZType) {
            case Renko::EType_Neutral:
                if (abs(nDiff) >= fDeviation) { // if more than deviation, accept new point
                    const BOOLEAN bUpwards = (0 < nDiff);
                    psPrev->byZZType = bUpwards ? Renko::EType_Down : Renko::EType_Up;
                    psCurrent->byZZType = bUpwards ? Renko::EType_Up : Renko::EType_Down;
                    byPrevValue = psCurrent->byIndicator;
                    psCurrent->byIndicator = bySetValue;
                    psPrev = psCurrent;
                    ++uSetCount;
                }
                break;
            case Renko::EType_Down: // prev was a down,
                {
                    if (0 == nDiff) { // if no diff.
                        const Renko::SShiftedRenkoInnerFrame& sPrevFrame = psFrames[psPrev->uActualIndex];
                        if ((sPrevFrame.byType != Renko::EType_Down) && (sFrame.byType == Renko::EType_Down)) {
                            ASSERT(sPrevFrame.uIndex == 0);
                            // replace previous down back to its older indicator value
                            psPrev->byIndicator = byPrevValue;
                            // replace previous with current
                            byPrevValue = psCurrent->byIndicator;
                            psCurrent->byIndicator = bySetValue;
                            psCurrent->byZZType = Renko::EType_Down;
                            psPrev = psCurrent;
                        }
                    }
                    else if (0 > nDiff) { // if current is still a down
                        // replace previous down back to its older indicator value
                        psPrev->byIndicator = byPrevValue;
                        // replace previous with current
                        byPrevValue = psCurrent->byIndicator;
                        psCurrent->byIndicator = bySetValue;
                        psCurrent->byZZType = Renko::EType_Down;
                        psPrev = psCurrent;
                    }
                    else {
                        if (fDeviation <= nDiff) { // if more than deviation, accept new point
                            psCurrent->byZZType = Renko::EType_Up;
                            byPrevValue = psCurrent->byIndicator;
                            psCurrent->byIndicator = bySetValue;
                            psPrev = psCurrent;
                            ++uSetCount;
                        }
                    }
                }
                break;
            case Renko::EType_Up: // prev was a up
                {
                    if (0 == nDiff) { // if no diff.
                        const Renko::SShiftedRenkoInnerFrame& sPrevFrame = psFrames[psPrev->uActualIndex];
                        if ((sPrevFrame.byType != Renko::EType_Up) && (sFrame.byType == Renko::EType_Up)) {
                            // replace previous down back to its older indicator value
                            psPrev->byIndicator = byPrevValue;
                            // replace previous with current
                            byPrevValue = psCurrent->byIndicator;
                            psCurrent->byIndicator = bySetValue;
                            psCurrent->byZZType = Renko::EType_Up;
                            psPrev = psCurrent;
                        }
                    }
                    else if (0 < nDiff) { // if current is still a up
                        // replace previous down back to its older indicator value
                        psPrev->byIndicator = byPrevValue;
                        // replace previous with current
                        byPrevValue = psCurrent->byIndicator;
                        psCurrent->byIndicator = bySetValue;
                        psCurrent->byZZType = Renko::EType_Up;
                        psPrev = psCurrent;
                    }
                    else {
                        if (-fDeviation >= nDiff) { // if more than deviation, accept new point
                            psCurrent->byZZType = Renko::EType_Down;
                            byPrevValue = psCurrent->byIndicator;
                            psCurrent->byIndicator = bySetValue;
                            psPrev = psCurrent;
                            ++uSetCount;
                        }
                    }
                }
                break;
            default:
                ASSERT(FALSE);
                break;
        }
    }
    if (1 == uSetCount) { // if only 1st entry, reset back
        psZZs[0].byIndicator = byPrevStartIndicator;
    }
    return TRUE;
}

static BOOLEAN ProcessSR_RythmZZ(ZZUtil::SRythmZZ* psZZs, const Renko::SShiftedRenko* psFrames, UINT uNum, UINT uMax, FLOAT fDeviation, BYTE bySetValue)
{
    if (5 > uMax) { // skip if less than 5 points..
        return TRUE;
    }
    ZZUtil::SRythmZZ* psPrev = &psZZs[0];
    ASSERT(uMax > psPrev->uActualIndex);
    psPrev->byZZType = Renko::EType_Neutral;
    const BYTE byPrevStartIndicator = psPrev->byIndicator;
    psPrev->byIndicator = bySetValue;
    BYTE byPrevValue = 0;
    ZZUtil::SRythmZZ* psCurrent = NULL;
    UINT uSetCount = 1;
    for (UINT uIndex = 1; uNum > uIndex; ++uIndex) {
        psCurrent = &psZZs[uIndex];
        ASSERT(uMax > psCurrent->uActualIndex);
        const Renko::SShiftedRenko& sFrame = psFrames[psCurrent->uActualIndex];
        const INT nDiff = psCurrent->nPriceIndex - psPrev->nPriceIndex;
        switch (psPrev->byZZType) {
            case Renko::EType_Neutral:
                if (abs(nDiff) >= fDeviation) { // if more than deviation, accept new point
                    const BOOLEAN bUpwards = (0 < nDiff);
                    psPrev->byZZType = bUpwards ? Renko::EType_Down : Renko::EType_Up;
                    psCurrent->byZZType = bUpwards ? Renko::EType_Up : Renko::EType_Down;
                    byPrevValue = psCurrent->byIndicator;
                    psCurrent->byIndicator = bySetValue;
                    psPrev = psCurrent;
                    ++uSetCount;
                }
                break;
            case Renko::EType_Down: // prev was a down,
                {
                    if (0 == nDiff) { // if no diff.
                        const Renko::SShiftedRenko& sPrevFrame = psFrames[psPrev->uActualIndex];
                        if ((sPrevFrame.byBarType != Renko::EType_Down) && (sFrame.byBarType == Renko::EType_Down)) {
                            // replace previous down back to its older indicator value
                            psPrev->byIndicator = byPrevValue;
                            // replace previous with current
                            byPrevValue = psCurrent->byIndicator;
                            psCurrent->byIndicator = bySetValue;
                            psCurrent->byZZType = Renko::EType_Down;
                            psPrev = psCurrent;
                        }
                    }
                    else if (0 > nDiff) { // if current is still a down
                        // replace previous down back to its older indicator value
                        psPrev->byIndicator = byPrevValue;
                        // replace previous with current
                        byPrevValue = psCurrent->byIndicator;
                        psCurrent->byIndicator = bySetValue;
                        psCurrent->byZZType = Renko::EType_Down;
                        psPrev = psCurrent;
                    }
                    else {
                        if (fDeviation <= nDiff) { // if more than deviation, accept new point
                            psCurrent->byZZType = Renko::EType_Up;
                            byPrevValue = psCurrent->byIndicator;
                            psCurrent->byIndicator = bySetValue;
                            psPrev = psCurrent;
                            ++uSetCount;
                        }
                    }
                }
                break;
            case Renko::EType_Up: // prev was a up
                {
                    if (0 == nDiff) { // if no diff.
                        const Renko::SShiftedRenko& sPrevFrame = psFrames[psPrev->uActualIndex];
                        if ((sPrevFrame.byBarType != Renko::EType_Up) && (sFrame.byBarType == Renko::EType_Up)) {
                            // replace previous down back to its older indicator value
                            psPrev->byIndicator = byPrevValue;
                            // replace previous with current
                            byPrevValue = psCurrent->byIndicator;
                            psCurrent->byIndicator = bySetValue;
                            psCurrent->byZZType = Renko::EType_Up;
                            psPrev = psCurrent;
                        }
                    }
                    else if (0 < nDiff) { // if current is still a up
                        // replace previous down back to its older indicator value
                        psPrev->byIndicator = byPrevValue;
                        // replace previous with current
                        byPrevValue = psCurrent->byIndicator;
                        psCurrent->byIndicator = bySetValue;
                        psCurrent->byZZType = Renko::EType_Up;
                        psPrev = psCurrent;
                    }
                    else {
                        if (-fDeviation >= nDiff) { // if more than deviation, accept new point
                            psCurrent->byZZType = Renko::EType_Down;
                            byPrevValue = psCurrent->byIndicator;
                            psCurrent->byIndicator = bySetValue;
                            psPrev = psCurrent;
                            ++uSetCount;
                        }
                    }
                }
                break;
            default:
                ASSERT(FALSE);
                break;
        }
    }
    if (1 == uSetCount) { // if only 1st entry, reset back
        psZZs[0].byIndicator = byPrevStartIndicator;
    }
    return TRUE;
}

BOOLEAN ZZUtil::GenerateZZ(SRythmZZ* psZZs,
                           const Renko::SShiftedRenkoInnerFrame* psFrames,
                           UINT uUsed,
                           UINT uMax,
                           UINT uCurrentIndex,
                           UINT uRenkoPerFrame)
{
    const UINT uTotalZZLevels = GetZZLevelNum();
    ASSERT(10 >= uTotalZZLevels);
    FLOAT afDeviations[10];
    for (UINT uIndex = 0; uTotalZZLevels > uIndex; ++uIndex) {    // for each zz level
        afDeviations[uIndex] = ZZUtil::GetInfo(uIndex).fDeviationPercent;
    }
    return GenerateCustomZZ(psZZs, psFrames, uUsed, uMax, uCurrentIndex, uRenkoPerFrame, uTotalZZLevels, afDeviations);
}
BOOLEAN ZZUtil::GenerateCustomZZ(SRythmZZ* psZZs, const Renko::SShiftedRenkoInnerFrame* psFrames, UINT uUsed, UINT uMax, UINT uCurrentIndex, UINT uRenkoPerFrame, UINT uZZLevelNum, const FLOAT* pfDeviations)
{
    if (5 > uUsed) { // less than 5, skip
        return TRUE;
    }
    ASSERT(0 != uMax);
    const UINT uStartIndex = (uCurrentIndex + uMax - uUsed) % uMax;
    UINT uActualIndex = uStartIndex;
    for (UINT uIndex = 0; uUsed > uIndex; ++uIndex) {
        const Renko::SShiftedRenkoInnerFrame& sFrame = psFrames[uActualIndex];
        SRythmZZ& sZZ = psZZs[uIndex];
        sZZ.uActualIndex = uActualIndex;
        sZZ.nPriceIndex = sFrame.nLowerIndex + sFrame.uIndex;
        sZZ.byZZType = Renko::EType_Neutral;
        sZZ.byIndicator = 0xFF;
        ++uActualIndex;
        ASSERT(uMax >= uActualIndex);
        if (uMax == uActualIndex) { // to revent using % as it is heavy operation
            uActualIndex -= uMax;
        }
    }
    ASSERT(0x7F > uZZLevelNum); // cannot be too big
    for (UINT uIndex = 0; uZZLevelNum > uIndex; ++uIndex) {    // for each zz level
        const FLOAT fDeviation = pfDeviations[uIndex] * 0.01f * uRenkoPerFrame;
        ProcessRythmZZ(psZZs, psFrames, uUsed, uMax, fDeviation, uIndex);
    }
    return TRUE;
}

static UINT GetRetraceIndex(const Renko::SShiftedRenko& sSR) {
    UINT uRetraceIndex;
    if (Renko::EType_Down == sSR.byBarType) {
        uRetraceIndex = (UNUSED_INDEX == sSR.byHighRetraceIndex) ? 0 : sSR.byHighRetraceIndex;
    }
    else {
        uRetraceIndex = (UNUSED_INDEX == sSR.byLowRetraceIndex) ? (sSR.byRankoNum - 1) : sSR.byLowRetraceIndex;
    }
    return uRetraceIndex;
}
static INT GetRetracePriceIndex(const Renko::SShiftedRenko& sSR) {
    return sSR.nLowerIndex + GetRetraceIndex(sSR);
}
// TODO: skip 1st SR and append the last SR twice (retrace and current index)
BOOLEAN ZZUtil::GenerateZZ(SRythmZZ* psZZs,
                           const Renko::SShiftedRenko* psFrames,
                           UINT uUsed,
                           UINT uMax,
                           UINT uCurrentIndex,
                           UINT uRenkoPerFrame)
{
    if (5 > uUsed) { // less than 5, skip
        return TRUE;
    }
    ASSERT(uMax > uCurrentIndex);
    const Renko::SShiftedRenko& sFirstFrame = psFrames[uCurrentIndex];
    const UINT uRetraceIndex = GetRetraceIndex(sFirstFrame);
    const BOOLEAN bNeedAddCurrentIndex = sFirstFrame.byCurrentIndex != uRetraceIndex;
    if (bNeedAddCurrentIndex) {
        --uUsed;    // reduce use count by 1
    }
    const UINT uStartIndex = (uCurrentIndex + uMax - (uUsed - 1)) % uMax;
    UINT uActualIndex = uStartIndex;
    UINT uIndex = 0;
    for (; uUsed > uIndex; ++uIndex) {
        SRythmZZ& sZZ = psZZs[uIndex];
        sZZ.uActualIndex = uActualIndex;
        sZZ.nPriceIndex = GetRetracePriceIndex(psFrames[uActualIndex]);
        sZZ.byZZType = Renko::EType_Neutral;
        sZZ.byIndicator = 0xFF;
        ++uActualIndex;
        ASSERT(uMax >= uActualIndex);
        if (uMax == uActualIndex) { // to revent using % as it is heavy operation
            uActualIndex -= uMax;
        }
    }
    ASSERT(uActualIndex == (uCurrentIndex + 1));
    if (bNeedAddCurrentIndex) {
        ASSERT(uIndex == uUsed);
        SRythmZZ& sZZ = psZZs[uIndex];
        sZZ.uActualIndex = uCurrentIndex;
        sZZ.nPriceIndex = sFirstFrame.nLowerIndex + sFirstFrame.byCurrentIndex;
        sZZ.byZZType = Renko::EType_Neutral;
        sZZ.byIndicator = 0xFF;
        ++uUsed; // add back used count
    }
    
    const UINT uTotalZZLevels = GetZZLevelNum();
    for (UINT uZZLevel = 0; uTotalZZLevels > uZZLevel; ++uZZLevel) {    // for each zz level
        const FLOAT fDeviation = ZZUtil::GetInfo(uZZLevel).fDeviationPercent * 0.01f * uRenkoPerFrame;
        ProcessSR_RythmZZ(psZZs, psFrames, uUsed, uMax, fDeviation, uZZLevel);
    }
    return TRUE;
}

static UINT GetRetraceIndex(const Renko::SSyncedRenkoFrame& sFrame, UINT uFrameBoxNum) {
    UINT uRetraceIndex;
    if (Renko::EType_Down == sFrame.byType) {
        uRetraceIndex = (UNUSED_INDEX == sFrame.byHighRetraceIndex) ? 0 : sFrame.byHighRetraceIndex;
    }
    else {
        uRetraceIndex = (UNUSED_INDEX == sFrame.byLowRetraceIndex) ? (uFrameBoxNum - 1) : sFrame.byLowRetraceIndex;
    }
    return uRetraceIndex;
}
static INT GetRetracePriceIndex(const Renko::SSyncedRenkoFrame& sFrame, UINT uFrameBoxNum) {
    return sFrame.nLowerIndex + GetRetraceIndex(sFrame, uFrameBoxNum);
}

BOOLEAN ZZUtil::GenerateZZ(SRythmZZ* psZZs,
                           const Renko::SSyncedRenkoFrame* psFrames,
                           UINT uUsed,
                           UINT uMax,
                           UINT uCurrentIndex,
                           UINT uRenkoPerFrame)
{
    if (5 > uUsed) { // less than 5, skip
        return TRUE;
    }
    ASSERT(uMax > uCurrentIndex);
    const UINT uStartIndex = (uCurrentIndex + uMax - (uUsed - 1)) % uMax;
    UINT uActualIndex = uStartIndex;
    UINT uIndex = 0;
    for (; uUsed > uIndex; ++uIndex) {
        ASSERT(uMax >= uActualIndex);
        SRythmZZ& sZZ = psZZs[uIndex];
        sZZ.uActualIndex = uActualIndex;
        sZZ.nPriceIndex = GetRetracePriceIndex(psFrames[uActualIndex], uRenkoPerFrame);
        sZZ.byZZType = Renko::EType_Neutral;
        sZZ.byIndicator = 0xFF;
        ++uActualIndex;
        if (uMax == uActualIndex) { // to revent using % as it is heavy operation
            uActualIndex -= uMax;
        }
    }
    ASSERT(uActualIndex == (uCurrentIndex + 1));
    const UINT uTotalZZLevels = GetZZLevelNum();
    for (UINT uZZLevel = 0; uTotalZZLevels > uZZLevel; ++uZZLevel) {    // for each zz level
        const FLOAT fDeviation = ZZUtil::GetInfo(uZZLevel).fDeviationPercent * 0.01f * uRenkoPerFrame;
        ProcessSR_RythmZZ(psZZs, psFrames, uUsed, uMax, fDeviation, uZZLevel);
    }
    return TRUE;
}
