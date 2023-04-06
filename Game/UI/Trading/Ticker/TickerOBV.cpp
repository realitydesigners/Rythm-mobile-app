#include "stdafx.h"
#include "CryptoUtil.h"
#include "GameRenderer.h"
#include "TickerOBV.h"
#include "TickerUIDefs.h"

CTickerOBV::CTickerOBV(BOOLEAN bSecondsMode) :
CUIContainer(),
m_bSecondsMode(bSecondsMode),
m_uMaxDisplayableNum(TICK_HISTORY_MINS),
m_pcMaster(NULL),
INITIALIZE_TEXT_LABEL(m_cNowDiff),
INITIALIZE_TEXT_LABEL(m_cLow),
INITIALIZE_TEXT_LABEL(m_cHigh),
m_uNum(0),
m_fFirstOBVY(0.0f),
m_pfOBVY(NULL),
m_pfOBVDiffY(NULL),
m_fOBVLastTickY(0.0f),
m_dOBVLastTick(0.0),
m_dPixelsPerOBV(0.0),
m_fOBVDiffZeroY(0.0f),
m_fWidthPerTick(0.0f),
m_dPrevLowest(0.0),
m_dPrevHighest(0.0)
{
    if (!m_bSecondsMode) {
        m_cNowDiff.SetFont(EGAMEFONT_SIZE_18);
        m_cNowDiff.SetAnchor(0.5f, 0.0f);
        AddChild(m_cNowDiff);
        
        m_cLow.SetFont(EGAMEFONT_SIZE_18);
        m_cLow.SetAnchor(1.0f, 0.5f);
        m_cLow.SetLocalSize(TICKER_WIDTH_FOR_LABEL, TICKER_HEIGHT_FOR_LABEL);
        AddChild(m_cLow);
    
        m_cHigh.SetFont(EGAMEFONT_SIZE_18);
        m_cHigh.SetAnchor(1.0f, 0.5f);
        m_cHigh.SetLocalSize(TICKER_WIDTH_FOR_LABEL, TICKER_HEIGHT_FOR_LABEL);
        AddChild(m_cHigh);
    }
    memset(m_afLines, 0, sizeof(m_afLines));
}

CTickerOBV::~CTickerOBV()
{
}

