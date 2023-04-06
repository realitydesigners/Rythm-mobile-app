#include "stdafx.h"
#include "ConfirmationWindow.h"
#include "PlayerData.h"
#include "SaveData.h"
#include "TradeChart.h"

VOID CTradeChart::ToggleCandle(VOID)
{
    CPlayerData::SetDisplayCandles(!CPlayerData::IsDisplayCandles());
    CSaveData::SaveData(FALSE);
}
VOID CTradeChart::ToggleMAChart(VOID)
{
    CPlayerData::SetDisplayMovingAverage(!CPlayerData::IsDisplayMovingAverage());
    if (CPlayerData::IsDisplayMovingAverage()) {
        GenerateMA();
    }
    CSaveData::SaveData(FALSE);
}

VOID CTradeChart::GenerateMA(UINT uIndex)
{
    ASSERT(3 > uIndex);
    if (NULL == m_psDisplayedCandles) {
        ASSERT(FALSE);
        return;
    }
    if (!CPlayerData::IsDisplayMovingAverage()) {
        return;
    }
    SVisibleMovingAverage& sMA = m_asMA[uIndex];
    if (EMA_NONE == sMA.sMA.eType) {
        return;
    }
    if (0 == m_uDisplayedCandleNum) {
        return;
    }
    // find first candle time
    const UINT uTimeSec = CCryptoUtil::FindFirstCandleTimeSec(m_cCurrentSetofCandles);
    if (!CMAUtil::GenerateMA(sMA.sMA, m_cCurrentSetofCandles, uTimeSec, m_uCandleDurationMins * 60, m_uDisplayedCandleNum, m_psDisplayedCandles[0].sData.uTimeSec)) {
        return;
    }
    if (NULL == sMA.sMA.pdPrice) {
        ASSERT(FALSE);
        return;
    }
    if (NULL == sMA.pfYValues) {
        ASSERT(FALSE);
        return;
    }
    ASSERT(m_uMaxLength >= m_uDisplayedCandleNum);
    memset(sMA.pfYValues, 0, sizeof(FLOAT) * m_uDisplayedCandleNum);
    sMA.fStartX = m_psDisplayedCandles[0].afLine[2];
    uIndex = 0;
    for ( ; m_uDisplayedCandleNum > uIndex; ++uIndex) {
        sMA.pfYValues[uIndex] = m_fChartHighestPriceY + (m_dChartHighestPrice - sMA.sMA.pdPrice[uIndex]) * m_dPixelPerPrice;
    }
}
VOID CTradeChart::GenerateMA(VOID)
{
    UINT uIndex = 0;
    for ( ; MOVING_AVERAGE_NUM > uIndex; ++uIndex) {
        GenerateMA(uIndex);
    }
}

VOID CTradeChart::GenerateVolume(VOID)
{
    if (NULL == m_psDisplayedCandles) {
        ASSERT(FALSE);
        return;
    }
    if (NULL == m_pfVolumeHeight) {
        ASSERT(FALSE);
        return;
    }
    ASSERT(m_uMaxLength >= m_uDisplayedCandleNum);
    memset(m_pfVolumeHeight, 0, sizeof(FLOAT) * m_uDisplayedCandleNum);
    DOUBLE dHigh = m_psDisplayedCandles[0].sData.dVolume;
    UINT uIndex = 0;
    for ( ; m_uDisplayedCandleNum > uIndex; ++uIndex) {
        const DOUBLE dVol = m_psDisplayedCandles[uIndex].sData.dVolume;
        if (dHigh < dVol) {
            dHigh = dVol;
        }
    }
    const SHLVector2D& sSize = GetWorldSize();
    const DOUBLE dPixelPerPrice = (sSize.y * 0.5f)/ (dHigh);
    uIndex = 0;
    for ( ; m_uDisplayedCandleNum > uIndex; ++uIndex) {
        const DOUBLE dVol = m_psDisplayedCandles[uIndex].sData.dVolume;
        m_pfVolumeHeight[uIndex] = dVol * dPixelPerPrice;
    }
}

