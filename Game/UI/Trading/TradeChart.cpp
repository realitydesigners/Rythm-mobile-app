#include "stdafx.h"
#include "ConfirmationWindow.h"
#include "Event.h"
#include "EventID.h"
#include "GameWidgetDefs.h"
#include "HLTime.h"
#include "PlayerData.h"
#include "TradeChart.h"
#include "UITextLabel.h"

CTradeChart::CTradeChart() :
CUIWidget(EGameWidget_TradeChart, EGLOBAL_TRADE_MAIN_CHART),
m_uDisplayedCandleNum(0),
m_uCandleDurationMins(0),
m_uTradeCandleStartTimeSec(0),
m_uTradeCandleEndTimeSec(0),
m_bRenderCandles(TRUE),
m_psDisplayedCandles(NULL),
m_pfVolumeHeight(NULL),
m_uMaxLength(0),
m_uColumnWidthSecs(0),
m_uColumnNum(0),
m_bShowDate(FALSE),
m_uStartTimeSecs(0),
m_uEndTimeSecs(0),
m_bTimeLabelDirty(FALSE),
m_fCandleWidth(0.0f),
INITIALIZE_TEXT_LABEL(m_cPercentage),
m_bPriceLabelDirty(FALSE),
m_dChartHighestPrice(0.0),
m_dChartTotalPrice(0.0),
m_fChartHighestPriceY(0.0f),
m_dPixelPerPrice(0.0),
m_bDrawToolTip(FALSE),
m_uSelectedDisplayedCandleIndex(0),
m_fTooltipX(0.0f),
m_fTooltipY(0.0f),
m_bTimeAdjusted(FALSE),
m_cBollingerBand(*this)
{
    memset(m_asMA, 0, sizeof(m_asMA));
    
    UINT uIndex = 0;
    for ( ; 3 > uIndex; ++uIndex) {
        const SMovingAverageData& sMa = CPlayerData::GetMovingAverageData(uIndex);
        m_asMA[uIndex].sMA.eType = (EMA_TYPE)sMa.byType;
        m_asMA[uIndex].sMA.eSource = (EMA_VALUE_SOURCE)sMa.bySource;
        m_asMA[uIndex].sMA.uIntervalNum = sMa.ushInterval;
    }
    m_uEndTimeSecs = CHLTime::GetTimeSecs(TRUE);
    
    m_cPercentage.SetFont(EGAMEFONT_SIZE_20);
    m_cPercentage.SetAnchor(0.0f, 0.5f);
    m_cPercentage.SetLocalSize(80.0f, 40.0f);
}
    
CTradeChart::~CTradeChart()
{
    
}

    
VOID CTradeChart::Release(VOID)
{
    m_cPercentage.Release();
    m_cPercentage.RemoveFromParent();
    
    m_cToolTip.Release();
    
    RemoveFromParent();
    m_cCurrentSetofCandles.clear();
        
    m_cBollingerBand.Release();
    ClearTimeLabels();
    ClearChart();
    ClearArrays();
}

VOID CTradeChart::ClearArrays(VOID)
{
    SAFE_DELETE_ARRAY(m_psDisplayedCandles);
    SAFE_DELETE_ARRAY(m_pfVolumeHeight);
    UINT uIndex = 0;
    for ( ; MOVING_AVERAGE_NUM > uIndex; ++uIndex) {
        SAFE_DELETE_ARRAY(m_asMA[uIndex].sMA.pdPrice);
        SAFE_DELETE_ARRAY(m_asMA[uIndex].pfYValues);
    }
    m_uMaxLength = 0;
}

VOID CTradeChart::AllocateDataArrays(UINT uNum)
{
    if (m_uMaxLength >= uNum) {
        return;
    }
    ClearArrays();
    m_psDisplayedCandles = new SDisplayCandle[uNum];
    m_pfVolumeHeight = new FLOAT[uNum];
    ASSERT(NULL != m_psDisplayedCandles);
    ASSERT(NULL != m_pfVolumeHeight);
    UINT uIndex = 0;
    for ( ; MOVING_AVERAGE_NUM > uIndex; ++uIndex) {
        m_asMA[uIndex].sMA.pdPrice = new DOUBLE[uNum];
        m_asMA[uIndex].pfYValues = new FLOAT[uNum];
        ASSERT(NULL != m_asMA[uIndex].sMA.pdPrice);
        ASSERT(NULL != m_asMA[uIndex].pfYValues);
    }
    m_uMaxLength = uNum;
}
    
VOID CTradeChart::ClearTimeLabels(VOID)
{
    for (auto& pcLabels : m_cTimeLabels) {
        if (NULL != pcLabels) {
            SAFE_RELEASE_DELETE(pcLabels);
        }
    }
    m_cTimeLabels.clear();
}

