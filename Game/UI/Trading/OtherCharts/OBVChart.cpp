#include "stdafx.h"
#include "Event.h"
#include "EventID.h"
#include "EventManager.h"
#include "GameRenderer.h"
#include "GameWidgetDefs.h"
#include "PlayerData.h"
#include "OBVChart.h"
#include "OBVChartDefs.h"
#include "TradeChart.h"
#include "VolumeUtil.h"

#include <math.h>

#define CHART_Y_PADDING (20.0f)

COBVChart::COBVChart(CTradeChart& cChart) :
CBaseChart(EGLOBAL_OBV_CHART, cChart),
INITIALIZE_TEXT_LABEL(m_cTotalVol),
m_uOBVNum(0),
m_pdOBV(NULL),
m_pfValuesY(NULL),
m_pfDifferenceY(NULL)
{
    m_cTotalVol.SetFont(EGAMEFONT_SIZE_20);
    m_cTotalVol.SetAnchor(0.0f, 0.5f);
    m_cTotalVol.AutoSize();
}
    
COBVChart::~COBVChart()
{
    
}

VOID COBVChart::Initialize(VOID)
{
    CBaseChart::Initialize();
}
VOID COBVChart::Release(VOID)
{
    m_cTotalVol.Release();
    m_cTotalVol.RemoveFromParent();

    m_uOBVNum = 0;
    SAFE_DELETE_ARRAY(m_pdOBV);
    SAFE_DELETE_ARRAY(m_pfValuesY);
    SAFE_DELETE_ARRAY(m_pfDifferenceY);
    CBaseChart::Release();
}

static VOID GenerateNumberString(CHAR* szBuffer, UINT uBufferLen, DOUBLE dVal, BOOLEAN bAddLineBreak)
{
    const DOUBLE dABS = abs(dVal);
    if (1000000 < dABS) { // if more than 1 Mil
        const INT uM = dVal / 1000000;
        const UINT uK = (((UINT)dABS)% 1000000) / 1000;
        if (bAddLineBreak) {
            snprintf(szBuffer, uBufferLen, "%dM\n%dK", uM, uK);
        }
        else {
            snprintf(szBuffer, uBufferLen, "%dM %dK", uM, uK);
        }
        return;
    }
    if (1000 < dABS) {
        const INT uK = dVal / 1000;
        const UINT uO = (((UINT)dABS)% 1000);
        if (bAddLineBreak) {
            snprintf(szBuffer, uBufferLen, "%dK\n%d", uK, uO);
        }
        else {
            snprintf(szBuffer, uBufferLen, "%dK %d", uK, uO);
        }
        return;
    }
    snprintf(szBuffer, uBufferLen, "%.1f", dVal);
}
VOID COBVChart::GetToolTipString(CHAR* szBuffer, UINT uBufferLen, UINT uCandleIndexSelected) const
{
    DOUBLE dDiff = 0.0;
    const UINT uActualIndex = m_uVisibleStartIndex + uCandleIndexSelected;
    const DOUBLE dNewVal = m_pdOBV[uActualIndex % m_uOBVNum];
    if (0 < uActualIndex) {
        const DOUBLE dOldVal = m_pdOBV[(uActualIndex - 1) % m_uOBVNum];
        dDiff = dNewVal - dOldVal;
    }
    CHAR szDiffString[128];
    CHAR szValString[128];
    GenerateNumberString(szDiffString, 128, dDiff, FALSE);
    GenerateNumberString(szValString, 128, dNewVal, FALSE);
    snprintf(szBuffer, uBufferLen, "OBV\nTotal: %s\nChange %s", szValString, szDiffString);
}
FLOAT COBVChart::GetToolTipY(UINT uCandleIndexSelected) const
{
    if (NULL == m_pfValuesY) {
        return 0.0f;
    }
    return m_pfValuesY[uCandleIndexSelected % m_uIntervalNum];
}

