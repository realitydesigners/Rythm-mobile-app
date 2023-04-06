#if !defined(TRADE_CHART_H)
#define TRADE_CHART_H

#include "BollingerBands.h"
#include "CryptoUtil.h"
#include "PlayerDataDef.h"
#include "MAUtil.h"
#include "ToolTip.h"
#include "SignalsDef.h"
#include "UIWidget.h"
#include <list>

class CUITextLabel;
typedef std::list<CUITextLabel*> CLabelList;

#define MA_LINE_1_COLOR RGBA(0xFF,  0xFF,   128,    0xFF)
#define MA_LINE_2_COLOR RGBA(0xFF,  0,      0,      0xFF)
#define MA_LINE_3_COLOR RGBA(122,   175,    255,    0xFF)

#define PRICE_PADDING      (0.0f)

class CTradeChart : public CUIWidget
{
public:
    CTradeChart();
    virtual ~CTradeChart();
    
    virtual VOID Release(VOID) OVERRIDE;

    virtual BOOLEAN RequireInputEvents(VOID) const OVERRIDE     { return TRUE; }
    virtual BOOLEAN OnTouchBegin(FLOAT fX, FLOAT fY)    OVERRIDE;
    virtual BOOLEAN OnTouchMove(FLOAT fX, FLOAT fY)     OVERRIDE;
    virtual BOOLEAN OnTouchEnd(FLOAT fX, FLOAT fY)      OVERRIDE;

    virtual VOID OnUpdate(FLOAT fLapsed) OVERRIDE;
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
    
    VOID ResetTime(VOID) { m_bTimeAdjusted = FALSE; }
    VOID ChangeTime(UINT uTimeSec, const CTradeCandleMap& cCandles, UINT uDurationMins);
    VOID SetGridInterval(UINT uSecs, UINT uColumnNum, BOOLEAN bShowDate);
    VOID SetupChart(const CTradeCandleMap& cCandles, UINT uDurationMins);
    
    VOID GenerateMA(UINT uIndex);
    
    VOID ToggleMAChart(VOID);
    SMovingAverage& GetMovingAverage(UINT uIndex) { return m_asMA[uIndex%3].sMA; }
    const CLabelList& GetTimeLabels(VOID) const { return m_cTimeLabels; }
    
    VOID ToggleCandle(VOID);
    const CTradeCandleMap& GetTradeCandleMap(VOID) const { return m_cCurrentSetofCandles; }
    const SDisplayCandle*  GetDisplayCandles(VOID) const { return m_psDisplayedCandles; }
    UINT GetDisplayCandleNum(VOID) const {  return  m_uDisplayedCandleNum; }
    
    UINT GetTradeCandleStartTimeSec(VOID) const { return m_uTradeCandleStartTimeSec; }
    UINT GetTradeCandleEndTimeSec(VOID) const { return m_uTradeCandleEndTimeSec; }
    UINT GetTradeCandleDurationSec(VOID) const { return m_uCandleDurationMins * 60; }
    UINT GetNowTimeSec(VOID) const { return m_uEndTimeSecs; }
    UINT GetVisibleTimeSec(VOID) const { return m_uColumnNum * m_uColumnWidthSecs; }
    
    VOID AdjustToolTip(INT nAdj);
    const CToolTip& GetToolTip(VOID) const { return m_cToolTip; }
    
    VOID UpdatePrediction(VOID);

private:
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    static VOID GenerateString(DOUBLE dValue, CHAR* szBuffer, UINT uBufferLen);
    virtual VOID OnWorldCoordinatesUpdated(VOID) OVERRIDE;

    VOID ClearArrays(VOID);
    VOID AllocateDataArrays(UINT uNum);
    VOID GenerateTimeLabels(VOID);
    VOID GeneratePriceLabels(VOID);
    VOID GenerateMA(VOID);
    VOID GenerateVolume(VOID);
    VOID GenerateBB(VOID);
    
    VOID GenerateToolTip(FLOAT fX, FLOAT fY);
    VOID SetToolTip(UINT uDisplayCandleIndex);
    VOID ClearToolTip(VOID);
    
    VOID ClearTimeLabels(VOID);
    VOID ClearPriceLabels(VOID);
    VOID ClearChart(VOID);
    
    VOID RenderCandles(VOID) const;
    VOID RenderVolume(VOID) const;
    VOID RenderMALines(VOID) const;
    VOID RenderTouchGrid(VOID) const;
    VOID RenderPrediction(VOID) const;

    typedef std::list<FLOAT> CLineList;
    enum {
        MOVING_AVERAGE_NUM=3,
    };
    
    struct SVisibleMovingAverage {
        SMovingAverage  sMA;
        FLOAT           fStartX;        // starting X value
        FLOAT*          pfYValues;
    };
    // candle related
    CTradeCandleMap m_cCurrentSetofCandles;
    UINT            m_uDisplayedCandleNum;                     // number of candles in the chart (visible by time)
    UINT            m_uCandleDurationMins;
    UINT            m_uTradeCandleStartTimeSec;       // 1st candle time in secs
    UINT            m_uTradeCandleEndTimeSec;         // last candle time.
    BOOLEAN         m_bRenderCandles;

    SDisplayCandle*         m_psDisplayedCandles;
    FLOAT*                  m_pfVolumeHeight;
    SVisibleMovingAverage   m_asMA[MOVING_AVERAGE_NUM];
    UINT                    m_uMaxLength;

    // rendering related
    CLineList       m_cAveragePriceLine;
    
    UINT            m_uColumnWidthSecs;
    UINT            m_uColumnNum;
    BOOLEAN         m_bShowDate;        // show date or not
    UINT            m_uStartTimeSecs;   // chart begin time
    UINT            m_uEndTimeSecs;     // chart end time
    CLabelList      m_cTimeLabels;
    BOOLEAN         m_bTimeLabelDirty;      // whether we need to refresh labels or not
    FLOAT           m_fCandleWidth;         // width of a candle in pixels
    
    // price labels
    CUITextLabel    m_cPercentage;
    CLabelList      m_cPriceLabels;
    BOOLEAN         m_bPriceLabelDirty;
    DOUBLE          m_dChartHighestPrice;           // visible chart highest price
    DOUBLE          m_dChartTotalPrice;             // price range of the visible chart
    FLOAT           m_fChartHighestPriceY;          // start Y value of the highest price
    DOUBLE          m_dPixelPerPrice;               // pixel per price
    
    // touch Grid
    BOOLEAN         m_bDrawToolTip;
    UINT            m_uSelectedDisplayedCandleIndex; // candle index for tooltip
    FLOAT           m_fTooltipX;
    FLOAT           m_fTooltipY;
    CToolTip        m_cToolTip;
    
    // time
    BOOLEAN         m_bTimeAdjusted;
    
    // bollinger band data
    CBollingerBands m_cBollingerBand;
    
    // Signal Prediction
    CSignalList     m_cPredictions;

};
#endif // #if !defined(TRADE_CHART_H)
