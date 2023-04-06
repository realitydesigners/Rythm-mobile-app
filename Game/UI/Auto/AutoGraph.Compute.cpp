#include "stdafx.h"
#include "AutoGraph.h"
#include "AutomationMgr.h"
#include "FractalData.h"
#include "OandaMgr.h"
#include "PlayerData.h"
#include "TradeLogDefs.h"

#define PURPLE_COLOR    RGBA(171,111,192,0xFF)
#define CYAN_COLOR      RGBA(119,233,252,0xFF)
#define WHITE_COLOR     RGBA(0xFF,0xFF,0xFF,0xFF)

#define GRADIENT(x)     RGBA((x),(x),(x),0xFF)
#define START_GRADIENT  80

VOID CAutoGraph::Refresh(UINT uPatternIndex)
{
    Refresh(&CPatternDataMgr::GetPattern(uPatternIndex));
}
VOID CAutoGraph::Refresh(const SPattern* psPattern)
{
    if (NULL == psPattern) {
        Clear();
        m_psPatternRows = NULL;
        m_uPatternRowNum = 0;
        return;
    }
    const SPatternRow* psRow = CPatternDataMgr::GetPatternRow(psPattern->uIndexOffset);
    Refresh(psRow, psPattern->uPatternNum);
}
VOID CAutoGraph::Refresh(const SPatternRow* psRows, UINT uRowNum)
{
    Clear();
    m_psPatternRows = psRows;
    m_uPatternRowNum = uRowNum;
    if (NULL == psRows) {
        return;
    }
    const BYTE byAlpha = m_bTransparent ? 0x7F : 0xFF;
    const BYTE byGradient1 = m_bTransparent ? 20 : 40;
    const BYTE byGradient2 = byGradient1 + (m_bTransparent ? 5 : 10);
    const BYTE byGradient3 = byGradient2 + (m_bTransparent ? 5 : 10);
    const BYTE byGradient4 = byGradient3 + (m_bTransparent ? 5 : 10);
    const BYTE byGradient5 = byGradient4 + (m_bTransparent ? 5 : 10);
    const BYTE byGradient6 = byGradient5 + (m_bTransparent ? 5 : 10);
    const BYTE byGradient7 = byGradient6 + (m_bTransparent ? 5 : 10);
    const BYTE byGradient8 = byGradient7 + (m_bTransparent ? 5 : 10);
    const BYTE byGradient9 = byGradient8 + (m_bTransparent ? 5 : 10);
    const UINT auBoxColor[9] = {
        RGBA(byGradient1, byGradient1, byGradient1, byAlpha),
        RGBA(byGradient2, byGradient2, byGradient2, byAlpha),
        RGBA(byGradient3, byGradient3, byGradient3, byAlpha),
        RGBA(byGradient4, byGradient4, byGradient4, byAlpha),
        RGBA(byGradient5, byGradient5, byGradient5, byAlpha),
        RGBA(byGradient6, byGradient6, byGradient6, byAlpha),
        RGBA(byGradient7, byGradient7, byGradient7, byAlpha),
        RGBA(byGradient8, byGradient8, byGradient8, byAlpha),
        RGBA(byGradient9, byGradient9, byGradient9, byAlpha),
    };
    const UINT auLineColor[9] = {
        GRADIENT(START_GRADIENT),
        GRADIENT(START_GRADIENT + 5),
        GRADIENT(START_GRADIENT + 10),
        GRADIENT(START_GRADIENT + 15),
        GRADIENT(START_GRADIENT + 20),
        GRADIENT(START_GRADIENT + 25),
        GRADIENT(START_GRADIENT + 30),
        GRADIENT(START_GRADIENT + 35),
        GRADIENT(START_GRADIENT + 40),
    };
    const SHLVector2D& sSize = GetLocalSize();
    const FLOAT fBaseDimension = sSize.x - 10.0f;

    // compute all dimensions
    FLOAT afDimension[9];
    const SFractal& sFractal = CFractalDataMgr::GetFractal(CAutomationMgr::GetInstance().GetFractalIndex());
    const FLOAT fBaseSize = sFractal.afPipSize[m_uStartDepthIndex];
    for (UINT uIndex = 0; 9 > uIndex; ++uIndex) {
        const UINT uCurrentDepthIdex = m_uStartDepthIndex + uIndex;
        if (MEGAZZ_MAX_DEPTH <= uCurrentDepthIdex) {
            afDimension[uIndex] = 0.0f;
        }
        else {
            afDimension[uIndex] = fBaseDimension * sFractal.afPipSize[uCurrentDepthIdex] / fBaseSize;
        }
    }
    UINT uEndDepthIndex = m_uStartDepthIndex + m_uDepthNum;
    if (9 < uEndDepthIndex) {
        uEndDepthIndex = 9;
    }
    FLOAT fPrevTopX = 5.0f + fBaseDimension;
    FLOAT fPrevTopY = 5.0f;
    FLOAT fPrevBottomX = fPrevTopX - fBaseDimension;
    FLOAT fPrevBottomY = fPrevTopY + fBaseDimension;
    BOOLEAN bStickToBottom = FALSE;
    UINT uIndex = 0;
    FLOAT afTops[9][2];
    FLOAT afBottoms[9][2];
    for (UINT uDepthIndex = m_uStartDepthIndex; uEndDepthIndex > uDepthIndex; ++uDepthIndex, ++uIndex) {
        ASSERT(9 > uIndex);
        const FLOAT fDimension = afDimension[uIndex];
        FLOAT fTopX;
        FLOAT fTopY;
        FLOAT fBottomX;
        FLOAT fBottomY;
        if (bStickToBottom) {
            fBottomX = fPrevBottomX;
            fBottomY = fPrevBottomY;
            fTopX = fBottomX + fDimension;
            fTopY = fBottomY - fDimension;
        }
        else {
            fTopX = fPrevTopX;
            fTopY = fPrevTopY;
            fBottomX = fTopX - fDimension;
            fBottomY = fTopY + fDimension;
        }
        
        BYTE byChannelType = 0xFF;
        BYTE byRetrace1 = 0xFF;
        BYTE byRetrace2 = 0xFF;
        BYTE byCurrent1 = 0xFF;
        BYTE byCurrent2 = 0xFF;
        EPATTERN_MATCH eMatch = EPATTERN_MATCH_NONE;
        UINT uMatchDepthIndex = 0;
        const BOOLEAN bFound = GetChannelInfo(psRows, uRowNum, uDepthIndex, byChannelType, byCurrent1, byCurrent2, byRetrace1, byRetrace2, eMatch, uMatchDepthIndex);
        if (0 == uIndex) {
            PrepareDepth(uDepthIndex, fBottomX, fTopY, fTopX, fBottomY, auBoxColor[uIndex], auLineColor[uIndex]);
            m_abEnabled[uDepthIndex] = TRUE;
            afTops[uDepthIndex][0] = fTopX;
            afTops[uDepthIndex][1] = fTopY;
            afBottoms[uDepthIndex][0] = fBottomX;
            afBottoms[uDepthIndex][1] = fBottomY;
        }
        else {
            if (!bFound) { // skip if not found
                m_abEnabled[uDepthIndex] = FALSE;
                continue;
            }
            switch (eMatch) {
                case EPATTERN_MATCH_NONE:
                    break;
                case EPATTERN_MATCH_TOP:
                    fTopX = afTops[uMatchDepthIndex][0];
                    fTopY = afTops[uMatchDepthIndex][1];
                    fBottomX = fTopX - fDimension;
                    fBottomY = fTopY + fDimension;
                    break;
                case EPATTERN_MATCH_BOTTOM:
                    fBottomX = afBottoms[uMatchDepthIndex][0];
                    fBottomY = afBottoms[uMatchDepthIndex][1];
                    fTopX = fBottomX + fDimension;
                    fTopY = fBottomY - fDimension;
                    break;
            }
            m_abEnabled[uDepthIndex] = TRUE;
            PrepareDepth(uDepthIndex, fBottomX, fTopY, fTopX, fBottomY, auBoxColor[uIndex], auLineColor[uIndex]);
            afTops[uDepthIndex][0] = fTopX;
            afTops[uDepthIndex][1] = fTopY;
            afBottoms[uDepthIndex][0] = fBottomX;
            afBottoms[uDepthIndex][1] = fBottomY;
        }
        const FLOAT fUnitMovement = fDimension * 0.1f;
        if (0xFF != byChannelType) {
            const UINT uRetraceColor = (MegaZZ::EType_Up == byChannelType) ? CYAN_COLOR : PURPLE_COLOR;
            if (0xFF != byRetrace2 && byRetrace1 != byRetrace2) {
                const FLOAT fAmountMoved = (byRetrace2 + 1) * fUnitMovement;
                FLOAT fX, fY;
                if (MegaZZ::EType_Up == byChannelType) {
                    fX = fTopX - fAmountMoved;
                    fY = fTopY + fAmountMoved - fUnitMovement;
                }
                else {
                    fX = fBottomX + fAmountMoved - fUnitMovement;
                    fY = fBottomY - fAmountMoved;
                }
                AddBox(fX + 1.0f, fY + 1.0f, fUnitMovement - 2.0f, fUnitMovement - 2.0f, uRetraceColor, uDepthIndex);
            }
            if (0xFF != byRetrace1) {
                const FLOAT fAmountMoved = (byRetrace1 + 1) * fUnitMovement;
                if (MegaZZ::EType_Up == byChannelType) {
                    bStickToBottom = TRUE;
                    fBottomX = fTopX - fAmountMoved;
                    fBottomY = fTopY + fAmountMoved - fUnitMovement;
                    AddBox(fBottomX + 1.0f, fBottomY + 1.0f, fUnitMovement - 2.0f, fUnitMovement - 2.0f, uRetraceColor, uDepthIndex);
                    fBottomX += fUnitMovement * 0.5f;
                    fBottomY += fUnitMovement * 0.5f;
                }
                else {
                    bStickToBottom = FALSE;
                    fTopX = fBottomX + fAmountMoved - fUnitMovement;
                    fTopY = fBottomY - fAmountMoved;
                    AddBox(fTopX + 1.0f, fTopY + 1.0f, fUnitMovement - 2.0f, fUnitMovement - 2.0f, uRetraceColor, uDepthIndex);
                    fTopX += fUnitMovement * 0.5f;
                    fTopY += fUnitMovement * 0.5f;
                }
            }

            if (0xFF != byCurrent1) {
                const FLOAT fAmountMoved = (byCurrent1 + 1) * fUnitMovement;
                FLOAT fX, fY;
                if (MegaZZ::EType_Up == byChannelType) {
                    fX = fTopX - fAmountMoved;
                    fY = fTopY + fAmountMoved - fUnitMovement;
                }
                else {
                    fX = fBottomX + fAmountMoved - fUnitMovement;
                    fY = fBottomY - fAmountMoved;
                }
                AddBox(fX + 1.0f, fY + 1.0f, fUnitMovement - 2.0f, fUnitMovement - 2.0f, WHITE_COLOR, uDepthIndex);
            }
            if (0xFF != byCurrent2 && byCurrent1 != byCurrent2) {
                const FLOAT fAmountMoved = (byCurrent2 + 1) * fUnitMovement;
                FLOAT fX, fY;
                if (MegaZZ::EType_Up == byChannelType) {
                    fX = fTopX - fAmountMoved;
                    fY = fTopY + fAmountMoved - fUnitMovement;
                }
                else {
                    fX = fBottomX + fAmountMoved - fUnitMovement;
                    fY = fBottomY - fAmountMoved;
                }
                AddBox(fX + 1.0f, fY + 1.0f, fUnitMovement - 2.0f, fUnitMovement - 2.0f, WHITE_COLOR, uDepthIndex);
            }
        }
        fPrevTopX = fTopX;
        fPrevTopY = fTopY;
        fPrevBottomX = fBottomX;
        fPrevBottomY = fBottomY;
    }
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        m_afSizes[uIndex] = sFractal.afPipSize[uIndex];
    }
    UpdateDepthDisplay();
}