VOID COBVChart::SetupChart(VOID)
{
    m_uOBVNum = 0;
    SAFE_DELETE_ARRAY(m_pdOBV);
    SAFE_DELETE_ARRAY(m_pfValuesY);
    SAFE_DELETE_ARRAY(m_pfDifferenceY);
    const UINT uVisibleCandleNum = m_cMainChart.GetDisplayCandleNum();
    const SDisplayCandle* psCandles = m_cMainChart.GetDisplayCandles();
    if (NULL == psCandles || 2 > uVisibleCandleNum) { // min need 2 candles
        return;
    }
    const CTradeCandleMap& cCandles = m_cMainChart.GetTradeCandleMap();
    const UINT uFirstCandleTime = CCryptoUtil::FindFirstCandleTimeSec(cCandles);
    const UINT uFirstVisibleCandleTime = psCandles[0].sData.uTimeSec;
    if (uFirstVisibleCandleTime < uFirstCandleTime) {
        ASSERT(FALSE);
        return;
    }
    const UINT uCandleDurationSec = psCandles[1].sData.uTimeSec - psCandles[0].sData.uTimeSec;
    ASSERT(((uFirstVisibleCandleTime - uFirstCandleTime) % uCandleDurationSec) == 0);
    m_uVisibleStartIndex = (uFirstVisibleCandleTime - uFirstCandleTime) / uCandleDurationSec;
    // Y value we do not add 1 candle (only show visible candles)
    m_pfValuesY = new FLOAT[uVisibleCandleNum];
    m_pfDifferenceY = new FLOAT[uVisibleCandleNum];
    if (NULL == m_pfValuesY || NULL == m_pfDifferenceY) {
        return;
    }
    memset(m_pfValuesY, 0, sizeof(FLOAT) * uVisibleCandleNum);
    memset(m_pfDifferenceY, 0, sizeof(FLOAT) * uVisibleCandleNum);
    
    m_fCandleWidth = psCandles[0].afBox[2] + 2.0f;
    m_fStartX = psCandles[0].afBox[0] - 1.0f + m_fCandleWidth * 0.5f;
    m_uIntervalNum = uVisibleCandleNum;
    
    m_pdOBV = CVolumeUtil::GenerateOBV(m_uOBVNum, cCandles, uCandleDurationSec, CPlayerData::IsOBVUsingPriceMultiplier());
    if (NULL == m_pdOBV) {
        return;
    }
    DOUBLE dHighest = m_pdOBV[m_uVisibleStartIndex];
    DOUBLE dLowest = m_pdOBV[m_uVisibleStartIndex];
    DOUBLE dPrevOBV = m_pdOBV[m_uVisibleStartIndex];
    DOUBLE dHighestDiff = m_pdOBV[m_uVisibleStartIndex + 1] - dPrevOBV;
    DOUBLE dLowestDiff = dHighestDiff;
    UINT uIndex = 1;
    for ( ; uVisibleCandleNum > uIndex; ++uIndex) {
        const UINT uActualIndex = m_uVisibleStartIndex + uIndex;
        if (m_uOBVNum <= uActualIndex) {
            ASSERT(FALSE);
            continue;
        }
        const DOUBLE dNowVal = m_pdOBV[uActualIndex];
        if (dHighest < dNowVal) {
            dHighest = dNowVal;
        }
        if (dLowest > dNowVal) {
            dLowest = dNowVal;
        }
        const DOUBLE dDiff = dNowVal - dPrevOBV;
        dPrevOBV = dNowVal;
        if (dHighestDiff < dDiff) {
            dHighestDiff = dDiff;
        }
        if (dLowestDiff > dDiff) {
            dLowestDiff = dDiff;
        }
    }
    {
        const DOUBLE dTotalVol = dHighest - dLowest;
        CHAR szTotalString[128];
        GenerateNumberString(szTotalString, 128, dTotalVol, TRUE);
        m_cTotalVol.SetString(szTotalString);
        m_cTotalVol.SetLocalPosition(GetLocalSize().x - 40.0f, 15.0f);
        m_cTotalVol.RemoveFromParent();
        AddChild(m_cTotalVol);
    }
    const FLOAT fHighestValueY = GetLocalPosition().y;
    {
        DOUBLE dTotalValueDiff = (dHighest - dLowest);
        const DOUBLE dPadding = dTotalValueDiff * 0.1f;
        dTotalValueDiff += 2.0 * dPadding;
        dHighest += dPadding;
        const FLOAT fPixelsPerValue = GetLocalSize().y / dTotalValueDiff;
        uIndex = 0;
        for ( ; uVisibleCandleNum > uIndex; ++uIndex) {
            const UINT uActualIndex = m_uVisibleStartIndex + uIndex;
            if (m_uOBVNum <= uActualIndex) {
                ASSERT(FALSE);
                continue;
            }
            m_pfValuesY[uIndex] = fHighestValueY + (dHighest - m_pdOBV[uActualIndex]) * fPixelsPerValue;
        }
    }
    {
        DOUBLE dTotalValueDiff = (dHighestDiff - dLowestDiff);
        const DOUBLE dPadding = dTotalValueDiff * 0.1f;
        dTotalValueDiff += 2.0 * dPadding;
        dHighestDiff += dPadding;
        const FLOAT fPixelsPerValue = GetLocalSize().y / dTotalValueDiff;
        uIndex = 0;
        for ( ; uVisibleCandleNum > uIndex; ++uIndex) {
            const UINT uActualIndex = m_uVisibleStartIndex + uIndex;
            if (m_uOBVNum <= uActualIndex) {
                ASSERT(FALSE);
                continue;
            }
            DOUBLE dDiff = 0.0;
            if (0 < uActualIndex) {
                dDiff = m_pdOBV[uActualIndex] - m_pdOBV[uActualIndex-1];
            }
            m_pfDifferenceY[uIndex] = fHighestValueY + (dHighestDiff - dDiff) * fPixelsPerValue;
        }
    }
}

