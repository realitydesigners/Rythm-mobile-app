#include "stdafx.h"
#include "GameRenderer.h"
#include "PlayerData.h"
#include "TradeChart.h"
#include "UITextLabel.h"
#include "GameWidgetDefs.h"

VOID CTradeChart::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    const SHLVector2D& sWorldPos = GetWorldPosition();
    const SHLVector2D& sWorldSize = GetWorldSize();
//    CGameRenderer::DrawRectNoTex(sWorldPos.x, sWorldPos.y, sWorldSize.x, sWorldSize.y, RGBA(0x7F, 0, 0, 0x7F));
    const FLOAT fStartX = sWorldPos.x;
    const FLOAT fStartY = sWorldPos.y;
    const FLOAT fEndX = sWorldPos.x + sWorldSize.x - 40.0f;
    const FLOAT fEndY = sWorldPos.y + sWorldSize.y;
    for (auto pcLbl : m_cTimeLabels) {
        if (NULL != pcLbl) {
            pcLbl->OnRender(0.0f, 0.0f);
            const SHLVector2D& sWorldPos = pcLbl->GetLocalPosition();
            CGameRenderer::DrawLine(sWorldPos.x,    fStartY,    sWorldPos.x,    fEndY,  GRID_COLOR);
        }
    }
    m_cPercentage.OnRender(0.0f, 0.0f);
    for (auto pcLbl : m_cPriceLabels) {
        if (NULL != pcLbl) {
            const SHLVector2D& sWorldPos = pcLbl->GetLocalPosition();
            CGameRenderer::DrawLine(fStartX, sWorldPos.y, fEndX, sWorldPos.y, GRID_COLOR);
            pcLbl->OnRender(0.0f, 0.0f);
        }
    }
    // render candles
    RenderCandles();
    // render volume
    RenderVolume();
    
    // render ma lines
    RenderMALines();
    
    m_cBollingerBand.Render(fOffsetX, fOffsetY);
    
    RenderPrediction();
    
    // touch grid
    RenderTouchGrid();
}

#define CANDLE_BORDER_COLOR RGBA(0x7F,0x7F,0x7F,0x1F)
VOID CTradeChart::RenderCandles(VOID) const
{
    if (NULL == m_psDisplayedCandles) {
        return;
    }
    if (!CPlayerData::IsDisplayCandles()) {
        // just render lines
        FLOAT fPrevX = m_psDisplayedCandles[0].afLine[2];
        FLOAT fPrevY = m_psDisplayedCandles[0].afBox[1];
        UINT uIndex = 1;
        for ( ; m_uDisplayedCandleNum > uIndex; ++uIndex) {
            const SDisplayCandle& sDisplay = m_psDisplayedCandles[uIndex];
            CGameRenderer::DrawLine(fPrevX, fPrevY, sDisplay.afLine[2], sDisplay.afBox[1], 0xFFFFFFFF);
            fPrevX = sDisplay.afLine[2];
            fPrevY = sDisplay.afBox[1];
        }
        return;
    }
    UINT uIndex = 0;
    for ( ; m_uDisplayedCandleNum > uIndex; ++uIndex) {
        const SDisplayCandle& sDisplay = m_psDisplayedCandles[uIndex];
        CGameRenderer::DrawLine(sDisplay.afLine[2], sDisplay.afLine[0], sDisplay.afLine[2], sDisplay.afLine[0] + sDisplay.afLine[1], 0xFFFFFFFF);
        // render white line for highest/lowest
        CGameRenderer::DrawRectNoTex(sDisplay.afBox[0], sDisplay.afBox[1], sDisplay.afBox[2], sDisplay.afBox[3], sDisplay.uColor);
        const FLOAT fX2 = sDisplay.afBox[0] + sDisplay.afBox[2];
        const FLOAT fY2 = sDisplay.afBox[1] + sDisplay.afBox[3];
        
        CGameRenderer::DrawLine(sDisplay.afBox[0], sDisplay.afBox[1], sDisplay.afBox[0], fY2, CANDLE_BORDER_COLOR);
        CGameRenderer::DrawLine(fX2, sDisplay.afBox[1], fX2, fY2, CANDLE_BORDER_COLOR);
        
        CGameRenderer::DrawLine(sDisplay.afBox[0], sDisplay.afBox[1], fX2, sDisplay.afBox[1], CANDLE_BORDER_COLOR);
        CGameRenderer::DrawLine(sDisplay.afBox[0], fY2, fX2, fY2, CANDLE_BORDER_COLOR);
    }
}

