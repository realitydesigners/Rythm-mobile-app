#include "stdafx.h"
#include "CryptoUtil.h"
#include "EventManager.h"
#include "GameRenderer.h"
#include "TickerPrice.h"
#include "TickerUIDefs.h"

#include <math.h>


CTickerPrice::CTickerPrice(BOOLEAN bSecondsMode, UINT uEventHandlerID, UINT uPickEventID) :
CUIContainer(),
m_bSecondsMode(bSecondsMode),
m_uEventHandlerID(uEventHandlerID),
m_uPickEventID(uPickEventID),
m_bSoloMode(FALSE),
m_uMaxDisplayableNum(TICK_HISTORY_MINS),
INITIALIZE_TEXT_LABEL(m_cLowPrice),
INITIALIZE_TEXT_LABEL(m_cHighPrice),
m_uNum(0),
m_pfYValues(NULL),
m_fWidthPerTick(0.0f),
m_fPriceChangeRatio(0.0f),
m_dPrevLowest(0.0),
m_dPrevHighest(0.0),
m_dPixelsPerPrice(0.0),
m_dHighestPrice(0.0),
m_pcMaster(NULL),
m_uFocusStartSec(0),
m_uFocusEndSec(0),
m_uFocusStartIndex(0),
m_uFocusEndIndex(0),
m_bPerformPickCheck(FALSE),
m_uStartTimeSec(0),
m_uPerTickTimeSec(0)
{
    if (!m_bSecondsMode) {
        m_cLowPrice.SetFont(EGAMEFONT_SIZE_18);
        const FLOAT fAnchorX = m_bSecondsMode ? 0.0f : 1.0f;
        m_cLowPrice.SetAnchor(fAnchorX, 0.5f);
        m_cLowPrice.SetLocalSize(TICKER_WIDTH_FOR_LABEL, TICKER_HEIGHT_FOR_LABEL);
        AddChild(m_cLowPrice);
        m_cHighPrice.SetFont(EGAMEFONT_SIZE_18);
        m_cHighPrice.SetAnchor(fAnchorX, 0.5f);
        m_cHighPrice.SetLocalSize(TICKER_WIDTH_FOR_LABEL, TICKER_HEIGHT_FOR_LABEL);
        AddChild(m_cHighPrice);
    }
    memset(m_afLines, 0, sizeof(m_afLines));
}

CTickerPrice::~CTickerPrice()
{
}