VOID COBVChart::GetTitleString(CHAR* szBuffer, UINT uBufferLen) const
{
    if (CPlayerData::IsOBVUsingPriceMultiplier()) {
        snprintf(szBuffer, uBufferLen, "OBV-Special");
    }
    else {
        snprintf(szBuffer, uBufferLen, "OBV-Normal");
    }
}

VOID COBVChart::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    // render time lines
    RenderTimeLines();
    // render horizontal lines too
    const SHLVector2D& sWorldPos = GetWorldPosition();
    const SHLVector2D& sWorldSize = GetWorldSize();
    const FLOAT fEndX = sWorldPos.x + sWorldSize.x - 40.0f;
    const FLOAT fYOffset = sWorldSize.y / 2;
    FLOAT fY = sWorldPos.y;
    CGameRenderer::DrawLine(sWorldPos.x, fY, fEndX, fY, GRID_COLOR); fY += fYOffset;
    CGameRenderer::DrawLine(sWorldPos.x, fY, fEndX, fY, GRID_COLOR); fY += fYOffset;
    CGameRenderer::DrawLine(sWorldPos.x, fY, fEndX, fY, GRID_COLOR);
    if (NULL != m_pfValuesY) {
        FLOAT fX = m_fStartX;
        FLOAT fPrevY = m_pfValuesY[0];
        UINT uIndex = 1;
        for ( ; m_uIntervalNum > uIndex; ++uIndex) {
            const FLOAT fY = m_pfValuesY[uIndex];
            CGameRenderer::DrawLine(fX, fPrevY, fX + m_fCandleWidth, fY, OBV_MAIN_LINE_COLOR);
            fX += m_fCandleWidth;
            fPrevY = fY;
        }
    }
    if (NULL != m_pfDifferenceY) {
        FLOAT fX = m_fStartX;
        FLOAT fPrevY = m_pfDifferenceY[0];
        UINT uIndex = 1;
        for ( ; m_uIntervalNum > uIndex; ++uIndex) {
            const UINT uActualIndex = m_uVisibleStartIndex + uIndex;
            DOUBLE dDiff = 0.0;
            if (1 < uActualIndex) {
                dDiff = m_pdOBV[uActualIndex] - m_pdOBV[uActualIndex-1];
            }
            const FLOAT fY = m_pfDifferenceY[uIndex];
            const UINT uColor = 0.0 < dDiff ? RGBA(0,0xFF,0,0xFF) : RGBA(0xFF, 0, 0, 0xFF);
            CGameRenderer::DrawLine(fX, fPrevY, fX + m_fCandleWidth, fY, uColor);
            fX += m_fCandleWidth;
            fPrevY = fY;
        }
    }
    
    CBaseChart::OnRender(fOffsetX, fOffsetY);
}