VOID CTickerOBV::UpdateTick(const STickListener& sTicker)
{
    SAFE_DELETE_ARRAY(m_pfOBVY);
    SAFE_DELETE_ARRAY(m_pfOBVDiffY);
    m_uNum = 0;
    const CTickList& cList = m_bSecondsMode ? sTicker.cSecondList : sTicker.cMinuteList;
    const UINT uSize = (UINT)cList.size();
    if (!m_bSecondsMode && (2 > uSize)) {
        return;
    }
    if (0 == uSize) {
        return;
    }
    
    DOUBLE dHighestOBV = 0.0;
    DOUBLE dLowestOBV = 0.0;
    DOUBLE dHighestOBVDiff = 0.001;
    DOUBLE dLowestOBVDiff = -0.001;
    BOOLEAN bFound = FALSE;
    UINT uIndex = 0;
    const UINT uStartIndex = m_bSecondsMode ? 0 : (uSize > m_uMaxDisplayableNum ? (uSize - m_uMaxDisplayableNum) : 0);
    for (auto& sTick : cList) {
        if (uIndex < uStartIndex) {
            ++uIndex;
            continue;
        }

        if (!bFound) {
            bFound = TRUE;
            dHighestOBV = dLowestOBV = sTick.dOBV;
        }
        if (sTick.dOBV > dHighestOBV) {
            dHighestOBV = sTick.dOBV;
        }
        else if (sTick.dOBV < dLowestOBV) {
            dLowestOBV = sTick.dOBV;
        }
        if (sTick.dOBVDiff > dHighestOBVDiff) {
            dHighestOBVDiff = sTick.dOBVDiff;
        }
        else if (sTick.dOBVDiff < dLowestOBVDiff) {
            dLowestOBVDiff = sTick.dOBVDiff;
        }
    }
    const SHLVector2D& sLocalSize = GetLocalSize();
    const FLOAT fChartYOffset = m_bSecondsMode ? 10.0f : (TICKER_HEIGHT_FOR_LABEL * 0.5f);
    const FLOAT fChartHeight = sLocalSize.y - (m_bSecondsMode ? 20.0f : TICKER_HEIGHT_FOR_LABEL);
    if (!m_bSecondsMode) {
        // update strings
        if (0.0 == m_dPrevLowest) {
            m_dPrevLowest = dLowestOBVDiff;
            m_dPrevHighest = dHighestOBVDiff;
        }
        CHAR szBuffer[64];
        UINT uColor = m_dPrevLowest > dLowestOBVDiff ? TICKER_RED : 0xFFFFFFFF;
        m_dPrevLowest = dLowestOBVDiff;
        m_cLow.SetLocalPosition(TICKER_WIDTH_FOR_LABEL - 1.0f, sLocalSize.y - TICKER_HEIGHT_FOR_LABEL * 0.5f);
        m_cLow.SetColor(uColor);
        snprintf(szBuffer, 64, "%.1f", dLowestOBVDiff);
        m_cLow.SetString(szBuffer);
        uColor = m_dPrevHighest < dHighestOBVDiff ? TICKER_GREEN : 0xFFFFFFFF;
        m_dPrevHighest = dHighestOBVDiff;
        m_cHigh.SetLocalPosition(TICKER_WIDTH_FOR_LABEL - 1.0f, fChartYOffset);
        m_cHigh.SetColor(uColor);
        snprintf(szBuffer, 64, "%.1f", dHighestOBVDiff);
        m_cHigh.SetString(szBuffer);
    }
    // now we update the Yvalues
    m_pfOBVY = new FLOAT[uSize];
    m_pfOBVDiffY = new FLOAT[uSize];
    if (NULL == m_pfOBVY || NULL == m_pfOBVDiffY) {
        ASSERT(FALSE);
        return;
    }
    m_uNum = uSize;
    FLOAT fChartableWidth = sLocalSize.x - 1.0f;
    if (!m_bSecondsMode) {
        fChartableWidth -= TICKER_WIDTH_FOR_LABEL;
        const UINT uNum = m_uNum < m_uMaxDisplayableNum ? m_uNum : m_uMaxDisplayableNum;
        m_fWidthPerTick = fChartableWidth / (uNum - 1);
    }
    else {
        m_fWidthPerTick = fChartableWidth / TICKS_PER_MINUTE;
    }
    FLOAT dPixelsPerOBV;
    FLOAT fOBVStartY;
    FLOAT fOBVDiffStartY;
    FLOAT dPixelsPerOBVDiff = fChartHeight / (dHighestOBVDiff - dLowestOBVDiff);
    if (NULL == m_pcMaster) {
        ASSERT(!m_bSecondsMode);
        dPixelsPerOBV = fChartHeight / (dHighestOBV - dLowestOBV);
        fOBVStartY = fOBVDiffStartY = fChartYOffset;
    }
    else {
        ASSERT(m_bSecondsMode);
        {
            const DOUBLE dLastTickOBV = m_pcMaster->GetLastTickOBV();
            DOUBLE dHighestOBVDiffFromLastTick = dHighestOBV - dLastTickOBV;
            if (0.5 > dHighestOBVDiffFromLastTick) {
                dHighestOBVDiffFromLastTick  = 0.5;
            }
            DOUBLE dLowestOBVDiffFromLastTick = dLastTickOBV - dLowestOBV;
            if (0.5 > dLowestOBVDiffFromLastTick) {
                dLowestOBVDiffFromLastTick = 0.5;
            }
            const DOUBLE dOriginalPixelsPerOBV = m_pcMaster->GetPixelsPerOBV();
            const FLOAT fOriginalHighestPixelHeight = dHighestOBVDiffFromLastTick * dOriginalPixelsPerOBV;
            const FLOAT fOriginalLowestPixelHeight = dLowestOBVDiffFromLastTick * dOriginalPixelsPerOBV;
            const FLOAT fLastTickOBVY = m_pcMaster->GetLastTickOBVY();
            const FLOAT fNowHighestPixelHeight = fLastTickOBVY - 10.0f;
            const FLOAT fNowLowestPixelHeight = (sLocalSize.y - 10.0f) - fLastTickOBVY;
            const FLOAT fUpperScale = fNowHighestPixelHeight / fOriginalHighestPixelHeight;
            const FLOAT fLowerScale = fNowLowestPixelHeight / fOriginalLowestPixelHeight;
            const FLOAT fActualScale = fLowerScale < fUpperScale ? fLowerScale : fUpperScale;
            m_fFirstOBVY = fOBVStartY = fLastTickOBVY;
            dPixelsPerOBV = dOriginalPixelsPerOBV * fActualScale;
            dHighestOBV = dLastTickOBV;
        }
        {
            fOBVDiffStartY = m_pcMaster->GetOBVDiffZeroY();
            const FLOAT fNowHighestPixelHeight = fOBVDiffStartY - 10.0f;
            const FLOAT fNowLowestPixelHeight = (sLocalSize.y - 10.0f) - fOBVDiffStartY;
            const FLOAT fUpperScale = fNowHighestPixelHeight / dHighestOBVDiff;
            const FLOAT fLowerScale = fNowLowestPixelHeight / -dLowestOBVDiff;
            const FLOAT fActualScale = fLowerScale < fUpperScale ? fLowerScale : fUpperScale;
            dPixelsPerOBVDiff = fActualScale;
            dHighestOBVDiff = 0.0;
        }
    }
    {   // compute OBV line
        UINT uIndex = 0;
        for (auto& sTick : cList) {
            if (m_uNum <= uIndex) {
                ASSERT(FALSE);
                return;
            }
            m_pfOBVY[uIndex] = fOBVStartY + (dHighestOBV - sTick.dOBV) * dPixelsPerOBV;
            if ((m_uNum - 2) == uIndex) { // if 2nd last tick
                m_dOBVLastTick = sTick.dOBV;
                m_fOBVLastTickY = m_pfOBVY[uIndex];
            }
            ++uIndex;
        }
        if (!m_bSecondsMode) {
            m_fFirstOBVY = m_pfOBVY[uStartIndex];
        }
    }
    {   // compute OBV diff line
        m_fOBVDiffZeroY = fOBVDiffStartY + dHighestOBVDiff * dPixelsPerOBVDiff;
        UINT uIndex = 0;
        for (auto& sTick : cList) {
            if (m_uNum <= uIndex) {
                ASSERT(FALSE);
                return;
            }
            m_pfOBVDiffY[uIndex] = fOBVDiffStartY + (dHighestOBVDiff - sTick.dOBVDiff) * dPixelsPerOBVDiff;
            ++uIndex;
        }
    }
    m_dPixelsPerOBV = dPixelsPerOBV;
    if (!m_bSecondsMode) {
        UpdateLines(sTicker);
    }
}

