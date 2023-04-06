#include "stdafx.h"
#include "BollingerBands.h"
#include "GameRenderer.h"
#include "PlayerData.h"
#include "TradeChart.h"
#include <math.h>

CBollingerBands::CBollingerBands(const CTradeChart& cChart) :
m_cChart(cChart),
m_pfPolygonValues(NULL),
m_uCandleNum(0),
m_uMaxNum(0)
{
    memset(&m_sBands, 0, sizeof(m_sBands));
}
CBollingerBands::~CBollingerBands()
{
    Release();
}

VOID CBollingerBands::Release(VOID)
{
    SAFE_DELETE_ARRAY(m_sBands.pdDeviation);
    SAFE_DELETE_ARRAY(m_sBands.sMA.pdPrice);
    SAFE_DELETE_ARRAY(m_pfPolygonValues);
    m_uCandleNum = 0;
    m_uMaxNum = 0;
}

VOID CBollingerBands::AllocateArrays(UINT uSize)
{
    if (m_uMaxNum >= uSize) {
        return;
    }
    Release();
    m_sBands.pdDeviation = new DOUBLE[uSize];
    m_sBands.sMA.pdPrice = new DOUBLE[uSize];
    m_pfPolygonValues = new FLOAT[uSize * 4];
    ASSERT(NULL != m_sBands.pdDeviation);
    ASSERT(NULL != m_sBands.sMA.pdPrice);
    ASSERT(NULL != m_pfPolygonValues);
    m_uMaxNum = uSize;

}
VOID CBollingerBands::SetupChart(FLOAT fHighestPriceY, DOUBLE dHighestPrice, DOUBLE dPixelPerPrice)
{
    m_uCandleNum = 0;
    if (!CPlayerData::IsDisplayBollingerBands()) {
        return;
    }
    const UINT uVisibleCandleNum = m_cChart.GetDisplayCandleNum();
    const SDisplayCandle* psCandles = m_cChart.GetDisplayCandles();
    if (NULL == psCandles|| 0 == uVisibleCandleNum) {
        return;
    }
    const CTradeCandleMap& cCandles = m_cChart.GetTradeCandleMap();
    AllocateArrays((UINT)cCandles.size());
    if (NULL == m_pfPolygonValues) {
        ASSERT(FALSE);
        return;
    }
    const SBollingerBandsParam& sParams = CPlayerData::GetBollingerBands();
    m_sBands.sMA.eSource = EMA_VALUE_CLOSE;
    m_sBands.sMA.eType = EMA_SMA;
    m_sBands.sMA.uIntervalNum = sParams.uInterval;
    
    // find start time
    const UINT uStartSec = CCryptoUtil::FindFirstCandleTimeSec(cCandles);
    const UINT uCandleDurationSec = psCandles[1].sData.uTimeSec - psCandles[0].sData.uTimeSec;
    CMAUtil::GenerateBollingerBands(m_sBands, cCandles, uStartSec, uCandleDurationSec, uVisibleCandleNum, psCandles[0].sData.uTimeSec);
    
    const FLOAT fCandleWidth = psCandles[1].afBox[0] - psCandles[0].afBox[0];
    FLOAT fX = psCandles[0].afLine[2] + fCandleWidth * 0.5f;
    UINT uIndex = 0;
    for ( ; uVisibleCandleNum > uIndex; ++uIndex, fX += fCandleWidth) {
        const FLOAT fY = (FLOAT)(fHighestPriceY + (dHighestPrice - m_sBands.sMA.pdPrice[uIndex]) * dPixelPerPrice);
        const FLOAT fDY = (FLOAT)(sParams.fDeviation * m_sBands.pdDeviation[uIndex] * dPixelPerPrice);
        m_pfPolygonValues[uIndex*4] = fX;
        m_pfPolygonValues[uIndex*4 + 1] = fY - fDY;
        m_pfPolygonValues[uIndex*4 + 2] = fX;
        m_pfPolygonValues[uIndex*4 + 3] = fY + fDY;
    }
    m_uCandleNum = uVisibleCandleNum;
}

VOID CBollingerBands::Render(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    if (NULL == m_pfPolygonValues) {
        return;
    }
    CGameRenderer::DrawFilledPolygon(m_pfPolygonValues, m_uCandleNum * 2, RGBA(0x20, 0x20, 0x7F, 0x2F));
}

VOID CBollingerBands::GetToolTipString(CHAR* szBuffer, UINT uBufferLen, UINT uSelectedIndex)
{
    if (NULL == m_sBands.pdDeviation) {
        szBuffer[0] = 0; // zero terminate
        return;
    }
    const UINT uVisibleCandleNum = m_cChart.GetDisplayCandleNum();
    const DOUBLE dValue = abs(m_sBands.pdDeviation[uSelectedIndex%uVisibleCandleNum]);
    snprintf(szBuffer, uBufferLen, "BB(%d) D:%f", CPlayerData::GetBollingerBands().uInterval, dValue);
}
