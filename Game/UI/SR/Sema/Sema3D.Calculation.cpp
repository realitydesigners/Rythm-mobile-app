#include "stdafx.h"
#include "HLTime.h"
#include "PlayerData.h"
#include "Sema3D.h"
#include <math.h>

#define BLACK_COLOR             RGBA(0,0,0,0xFF)
#define ZZ_LINE_COLOR           BLACK_COLOR
#define PRICE_TO_HEIGHT_RATIO   (0.5f)
#define OFFSET_PER_DEPTH        (40.0f)

VOID CSema3D::PrepareZZ(const CMegaZZ& cMegaZZ)
{
    CMegaZZ* pcMegaZZ = const_cast<CMegaZZ*>(&cMegaZZ);
    m_uUsedChannelZZ = pcMegaZZ->GenerateMegaZZ(m_ppsZZs, CHANNEL_ZZ_DATA_NUM);
    if (5 > m_uUsedChannelZZ) {
        return;
    }
    ASSERT(CHANNEL_ZZ_DATA_NUM >= m_uUsedChannelZZ);
    const MegaZZ::SFrame* psFrames = pcMegaZZ->GetFrames();
    const UINT uFrameNum = pcMegaZZ->GetFrameUsedNum();
    for (UINT uIndex = 0; m_uUsedChannelZZ > uIndex; ++uIndex) {
        const ZZUtil::SFloatValue* psZZ = m_ppsZZs[uIndex];
        // compute actual Y from price
        const DOUBLE dCurrentPrice = psZZ->fValue;
        const FLOAT fValue = (FLOAT)(m_fReferenceValue + (dCurrentPrice - m_fReferencePrice) * m_fPixelPerPrice);
        ASSERT(uFrameNum > psZZ->ushActualIndex);
        const MegaZZ::SFrame& sFrame = psFrames[psZZ->ushActualIndex];
        SChannelZZData& sData = m_asChannelZZ[uIndex];
        sData.fX = 0.0f;
        sData.fPriceY = fValue;
        sData.uTimeStamp = sFrame.uUpdateTime;
        sData.byZZLevel = psZZ->byZZIndicator;
    }
    GenerateRenderables();
}

VOID CSema3D::InitializeChannelData(SChannelData& sChannel)
{
    // determine if 1st line is going up or down
    ASSERT(0 < m_uUsedChannelZZ);
    ASSERT(NULL != m_pcMegaZZ);
    ASSERT(NULL != m_ppsZZs);
    const UINT uActualIndex = m_ppsZZs[0]->ushActualIndex;
    ASSERT(m_pcMegaZZ->GetFrameUsedNum() > uActualIndex);
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        const MegaZZ::SChannel& sOldest = m_pcMegaZZ->GetChannels(uIndex)[uActualIndex];
        const FLOAT fBottom = m_fReferenceValue + (sOldest.fBottom - m_fReferencePrice) * m_fPixelPerPrice;
        const FLOAT fTop = m_fReferenceValue + (sOldest.fTop - m_fReferencePrice) * m_fPixelPerPrice;
        sChannel.afX[uIndex] = 0.0f;
        sChannel.abUp[uIndex] = (sOldest.byType == MegaZZ::EType_Up);
        sChannel.afBottom[uIndex] = fBottom;
        sChannel.afTop[uIndex] = fTop;
    }
    // check and make sure within range
    const FLOAT fNowPriceValue = m_asChannelZZ[0].fPriceY;
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        if (sChannel.afBottom[uIndex] > fNowPriceValue) {
            sChannel.afBottom[uIndex] = fNowPriceValue;
        }
        else if (sChannel.afTop[uIndex] < fNowPriceValue) {
            sChannel.afTop[uIndex] = fNowPriceValue;
        }
    }
}

