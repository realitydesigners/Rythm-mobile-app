#include "stdafx.h"
#include "CMain.h"
#include "EventManager.h"
#include "GameRenderer.h"
#include "GameWidgetDefs.h"
#include "PlayerData.h"
#include "TradeChart.h"
#include "HLTime.h"

#if defined(WINDOWS_DEV)
#define STRING_CAT(dst, src, len) strcat_s(dst, len, src)
#else // #if defined(WINDOWS_DEV)
#define STRING_CAT(dst, src, len) strlcat(dst, src, len)
#endif // #if defined(WINDOWS_DEV)

BOOLEAN CTradeChart::OnTouchBegin(FLOAT fX, FLOAT fY)
{
    const SHLVector2D sPos = { fX, fY };
    if (HitTest(sPos)) {
        GenerateToolTip(fX, fY);
        return TRUE;
    }
    return FALSE;
}
BOOLEAN CTradeChart::OnTouchMove(FLOAT fX, FLOAT fY)
{
    const SHLVector2D sPos = { fX, fY };
    if (HitTest(sPos)) {
        GenerateToolTip(fX, fY);
        return TRUE;
    }
    return FALSE;
}
BOOLEAN CTradeChart::OnTouchEnd(FLOAT fX, FLOAT fY)
{
    return OnTouchMove(fX, fY);
}

VOID CTradeChart::RenderTouchGrid(VOID) const
{
    const SHLVector2D& sPos = GetWorldPosition();
    const SHLVector2D& sSize = GetWorldSize();
    if (0.0f != m_fTooltipX) {
        const FLOAT fY1 = sPos.y;
        const FLOAT fY2 = fY1  + sSize.y - 15.0f;
        CGameRenderer::DrawLine(m_fTooltipX, fY1, m_fTooltipX, fY2, 0xFFFFFFFF);
    }
    if (0.0f != m_fTooltipY) {
        const FLOAT fX1 = sPos.x;
        const FLOAT fX2 = fX1 + sSize.x - 40.0f;
        CGameRenderer::DrawLine(fX1, m_fTooltipY, fX2, m_fTooltipY, 0xFFFFFFFF);
    }
    
    if (m_bDrawToolTip) {
        FLOAT fX = m_fTooltipX + FINGER_FATNESS;
        const SHLVector2D& sTooltipSize = m_cToolTip.GetWorldSize();
        if ((fX + sTooltipSize.x) > sSize.x) {
            fX = m_fTooltipX - FINGER_FATNESS - sTooltipSize.x;
        }
        m_cToolTip.OnRender(fX, 0.0);
    }
}