VOID CTickerOBV::Release(VOID)
{
    SAFE_DELETE_ARRAY(m_pfOBVY);
    SAFE_DELETE_ARRAY(m_pfOBVDiffY);
    m_uNum = 0;
    
    m_cNowDiff.Release();
    m_cNowDiff.RemoveFromParent();
    
    m_cHigh.Release();
    m_cHigh.RemoveFromParent();
    
    m_cLow.Release();
    m_cLow.RemoveFromParent();
    
    CUIContainer::Release();
}

VOID CTickerOBV::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    SHLVector2D sWorldPos = GetWorldPosition();
    sWorldPos.x += fOffsetX;
    sWorldPos.y += fOffsetY;
    const SHLVector2D& sWorldSize = GetWorldSize();
    CGameRenderer::DrawLine(sWorldPos.x, sWorldPos.y, sWorldPos.x + sWorldSize.x, sWorldPos.y, TICKER_GRAY);
    {   // render vertical line (price)
        FLOAT fLineX = sWorldPos.x;
        if (!m_bSecondsMode) {
            fLineX += TICKER_WIDTH_FOR_LABEL - 1.0f;
        }
        
        CGameRenderer::DrawLine(fLineX, sWorldPos.y, fLineX, sWorldPos.y + sWorldSize.y, TICKER_GRAY);
    }

    // render zero line
    const FLOAT fZeroY = sWorldPos.y + m_fOBVDiffZeroY;
    FLOAT fX = sWorldPos.x + (m_bSecondsMode ? 1.0f : TICKER_WIDTH_FOR_LABEL);
    CGameRenderer::DrawLine(fX, fZeroY, sWorldPos.x + sWorldSize.x, fZeroY, TICKER_GRAY);
    
    if (m_bSecondsMode) {
        fX = sWorldPos.x + sWorldSize.x - 1.0f;
    }
    const FLOAT fXOffset = m_bSecondsMode ? -m_fWidthPerTick : m_fWidthPerTick;
    if (NULL != m_pfOBVDiffY) {
        UINT uIndex = m_bSecondsMode ? 0 : (m_uNum - m_uMaxDisplayableNum);
        FLOAT fX1 = fX;
        for ( ; m_uNum > uIndex; ++uIndex, fX1 += fXOffset) {
            const FLOAT fY = sWorldPos.y + m_pfOBVDiffY[uIndex];
            const UINT uColor = (m_pfOBVDiffY[uIndex] < m_fOBVDiffZeroY) ? TICKER_GREEN : TICKER_RED;
            CGameRenderer::DrawLine(fX1, fZeroY, fX1, fY, uColor);
        }
    }
    if (NULL != m_pfOBVY) {
        UINT uIndex = m_bSecondsMode ? 0 : (m_uNum - m_uMaxDisplayableNum);
        FLOAT fPrevX = fX;
        FLOAT fPrevY = sWorldPos.y + m_pfOBVY[uIndex];
        ++uIndex;
        fX += fXOffset;
        for ( ; m_uNum > uIndex; ++uIndex, fX += fXOffset) {
            const FLOAT fY = sWorldPos.y + m_pfOBVY[uIndex];
            if (0.0 <= fPrevX) {
                CGameRenderer::DrawLine(fPrevX, fPrevY, fX, fY, 0xFFFFFFFF);
            }
            fPrevX = fX;
            fPrevY = fY;
        }
    }
    if (!m_bSecondsMode) {
        const FLOAT fX1 = m_afLines[0][0][0] + fOffsetX;
        const FLOAT fX2 = m_afLines[0][1][0] + fOffsetX;
        const FLOAT fX3 = m_afLines[1][0][0] + fOffsetX;
        const FLOAT fX4 = m_afLines[1][1][0] + fOffsetX;
        
        const FLOAT fY1 = m_afLines[0][0][1] + fOffsetY;
        const FLOAT fY2 = m_afLines[0][1][1] + fOffsetY;
        const FLOAT fY3 = m_afLines[1][0][1] + fOffsetY;
        const FLOAT fY4 = m_afLines[1][1][1] + fOffsetY;
        
        CGameRenderer::DrawLine(fX1, fY1, fX2, fY2, TICKER_GREEN);
        CGameRenderer::DrawLine(fX3, fY3, fX4, fY4, TICKER_RED);
                                                                 
    }
    CUIContainer::OnRender(fOffsetX, fOffsetY);
}

