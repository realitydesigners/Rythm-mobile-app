#include "stdafx.h"
#include "Event.h"
#include "EventID.h"
#include "EventManager.h"
#include "GameRenderer.h"
#include "GameWidgetDefs.h"
#include "MACDChart.h"
#include "PlayerData.h"
#include "TradeChart.h"
#include "MAUtil.h"

#include <math.h>

#define CHART_Y_PADDING (20.0f)

CMACDChart::CMACDChart(CTradeChart& cChart) :
CBaseChart(EGLOBAL_MACD_CHART, cChart),
m_pdPrices(NULL),
m_pdMA1(NULL),
m_pdMA2(NULL),
m_pfHistogramHeight(NULL),
m_uMaxSize(0),
m_fSignalZeroY(0.0f)
{
    memset(m_pcLabels, 0, sizeof(m_pcLabels));
    memset(m_pfPosY, 0, sizeof(m_pfPosY));
    memset(m_pdValues, 0, sizeof(m_pdValues));
}
    
CMACDChart::~CMACDChart()
{
    
}

VOID CMACDChart::Initialize(VOID)
{
    CBaseChart::Initialize();
    UINT uIndex = 0;
    for ( ; 2 > uIndex; ++uIndex) {
        CUITextLabel* pcLabel = NEW_TEXT_LABEL;
        if (NULL == pcLabel) {
            ASSERT(FALSE);
            return;
        }
        pcLabel->SetColor(RGBA(0x7F, 0x7F, 0x7F, 0xFF));
        pcLabel->SetFont(EGAMEFONT_SIZE_16);
        pcLabel->SetAnchor(0.0f, 0.5f);
        AddChild(*pcLabel);
        m_pcLabels[uIndex] = pcLabel;
    }
}
    
VOID CMACDChart::Release(VOID)
{
    UINT uIndex = 0;
    for ( ; 2 > uIndex; ++uIndex) {
        if (NULL != m_pcLabels[uIndex]) {
            m_pcLabels[uIndex]->RemoveFromParent();
            m_pcLabels[uIndex]->Release();
            delete m_pcLabels[uIndex];
            m_pcLabels[uIndex] = NULL;
        }
    }
    ClearDataArrays();
    CBaseChart::Release();
}
VOID CMACDChart::ClearDataArrays(VOID)
{
    UINT uIndex = 0;
    for ( ; 2 > uIndex; ++uIndex) {
        SAFE_DELETE_ARRAY(m_pfPosY[uIndex]);
        SAFE_DELETE_ARRAY(m_pdValues[uIndex]);
    }
    SAFE_DELETE_ARRAY(m_pdPrices);
    SAFE_DELETE_ARRAY(m_pdMA1);
    SAFE_DELETE_ARRAY(m_pdMA2);
    SAFE_DELETE_ARRAY(m_pfHistogramHeight);
    m_uMaxSize = 0;
}

BOOLEAN CMACDChart::SetupDataArrays(UINT uNum)
{
    if (m_uMaxSize >= uNum) {
        ASSERT(NULL != m_pfPosY[0]);
        ASSERT(NULL != m_pfPosY[1]);
        ASSERT(NULL != m_pdValues[0]);
        ASSERT(NULL != m_pdValues[1]);
        ASSERT(NULL != m_pdPrices);
        ASSERT(NULL != m_pdMA1);
        ASSERT(NULL != m_pdMA2);
        ASSERT(NULL != m_pfHistogramHeight);
        return TRUE;
    }
    m_pfPosY[0] = new FLOAT[uNum];
    m_pfPosY[1] = new FLOAT[uNum];
    m_pdValues[0] = new DOUBLE[uNum];
    m_pdValues[1] = new DOUBLE[uNum];
    m_pdPrices = new DOUBLE[uNum];
    m_pdMA1 = new DOUBLE[uNum];
    m_pdMA2 = new DOUBLE[uNum];
    m_pfHistogramHeight = new FLOAT[uNum];
    ASSERT(NULL != m_pfPosY[0]);
    ASSERT(NULL != m_pfPosY[1]);
    ASSERT(NULL != m_pdValues[0]);
    ASSERT(NULL != m_pdValues[1]);
    ASSERT(NULL != m_pdPrices);
    ASSERT(NULL != m_pdMA1);
    ASSERT(NULL != m_pdMA2);
    ASSERT(NULL != m_pfHistogramHeight);
    m_uMaxSize = uNum;
    return TRUE;
}

