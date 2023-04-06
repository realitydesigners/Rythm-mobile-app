#include "stdafx.h"
#include "AutoPtr.h"
#include "CryptoUtil.h"
#include "GameRenderer.h"
#include "MAUtil.h"
#include "PlayerData.h"
#include "TickerMACD.h"
#include "TickerUIDefs.h"
#include <math.h>

CTickerMACD::CTickerMACD() :
CUIContainer(),
m_uMaxDisplayableNum(TICK_HISTORY_MINS),
m_uNum(0),
m_fMACDZeroY(0.0f),
m_pfHistogramHeight(NULL),
m_fHistogramZeroY(0.0f),
m_fWidthPerTick(0.0f)
{
    m_apfMACDY[0] = m_apfMACDY[1] = NULL;
}

CTickerMACD::~CTickerMACD()
{
}

VOID CTickerMACD::ReleaseArrays(VOID)
{
    SAFE_DELETE_ARRAY(m_apfMACDY[0]);
    SAFE_DELETE_ARRAY(m_apfMACDY[1]);
    SAFE_DELETE_ARRAY(m_pfHistogramHeight);
    m_uNum = 0;
}
VOID CTickerMACD::Release(VOID)
{
    ReleaseArrays();
    CUIContainer::Release();
}

VOID CTickerMACD::UpdateMaxDisplayableNum(UINT uNum)
{
    m_uMaxDisplayableNum = uNum;
    if (NULL == m_apfMACDY[0]) {
        return;
    }
    if (m_uNum < uNum) {
        ASSERT(FALSE);
        return;
    }
}

VOID CTickerMACD::UpdateTick(const STickListener& sTickListener)
{
    ReleaseArrays();
    const UINT uSize = sTickListener.sProcessed.uNum;
    if (20 > uSize) {
        return;
    }    
    m_apfMACDY[0] = new FLOAT[uSize];
    m_apfMACDY[1] = new FLOAT[uSize];
    m_pfHistogramHeight = new FLOAT[uSize];
    if (NULL == m_apfMACDY[0] || NULL == m_apfMACDY[1] || NULL == m_pfHistogramHeight) {
        ASSERT(FALSE);
        return;
    }
    memset(m_apfMACDY[0], 0, sizeof(FLOAT) * uSize);
    memset(m_apfMACDY[1], 0, sizeof(FLOAT) * uSize);
    memset(m_pfHistogramHeight, 0, sizeof(FLOAT) * uSize);
    
    m_uNum = uSize;
    
    DOUBLE dMaxMACD = 0.0;
    DOUBLE dMinMACD = 0.0;
    DOUBLE dMaxHistogram = 0.0;
    DOUBLE dMinHistogram = 0.0;
    const UINT uStartIndex = (uSize > m_uMaxDisplayableNum ? (uSize - m_uMaxDisplayableNum) : 0);
    UINT uIndex = 0;
    for ( ; uSize > uIndex; ++uIndex) {
        if (uIndex < uStartIndex) {
            continue;
        }
        ASSERT(sTickListener.sProcessed.uNum >= uIndex);
        const DOUBLE dMACD = sTickListener.sProcessed.pdMACD[uIndex];
        const DOUBLE dHistogram = dMACD - sTickListener.sProcessed.pdSignal[uIndex];
        if (dMaxMACD < dMACD) {
            dMaxMACD = dMACD;
        }
        else if (dMinMACD > dMACD) {
            dMinMACD = dMACD;
        }
        if (dMaxHistogram < dHistogram) {
            dMaxHistogram = dHistogram;
        }
        else if (dMinHistogram > dHistogram) {
            dMinHistogram = dHistogram;
        }
    }
    const SHLVector2D& sLocalSize = GetLocalSize();
    const FLOAT fHighestValueY = GetWorldPosition().y;
    {
        DOUBLE dTotalHeightValue = (dMaxMACD - dMinMACD);
        if (0.0 == dTotalHeightValue) {
            m_fWidthPerTick = 0.0f;
            return;
        }
        const DOUBLE dPaddingValue = dTotalHeightValue * 0.1f;
        dTotalHeightValue += 2.0 * dPaddingValue;
        dMaxMACD += dPaddingValue;
        const DOUBLE dPixelPerHeight = sLocalSize.y / dTotalHeightValue;
        uIndex = 0;
        for ( ; uSize > uIndex; ++uIndex) {
            // MACD line Y pos
            m_apfMACDY[0][uIndex] = fHighestValueY + (dMaxMACD - sTickListener.sProcessed.pdMACD[uIndex]) * dPixelPerHeight;
            // signal line Y Pos
            m_apfMACDY[1][uIndex] = fHighestValueY + (dMaxMACD - sTickListener.sProcessed.pdSignal[uIndex]) * dPixelPerHeight;
        }
        m_fMACDZeroY = fHighestValueY + dMaxMACD * dPixelPerHeight;
    }
    {
        DOUBLE dTotalHeightValue = (dMaxHistogram - dMinHistogram);
        if (0.0 == dTotalHeightValue) {
            return;
        }
        const DOUBLE dPaddingValue = dTotalHeightValue * 0.1f;
        dTotalHeightValue += 2.0 * dPaddingValue;
        dMaxHistogram += dPaddingValue;
        const DOUBLE dPixelPerHeight = sLocalSize.y / dTotalHeightValue;
        uIndex = 0;
        for ( ; uSize > uIndex; ++uIndex) {
            const DOUBLE dHistogram = sTickListener.sProcessed.pdMACD[uIndex] - sTickListener.sProcessed.pdSignal[uIndex];
            m_pfHistogramHeight[uIndex] = dHistogram * dPixelPerHeight;
        }
        m_fHistogramZeroY = fHighestValueY + dMaxHistogram * dPixelPerHeight;
    }
}