VOID CTickerOBV::UpdateMaxDisplayableNum(UINT uNum)
{
    if (m_bSecondsMode) {
        return;
    }
    m_uMaxDisplayableNum = uNum;
    if (NULL == m_pfOBVDiffY) {
        return;
    }
    if (m_uNum < uNum) {
        ASSERT(FALSE);
        return;
    }
    const FLOAT fChartableWidth = GetLocalSize().x - 1.0f - TICKER_WIDTH_FOR_LABEL;
    m_fWidthPerTick = fChartableWidth / (uNum - 1);
}

VOID CTickerOBV::UpdateLines(const STickListener& sTicker)
{
    const SHLVector2D& sWorldPos = GetWorldPosition();
    const FLOAT fX = sWorldPos.x + TICKER_WIDTH_FOR_LABEL;
    EOBV_TYPE eType;
    if (240 == m_uMaxDisplayableNum) {
        eType = EOBV_TYPE_4HR;
    }
    else if (120 == m_uMaxDisplayableNum) {
        eType = EOBV_TYPE_2HR;
    }
    else if (60 == m_uMaxDisplayableNum) {
        eType = EOBV_TYPE_1HR;
    }
    else {
//        ASSERT(30 == m_uMaxDisplayableNum);
        eType = EOBV_TYPE_30_MINS;
    }
    const UINT uStartIndex = (m_uNum > m_uMaxDisplayableNum ? (m_uNum - m_uMaxDisplayableNum) : 0);
    const UINT uUpperIndex1 = sTicker.sProcessed.aauUpperIndexOBV[eType][0];
    m_afLines[0][0][0] = fX + (uUpperIndex1 - uStartIndex) * m_fWidthPerTick;
    m_afLines[0][0][1] = sWorldPos.y + m_pfOBVY[uUpperIndex1] - 5.0f;
    
    const UINT uUpperIndex2 = sTicker.sProcessed.aauUpperIndexOBV[eType][1];
    m_afLines[0][1][0] = fX + (uUpperIndex2 - uStartIndex) * m_fWidthPerTick;
    m_afLines[0][1][1] = sWorldPos.y + m_pfOBVY[uUpperIndex2] - 5.0f;
    
    const UINT uLowerIndex1 = sTicker.sProcessed.aauLowerIndexOBV[eType][0];
    m_afLines[1][0][0] = fX + (uLowerIndex1 - uStartIndex) * m_fWidthPerTick;
    m_afLines[1][0][1] = sWorldPos.y + m_pfOBVY[uLowerIndex1] + 5.0f;
    
    const UINT uLowerIndex2 = sTicker.sProcessed.aauLowerIndexOBV[eType][1];
    m_afLines[1][1][0] = fX + (uLowerIndex2 - uStartIndex) * m_fWidthPerTick;
    m_afLines[1][1][1] = sWorldPos.y + m_pfOBVY[uLowerIndex2] + 5.0f;
}