VOID CTradeChart::RenderVolume(VOID) const
{
    if (NULL == m_psDisplayedCandles || NULL == m_pfVolumeHeight) {
        return;
    }
    const FLOAT fBottomY = GetWorldSize().y + GetWorldPosition().y;
    UINT uIndex = 0;
    for ( ; m_uDisplayedCandleNum > uIndex; ++uIndex) {
        const SDisplayCandle& sDisplay = m_psDisplayedCandles[uIndex];
        CGameRenderer::DrawRectNoTex(sDisplay.afBox[0], fBottomY, sDisplay.afBox[2], -m_pfVolumeHeight[uIndex], RGBA(0x3F,0x3F,0x3F,0x3F));
    }

}
VOID CTradeChart::RenderMALines(VOID) const
{
    if (!CPlayerData::IsDisplayMovingAverage()) {
        return;
    }
    static_assert(3 == MOVING_AVERAGE_NUM, "duh");
    const UINT auMAColor[3] = {
        MA_LINE_1_COLOR,
        MA_LINE_2_COLOR,
        MA_LINE_3_COLOR
    };
    UINT uIndex = 0;
    for ( ; MOVING_AVERAGE_NUM > uIndex; ++uIndex) {
        const SVisibleMovingAverage& sMA = m_asMA[uIndex];
        if (EMA_NONE == sMA.sMA.eType) {
            continue;
        }
        if (NULL == sMA.pfYValues) { // not yet initialized yet
            continue;
        }
        const UINT uColor = auMAColor[uIndex];
        FLOAT fPrevY = 0.0f;
        UINT uIndex2 = 0;
        for (; m_uDisplayedCandleNum > uIndex2; ++uIndex2) {
            const FLOAT fY = sMA.pfYValues[uIndex2];
            if (0.0f != fPrevY) {
                const FLOAT fX1 = sMA.fStartX + (uIndex2 - 1) * m_fCandleWidth;
                const FLOAT fX2 = fX1 + m_fCandleWidth;
                CGameRenderer::DrawLine(fX1, fPrevY, fX2, fY, uColor);
            }
            fPrevY = fY;
        }
    }
}

VOID CTradeChart::RenderPrediction(VOID) const
{
    if (NULL == m_psDisplayedCandles) {
        return;
    }
    const UINT uStartCandleIndex = (m_psDisplayedCandles[0].sData.uTimeSec - m_uTradeCandleStartTimeSec) / (m_uCandleDurationMins * 60);
    const UINT uEndCandleIndex = uStartCandleIndex + m_uDisplayedCandleNum;
    const SSignal* psStartSignal = NULL;
    const SHLVector2D& sWorldPos = GetWorldPosition();
    DOUBLE dVolumeBought = 0.0;
    DOUBLE dPriceAtPurchase = 0.0;
    FLOAT fX = 0.0;
    FLOAT fY = 0.0;
    for (auto& sSignal : m_cPredictions) {
        if (NULL == psStartSignal) {
            if (ESIG_BUY != sSignal.eType) {
                continue;
            }
            if (uStartCandleIndex > sSignal.uCandleIndex) {
                continue;
            }
            if (uEndCandleIndex <= sSignal.uCandleIndex){ // time to skip
                return;
            }
            psStartSignal = &sSignal;
            dVolumeBought = sSignal.dVolume;
            dPriceAtPurchase = sSignal.dPrice;
            ASSERT(sSignal.uCandleIndex >= uStartCandleIndex);
            const SDisplayCandle& sStart = m_psDisplayedCandles[sSignal.uCandleIndex - uStartCandleIndex];
            fX = sStart.afBox[0];
            fY = sWorldPos.y + 10.0f;
            continue;
        }
        if (uEndCandleIndex <= sSignal.uCandleIndex){ // time to skip
            return;
        }
        ASSERT(sSignal.uCandleIndex >= uStartCandleIndex);
        const SDisplayCandle& sEnd = m_psDisplayedCandles[sSignal.uCandleIndex - uStartCandleIndex];
        const FLOAT fX2 = sEnd.afBox[0] + sEnd.afBox[2];
        const UINT uColor = (dPriceAtPurchase >= sSignal.dPrice) ? RGBA(0x7F, 0, 0, 0x7F) : RGBA(0, 0x7F, 0, 0x7F);
        CGameRenderer::DrawRectNoTex(fX, fY, fX2 - fX, 10.0f, uColor);
        fX = fX2 + 1.0f;
        fY += 12.0f;
        
        ASSERT(ESIG_BUY != sSignal.eType);
        dVolumeBought -= sSignal.dVolume;
        if (0.0 == dVolumeBought) {
            psStartSignal = NULL;
        }
    }
}