VOID CMACDChart::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    // render time lines
    RenderTimeLines();
    const SHLVector2D& sWorldPos = GetWorldPosition();
    const SHLVector2D& sWorldSize = GetWorldSize();
    // render labels
    const FLOAT fX1 = sWorldPos.x;
    const FLOAT fX2 = fX1 + sWorldSize.x - 40.0f;
    CGameRenderer::DrawLine(fX1, sWorldPos.y, fX2, sWorldPos.y, GRID_COLOR);
    const FLOAT fY = sWorldPos.y + sWorldSize.y;
    CGameRenderer::DrawLine(fX1, fY, fX2, fY, GRID_COLOR);
    UINT uIndex = 0;
    for ( ; 2 > uIndex; ++uIndex) {
        CUITextLabel* pcLbl = m_pcLabels[uIndex];
        if (NULL != pcLbl && pcLbl->IsVisible()) {
            const FLOAT fY = pcLbl->GetWorldPosition().y + pcLbl->GetWorldSize().y * 0.5f;
            CGameRenderer::DrawLine(fX1, fY, fX2, fY, GRID_COLOR);
        }
    }
    
    // render histogram
    if (NULL != m_pfHistogramHeight) {
        uIndex = 0;
        FLOAT fX = m_fStartX + 1.0f - m_fCandleWidth * 0.5f;
        const FLOAT fHistogramW = m_fCandleWidth - 2.0f;
        for ( ; m_uIntervalNum > uIndex; ++uIndex) {
            const FLOAT fY = m_pfHistogramHeight[uIndex];
            CGameRenderer::DrawRectNoTex(fX, m_fSignalZeroY, fHistogramW, -fY, 0.0f <= fY ? MCAD_COLOR_BLUE : MCAD_COLOR_RED);
            fX += m_fCandleWidth;
        }
    }
    const UINT auColor[2] = { MCAD_COLOR_1, MCAD_COLOR_2 };
    uIndex = 0;
    for ( ; 2 > uIndex; ++uIndex) {
        if (NULL != m_pfPosY[uIndex]) {
            FLOAT fX = m_fStartX;
            FLOAT fPrevY = m_pfPosY[uIndex][0];
            const UINT uColor = auColor[uIndex];
            UINT uIndex2 = 1;
            for ( ; m_uIntervalNum > uIndex2; ++uIndex2) {
                const FLOAT fY = m_pfPosY[uIndex][uIndex2];
                CGameRenderer::DrawLine(fX, fPrevY, fX + m_fCandleWidth, fY, uColor);
                fX += m_fCandleWidth;
                fPrevY = fY;
            }
        }
    }
    CBaseChart::OnRender(fOffsetX, fOffsetY);
}

VOID CMACDChart::GetToolTipString(CHAR* szBuffer, UINT uBufferLen, UINT uCandleIndexSelected) const
{
    if (NULL != m_pdValues[0] && NULL != m_pdValues[1]) {
        const DOUBLE dV1 = m_pdValues[0][uCandleIndexSelected%m_uIntervalNum];
        const DOUBLE dV2 = m_pdValues[1][uCandleIndexSelected%m_uIntervalNum];
        snprintf(szBuffer, uBufferLen, "MACD\n%f,%f\n%f", dV1, dV2, dV1 - dV2);
    }
    else {
        strncpy(szBuffer, "MACD\n0.0", uBufferLen);
    }
}
FLOAT CMACDChart::GetToolTipY(UINT uCandleIndexSelected) const
{
    if (NULL == m_pfPosY[0]) {
        return 0.0f;
    }
    return m_pfPosY[0][uCandleIndexSelected % m_uIntervalNum];
}