UINT CSema3D::ProcessChannelData(SChannelData& sChannel)
{
    FLOAT fPrevMovementValue = 0.0f;
    FLOAT fPrevPriceValue = 0.0f;
    const SChannelZZData* psPrev = NULL;
    UINT uLastDataAccessIndex = m_uUsedChannelZZ;
    for (UINT uIndex = 0; m_uUsedChannelZZ > uIndex; ++uIndex) { // from oldest to newest
        SChannelZZData& sData = m_asChannelZZ[uIndex];
        const FLOAT fPriceY = sData.fPriceY;
        if (NULL == psPrev) { // if prev point does not exist
            psPrev = &sData;
            fPrevPriceValue = fPriceY;
            continue;
        }
        const FLOAT fDiffPriceValue = fPriceY - fPrevPriceValue;
        if (0 == fDiffPriceValue) {
            sData.fX = psPrev->fX;
            psPrev = &sData;
            fPrevPriceValue = fPriceY;
            continue;
        }
        sData.fX = psPrev->fX + PRICE_TO_HEIGHT_RATIO * abs(fDiffPriceValue);
        // for each channel (biggest to smallest)
        for (UINT uChannelIndex = 0; MEGAZZ_MAX_DEPTH > uChannelIndex; ++uChannelIndex) {
            const FLOAT fTop = sChannel.afTop[uChannelIndex];
            const FLOAT fBottom = sChannel.afBottom[uChannelIndex];
            const FLOAT fChannelStart = sChannel.afX[uChannelIndex];
            ASSERT(fPrevPriceValue <= fTop);
            ASSERT(fPrevPriceValue >= fBottom);
            if (fPriceY > fTop) { // line intersect top
                ASSERT(0.0f < fDiffPriceValue);
                const FLOAT fNewTop = fPriceY;
                const FLOAT fTopDiff = fNewTop - fTop;
                ASSERT(0.0f < fTopDiff);
                const FLOAT fMoved = fChannelStart + fTopDiff * PRICE_TO_HEIGHT_RATIO;
                const FLOAT fNewBottom = fNewTop - m_afChannelPixelSize[uChannelIndex];
                sChannel.abUp[uChannelIndex] = TRUE;
                sChannel.afX[uChannelIndex] = fMoved;
                sChannel.afTop[uChannelIndex] = fNewTop;
                sChannel.afBottom[uChannelIndex] = fNewBottom;
                if (!m_bAllowFlatline) {
                    fPrevMovementValue = fMoved;
                }
                uLastDataAccessIndex = uIndex;
            }
            else if (fPriceY < fBottom) { // line intersect bottom
                ASSERT(0.0f > fDiffPriceValue);
                const FLOAT fNewBottom = fPriceY;
                const FLOAT fBottomDiff = fBottom - fNewBottom;
                ASSERT(0.0f < fBottomDiff);
                const FLOAT fMoved = fChannelStart + fBottomDiff * PRICE_TO_HEIGHT_RATIO;
                const FLOAT fNewTop = fNewBottom + m_afChannelPixelSize[uChannelIndex];
                sChannel.abUp[uChannelIndex] = FALSE;
                sChannel.afX[uChannelIndex] = fMoved;
                sChannel.afTop[uChannelIndex] = fNewTop;
                sChannel.afBottom[uChannelIndex] = fNewBottom;
                if (!m_bAllowFlatline) {
                    fPrevMovementValue = fMoved;
                }
                uLastDataAccessIndex = uIndex;
            } // end if
        } // end for
        psPrev = &sData;
        if (m_bAllowFlatline) {
            fPrevMovementValue = sData.fX;
        }
        else {
            sData.fX = fPrevMovementValue;
        }
        fPrevPriceValue = fPriceY;
    }
    return uLastDataAccessIndex;
}

VOID CSema3D::ProcessProtudingZZLine(FLOAT fZZLineStart, UINT uLastDataAccessIndex)
{
    const FLOAT fLineZ = (FLOAT)(Z_THICKNESS + MEGAZZ_MAX_DEPTH);
    if (m_uUsedChannelZZ > uLastDataAccessIndex && 0 < uLastDataAccessIndex) { // if previous ZZ exists
        const SChannelZZData& sPrev = m_asChannelZZ[uLastDataAccessIndex-1];
        const SChannelZZData& sData = m_asChannelZZ[uLastDataAccessIndex];
        if (sData.byZZLevel != 0xFF && m_bDisplaySemaphore) {
            const BOOLEAN bUp = sPrev.fPriceY < sData.fPriceY;
            AddSemaphore(fZZLineStart, sData.fPriceY, fLineZ, sData.byZZLevel, bUp, 0xFFFFFFFF);
        }
    }
    FLOAT fPrevMovement = fZZLineStart - 15.0f;
    const SChannelZZData* psPrev = NULL;
    for (UINT uIndex = uLastDataAccessIndex; m_uUsedChannelZZ > uIndex; ++uIndex) { // from oldest to newest
        const SChannelZZData& sData = m_asChannelZZ[uIndex];
        if (NULL == psPrev) {
            psPrev = &sData;
            continue;
        }
        const FLOAT fDiffPrice = abs(psPrev->fPriceY - sData.fPriceY);
        const FLOAT fNowMovement = fPrevMovement + fDiffPrice * PRICE_TO_HEIGHT_RATIO;
        AddLine(fPrevMovement, psPrev->fPriceY, 0.0f, fNowMovement, sData.fPriceY, 0.0f, ZZ_LINE_COLOR);
        if (sData.byZZLevel != 0xFF && m_bDisplaySemaphore) {
            const BOOLEAN bUp = psPrev->fPriceY < sData.fPriceY;
            AddSemaphore(fNowMovement, sData.fPriceY, 0.0f, sData.byZZLevel, bUp, 0xFFFFFFFF);
        }
        fPrevMovement = fNowMovement;
        psPrev = &sData;
    }
}