VOID CAutoGraph::PrepareDepth(UINT uDepthIndex, FLOAT fStartX, FLOAT fStartY, FLOAT fEndX, FLOAT fEndY, UINT uColor, UINT uGridColor)
{
    ASSERT(fEndX > fStartX);
    ASSERT(fEndY > fStartY);
    AddBox(fStartX, fStartY, fEndX - fStartX, fEndY - fStartY, uColor, uDepthIndex);
    if (!m_bDrawGrid) {
        AddLine(fStartX, fStartY, fEndX, fStartY, uGridColor, uDepthIndex);
        AddLine(fStartX, fEndY, fEndX, fEndY, uGridColor, uDepthIndex);
        
        AddLine(fStartX, fStartY, fStartX, fEndY, uGridColor, uDepthIndex);
        AddLine(fEndX, fStartY, fEndX, fEndY, uGridColor, uDepthIndex);
        return;
    }
    const FLOAT fDistToMove = (fEndX - fStartX) * 0.1f;
    for (UINT uIndex = 0; 10 >= uIndex; ++uIndex) {
        const FLOAT fX = fStartX + uIndex * fDistToMove;
        AddLine(fX, fStartY, fX, fEndY, uGridColor, uDepthIndex);
        const FLOAT fY = fStartY + uIndex * fDistToMove;
        AddLine(fStartX, fY, fEndX, fY, uGridColor, uDepthIndex);
    }
}

BOOLEAN CAutoGraph::GetChannelInfo(const SPatternRow* psRow, UINT uRowNum, UINT uDepthIndex, BYTE& byChannelType,
                                   BYTE& byCurrentNumber1, BYTE& byCurrentNumber2, BYTE& byR1, BYTE& byR2, EPATTERN_MATCH& eMatch, UINT& uMatchDepthIndex)
{
    // iteratr thru all the pattern rows
    for (UINT uIndex = 0; uRowNum > uIndex; ++uIndex) {
        const SPatternRow& sRow = psRow[uIndex];
        if (sRow.byDepthIndex != uDepthIndex) {
            continue;
        }
        byChannelType = sRow.bUp ? MegaZZ::EType_Up : MegaZZ::EType_Down;
        byR1 = sRow.byWasLow;
        byR2 = sRow.byWasHigh;
        byCurrentNumber1 = sRow.byIsLow;
        byCurrentNumber2 = sRow.byIsHigh;
        eMatch = sRow.eMatch;
        uMatchDepthIndex = sRow.byMatchTargetDepthIndex;
        return TRUE;
    }
    return FALSE;
}