VOID CTradeChart::ClearPriceLabels(VOID)
{
    for (auto& pcLabels : m_cPriceLabels) {
        if (NULL != pcLabels) {
            SAFE_RELEASE_DELETE(pcLabels);
        }
    }
    m_cPriceLabels.clear();
}

VOID CTradeChart::ClearChart(VOID)
{
    m_cAveragePriceLine.clear();
    m_uDisplayedCandleNum = 0;
}

VOID CTradeChart::OnUpdate(FLOAT fLapsed)
{
    CUIWidget::OnUpdate(fLapsed);
    if (m_bTimeLabelDirty) {
        GenerateTimeLabels();
    }
    if (m_bPriceLabelDirty) {
        GeneratePriceLabels();
        m_bPriceLabelDirty = FALSE;
    }
}


VOID CTradeChart::OnWorldCoordinatesUpdated(VOID)
{
    
}

VOID CTradeChart::SetGridInterval(UINT uSecs, UINT uColumnNum, BOOLEAN bShowDate)
{
    m_uColumnWidthSecs = uSecs;
    m_uColumnNum = uColumnNum;
    m_bShowDate = bShowDate;
    m_uStartTimeSecs = m_uEndTimeSecs - (m_uColumnNum * m_uColumnWidthSecs); // update start time since time interval changed
    m_bTimeLabelDirty = TRUE;
}

VOID CTradeChart::GenerateString(DOUBLE dValue, CHAR* szBuffer, UINT uBufferLen)
{
    CHAR szFormat[32] = { 0 };
    if (100000 <= dValue) {
        strcpy(szFormat, "%.0f");
    }
    else if (10000 <= dValue) {
        strcpy(szFormat, "%.1f");
    }
    else if (1000 <= dValue) {
        strcpy(szFormat, "%.2f");
    }
    else if (100 <= dValue) {
        strcpy(szFormat, "%.3f");
    }
    else if (10 <= dValue) {
        strcpy(szFormat, "%.4f");
    }
    else if (0.0001 <= dValue){
        strcpy(szFormat, "%.5f");
    }
    else{
        strcpy(szFormat, "%.9f");
    }
    snprintf(szBuffer, uBufferLen, szFormat, dValue);
}
#define PRICE_SEGMENT_NUM   (5)
VOID CTradeChart::GeneratePriceLabels(VOID)
{
    ClearPriceLabels();
    const SHLVector2D& sWorldPos = GetWorldPosition();
    const SHLVector2D& sLocalSize = GetLocalSize();
    const FLOAT fX = sWorldPos.x + sLocalSize.x - CHART_OFFSET_FROM_RIGHT + 5.0f;
    const FLOAT fHeight = sLocalSize.y - (PRICE_PADDING * 2.0f);
    const FLOAT fSegmentHeight = fHeight / PRICE_SEGMENT_NUM;
    const DOUBLE dPricePerSegment = fSegmentHeight / m_dPixelPerPrice;
    CHAR szBuffer[64];
    UINT uIndex = 0;
    for ( ; PRICE_SEGMENT_NUM >= uIndex; ++uIndex) {
        const FLOAT fLabelY = m_fChartHighestPriceY + uIndex * fSegmentHeight;
        const DOUBLE dPrice = m_dChartHighestPrice - (uIndex * dPricePerSegment);
        GenerateString(dPrice, szBuffer, 64);
        CUITextLabel* pcLabel = NEW_TEXT_LABEL;
        if (NULL == pcLabel) {
            ASSERT(FALSE);
            return;
        }
        pcLabel->SetColor(RGBA(0x7F, 0x7F, 0x7F, 0xFF));
        pcLabel->SetFont(EGAMEFONT_SIZE_16);
        pcLabel->SetAnchor(0.0f, 0.5f);
        pcLabel->SetLocalSize(80.0f, 40.0f);
        pcLabel->SetLocalPosition(fX, fLabelY);
        pcLabel->SetString(szBuffer);
        pcLabel->OnUpdate(0.0f);
        m_cPriceLabels.push_back(pcLabel);
        if (0 == uIndex || PRICE_SEGMENT_NUM == uIndex) {
            pcLabel->SetVisibility(FALSE);
        }
    }
    m_cPercentage.SetLocalPosition(fX, sWorldPos.y + 15.0f);
    const DOUBLE dTotalChartPrice = fHeight / m_dPixelPerPrice;
    const FLOAT fPercentageChange = dTotalChartPrice * 100.0 / m_dChartHighestPrice;
    snprintf(szBuffer, 64, "%.2f%%", fPercentageChange);
    m_cPercentage.SetString(szBuffer);
    m_cPercentage.OnUpdate(0.0f);
}