VOID CTradeChart::GenerateToolTip(FLOAT fX, FLOAT fY)
{
    if (NULL == m_psDisplayedCandles || 0 == m_uDisplayedCandleNum) {
        return;
    }
    // find displayed candle
    UINT uIndex = 0;
    for ( ; m_uDisplayedCandleNum > uIndex; ++uIndex) {
        const SDisplayCandle& sCandle = m_psDisplayedCandles[uIndex];
        const FLOAT fDiffX = fX - sCandle.afBox[0] - 1.0f;
        if (0.0f <= fDiffX && (sCandle.afBox[2] + 2.0f) >= fDiffX) {
            break;
        }
    }
    if (m_uDisplayedCandleNum == uIndex) { // cannot find. clear
        ClearToolTip();
        m_fTooltipX = fX;
        m_fTooltipY = fY;
        return;
    }
    SetToolTip(uIndex);
}
VOID CTradeChart::SetToolTip(UINT uDisplayCandleIndex)
{
    if (NULL == m_psDisplayedCandles || 0 == m_uDisplayedCandleNum) {
        return;
    }
    uDisplayCandleIndex %= m_uDisplayedCandleNum;
    m_uSelectedDisplayedCandleIndex = uDisplayCandleIndex;
    const SDisplayCandle& sCandle = m_psDisplayedCandles[uDisplayCandleIndex];
    m_bDrawToolTip = TRUE;
    m_fTooltipX = sCandle.afLine[2];
    m_fTooltipY = sCandle.afBox[1];
    
    const UINT uLocalTimeSecs = CHLTime::GetLocalTimeSecs(sCandle.sData.uTimeSec);
    UINT uMth = 0;
    UINT uDay = 0;
    CHLTime::GetMonthAndDay(uLocalTimeSecs, uMth, uDay);
    UINT uHR = 0;
    UINT uMin = 0;
    UINT uSec = 0;
    CHLTime::GetHHMMSS(uLocalTimeSecs, uHR, uMin, uSec);

    const CHAR* aszMths[12] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    CHAR szBuffer[2048];
    CHAR szOpen[64];
    CHAR szClose[64];
    CHAR szHigh[64];
    CHAR szLow[64];
    CHAR szVWAP[64];
    CHAR szVol[64];
    GenerateString(sCandle.sData.dOpen, szOpen, 64);
    GenerateString(sCandle.sData.dClose, szClose, 64);
    GenerateString(sCandle.sData.dHigh, szHigh, 64);
    GenerateString(sCandle.sData.dLow, szLow, 64);
    GenerateString(sCandle.sData.dVWAP, szVWAP, 64);
    GenerateString(sCandle.sData.dVolume, szVol, 64);
    const UINT uAllCandleIndex = (m_psDisplayedCandles[uDisplayCandleIndex].sData.uTimeSec - GetTradeCandleStartTimeSec()) / GetTradeCandleDurationSec();
    snprintf(szBuffer, 2048,
             "%d %s %d:%02d [%d]\nO: %s\nC: %s\nV: %s",
             uDay, aszMths[uMth], uHR, uMin, uAllCandleIndex, szOpen, szClose, szVol);
    CHAR szBuffer2[1024];
    if (CPlayerData::IsDisplayMovingAverage()) {
        UINT uIndex = 0;
        for ( ; MOVING_AVERAGE_NUM > uIndex; ++uIndex) {
            const SVisibleMovingAverage& sMA = m_asMA[uIndex];
            if (EMA_NONE == sMA.sMA.eType) {
                continue;
            }
            if (NULL == sMA.sMA.pdPrice) { // not yet initialized yet
                continue;
            }
            if (EMA_EMA == sMA.sMA.eType) {
                snprintf(szBuffer2, 1024, "EMA(%d): %f", sMA.sMA.uIntervalNum, sMA.sMA.pdPrice[uDisplayCandleIndex]);
            }
            else {
                snprintf(szBuffer2, 1024, "SMA(%d): %f", sMA.sMA.uIntervalNum, sMA.sMA.pdPrice[uDisplayCandleIndex]);
            }
            STRING_CAT(szBuffer, "\n", 2048);
            STRING_CAT(szBuffer, szBuffer2, 2048);
        }
    }
    
    if (CPlayerData::IsDisplayBollingerBands()) {
        m_cBollingerBand.GetToolTipString(szBuffer2, 1024, uDisplayCandleIndex);
        STRING_CAT(szBuffer, "\n", 2048);
        STRING_CAT(szBuffer, szBuffer2, 2048);
    }
    m_cToolTip.Initialize(szBuffer);
    
    FLOAT fY = m_fTooltipY - FINGER_FATNESS - m_cToolTip.GetLocalSize().y;
    if (0.0f > fY) {
        fY = m_fTooltipY + FINGER_FATNESS;
    }
    m_cToolTip.SetLocalPosition(0.0f, fY);
    m_cToolTip.OnUpdate(0.0f);
    CEventManager::PostGlobalEvent(EGLOBAL_TRADE_MAIN_WINDOW, CEvent(EGLOBAL_EVT_TRADE_MAIN_WINDOW_DISPLAY_TOOLTIP, TRUE, uDisplayCandleIndex));
}

VOID CTradeChart::ClearToolTip(VOID)
{
    m_bDrawToolTip = FALSE;
    m_uSelectedDisplayedCandleIndex = 0;
    m_fTooltipX = m_fTooltipY = 0.0f;
    CEventManager::PostGlobalEvent(EGLOBAL_TRADE_MAIN_WINDOW, CEvent(EGLOBAL_EVT_TRADE_MAIN_WINDOW_DISPLAY_TOOLTIP, FALSE, 0));
}

VOID CTradeChart::AdjustToolTip(INT nAdj)
{
    if (!m_bDrawToolTip) {
        return;
    }
    if (NULL == m_psDisplayedCandles || 0 == m_uDisplayedCandleNum) {
        return;
    }
    m_uSelectedDisplayedCandleIndex = (m_uSelectedDisplayedCandleIndex + m_uDisplayedCandleNum + nAdj) % m_uDisplayedCandleNum;
    SetToolTip(m_uSelectedDisplayedCandleIndex);
}