VOID CMACDChart::SetupChart(VOID)
{
    const UINT uVisibleCandleNum = m_cMainChart.GetDisplayCandleNum();
    const SDisplayCandle* psCandles = m_cMainChart.GetDisplayCandles();
    if (NULL == psCandles || 2 > uVisibleCandleNum) {
        m_uIntervalNum = 0;
        return;
    }
    if (!SetupDataArrays((UINT)m_cMainChart.GetTradeCandleMap().size())) {
        m_uIntervalNum = 0;
        return;
    }
    m_fCandleWidth = psCandles[0].afBox[2] + 2.0f;
    m_fStartX = psCandles[0].afBox[0] - 1.0f + m_fCandleWidth * 0.5f;
    m_uIntervalNum = uVisibleCandleNum;
    
    const UINT uCandleDurationSec = psCandles[1].sData.uTimeSec - psCandles[0].sData.uTimeSec;
    const UINT uFirstVisibleCandleTimeSec = psCandles[0].sData.uTimeSec;
    const CTradeCandleMap& cCandles = m_cMainChart.GetTradeCandleMap();
    
    SMACD sParam(CPlayerData::GetMACDParam());
    sParam.pdMA1 = m_pdMA1;
    sParam.pdMA2 = m_pdMA2;
    sParam.pdMACD = m_pdValues[0];
    sParam.pdSignal = m_pdValues[1];
    sParam.uNum = m_uMaxSize;
    const UINT uTimeSec = CCryptoUtil::FindFirstCandleTimeSec(cCandles);
    if (!CMAUtil::GenerateMACD(sParam, cCandles, uTimeSec, uCandleDurationSec, uVisibleCandleNum, uFirstVisibleCandleTimeSec)) {
        return;
    }
    DOUBLE dMax = 0.0;
    DOUBLE dMin = 0.0;
    DOUBLE dHistogramMax = 0.0;
    DOUBLE dHistogramMin = 0.0;
    UINT uIndex = 0;
    for ( ; m_uIntervalNum > uIndex; ++uIndex) {
        const DOUBLE dVal1 = m_pdValues[0][uIndex];
        if (0 == uIndex) {
            dMax = dMin = dVal1;
        }
        if (dMax < dVal1) {
            dMax = dVal1;
        }
        else if (dMin > dVal1) {
            dMin = dVal1;
        }
        const DOUBLE dVal2 = m_pdValues[1][uIndex];
        if (dMax < dVal2) {
            dMax = dVal2;
        }
        else if (dMin > dVal2) {
            dMin = dVal2;
        }
        const DOUBLE dHistogram = dVal1 - dVal2;
        if (dHistogramMax < dHistogram) {
            dHistogramMax = dHistogram;
        }
        else if (dHistogramMin > dHistogram) {
            dHistogramMin = dHistogram;
        }
    }
    const SHLVector2D& sSize = GetWorldSize();
    {   // compute the place to draw the 2 curves (MACD and signal line)
        DOUBLE dTotalHeightValue = (dMax - dMin);
        const DOUBLE dPaddingValue = dTotalHeightValue * 0.1f;
        dTotalHeightValue += 2.0 * dPaddingValue;
        const DOUBLE dComputedMax = dMax + dPaddingValue;
        const DOUBLE dPixelPerHeight = sSize.y / dTotalHeightValue;
        const FLOAT fHighestValueY = GetWorldPosition().y;
        uIndex = 0;
        for ( ; m_uIntervalNum > uIndex; ++uIndex) {
            // MACD line Y pos
            m_pfPosY[0][uIndex] = fHighestValueY + (dComputedMax - m_pdValues[0][uIndex]) * dPixelPerHeight;
            // signal line Y Pos
            m_pfPosY[1][uIndex] = fHighestValueY + (dComputedMax - m_pdValues[1][uIndex]) * dPixelPerHeight;
        }
        // also mark the labels
        CHAR szFormatter[32];
        const DOUBLE dNumber = abs(dMax);
        if (1.0 < dNumber) {
            strncpy(szFormatter, "%.1f", 32);
        }
        else if (0.1f < dNumber) {
            strncpy(szFormatter, "%.2f", 32);
        }
        else if (0.01f < dNumber) {
            strncpy(szFormatter, "%.3f", 32);
        }
        else if (0.001f < dNumber) {
            strncpy(szFormatter, "%.4f", 32);
        }
        else if (0.0001f < dNumber) {
            strncpy(szFormatter, "%.5f", 32);
        }
        else if (0.00001f < dNumber) {
            strncpy(szFormatter, "%.6f", 32);
        }
        
        const DOUBLE adValues[2] = { dMax, dMin };
        const FLOAT fX = GetLocalSize().x - CHART_OFFSET_FROM_RIGHT + 5.0f;
        CHAR szBuffer[64];
        uIndex = 0;
        for ( ; 2 > uIndex; ++uIndex) {
            snprintf(szBuffer, 64, szFormatter, adValues[uIndex]);
            m_pcLabels[uIndex]->SetString(szBuffer);
            const FLOAT fY = (dComputedMax - adValues[uIndex]) * dPixelPerHeight;
            //const FLOAT fY = CHART_Y_PADDING * 0.5f + (dHistogramMax - adValues[uIndex]) * dMultiplier * dPixelPerHeight;
            m_pcLabels[uIndex]->SetLocalPosition(fX, fY);
            m_pcLabels[uIndex]->SetVisibility(TRUE);
        }
    }
    // compute the histogram height
    {
        const DOUBLE dAbsMin = abs(dHistogramMin);
        const DOUBLE dBiggestNumer = dHistogramMax > dAbsMin ? dHistogramMax : dAbsMin;
        dHistogramMax = dBiggestNumer;
        dHistogramMin = -dBiggestNumer;
        DOUBLE dTotalHeightValue = (dHistogramMax - dHistogramMin);
        const DOUBLE dPaddingValue = dTotalHeightValue * 0.1f;
        dTotalHeightValue += 2.0 * dPaddingValue;
        dHistogramMax += dPaddingValue;
        
        const DOUBLE dPixelPerHeight = sSize.y / dTotalHeightValue;
        m_fSignalZeroY = GetLocalPosition().y + dHistogramMax * dPixelPerHeight;
        uIndex = 0;
        for ( ; m_uIntervalNum > uIndex; ++uIndex) {
            const DOUBLE dVal1 = m_pdValues[0][uIndex];
            const DOUBLE dVal2 = m_pdValues[1][uIndex];
            const DOUBLE dHistogram = dVal1 - dVal2;
            m_pfHistogramHeight[uIndex] = dHistogram * dPixelPerHeight;
        }
    }
}

VOID CMACDChart::GetTitleString(CHAR* szBuffer, UINT uBufferLen) const
{
    const SMACDParam& sParam = CPlayerData::GetMACDParam();
    snprintf(szBuffer, uBufferLen, "MACD(%d,%d,%d)", sParam.sMA1.ushInterval, sParam.sMA2.ushInterval, sParam.sSignal.ushInterval);
}