VOID CTradeChart::GenerateTimeLabels(VOID)
{
    m_bTimeLabelDirty = FALSE;
    ClearTimeLabels(); // safety
    const SHLVector2D& sWorldPos = GetWorldPosition();
    const SHLVector2D& sWorldSize = GetWorldSize();
    const FLOAT fY = sWorldPos.y;
    const FLOAT fStartX = sWorldPos.x;
    const FLOAT fColumnWidth = (sWorldSize.x - CHART_OFFSET_FROM_RIGHT) / m_uColumnNum; // per column width
    const UINT uStartTimeSecs = CHLTime::GetLocalTimeSecs(m_uStartTimeSecs);
    
    UINT uMth = 0;
    UINT uDay = 0;
    UINT uHR = 0;
    UINT uMin = 0;
    UINT uSec = 0;
    CHAR szBuffer[64];
    const CHAR* aszMths[12] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    UINT uCurrentDisplayedMonth = 0xFF;
    UINT uIndex = 0;
    for ( ; m_uColumnNum >= uIndex; ++uIndex ) {
        const FLOAT fX = fStartX + uIndex * fColumnWidth;
        const UINT uColumnTimeSec = uStartTimeSecs + uIndex * m_uColumnWidthSecs;
        
        if (m_bShowDate) {
            CHLTime::GetMonthAndDay(uColumnTimeSec, uMth, uDay);
            if (uCurrentDisplayedMonth != uMth) {
                uCurrentDisplayedMonth = uMth;
                ASSERT(12 > uMth);
                snprintf(szBuffer, 64, "%s\n%02d", aszMths[uMth], uDay);
            }
            else {
                snprintf(szBuffer, 64, "%02d", uDay);
            }
        }
        else {
            CHLTime::GetHHMMSS(uColumnTimeSec, uHR, uMin, uSec);
            snprintf(szBuffer, 64, "%02d:%02d", uHR, uMin);
        }
        CUITextLabel* pcLabel = NEW_TEXT_LABEL;
        if (NULL == pcLabel) {
            ASSERT(FALSE);
            return;
        }
        pcLabel->SetColor(RGBA(0x7F, 0x7F, 0x7F, 0xFF));
        pcLabel->SetFont(EGAMEFONT_SIZE_16);
        pcLabel->SetAnchor(0.5f, 1.0f);
        pcLabel->AutoSize();
        pcLabel->SetLocalPosition(fX, fY);
        pcLabel->SetString(szBuffer);
        pcLabel->OnUpdate(0.0f);
        m_cTimeLabels.push_back(pcLabel);
    }
}