VOID CTickerMACD::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    SHLVector2D sWorldPos = GetWorldPosition();
    sWorldPos.x += fOffsetX;
    sWorldPos.y += fOffsetY;
    const SHLVector2D& sWorldSize = GetWorldSize();
    CGameRenderer::DrawLine(sWorldPos.x, sWorldPos.y, sWorldPos.x + sWorldSize.x, sWorldPos.y, 0xFFFFFFFF);
    {   // render vertical lines
        const FLOAT fX1 = sWorldPos.x + TICKER_WIDTH_FOR_LABEL - 1.0f;
        const FLOAT fX2 = sWorldPos.x + sWorldSize.x;
        const FLOAT fY2 = sWorldPos.y + sWorldSize.y;
        
        CGameRenderer::DrawLine(fX1, sWorldPos.y, fX1, fY2, TICKER_GRAY);
        CGameRenderer::DrawLine(fX2, sWorldPos.y, fX2, fY2, TICKER_GRAY);
    }
    const UINT uNum = m_uNum < m_uMaxDisplayableNum ? m_uNum : m_uMaxDisplayableNum;
    const FLOAT fChartableWidth = GetLocalSize().x - 1.0f - TICKER_WIDTH_FOR_LABEL;

    
    // render histogram
    if (NULL != m_pfHistogramHeight) {
        const FLOAT fWidthPerTick = fChartableWidth / uNum;
        const FLOAT fWidth = fWidthPerTick * 0.9f;
        UINT uIndex = (m_uNum - m_uMaxDisplayableNum);
        FLOAT fX = sWorldPos.x + TICKER_WIDTH_FOR_LABEL;
        for ( ; m_uNum > uIndex; ++uIndex, fX += fWidthPerTick) {
            const FLOAT fHeight = m_pfHistogramHeight[uIndex];
            const UINT uColor = (0.0f < fHeight) ? TICKER_BLUE : TICKER_RED;
            CGameRenderer::DrawRectNoTex(fX, m_fHistogramZeroY, fWidth, -fHeight, uColor);
        }
    }
    // render the 2 macd lines
    if (NULL != m_apfMACDY[0] && NULL != m_apfMACDY[1]) {
        const FLOAT fWidthPerTick = fChartableWidth / (uNum - 1);
        UINT uIndex = (m_uNum > m_uMaxDisplayableNum ? (m_uNum - m_uMaxDisplayableNum) : 0);
        FLOAT fX = sWorldPos.x + TICKER_WIDTH_FOR_LABEL;
        FLOAT fPrevY1 = m_apfMACDY[0][uIndex];
        FLOAT fPrevY2 = m_apfMACDY[1][uIndex];
        ++uIndex;
        for ( ; m_uNum > uIndex; ++uIndex, fX += fWidthPerTick) {
            const FLOAT fY2 = m_apfMACDY[1][uIndex];
            CGameRenderer::DrawLine(fX, fPrevY2, fX + fWidthPerTick, fY2, TICKER_RED);
            fPrevY2 = fY2;
            const FLOAT fY1 = m_apfMACDY[0][uIndex];
            CGameRenderer::DrawLine(fX, fPrevY1, fX + fWidthPerTick, fY1, TICKER_GREEN);
            fPrevY1 = fY1;
        }
        // render the zero macd line
        CGameRenderer::DrawLine(sWorldPos.x + TICKER_WIDTH_FOR_LABEL, m_fMACDZeroY, fX, m_fMACDZeroY, 0xFFFFFFFF);
    }
    CUIContainer::OnRender(fOffsetX, fOffsetY);
}
