#if !defined(TRADING_MAIN_WINDOW_H)
#define TRADING_MAIN_WINDOW_H

#include "CryptoUtil.h"
#include "GameWindow.h"
#include "SliderBar.h"
#include "TradeChart.h"
#include "UIScroller.h"

class CBaseChart;
class CTradingMainWindow : public CGameWindow
{
public:
    static UINT GetDurationSeconds(UINT uDurationIndex);
    enum EBTN_ID {
        EBTN_CRYPTO_DOT_COM_CHART=0,
        EBTN_BINANCE_CHART,
        EBTN_OANDA_CHART,
        EBTN_PREDICT,
        EBTN_RENKO,
        EBTN_CHANGE_TRADE,
        EBTN_OVERLAY_BTN,
        EBTN_CANDLE_PREV,
        EBTN_CANDLE_NEXT,
        EBTN_DISPLAY_DURATION_CHANGE_START,
        EBTN_DISPLAY_DURATION_CHANGE_END = EBTN_DISPLAY_DURATION_CHANGE_START + 20,
        EBTN_CANDLE_DURATION_CHANGE_START,
        EBTN_CANDLE_DURATION_CHANGE_END = EBTN_CANDLE_DURATION_CHANGE_START + 100,
        EEVENT_ON_SLIDER_SLIDED,
    };
    
    CTradingMainWindow();
    virtual ~CTradingMainWindow();
    
    virtual VOID OnUpdate(FLOAT fLapsed) OVERRIDE;
    
protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    enum {
        MAX_ATTACHED_CHARTS=3,
    };
    ESOURCE_MARKET      m_eCurrentSource;       // current source of data to display
    UINT                m_uTradeIndex;          // the current trade index
    UINT                m_uDurationIndex;       // chart duration index
    UINT                m_uCandleDurationIndex; // candle duration index
    CUIImage            m_cBG;

    CUIButton*          m_pcChangeTradeBtn;
    CUIButton*          m_apcSourceBtns[ESOURCE_MARKET_NUM];
    CUIButton**         m_ppcCandleBtns;
    CUIButton*          m_pcRewind;
    CUIButton*          m_pcFwd;
    UINT                m_uMaxCandleBtnNum; // max number irregardless of which source we use
    CUIScroller         m_cDurationScroller;
    CUIScroller         m_cCandleIntervalScroller;
    CUIButton**         m_ppcDurationBtns;
    CTradeChart         m_cChart;
    CSliderBar          m_cTimeSlider;
    UINT                m_auAttachedChartID[MAX_ATTACHED_CHARTS];
    CBaseChart*         m_apcAttachedChart[MAX_ATTACHED_CHARTS];
    
    FLOAT               m_fRefreshCounter;
    ESOURCE_MARKET      m_ePrevCandleSource;
    UINT                m_uPrevCandleTradeIndex;
    CUITextLabel        m_cMarketClosedLbl;
    

    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID TryChangeSource(ESOURCE_MARKET eSource);
    VOID OnToggleSource(ESOURCE_MARKET eSource);
    VOID OnToggleDuration(UINT uDurationIndex);
    VOID OnToggleCandleDuration(UINT uIndex, BOOLEAN bUserInitiated);
    
    VOID TryQueryCandles(BOOLEAN bRefresh);
    VOID OnQueryCandlesEnd(BOOLEAN bSuccess);
    
    VOID InitializeCandleButtons(VOID);
    
    VOID OnOverlayBtnPressed(VOID);
    
    VOID ReleaseAttachment(VOID);
    
    VOID AttachChart(UINT uSubChartID, BOOLEAN bSkipRecalculateSize);
    VOID RefreshAttachedCharts(UINT uSubChartID);
    VOID RefreshAllChartSizes(VOID);
    VOID ToogleAttachChart(UINT uSubChartID);
    
    VOID UpdateTradeTypeBtn(VOID);
    VOID OnPressTradeChangeBtn(VOID);
    VOID OnPressRenkoBtn(VOID);
    VOID OnTradeChange(UINT uIndex);
    
    VOID ReattachMoveCandleBtns(VOID);
    VOID AdjustSelectedCandle(INT nAdj);
    
    VOID PerformPrediction(VOID);
    
    VOID OnSliderSlided(UINT uSecs);
    
    VOID OnToolTip(BOOLEAN bShow, UINT uDisplayCandleIndex);
    
    VOID OnCryptoDotComTradePairUpdate(BOOLEAN bSuccess);
    VOID OnCryptoDotComCandleUpdate(UINT uTradePairIndex, UINT uCandleIndex, BOOLEAN bSuccess);
    VOID OnCryptoDotComSocketError(VOID);
    
    VOID OnBinanceTradePairsQuery(BOOLEAN bSuccess);

    VOID OnOandaSignInReply(BOOLEAN bSuccess);
    VOID OnOandaTradePairQueryReply(BOOLEAN bSuccess);
};
#endif // #if !defined(TRADING_MAIN_WINDOW_H)