#define CANDLE_RED RGBA(0xFF,0,0,0xFF)
#define CANDLE_GREEN RGBA(0,0xFF,0,0xFF)
VOID CTradeChart::SetupChart(const CTradeCandleMap& cCandles, UINT uDurationMins)
{
    ClearChart();
    const UINT uCandleSize = (UINT)cCandles.size();
    if (0 == uCandleSize) {
        m_uDisplayedCandleNum = 0;
        return;
    }
    AllocateDataArrays(uCandleSize);
    
    m_cCurrentSetofCandles = cCandles;
    m_uCandleDurationMins = uDurationMins;
    m_uTradeCandleEndTimeSec = 0;
    m_uTradeCandleStartTimeSec = 0xFFFFFFFF;
    for (const auto& it : m_cCurrentSetofCandles) {
        const UINT uInterval = it.first;
        if (m_uTradeCandleEndTimeSec < uInterval) {
            m_uTradeCandleEndTimeSec = uInterval;
        }
        if (m_uTradeCandleStartTimeSec > uInterval) {
            m_uTradeCandleStartTimeSec = uInterval;
        }
    }
    if (!m_bTimeAdjusted) {
        m_uEndTimeSecs = m_uTradeCandleEndTimeSec + m_uCandleDurationMins * 60;
        m_uStartTimeSecs = m_uEndTimeSecs - (m_uColumnNum * m_uColumnWidthSecs);
        GenerateTimeLabels();
    }

    const UINT uChartStartTime = m_uStartTimeSecs - uDurationMins * 60;
    const UINT uChartEndTime = m_uEndTimeSecs;
    m_dChartHighestPrice = 0.0f;
    DOUBLE dChartLowestPrice = 1000000.0f;
    UINT uCandleNum = 0;
    for (const auto& it : m_cCurrentSetofCandles) {
        // find the start and end of the intervals
        const STradeCandle& sCandle = it.second;
        if (sCandle.uTimeSec < uChartStartTime) {   // if outside of the chart (on the left)
            continue;
        }
        if (sCandle.uTimeSec > uChartEndTime) {     // if outside of the chart (on the right)
            continue;
        }
        // within the chart time range
        if (m_dChartHighestPrice < sCandle.dHigh) {
            m_dChartHighestPrice = sCandle.dHigh;
        }
        if (dChartLowestPrice > sCandle.dLow) {
            dChartLowestPrice = sCandle.dLow;
        }
        ++uCandleNum;
    }
    m_dChartTotalPrice = m_dChartHighestPrice - dChartLowestPrice;
    const DOUBLE dPadPrice = m_dChartTotalPrice * 0.05f;
    m_dChartHighestPrice += dPadPrice;
    m_dChartTotalPrice += dPadPrice * 2.0;
    
    const SHLVector2D& sWorldPos = GetWorldPosition();
    const SHLVector2D& sWorldSize = GetWorldSize();

    m_fChartHighestPriceY = sWorldPos.y + PRICE_PADDING;
    m_dPixelPerPrice = (sWorldSize.y - PRICE_PADDING * 2.0) / m_dChartTotalPrice;
    m_bPriceLabelDirty = TRUE;
    
    if (NULL == m_psDisplayedCandles) {
        ASSERT(FALSE);
        return;
    }
    m_uDisplayedCandleNum = uCandleNum;
    ASSERT(m_uMaxLength >= uCandleNum);
    memset(m_psDisplayedCandles, 0, sizeof(SDisplayCandle) * uCandleNum); // clear memory for safety
    
    const FLOAT fEndX = sWorldPos.x + sWorldSize.x - CHART_OFFSET_FROM_RIGHT;
    const UINT uTotalSecondsInChart = m_uColumnNum * m_uColumnWidthSecs;
    const DOUBLE dPixelPerSec = (DOUBLE)(sWorldSize.x - CHART_OFFSET_FROM_RIGHT)/ uTotalSecondsInChart;
    
    m_fCandleWidth = m_uCandleDurationMins * 60 * dPixelPerSec;
    UINT uIndex = 0;
    for (auto itCandle : m_cCurrentSetofCandles) {
        const STradeCandle& sCandle = itCandle.second;
        if (sCandle.uTimeSec < uChartStartTime) {   // if outside of the chart (on the left)
            continue;
        }
        if (sCandle.uTimeSec > uChartEndTime) {     // if outside of the chart (on the right)
            continue;
        }
        if (uIndex >= m_uDisplayedCandleNum) {
            ASSERT(FALSE);
            return;
        }
        SDisplayCandle& sDisplay = m_psDisplayedCandles[uIndex];
        sDisplay.afBox[0] = fEndX - (((INT)m_uEndTimeSecs - (INT)sCandle.uTimeSec) * dPixelPerSec) + 1.0f;
        const DOUBLE dHigh = sCandle.dClose;
        const DOUBLE dLow = sCandle.dOpen;
        sDisplay.afBox[1] = m_fChartHighestPriceY + (m_dChartHighestPrice - dHigh) * m_dPixelPerPrice;
        sDisplay.afBox[2] = m_fCandleWidth - 2.0f;
        sDisplay.afBox[3] = (dHigh - dLow) * m_dPixelPerPrice;
        sDisplay.afLine[0] = m_fChartHighestPriceY + (m_dChartHighestPrice - sCandle.dHigh) * m_dPixelPerPrice;
        sDisplay.afLine[1] = (sCandle.dHigh - sCandle.dLow) * m_dPixelPerPrice;
        sDisplay.afLine[2] = sDisplay.afBox[0] + m_fCandleWidth * 0.5f;
        sDisplay.uColor = sCandle.dClose >= sCandle.dOpen ? CANDLE_GREEN : CANDLE_RED;
        
        if (0 == m_cAveragePriceLine.size()) {
            m_cAveragePriceLine.push_back(sDisplay.afLine[2]); // insert x position
        }
        if (0.0f == sCandle.dVWAP) {
            m_cAveragePriceLine.push_back(sDisplay.afBox[1]);
        }
        else {
            const FLOAT fY = m_fChartHighestPriceY + (m_dChartHighestPrice - sCandle.dVWAP) * m_dPixelPerPrice;
            m_cAveragePriceLine.push_back(fY);
        }
        sDisplay.sData = sCandle;
        ++uIndex;
    }
    GenerateMA();
    GenerateVolume();
    GenerateBB();
    if (m_bDrawToolTip) {
        SetToolTip(m_uSelectedDisplayedCandleIndex);
    }
}