VOID CSema3D::ProcessEndChannelData(FLOAT fZZLineStart, const SChannelData& sChannel)
{
    const SHLVector2D& sSize = GetLocalSize();
    const FLOAT fEndX = sSize.x * 0.5f;
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        if (!m_abDepthEnabled[uIndex]) {
            continue;
        }
        const FLOAT fEndMove = fEndX - (15.0f + uIndex * OFFSET_PER_DEPTH);
        const FLOAT fTop = sChannel.afTop[uIndex];
        const FLOAT fBottom = sChannel.afBottom[uIndex];
        const UINT uColor = m_auSelectableDepthColors[uIndex];
        const UINT uLineColor = m_auLineDepthColors[uIndex];
        const FLOAT fZ = (FLOAT)(Z_THICKNESS + uIndex);
        AddPolygonAsBox(fZZLineStart, fTop, fEndMove, fBottom, fZ, uColor, uIndex);
        AddLine(fZZLineStart, fTop, fZ, fEndMove, fTop, fZ, uLineColor);
        AddLine(fZZLineStart, fBottom, fZ, fEndMove, fBottom, fZ, uLineColor);
        AddLine(fEndMove, fTop, fZ, fEndMove, fBottom, fZ, uLineColor);
        CUITextLabel* pcLbl = m_apcChannelTxt[uIndex];
        if (NULL == pcLbl) {
            continue;
        }
        pcLbl->SetVisibility(FALSE);
        const BOOLEAN bUp = sChannel.abUp[uIndex];
        pcLbl->SetAlignment(CUITextLabel::EALIGNMENT_RIGHT);
        pcLbl->SetAnchor(1.0f, bUp ? 1.0f : 0.0f);
        pcLbl->SetLocalPosition(fEndMove, bUp ? fBottom : fTop);
        pcLbl->SetVisibility(TRUE);
        pcLbl->OnUpdate(0.0f);
    }
}
VOID CSema3D::GenerateRenderables(VOID)
{
    ClearChart(); // clear for safety
    ClearExperiment();
    if (5 > m_uUsedChannelZZ) {
        return;
    }
    UINT uDrawBoxIndex = MEGAZZ_MAX_DEPTH;
    UINT uLargestChannelIndex = MEGAZZ_MAX_DEPTH;
    for (UINT uBoxIndex = 0; MEGAZZ_MAX_DEPTH > uBoxIndex; ++uBoxIndex) {
        if (m_abDepthEnabled[uBoxIndex]) {
            uDrawBoxIndex = uBoxIndex;
            if (uBoxIndex < uLargestChannelIndex) {
                uLargestChannelIndex = uBoxIndex;
            }
        }
    }
    if (MEGAZZ_MAX_DEPTH == uDrawBoxIndex) { // none of the depths are enabled skip
        return;
    }
    SChannelData sChannelData;
    InitializeChannelData(sChannelData); // initialize the channel data
    
    // do 1 compute to determine channel Value (axis of movement)
    const UINT uLastDataAccessIndex = ProcessChannelData(sChannelData);
    
    // compute distance required to "extend from the vertical line"
    FLOAT fDist = 5.0f;
    {
        const SChannelZZData* psPrev = NULL;
        for (UINT uIndex = uLastDataAccessIndex; m_uUsedChannelZZ > uIndex; ++uIndex) { // from oldest to newest
            const SChannelZZData& sData = m_asChannelZZ[uIndex];
            if (NULL == psPrev) {
                psPrev = &sData;
                continue;
            }
            const FLOAT fDiffPrice = abs(psPrev->fPriceY - sData.fPriceY);
            const FLOAT fDiffMovement = fDiffPrice * PRICE_TO_HEIGHT_RATIO;
            fDist += fDiffMovement;
            psPrev = &sData;
        }
    }
    const SHLVector2D& sSize = GetLocalSize();
    const FLOAT fEndX = sSize.x * 0.5f;
    const FLOAT fRenderEndMovement = fEndX - (15.0f + MEGAZZ_MAX_DEPTH * OFFSET_PER_DEPTH);
    const FLOAT fZZLineStart = fRenderEndMovement;
    
    // render the historical channel data
    ProcessHistory(fZZLineStart - fDist, uDrawBoxIndex, uLargestChannelIndex, uLastDataAccessIndex, sChannelData);
    // update channel labels
    UpdateChannelLabels(sChannelData);
    if (!m_bNewMethod) {
        // render the ends of the channels to the zz line
        ProcessBeforeEndChannelData(fZZLineStart, sChannelData);
        // render the zz line
        ProcessProtudingZZLine(fZZLineStart - fDist, uLastDataAccessIndex);
        // render end channel lines and channel text
        ProcessEndChannelData(fZZLineStart, sChannelData);
        return;
    }
    ProcessNewEndChannelData(fZZLineStart + fDist, fDist, uLastDataAccessIndex, sChannelData);
    EndExperiment(fZZLineStart + fDist, m_asChannelZZ[m_uUsedChannelZZ-1].fPriceY, sChannelData);
}

