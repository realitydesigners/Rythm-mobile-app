#include "stdafx.h"
#include "Event.h"
#include "EventID.h"
#include "EventManager.h"
#include "GameRenderer.h"
#include "GameWidgetDefs.h"
#include "PlayerData.h"
#include "RSIChart.h"
#include "TradeChart.h"
#include "MAUtil.h"

#include <math.h>

#define CHART_Y_PADDING (20.0f)

CRSIChart::CRSIChart(CTradeChart& cChart) :
CBaseChart(EGLOBAL_RSI_CHART, cChart),
m_pdValues(NULL),
m_pfValuesY(NULL)
{
    memset(m_pcLabels, 0, sizeof(m_pcLabels));
}
    
CRSIChart::~CRSIChart()
{
    
}

VOID CRSIChart::Initialize(VOID)
{
    CBaseChart::Initialize();
    UINT uIndex = 0;
    for ( ; 3 > uIndex; ++uIndex) {
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
    
VOID CRSIChart::Release(VOID)
{
    UINT uIndex = 0;
    for ( ; 3 > uIndex; ++uIndex) {
        if (NULL != m_pcLabels[uIndex]) {
            m_pcLabels[uIndex]->RemoveFromParent();
            m_pcLabels[uIndex]->Release();
            delete m_pcLabels[uIndex];
            m_pcLabels[uIndex] = NULL;
        }
    }
    SAFE_DELETE_ARRAY(m_pdValues);
    SAFE_DELETE_ARRAY(m_pfValuesY);
    CBaseChart::Release();
}

VOID CRSIChart::GetToolTipString(CHAR* szBuffer, UINT uBufferLen, UINT uCandleIndexSelected) const
{
    
    if (NULL != m_pdValues) {
        snprintf(szBuffer, uBufferLen, "RSI\n%.1f", m_pdValues[uCandleIndexSelected%m_uIntervalNum]);
    }
    else {
        strncpy(szBuffer, "RSI\n0.0", uBufferLen);
    }
}
FLOAT CRSIChart::GetToolTipY(UINT uCandleIndexSelected) const
{
    if (NULL == m_pfValuesY) {
        ASSERT(FALSE);
        return 0.0f;
    }
    return m_pfValuesY[uCandleIndexSelected % m_uIntervalNum];
}

VOID CRSIChart::SetupChart(VOID)
{
    SAFE_DELETE_ARRAY(m_pdValues);
    SAFE_DELETE_ARRAY(m_pfValuesY);
    m_uIntervalNum = 0;
    // setup label positions
    AdjustLabels();
    const UINT uVisibleCandleNum = m_cMainChart.GetDisplayCandleNum();
    const SDisplayCandle* psCandles = m_cMainChart.GetDisplayCandles();
    if (NULL == psCandles || 0 == uVisibleCandleNum) {
        return;
    }
    m_pfValuesY = new FLOAT[uVisibleCandleNum];
    if (NULL == m_pfValuesY) {
        return;
    }
    m_fCandleWidth = psCandles[0].afBox[2] + 2.0f;
    m_fStartX = psCandles[0].afBox[0] - 1.0f + m_fCandleWidth * 0.5f;
    m_uIntervalNum = uVisibleCandleNum;
    
    const UINT uCandleDurationSec = psCandles[1].sData.uTimeSec - psCandles[0].sData.uTimeSec;
    const UINT uFirstVisibleCandleTimeSec = psCandles[0].sData.uTimeSec;
    const CTradeCandleMap& cCandles = m_cMainChart.GetTradeCandleMap();
    m_pdValues = CMAUtil::GenerateRSI(CPlayerData::GetRsiInterval(), cCandles, uCandleDurationSec, uVisibleCandleNum, uFirstVisibleCandleTimeSec);
    if (NULL == m_pdValues) {
        return;
    }
    
    
    const FLOAT fPixelsPerValue = GetLocalSize().y / 100.0f;
    const FLOAT fHighestValueY = GetLocalPosition().y;
    UINT uIndex = 0;
    for ( ; uVisibleCandleNum > uIndex; ++uIndex) {
        m_pfValuesY[uIndex] = fHighestValueY + (100.0 - m_pdValues[uIndex]) * fPixelsPerValue;
    }
}

VOID CRSIChart::AdjustLabels(VOID)
{
    const FLOAT fHighestValue = 100.0f;
    const FLOAT fPixelsPerValue = GetLocalSize().y / fHighestValue;
    const FLOAT fX = GetLocalSize().x - CHART_OFFSET_FROM_RIGHT + 5.0f;
    const FLOAT afValues[3] = { 30.0f, 50.0f, 70.0f };
    CHAR szBuffer[64];
    UINT uIndex = 0;
    for ( ; 3 > uIndex; ++uIndex) {
        if (NULL != m_pcLabels[uIndex]) {
            const FLOAT fY = (fHighestValue - afValues[uIndex]) * fPixelsPerValue;
            m_pcLabels[uIndex]->SetLocalPosition(fX, fY);
            snprintf(szBuffer, 64, "%.f", afValues[uIndex]);
            m_pcLabels[uIndex]->SetString(szBuffer);
            m_pcLabels[uIndex]->RemoveFromParent();
            AddChild(*m_pcLabels[uIndex]);
        }
    }
}

VOID CRSIChart::GetTitleString(CHAR* szBuffer, UINT uBufferLen) const
{
    snprintf(szBuffer, uBufferLen, "RSI(%d)", CPlayerData::GetRsiInterval());
}

VOID CRSIChart::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    // render time lines
    RenderTimeLines();
    const SHLVector2D& sWorldPos = GetWorldPosition();
    const SHLVector2D& sWorldSize = GetWorldSize();
    const FLOAT fX1 = sWorldPos.x;
    const FLOAT fX2 = fX1 + sWorldSize.x - 40.0f;
    UINT uIndex = 0;
    FLOAT fHighestY = 0.0f;
    FLOAT fLowestY = 2000.0f;
    for ( ; 3 > uIndex; ++uIndex) {
        CUITextLabel* pcLbl = m_pcLabels[uIndex];
        if (NULL != pcLbl && pcLbl->IsVisible()) {
            const FLOAT fY = pcLbl->GetWorldPosition().y + pcLbl->GetWorldSize().y * 0.5f;
            if (fHighestY < fY) {
                fHighestY = fY;
            }
            if (fLowestY > fY) {
                fLowestY = fY;
            }
            CGameRenderer::DrawLine(fX1, fY, fX2, fY, GRID_COLOR);
        }
    }
    CGameRenderer::DrawLine(fX1, sWorldPos.y, fX2, sWorldPos.y, GRID_COLOR);
    const FLOAT fY = sWorldPos.y + sWorldSize.y;
    CGameRenderer::DrawLine(fX1, fY, fX2, fY, GRID_COLOR);

    
    if (NULL != m_pfValuesY) {
//        const FLOAT fHeight = fHighestY - fLowestY;
        FLOAT fX = m_fStartX;
        FLOAT fPrevY = m_pfValuesY[0];
        uIndex = 1;
//        UINT uSkipInterval = 0;
        for ( ; m_uIntervalNum > uIndex; ++uIndex) {
            const FLOAT fY = m_pfValuesY[uIndex];
            CGameRenderer::DrawLine(fX, fPrevY, fX + m_fCandleWidth, fY, 0xFFFFFFFF);
            fX += m_fCandleWidth;
            fPrevY = fY;
//            if (0 == uSkipInterval) {
//                const DOUBLE dVal = m_pdValues[uIndex];
//                if (0.0f != dVal) {
//                    if (69.0f <= dVal) {
//                        CGameRenderer::DrawRectNoTex(fX + m_fCandleWidth * 0.5f, fLowestY, m_fCandleWidth * 3, fHeight, RGBA(0x7F, 0, 0, 0x7F));
//                        uSkipInterval = 2;
//                    }
//                    else if (31.0f >= dVal) {
//                        CGameRenderer::DrawRectNoTex(fX + m_fCandleWidth * 0.5f, fLowestY, m_fCandleWidth * 2, fHeight, RGBA(0, 0x7F, 0, 0x7F));
//                        uSkipInterval = 1;
//                    }
//                }
//            }
//            else {
//                --uSkipInterval;
//            }
        }
    }
    CBaseChart::OnRender(fOffsetX, fOffsetY);
}