VOID CTradeChart::ChangeTime(UINT uTimeSec, const CTradeCandleMap& cCandles, UINT uDurationMins)
{
    m_uEndTimeSecs = uTimeSec + uDurationMins * 60;
    const UINT uEndTimeSec = (m_uTradeCandleEndTimeSec + m_uCandleDurationMins * 60);
    if (m_uEndTimeSecs >= uEndTimeSec) {
        m_uEndTimeSecs = uEndTimeSec;
        m_bTimeAdjusted = FALSE;
    }
    else {
        m_bTimeAdjusted = TRUE;
    }
    m_uStartTimeSecs = m_uEndTimeSecs - (m_uColumnNum * m_uColumnWidthSecs);
    GenerateTimeLabels();
    ClearToolTip();
    SetupChart(cCandles, uDurationMins);
}

VOID CTradeChart::GenerateBB(VOID)
{
    m_cBollingerBand.SetupChart(m_fChartHighestPriceY, m_dChartHighestPrice, m_dPixelPerPrice);
}

VOID CTradeChart::OnReceiveEvent(CEvent& cEvent)
{
    switch (cEvent.GetIntParam(0)) {
        case EGLOBAL_EVT_TRADE_CHART_REFRESH_BB:
            GenerateBB();
            break;
            
        default:
            break;
    }
}

#include "SignalsUtil.h"
VOID CTradeChart::UpdatePrediction(VOID)
{
    const SDisplayCandle* psDisplayCandles = GetDisplayCandles();
    if (NULL == psDisplayCandles) {
       return;
    }
    const UINT uCandleNum = GetDisplayCandleNum();
    if (0 == uCandleNum) {
       return;
    }
    const CTradeCandleMap& cCandleMap = GetTradeCandleMap();
    const UINT uStartSec = GetTradeCandleStartTimeSec();
    const UINT uVisibleStartSec = psDisplayCandles[0].sData.uTimeSec;
    const UINT uEndSec = GetTradeCandleEndTimeSec();
    const UINT uCandleDurationSec = GetTradeCandleDurationSec();
    ASSERT(((uVisibleStartSec - uStartSec) % uCandleDurationSec) == 0);
    DOUBLE dAmountAllocated = 100.0;
    CSignalsUtil::GenerateSignals(m_cPredictions, dAmountAllocated, cCandleMap, uStartSec, uEndSec, uCandleDurationSec);
    TRACE("******** PerformPrediction ********\n");
    const DOUBLE dInitialAmount = dAmountAllocated;
    DOUBLE dLastBuyPrice = 0.0;
    FLOAT fPercentageChange = 0.0f;
    for (auto& sSignal : m_cPredictions) {
       switch (sSignal.eType) {
           case ESIG_BUY:
               TRACE("Buy Candle[%d] Price:%f\n", sSignal.uCandleIndex, sSignal.dPrice);
               dAmountAllocated -= sSignal.dAmount;
               dLastBuyPrice = sSignal.dPrice;
               break;
           case ESIG_STOP_LOSS:
               {
                   const FLOAT fPercentage = ((sSignal.dPrice - dLastBuyPrice) * 100.0f / dLastBuyPrice);
                   TRACE("SL Candle[%d] Price:%f Change => %f%%\n", sSignal.uCandleIndex, sSignal.dPrice, fPercentage);
                   dAmountAllocated += sSignal.dAmount;
                   fPercentageChange += fPercentage;
               }
               break;
           case ESIG_TAKE_PROFIT:
               {
                   const FLOAT fPercentage = ((sSignal.dPrice - dLastBuyPrice) * 100.0f / dLastBuyPrice);
                   TRACE("TP Candle[%d] Price:%f Change => %f%%\n", sSignal.uCandleIndex, sSignal.dPrice, fPercentage);
                   dAmountAllocated += sSignal.dAmount;
                   fPercentageChange += fPercentage;
               }
               break;
           case ESIG_END_PROFIT:
               {
                   const FLOAT fPercentage = ((sSignal.dPrice - dLastBuyPrice) * 100.0f / dLastBuyPrice);
                   TRACE("End Candle[%d] Price:%f Change => %f%%\n", sSignal.uCandleIndex, sSignal.dPrice, fPercentage);
                   dAmountAllocated += sSignal.dAmount;
                   fPercentageChange += fPercentage;
               }
           break;
       }
    }
    TRACE("************* RESULTS *************\n");
    TRACE("Change %f => %f\n", dInitialAmount, dAmountAllocated);
    TRACE("Percentage = %f\n", fPercentageChange);
    TRACE("*************** END ***************\n");
}