VOID CTickerPrice::UpdateTick(const STickListener& sTicker)
{
    SAFE_DELETE_ARRAY(m_pfYValues);
    m_uNum = 0;
    const CTickList& cList = m_bSecondsMode ? sTicker.cSecondList : sTicker.cMinuteList;
    const UINT uSize = (UINT)cList.size();
    if (0 == uSize) {
        return;
    }
    DOUBLE dHighestPrice = 0.0;
    DOUBLE dLowestPrice = 0.0;
    UINT uIndex = 0;
    const UINT uStartIndex = m_bSecondsMode ? 0 : (uSize > m_uMaxDisplayableNum ? (uSize - m_uMaxDisplayableNum) : 0);
    for (auto& sTick : cList) {
        if (uIndex < uStartIndex) {
            ++uIndex;
            continue;
        }
        if (0.0 == dHighestPrice) {
            dLowestPrice = dHighestPrice = sTick.dCLosePrice;
            continue;
        }
        if (sTick.dCLosePrice > dHighestPrice) {
            dHighestPrice = sTick.dCLosePrice;
        }
        else if (sTick.dCLosePrice < dLowestPrice) {
            dLowestPrice = sTick.dCLosePrice;
        }
    }
    m_fPriceChangeRatio = (dHighestPrice - dLowestPrice) * 100.0f / dHighestPrice;
    const SHLVector2D& sLocalSize = GetLocalSize();
    FLOAT fChartYOffset = TICKER_HEIGHT_FOR_LABEL * 0.5f;
    FLOAT fChartHeight = sLocalSize.y - TICKER_HEIGHT_FOR_LABEL;
    
    if (!m_bSecondsMode) {
        // update strings
        if (0.0 == m_dPrevLowest) {
            m_dPrevLowest = dLowestPrice;
            m_dPrevHighest = dHighestPrice;
        }
        CHAR szBuffer[64];
        UINT uColor = m_dPrevLowest > dLowestPrice ? RGBA(0xFF,0,0,0xFF) : 0xFFFFFFFF;
        m_dPrevLowest = dLowestPrice;
        m_cLowPrice.SetColor(uColor);
        CCryptoUtil::GenerateString(dLowestPrice, szBuffer, 64);
        m_cLowPrice.SetString(szBuffer);
        uColor = m_dPrevHighest < dHighestPrice ? RGBA(0,0xFF,0,0xFF) : 0xFFFFFFFF;
        m_dPrevHighest = dHighestPrice;
        m_cHighPrice.SetColor(uColor);
        CCryptoUtil::GenerateString(dHighestPrice, szBuffer, 64);
        m_cHighPrice.SetString(szBuffer);
        
        const FLOAT fPosX = m_bSecondsMode ? sLocalSize.x : TICKER_WIDTH_FOR_LABEL - 1.0f;
        m_cLowPrice.SetLocalPosition(fPosX, sLocalSize.y - TICKER_HEIGHT_FOR_LABEL * 0.5f);
        m_cHighPrice.SetLocalPosition(fPosX, fChartYOffset);
    }
    // now we update the Yvalues
    m_pfYValues = new FLOAT[uSize];
    if (NULL == m_pfYValues) {
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
    if (NULL == m_pcMaster) {
        m_dPixelsPerPrice = fChartHeight / (dHighestPrice - dLowestPrice);
    }
    else {
        m_dPixelsPerPrice = m_pcMaster->GetPixelsPerPrice();
        dHighestPrice = m_pcMaster->GetHighestPrice();
    }
    m_uFocusStartIndex = 0xFFFFFFFF;
    m_uFocusEndIndex = 0;
    uIndex = 0;
    m_uStartTimeSec = 0xFFFFFFFF;
    m_uPerTickTimeSec = 0xFFFFFFFF;
    for (auto& sTick : cList) {
        if (m_uNum <= uIndex) {
            ASSERT(FALSE);
            return;
        }
        if (0xFFFFFFFF == m_uStartTimeSec) { //only first entry
            m_uStartTimeSec = sTick.uOpenSec;
            m_uPerTickTimeSec = 0xFFFFFFFF;
        }
        else if (0xFFFFFFFF == m_uPerTickTimeSec) {
            if (sTick.uOpenSec > m_uStartTimeSec) {
                m_uPerTickTimeSec = sTick.uOpenSec - m_uStartTimeSec;
            }
        }
        if (0xFFFFFFFF == m_uFocusStartIndex) {
            if (sTick.uOpenSec >= m_uFocusStartSec) {
//                TRACE("Start Tick: %d\n", sTick.uOpenSec);
                m_uFocusStartIndex = uIndex;
            }
        }
        if (0xFFFFFFFF != m_uFocusStartIndex) {
            if (0 == m_uFocusEndIndex) {
//                TRACE("Check End Tick: %d vs End:%d\n", sTick.uCloseSec, m_uFocusEndSec);
                if (sTick.uCloseSec >= m_uFocusEndSec) {
                    m_uFocusEndIndex = uIndex;
                }
            }
        }
        m_pfYValues[uIndex] = fChartYOffset + (dHighestPrice - sTick.dCLosePrice) * m_dPixelsPerPrice;
        ++uIndex;
    }
    m_dHighestPrice = dHighestPrice;
    if (!m_bSecondsMode) {
        UpdateLines(sTicker);
    }
//    TRACE("S: %d E:%d\n", m_uFocusStartIndex, m_uFocusEndIndex);
}

VOID CTickerPrice::Release(VOID)
{
    SAFE_DELETE_ARRAY(m_pfYValues);
    m_cHighPrice.Release();
    m_cHighPrice.RemoveFromParent();
    m_cLowPrice.Release();
    m_cLowPrice.RemoveFromParent();
    CUIContainer::Release();
}

VOID CTickerPrice::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    SHLVector2D sWorldPos = GetWorldPosition();
    sWorldPos.x += fOffsetX;
    sWorldPos.y += fOffsetY;
    const SHLVector2D& sWorldSize = GetWorldSize();
    const FLOAT fY2 = sWorldPos.y + sWorldSize.y;;
    CGameRenderer::DrawLine(sWorldPos.x, sWorldPos.y, sWorldPos.x + sWorldSize.x, sWorldPos.y, 0xFFFFFFFF);
    {   // render vertical line (price)
        FLOAT fLineX = sWorldPos.x;
        if (!m_bSecondsMode) {
            fLineX += TICKER_WIDTH_FOR_LABEL - 1.0f;
        }
        CGameRenderer::DrawLine(fLineX, sWorldPos.y, fLineX, fY2, TICKER_GRAY);
    }
    if (m_bSoloMode) {
        const FLOAT fX2 = sWorldPos.x + sWorldSize.x;
        CGameRenderer::DrawLine(sWorldPos.x, fY2, fX2, fY2, 0xFFFFFFFF);
        CGameRenderer::DrawLine(sWorldPos.x, sWorldPos.y, sWorldPos.x, fY2, 0xFFFFFFFF);
        CGameRenderer::DrawLine(fX2, sWorldPos.y, fX2, fY2, 0xFFFFFFFF);
    }

    if (NULL != m_pfYValues) {
        UINT uIndex = 0;
        if (!m_bSecondsMode) {
            uIndex = (m_uNum > m_uMaxDisplayableNum) ? (m_uNum - m_uMaxDisplayableNum) : 0;
        }
        FLOAT fPrevX = -1.0f;
        FLOAT fPrevY = -1.0f;
        FLOAT fX = sWorldPos.x + (m_bSecondsMode ? (sWorldSize.x - 1.0f) : TICKER_WIDTH_FOR_LABEL);
        const FLOAT fXOffset = m_bSecondsMode ? -m_fWidthPerTick : m_fWidthPerTick;
        for ( ; m_uNum > uIndex; ++uIndex, fX += fXOffset) {
            const FLOAT fY = sWorldPos.y + m_pfYValues[uIndex];
            if (0.0 <= fPrevX) {
                UINT ulineColor = 0xFFFFFFFF;
                if (m_uFocusStartIndex <= uIndex && m_uFocusEndIndex >= uIndex) {
                    ulineColor = RGBA(0x7F, 0x7F, 0xFF, 0xFF);
                }
                CGameRenderer::DrawLine(fPrevX, fPrevY, fX, fY, ulineColor);
            }
            fPrevX = fX;
            fPrevY = fY;
        }
        if (!m_bSecondsMode) {
            if (0xFFFFFFFF != m_uFocusStartIndex && m_uNum > m_uFocusStartIndex) {
                const FLOAT fX = sWorldPos.x + TICKER_WIDTH_FOR_LABEL + m_uFocusStartIndex * m_fWidthPerTick;
                CGameRenderer::DrawLine(fX, m_sWorldPos.y, fX, fY2, 0xFFFFFFFF);
            }
            if (0 != m_uFocusEndIndex && m_uNum > m_uFocusEndIndex) {
                const FLOAT fX = sWorldPos.x + TICKER_WIDTH_FOR_LABEL + m_uFocusEndIndex * m_fWidthPerTick;
                CGameRenderer::DrawLine(fX, m_sWorldPos.y, fX, fY2, 0xFFFFFFFF);
            }
        }
    }
    if (!m_bSoloMode && !m_bSecondsMode) {
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

VOID CTickerPrice::UpdateMaxDisplayableNum(UINT uNum)
{
    if (m_bSecondsMode) {
        return;
    }
    m_uMaxDisplayableNum = uNum;
    if (NULL == m_pfYValues) {
        return;
    }
    if (m_uNum < uNum) {
        ASSERT(FALSE);
        return;
    }
    const FLOAT fChartableWidth = GetLocalSize().x - 1.0f - TICKER_WIDTH_FOR_LABEL;
    m_fWidthPerTick = fChartableWidth / (uNum - 1);
}

VOID CTickerPrice::UpdateLines(const STickListener& sTicker)
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
    m_afLines[0][0][1] = sWorldPos.y + m_pfYValues[uUpperIndex1] - 5.0f;
    
    const UINT uUpperIndex2 = sTicker.sProcessed.aauUpperIndexOBV[eType][1];
    m_afLines[0][1][0] = fX + (uUpperIndex2 - uStartIndex) * m_fWidthPerTick;
    m_afLines[0][1][1] = sWorldPos.y + m_pfYValues[uUpperIndex2] - 5.0f;
    
    const UINT uLowerIndex1 = sTicker.sProcessed.aauLowerIndexOBV[eType][0];
    m_afLines[1][0][0] = fX + (uLowerIndex1 - uStartIndex) * m_fWidthPerTick;
    m_afLines[1][0][1] = sWorldPos.y + m_pfYValues[uLowerIndex1] + 5.0f;
    
    const UINT uLowerIndex2 = sTicker.sProcessed.aauLowerIndexOBV[eType][1];
    m_afLines[1][1][0] = fX + (uLowerIndex2 - uStartIndex) * m_fWidthPerTick;
    m_afLines[1][1][1] = sWorldPos.y + m_pfYValues[uLowerIndex2] + 5.0f;
}

VOID CTickerPrice::Focus(UINT uOpenSec, UINT uCloseSec)
{
    m_uFocusStartSec = uOpenSec;
    m_uFocusEndSec = uCloseSec;
}
VOID CTickerPrice::ClearFocus(VOID)
{
    m_uFocusStartSec = m_uFocusEndSec = 0;
}

BOOLEAN CTickerPrice::OnTouchBegin(FLOAT fX, FLOAT fY)
{
    if (m_bSecondsMode) {
        m_bPerformPickCheck = FALSE;
        return FALSE;
    }
    if (0 != m_uEventHandlerID) {
        const SHLVector2D sPos = { fX, fY };
        m_bPerformPickCheck = HitTest(sPos);
        if (m_bPerformPickCheck) {
            OnTouchMove(fX, fY);
        }
    }
    return m_bPerformPickCheck;
}

BOOLEAN CTickerPrice::OnTouchMove(FLOAT fX, FLOAT fY)
{
    if (!m_bPerformPickCheck) {
        return FALSE;
    }
    if (0 == m_uEventHandlerID) {
        return TRUE;
    }
    // try pick the time on the line
    const FLOAT fStartX = GetWorldPosition().x + TICKER_WIDTH_FOR_LABEL;
    const FLOAT fDiffX = fX - fStartX;
    INT nPickedIndex = fDiffX / m_fWidthPerTick;
    if (0 > nPickedIndex) {
        nPickedIndex = 0;
    }
    else if (m_uNum <= nPickedIndex) {
        nPickedIndex = m_uNum - 1;
    }
    
    const UINT uIndex = (m_uNum > m_uMaxDisplayableNum) ? (m_uNum - m_uMaxDisplayableNum) : 0;
    nPickedIndex += uIndex;
    const UINT uTimePicked = m_uStartTimeSec + nPickedIndex * m_uPerTickTimeSec;
//    TRACE("Picked %d\n", uTimePicked);
    CEventManager::PostGlobalEvent(m_uEventHandlerID, CEvent(m_uPickEventID, uTimePicked, uTimePicked));
    return TRUE;
}

BOOLEAN CTickerPrice::OnTouchEnd(FLOAT fX, FLOAT fY)
{
    return OnTouchMove(fX, fY);
}