VOID CSema3D::ProcessBeforeEndChannelData(FLOAT fZZLineStart, const SChannelData& sChannel)
{
    for (UINT uDepthIndex = 0; MEGAZZ_MAX_DEPTH > uDepthIndex; ++uDepthIndex) {
        if (!m_abDepthEnabled[uDepthIndex]) {
            continue;;
        }
        const FLOAT fEndMove = sChannel.afX[uDepthIndex];
        if (fEndMove >= fZZLineStart) {
            continue;
        }
        const FLOAT fTop = sChannel.afTop[uDepthIndex];
        const FLOAT fBottom = sChannel.afBottom[uDepthIndex];
        const UINT uColor = m_auSelectableDepthColors[uDepthIndex];
        const UINT uLineColor = m_auLineDepthColors[uDepthIndex];
        AddPolygonAsBox(fEndMove, fTop, fZZLineStart, fBottom, 0.0f, uColor, uDepthIndex);
        AddLine(fZZLineStart, fTop, 0.0f, fEndMove, fTop, 0.0f, uLineColor);
        AddLine(fZZLineStart, fBottom, 0.0f, fEndMove, fBottom, 0.0f, uLineColor);
    }
}


VOID CSema3D::ProcessHistory(FLOAT fZZLineStart, UINT uDrawBoxIndex, UINT uLargestChannelIndex, UINT uLastDataAccessIndex, SChannelData& sChannelData)
{
    if (0 == m_uUsedChannelZZ) {
        ASSERT(FALSE);
        return;
    }
    const FLOAT fLatestBiggestChannelBottom = sChannelData.afBottom[uLargestChannelIndex];
    const FLOAT fLineZ = 0.0f;
    const FLOAT fHistoryMovementEnd = m_bAllowFlatline ? m_asChannelZZ[m_uUsedChannelZZ-1].fX : sChannelData.afX[uDrawBoxIndex];
    const FLOAT fOffsetMovement = fZZLineStart - fHistoryMovementEnd - 15.0f;
    InitializeChannelData(sChannelData); // reset the boxes
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        sChannelData.afX[uIndex] = fOffsetMovement;
    }
    StartExperiment(sChannelData, fOffsetMovement);
    FLOAT fPrevMovementValue = fOffsetMovement;
    FLOAT fPrevPriceValue = 0.0f;
    const SChannelZZData* psPrev = NULL;
    const SChannelZZData* psPreviousPossibleZZ = NULL;
    const SChannelZZData* psPreviousPossibleZZ2 = NULL;
    
    const UINT uNowSec = CHLTime::GetTimeSecs();
    const FLOAT fMidY = GetLocalSize().y * 0.5f;
    const BOOLEAN bShowTimeline = CPlayerData::IsTimelineEnabled();
    const SHLVector2D& sSize = GetLocalSize();
    for (UINT uIndex = 0; m_uUsedChannelZZ > uIndex; ++uIndex) { // from oldest to newest
        const SChannelZZData& sData = m_asChannelZZ[uIndex];
        const FLOAT fPriceY = sData.fPriceY;
        if (NULL == psPrev) { // if prev point does not exist
            psPrev = &sData;
            if (m_bAllowFlatline) {
                fPrevMovementValue = fOffsetMovement + sData.fX;
            }
            fPrevPriceValue = sData.fPriceY;
            continue;
        }
        const FLOAT fDiffPriceValue = fPriceY - fPrevPriceValue;
        if (0 == fDiffPriceValue) {
            psPrev = &sData;
            if (m_bAllowFlatline) {
                fPrevMovementValue = fOffsetMovement + sData.fX;
            }
            fPrevPriceValue = sData.fPriceY;
            continue;
        }
        
        const FLOAT fPrevFlatlinePriceValue = fPrevPriceValue;
        const FLOAT fNowFlatlinePriceValue = sData.fPriceY;
        // we do smallest channel to biggest channel this time
        for (INT nChannelIndex = MEGAZZ_MAX_DEPTH-1; 0 <= nChannelIndex; --nChannelIndex) {
            const BOOLEAN bSmallestChannel = uDrawBoxIndex == nChannelIndex;
            const FLOAT fPrevTop = sChannelData.afTop[nChannelIndex];
            const FLOAT fPrevBottom = sChannelData.afBottom[nChannelIndex];
            const FLOAT fPrevChannelStart = sChannelData.afX[nChannelIndex];
            ASSERT(fPrevPriceValue <= fPrevTop);
            ASSERT(fPrevPriceValue >= fPrevBottom);
            BOOLEAN bDrawLines = FALSE;
            BOOLEAN bWasFlipped = FALSE;
            const BOOLEAN bWasUp = sChannelData.abUp[nChannelIndex];
            FLOAT fMidPointMoveValue = 0.0f;
            if (fPriceY > fPrevTop) { // line intersect top
                ASSERT(0.0f < fDiffPriceValue);
                const FLOAT fNewTop = fPriceY;
                const FLOAT fNewBottom = fNewTop - m_afChannelPixelSize[nChannelIndex];
                bWasFlipped = !bWasUp;
                sChannelData.abUp[nChannelIndex] = TRUE;
                sChannelData.afTop[nChannelIndex] = fNewTop;
                sChannelData.afBottom[nChannelIndex] = fNewBottom;
                bDrawLines = TRUE;
                if (bSmallestChannel && !m_bAllowFlatline) {
                    const FLOAT fTopDiff = fNewTop - fPrevTop;
                    ASSERT(0.0f < fTopDiff);
                    const FLOAT fMovedPriceValue = fPrevChannelStart + fTopDiff * PRICE_TO_HEIGHT_RATIO;
                    sChannelData.afX[nChannelIndex] = fMovedPriceValue;
                    fPrevMovementValue = fPrevChannelStart;
                    fPrevPriceValue = fPrevTop;
                    psPreviousPossibleZZ2 = psPreviousPossibleZZ;
                    psPreviousPossibleZZ = &sData;
                }
                else {
                    // compute mid point x
                    const FLOAT fTopDiff = fPrevTop - fPrevPriceValue;
                    ASSERT(0.0f <= fTopDiff);
                    fMidPointMoveValue = fPrevMovementValue + fTopDiff * PRICE_TO_HEIGHT_RATIO;

                    const FLOAT fLineDiff = fPriceY - fPrevPriceValue;
                    ASSERT(0.0f < fLineDiff);
                    const FLOAT fNewMovementValue = fPrevMovementValue + fLineDiff * PRICE_TO_HEIGHT_RATIO;
                    sChannelData.afX[nChannelIndex] = fNewMovementValue;
                }
            }
            else if (fPriceY < fPrevBottom) { // line intersect bottom
                ASSERT(0.0f > fDiffPriceValue);
                const FLOAT fNewBottom = fPriceY;
                const FLOAT fNewTop = fNewBottom + m_afChannelPixelSize[nChannelIndex];
                bWasFlipped = bWasUp;
                sChannelData.abUp[nChannelIndex] = FALSE;
                sChannelData.afTop[nChannelIndex] = fNewTop;
                sChannelData.afBottom[nChannelIndex] = fNewBottom;
                bDrawLines = TRUE;
                if (bSmallestChannel && !m_bAllowFlatline) {
                    const FLOAT fBottomDiff = fPrevBottom - fNewBottom;
                    ASSERT(0.0f < fBottomDiff);
                    const FLOAT fNewMovementValue = fPrevChannelStart + fBottomDiff * PRICE_TO_HEIGHT_RATIO;
                    sChannelData.afX[nChannelIndex] = fNewMovementValue;
                    fPrevMovementValue = fPrevChannelStart;
                    fPrevPriceValue = fPrevBottom;
                    psPreviousPossibleZZ2 = psPreviousPossibleZZ;
                    psPreviousPossibleZZ = &sData;
                }
                else {
                    const FLOAT fMidPtDiff = fPrevPriceValue - fPrevBottom;
                    ASSERT(0.0f <= fMidPtDiff);
                    fMidPointMoveValue = fPrevMovementValue + fMidPtDiff * PRICE_TO_HEIGHT_RATIO;

                    const FLOAT fLineDiff = fPrevPriceValue - fPriceY;
                    ASSERT(0.0f <= fLineDiff);
                    const FLOAT fNewMovementValue = fPrevMovementValue + fLineDiff * PRICE_TO_HEIGHT_RATIO;
                    sChannelData.afX[nChannelIndex] = fNewMovementValue;
                }
            } // end if
            sChannelData.abDrawLine[nChannelIndex] = bDrawLines;
            sChannelData.abWasFlipped[nChannelIndex] = bWasFlipped;
            sChannelData.afPrevX[nChannelIndex] = fPrevChannelStart;
            sChannelData.afMidPtX[nChannelIndex] = fMidPointMoveValue;
            sChannelData.afPrevTop[nChannelIndex] = fPrevTop;
            sChannelData.afPrevBottom[nChannelIndex] = fPrevBottom;
        } // end for
        ProcessExperiment(sChannelData, fOffsetMovement + sData.fX, fNowFlatlinePriceValue);
        // find offset from the current latest frame top and bottom
        const FLOAT fPrevPriceOffset = m_bFlattenMode ? (fLatestBiggestChannelBottom - sChannelData.afPrevBottom[uLargestChannelIndex]) : 0.0f;
        const FLOAT fNowPriceOffset = m_bFlattenMode ? (fLatestBiggestChannelBottom - sChannelData.afBottom[uLargestChannelIndex]) : 0.0f;
        if (m_bAllowFlatline && uIndex <= uLastDataAccessIndex) {
            const FLOAT fX = fOffsetMovement + sData.fX;
            const FLOAT fPrevPriceValue = fPrevFlatlinePriceValue + fPrevPriceOffset;
            const FLOAT fNowPriceValue = fNowFlatlinePriceValue + fNowPriceOffset;
            AddLine(fPrevMovementValue, fPrevPriceValue, fLineZ, fX, fNowPriceValue, fLineZ, ZZ_LINE_COLOR);
        }
        if (m_bDisplayHistory) {
            for (INT nChannelIndex = 0; MEGAZZ_MAX_DEPTH > nChannelIndex; ++nChannelIndex) {
                const BOOLEAN bDrawLines = sChannelData.abDrawLine[nChannelIndex];
                if (!bDrawLines || !m_abDepthEnabled[nChannelIndex]) {
                    continue;
                }
                const BOOLEAN bSmallestChannel = uDrawBoxIndex == nChannelIndex;
                const UINT uColor = m_auSelectableDepthColors[nChannelIndex];
                const UINT uLineColor = m_auLineDepthColors[nChannelIndex];
                const FLOAT fNowValue = sChannelData.afX[nChannelIndex];
                const FLOAT fNowTop = sChannelData.afTop[nChannelIndex] + fNowPriceOffset;
                const FLOAT fNowBottom = sChannelData.afBottom[nChannelIndex] + fNowPriceOffset;
                const FLOAT fPrevChannelStart = sChannelData.afPrevX[nChannelIndex];
                const FLOAT fMidPointMoveValue = sChannelData.afMidPtX[nChannelIndex];
                const FLOAT fPrevTop = sChannelData.afPrevTop[nChannelIndex] + fPrevPriceOffset;
                const FLOAT fPrevBottom = sChannelData.afPrevBottom[nChannelIndex] + fPrevPriceOffset;
                const BOOLEAN bWasFlipped = sChannelData.abWasFlipped[nChannelIndex];
                if (bSmallestChannel) {
                    if (m_bAllowFlatline) {
                        if (fPrevChannelStart < fMidPointMoveValue) {
                            AddPolygonAsBox(fPrevChannelStart, fPrevTop, fMidPointMoveValue, fPrevBottom, 0.0f, uColor, nChannelIndex);
                            AddLine(fPrevChannelStart, fPrevTop, 0.0f, fMidPointMoveValue, fPrevTop, 0.0f, uLineColor);
                            AddLine(fPrevChannelStart, fPrevBottom, 0.0f, fMidPointMoveValue, fPrevBottom, 0.0f, uLineColor);
                        }
                        AddPolygon(fMidPointMoveValue, fPrevTop, fNowValue, fNowTop, fMidPointMoveValue, fPrevBottom, fNowValue, fNowBottom, 0.0f, uColor, nChannelIndex);
                        AddLine(fNowValue, fNowTop, 0.0f, fMidPointMoveValue, fPrevTop, 0.0f, uLineColor);
                        AddLine(fNowValue, fNowBottom, 0.0f, fMidPointMoveValue, fPrevBottom, 0.0f, uLineColor);
                    }
                    else {
                        AddPolygon(fPrevMovementValue, fPrevTop, fNowValue, fNowTop, fPrevMovementValue, fPrevBottom, fNowValue, fNowBottom, 0.0f, uColor, nChannelIndex);
                        AddLine(fNowValue, fNowBottom, 0.0f, fPrevMovementValue, fPrevBottom, 0.0f, uLineColor);
                        AddLine(fNowValue, fNowTop, 0.0f, fPrevMovementValue, fPrevTop, 0.0f, uLineColor);
                        if (NULL != psPreviousPossibleZZ2) {
                            if (psPreviousPossibleZZ2->byZZLevel != 0xFF && bWasFlipped) {
                                const BOOLEAN bWasUp = fPrevBottom < fNowBottom;
                                const FLOAT fPriceY = bWasUp ? fPrevBottom : fPrevTop;
                                if (m_bDisplaySemaphore) {
                                    AddSemaphore(fPrevChannelStart, fPriceY, 0.0f, psPreviousPossibleZZ2->byZZLevel, !bWasUp, 0xFFFFFFFF);
                                }
                                if (bShowTimeline) {
                                    if (m_fCullTopLeftX < fPrevChannelStart && m_fCullBottomRightX > fPrevChannelStart) {
                                        const UINT uTimeDiff = uNowSec - psPreviousPossibleZZ2->uTimeStamp;
                                        SHLVector2D sTransformed;
                                        Transform(sTransformed, fPrevChannelStart, fPriceY, 0.0f);
                                        if (5 < uTimeDiff) {
                                            FLOAT fTimeY;
                                            if (fMidY < sTransformed.y) {
                                                fTimeY = 50.0f;
                                            }
                                            else {
                                                fTimeY = sSize.y - 50.0f;
                                            }
                                            if (AddTimeLine(sTransformed.x, fTimeY, uTimeDiff)) {
                                                UnTransform(sTransformed, sTransformed.x, fTimeY);
                                                AddLine(fPrevChannelStart, fPriceY, 0.0f, sTransformed.x, sTransformed.y, 0.0f, 0xFFFFFFFF);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                } // if smallest channel
                else {
                    if (fPrevChannelStart < fMidPointMoveValue) {
                        AddPolygonAsBox(fPrevChannelStart, fPrevTop, fMidPointMoveValue, fPrevBottom, 0.0f, uColor, nChannelIndex);
                        AddLine(fPrevChannelStart, fPrevBottom, 0.0f, fMidPointMoveValue, fPrevBottom, 0.0f, uLineColor);
                        AddLine(fPrevChannelStart, fPrevTop, 0.0f, fMidPointMoveValue, fPrevTop, 0.0f, uLineColor);
                    }
                    AddPolygon(fMidPointMoveValue, fPrevTop, fNowValue, fNowTop, fMidPointMoveValue, fPrevBottom, fNowValue, fNowBottom, 0.0f, uColor, nChannelIndex);
                    AddLine(fMidPointMoveValue, fPrevBottom, 0.0f, fNowValue, fNowBottom, 0.0f, uLineColor);
                    AddLine(fMidPointMoveValue, fPrevTop, 0.0f, fNowValue, fNowTop, 0.0f, uLineColor);
                } // end for each depth
            }   // end if
        }
        const BOOLEAN bWasUp = fPrevPriceValue < fPriceY;
        psPrev = &sData;
        if (m_bAllowFlatline) {
            fPrevMovementValue = fOffsetMovement + sData.fX;
        }
        fPrevPriceValue = fPriceY;
        if (m_bDisplayHistory && sData.byZZLevel != 0xFF && m_bAllowFlatline) {
            if (uIndex <= uLastDataAccessIndex) {
                const FLOAT fY = fPrevPriceValue + fNowPriceOffset;
                if (m_bDisplaySemaphore) {
                    AddSemaphore(fPrevMovementValue, fY, fLineZ, sData.byZZLevel, bWasUp, 0xFFFFFFFF);
                }
                if (bShowTimeline) {
                    if (m_fCullTopLeftX < fPrevMovementValue && m_fCullBottomRightX > fPrevMovementValue) {
                        const UINT uTimeDiff = uNowSec - sData.uTimeStamp;
                        SHLVector2D sTransformed;
                        Transform(sTransformed, fPrevMovementValue, fY, 0.0f);
                        if (5 < uTimeDiff) {
                            FLOAT fTimeY;
                            if (fMidY < sTransformed.y) {
                                fTimeY = 50.0f;
                            }
                            else {
                                fTimeY = sSize.y - 50.0f;
                            }
                            if (AddTimeLine(sTransformed.x, fTimeY, uTimeDiff)) {
                                UnTransform(sTransformed, sTransformed.x, fTimeY);
                                AddLine(fPrevMovementValue, fY, 0.0f, sTransformed.x, sTransformed.y, 0.0f, 0xFFFFFFFF);
                            }
                        }
                    }
                }

            }
        }
    }
}

VOID CSema3D::UpdateChannelLabels(const SChannelData& sChannel)
{
    // get current price label
    if (NULL == m_pcMegaZZ) {
        return;
    }
    const FLOAT fPrice = m_pcMegaZZ->GetCurrentPrice();
    const FLOAT fPriceY = m_fReferenceValue + (fPrice - m_fReferencePrice) * m_fPixelPerPrice;
    UINT uIndex = 0;
    CHAR szBuffer[128];
    for ( ; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        if (!m_abDepthEnabled[uIndex] || NULL == m_apcChannelTxt[uIndex]) {
            continue;
        }
        const FLOAT fBottom = sChannel.afBottom[uIndex];
        FLOAT fAccess = fPriceY - fBottom;
        if (0.0f > fAccess) {
            fAccess = 0.0f;
        }
        else if (m_afChannelPixelSize[uIndex] < fAccess) {
            fAccess = m_afChannelPixelSize[uIndex];
        }
        UINT uFinalVal = (UINT)(fAccess / (m_afChannelPixelSize[uIndex] * 0.1f));
//        ASSERT(10 >= uFinalVal);
        if (10 <= uFinalVal) {
            uFinalVal = 9;
        }
        if (sChannel.abUp[uIndex]) {
            uFinalVal = 9 - uFinalVal;
        }
        if (m_bNewMethod) {
            snprintf(szBuffer, 128, "D%d %s L%d", MEGAZZ_MAX_DEPTH - uIndex, sChannel.abUp[uIndex] ? "UP" : "DN", uFinalVal);
        }
        else {
            snprintf(szBuffer, 128, "D%d\n%sL%d", MEGAZZ_MAX_DEPTH - uIndex, sChannel.abUp[uIndex] ? "UP" : "DN", uFinalVal);
        }
        m_apcChannelTxt[uIndex]->SetString(szBuffer);
    }
}

VOID CSema3D::ProcessNewEndChannelData(FLOAT fZZLineStart, FLOAT fDistAdjusted, UINT uLastDataAccessIndex, const SChannelData& sChannel)
{
    if (m_uUsedChannelZZ > uLastDataAccessIndex && 0 < uLastDataAccessIndex) { // if previous ZZ exists
        const SChannelZZData& sPrev = m_asChannelZZ[uLastDataAccessIndex-1];
        const SChannelZZData& sData = m_asChannelZZ[uLastDataAccessIndex];
        if (sData.byZZLevel != 0xFF) {
            const BOOLEAN bUp = sPrev.fPriceY < sData.fPriceY;
            const FLOAT fX = fZZLineStart - fDistAdjusted - 15.0f;
            if (m_bDisplaySemaphore) {
                AddSemaphore(fX, sData.fPriceY, 0.0f, sData.byZZLevel, bUp, 0xFFFFFFFF);
            }
        }
    }

    FLOAT fPrevMovement = fZZLineStart;
    const UINT uUpColor = CPlayerData::GetBiBColor(0);
    const UINT uDownColor = CPlayerData::GetBiBColor(1);
    FLOAT fBiggestX = 0.0f;
    FLOAT afDiamondX[MEGAZZ_MAX_DEPTH];
    for (UINT uDepthIndex = 0; MEGAZZ_MAX_DEPTH > uDepthIndex; ++uDepthIndex) {
        if (!m_abDepthEnabled[uDepthIndex]) {
            continue;;
        }
        const FLOAT fEndMove = sChannel.afX[uDepthIndex];
        if (fEndMove >= fZZLineStart) {
            continue;
        }
        fPrevMovement = fEndMove;
        const FLOAT fTop = sChannel.afTop[uDepthIndex];
        const FLOAT fBottom = sChannel.afBottom[uDepthIndex];

        const UINT uColor = sChannel.abUp[uDepthIndex] ? uUpColor : uDownColor;
        const FLOAT fDistToExtend = (fTop - fBottom) * 0.5f;
        const FLOAT fDiamondX = fZZLineStart + fDistToExtend;
        const FLOAT fDiamondY = fBottom + fDistToExtend;
        AddPolygon(fEndMove, fTop, fZZLineStart, fTop, fEndMove, fBottom, fZZLineStart, fBottom, 0.0f, uColor, uDepthIndex);
        AddLine(fZZLineStart, fTop, 0.0f, fEndMove, fTop, 0.0f, BLACK_COLOR);
        AddLine(fZZLineStart, fBottom, 0.0f, fEndMove, fBottom, 0.0f, BLACK_COLOR);

        AddLine(fZZLineStart, fTop, 0.0f, fDiamondX, fDiamondY, 0.0f, BLACK_COLOR);
        AddLine(fZZLineStart, fBottom, 0.0f, fDiamondX, fDiamondY, 0.0f, BLACK_COLOR);
        AddPolygon(fZZLineStart, fTop, fZZLineStart, fBottom, fDiamondX, fDiamondY, fDiamondX, fDiamondY, 0.0f, uColor, uDepthIndex);
        if (fDiamondX > fBiggestX) {
            fBiggestX = fDiamondX;
        }
        afDiamondX[uDepthIndex] = fDiamondX;
    }
    //fBiggestX = (fBiggestX + fZZLineStart) * 0.5f;
    for (INT nDepthIndex = MEGAZZ_MAX_DEPTH - 1; 0 <= nDepthIndex; --nDepthIndex) {
        if (!m_abDepthEnabled[nDepthIndex]) {
            continue;;
        }
        CUITextLabel* pcLbl = m_apcChannelTxt[nDepthIndex];
        if (NULL == pcLbl) {
            continue;
        }
        const FLOAT fTop = sChannel.afTop[nDepthIndex];
        const FLOAT fBottom = sChannel.afBottom[nDepthIndex];
        const BOOLEAN bUp = sChannel.abUp[nDepthIndex];
        const FLOAT fY = bUp ? fBottom : fTop;
        AddLine(fBiggestX, fY, 0.0f, fZZLineStart, fY, 0.0f, BLACK_COLOR);

        pcLbl->SetVisibility(FALSE);
        pcLbl->SetAlignment(CUITextLabel::EALIGNMENT_RIGHT);
        pcLbl->SetAnchor(0.0f, 0.5f);
        pcLbl->SetLocalPosition(fBiggestX, fY);
        pcLbl->SetVisibility(TRUE);
        pcLbl->OnUpdate(0.0f);
    }
    const SChannelZZData* psPrev = NULL;
    for (UINT uIndex = uLastDataAccessIndex; m_uUsedChannelZZ > uIndex; ++uIndex) { // from oldest to newest
        const SChannelZZData& sData = m_asChannelZZ[uIndex];
        if (NULL == psPrev) {
            psPrev = &sData;
            continue;
        }
        const FLOAT fDiffPrice = abs(psPrev->fPriceY - sData.fPriceY);
        const FLOAT fNowMovement = fPrevMovement + fDiffPrice * PRICE_TO_HEIGHT_RATIO;
        AddLine(fPrevMovement, psPrev->fPriceY, 0.0f, fNowMovement, sData.fPriceY, 0.0f, ZZ_LINE_COLOR);
        if (sData.byZZLevel != 0xFF) {
            const BOOLEAN bUp = psPrev->fPriceY < sData.fPriceY;
            if (m_bDisplaySemaphore) {
                AddSemaphore(fNowMovement, sData.fPriceY, 0.0f, sData.byZZLevel, bUp, 0xFFFFFFFF);
            }
        }
        fPrevMovement = fNowMovement;
        psPrev = &sData;
    }
}
