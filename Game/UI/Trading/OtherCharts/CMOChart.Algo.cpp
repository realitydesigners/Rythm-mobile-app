#include "stdafx.h"
#include "CMOChart.h"
#include "TradeChart.h"
#include "PlayerData.h"

static DOUBLE GetCMOValue(const CTradeCandleMap& cCandles, UINT uNowCandleTimeSec, UINT uCandleDurationSec, UINT uInterval)
{
    UINT uCandleTimeSec = uNowCandleTimeSec - (uInterval-1) * uCandleDurationSec;
    UINT uIndex = 0;
    DOUBLE dLow = 0.0;
    DOUBLE dHigh = 0.0;
    for ( ; uInterval > uIndex; ++uIndex, uCandleTimeSec += uCandleDurationSec) {
        CTradeCandleMap::const_iterator itNow = cCandles.find(uCandleTimeSec);
        CTradeCandleMap::const_iterator itPrev = cCandles.find(uCandleTimeSec - uCandleDurationSec);
        if (cCandles.end() == itNow) {
            continue;
        }
        const STradeCandle& sCandle = itNow->second;
        if (cCandles.end() == itPrev) {
            if (sCandle.dOpen < sCandle.dClose) { // higher close
                dHigh += (sCandle.dClose - sCandle.dOpen);
            }
            else if (sCandle.dOpen > sCandle.dClose) { // lower close
                dLow += (sCandle.dOpen - sCandle.dClose);
            }
        }
        else {
            const STradeCandle& sCandle2 = itPrev->second;
            if (sCandle2.dClose < sCandle.dClose) { // higher close
                dHigh += (sCandle.dClose - sCandle2.dClose);
            }
            else if (sCandle2.dClose > sCandle.dClose) { // lower close
                dLow += (sCandle2.dClose - sCandle.dClose);
            }
        }
    }
    const DOUBLE dSum = dLow + dHigh;
    if (0.0f == dSum) {
        return 0.0f;
    }
    return (dHigh - dLow) * 100.0 / dSum;
}

VOID CCMOChart::SetupChart(VOID)
{
    SAFE_DELETE_ARRAY(m_pfPosY);
    SAFE_DELETE_ARRAY(m_pdValues);
    m_uIntervalNum = 0;
    const UINT uVisibleCandleNum = m_cMainChart.GetDisplayCandleNum();
    const SDisplayCandle* psCandles = m_cMainChart.GetDisplayCandles();
    if (NULL == psCandles) {
        return;
    }
    m_pfPosY = new FLOAT[uVisibleCandleNum];
    m_pdValues = new DOUBLE[uVisibleCandleNum];
    if (NULL == m_pfPosY || NULL == m_pdValues) {
        ASSERT(FALSE);
        return;
    }
    m_fCandleWidth = psCandles[0].afBox[2] + 2.0f;
    m_fStartX = psCandles[0].afBox[0] - 1.0f + m_fCandleWidth * 0.5f;
    m_uIntervalNum = uVisibleCandleNum;
    UINT uInterval = CPlayerData::GetCMOInterval();
    if (1 > uInterval) {
        ASSERT(FALSE);
        uInterval = 1;
    }
    const UINT uCandleDurationSec = psCandles[1].sData.uTimeSec - psCandles[0].sData.uTimeSec;
    const SHLVector2D& sWorldPos = GetWorldPosition();
    const SHLVector2D& sWorldSize = GetWorldSize();
    const FLOAT fCenterY = sWorldPos.y + sWorldSize.y * 0.5f;
    const FLOAT fPixelsPerValue = (sWorldSize.y * 0.5f - CHART_PADDING) / 100.0f;
    const UINT uFirstVisibleCandleTimeSec = psCandles[0].sData.uTimeSec;
    const CTradeCandleMap& cCandles = m_cMainChart.GetTradeCandleMap();
    UINT uIndex = 0;
    for ( ; uVisibleCandleNum > uIndex; ++uIndex) {
        const UINT uNowCandleTimeSec = uFirstVisibleCandleTimeSec + uIndex * uCandleDurationSec;
        m_pdValues[uIndex] = GetCMOValue(cCandles, uNowCandleTimeSec, uCandleDurationSec, uInterval);
        m_pfPosY[uIndex] = (FLOAT)(fCenterY - m_pdValues[uIndex] * fPixelsPerValue);
    }
    AdjustLabels();
}
