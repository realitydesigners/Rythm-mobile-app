#include "stdafx.h"
#include "BinanceMgr.h"
#include "CMain.h"
#include "CMOChart.h"
#include "ConfirmationWindow.h"
#include "CryptoDotComMgr.h"
#include "Event.h"
#include "EventID.h"
#include "EventManager.h"
#include "HLTime.h"
#include "MACDChart.h"
#include "NowLoadingWindow.h"
#include "OandaMgr.h"
#include "OBVChart.h"
#include "PlayerData.h"
#include "RSIChart.h"
#include "SaveData.h"
#include "TradeMainWindow.h"
#include "TradeOverlayWindow.h"
#include "TradeChangeWindow.h"
#include "ToolTip.h"

#define SELECTED_COLOR      RGBA(0x1F,  0x1F,   0xFF,   0xFF)
#define UNSELECTED_COLOR    RGBA(0x7F,  0x7F,   0x7F,   0x7F)

struct SDurationInfo {
    const CHAR* szName;
    UINT        uIntervalSec;
};
static SDurationInfo DURATION_INFO[] = {
    { "30M",    60*3    },
    { "1H",     360     },
    { "2H",     360*2   },
    { "4H",     360*4   },
    { "6H",     360*6   },
    { "12H",    360*12  },
    { "1D",     8640    },
    { "2D",     8640*2  },
    { "3D",     8640*3  },
    { "5D",     8640*5  },
    { "7D",     8640*7  },
    { "15D",    8640*15 },
    { "1M",     8640*30     },
    { "3M",     8640*30*3   },
    { "6M",     8640*30*6   },
    { "1Y",     8640*30*12  },
    { "2Y",     8640*30*12*2},
    { "4Y",     8640*30*12*3},
};
static UINT DURATION_INFO_NUM = sizeof(DURATION_INFO)/sizeof(SDurationInfo);

UINT CTradingMainWindow::GetDurationSeconds(UINT uDurationIndex)
{
    return DURATION_INFO[uDurationIndex%DURATION_INFO_NUM].uIntervalSec * 10;
}
static ESOURCE_MARKET s_ePrevSource = ESOURCE_MARKET_BINANCE;
static UINT s_uPrevTradeIndex = 0;
static UINT s_uPrevCandleDurationIndex = 0;
static UINT s_uPrevDurationIndex = 2;

#define CREATE_BTN(x, y, w, h, id, text, addchild) CreateColorButton(x, y, w, h, id, UNSELECTED_COLOR, text, addchild)

CTradingMainWindow::CTradingMainWindow() :
CGameWindow(EGLOBAL_TRADE_MAIN_WINDOW),
m_eCurrentSource(s_ePrevSource),
m_uTradeIndex(s_uPrevTradeIndex),
m_uCandleDurationIndex(s_uPrevCandleDurationIndex),
m_pcChangeTradeBtn(NULL),
m_uDurationIndex(s_uPrevDurationIndex),
m_ppcCandleBtns(NULL),
m_pcRewind(NULL),
m_pcFwd(NULL),
m_uMaxCandleBtnNum(0),
m_ppcDurationBtns(NULL),
m_cDurationScroller(FALSE),
m_cCandleIntervalScroller(FALSE),
m_cTimeSlider(EGLOBAL_TRADE_MAIN_WINDOW, EEVENT_ON_SLIDER_SLIDED),
m_fRefreshCounter(30.0f),
m_ePrevCandleSource(s_ePrevSource),
m_uPrevCandleTradeIndex(s_uPrevTradeIndex),
INITIALIZE_TEXT_LABEL(m_cMarketClosedLbl)
{
    m_bDestroyOnExitModal = TRUE;
    memset(m_apcSourceBtns, 0, sizeof(m_apcSourceBtns));
    memset(m_auAttachedChartID, 0, sizeof(m_auAttachedChartID));
    memset(m_apcAttachedChart, 0, sizeof(m_apcAttachedChart));
}

CTradingMainWindow::~CTradingMainWindow()
{
    s_ePrevSource = m_eCurrentSource;
    s_uPrevTradeIndex = m_uTradeIndex;
    s_uPrevCandleDurationIndex = m_uCandleDurationIndex;
    s_uPrevDurationIndex = m_uDurationIndex;
}

#define BUTTON_H        (50.0f)
#define DURATION_BTN_W  (50.0f)
#define CANDLE_BTN_W    (50.0f)
#define CANDLE_CHART_Y  (160.0f)
#define SLIDER_BAR_HEIGHT (50.0f)
VOID CTradingMainWindow::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();
    const FLOAT fW = CMain::GetScreenWidth();
    const FLOAT fH = CMain::GetScreenHeight();
    SetLocalSize(fW, fH);
    m_cBG.SetTexture(ETEX::ID::white_tex);
    m_cBG.SetColor(RGBA(0x1F, 0x1F, 0x1F, 0xFF));
    m_cBG.SetLocalSize(fW, fH);
    AddChild(m_cBG);
    AddChild(m_cTimeSlider);
        
    m_cMarketClosedLbl.SetFont(EGAMEFONT_SIZE_20);
    m_cMarketClosedLbl.SetColor(RGBA(0xFF, 0, 0, 0xFF));
    m_cMarketClosedLbl.SetAnchor(1.0f, 0.5f);
    m_cMarketClosedLbl.SetLocalPosition(fW - CMain::GetDisplaySideMargin() - 100.0f, CANDLE_CHART_Y + 10.0f);
    m_cMarketClosedLbl.SetString("Market Closed");

    const FLOAT fSideMargin = CMain::GetDisplaySideMargin();
    CREATE_BTN(fSideMargin + 20.0f, 10.0f, 50.0f, 50.0f, EBTN_OVERLAY_BTN,  "!", TRUE);
    CREATE_BTN(fSideMargin + 20.0f, 70.0f, 50.0f, 50.0f, EBTN_PREDICT,      "P", TRUE);
    CREATE_BTN(fSideMargin + 80.0f, 10.0f, 50.0f, 50.0f, EBTN_RENKO,        "R", TRUE);
    
    m_apcSourceBtns[ESOURCE_MARKET_CRYPTO_COM] = CREATE_BTN(fSideMargin + 140.0f, 10.0f, 100.0f, BUTTON_H, EBTN_CRYPTO_DOT_COM_CHART, "Crypto.com", TRUE);
    m_apcSourceBtns[ESOURCE_MARKET_BINANCE]    = CREATE_BTN(fSideMargin + 250.0f, 10.0f, 100.0f, BUTTON_H, EBTN_BINANCE_CHART, "Binance", TRUE);
    m_apcSourceBtns[ESOURCE_MARKET_OANDA]      = CREATE_BTN(fSideMargin + 140.0f, 10.0f + BUTTON_H + 10.0f, 100.0f, BUTTON_H, EBTN_OANDA_CHART, "Oanda", TRUE);

    m_pcChangeTradeBtn = CREATE_BTN(fSideMargin + 250.0f, 70.0f, 100.0f, BUTTON_H, EBTN_CHANGE_TRADE, "XXX-XXX", TRUE);
    
    {
        FLOAT fTimeBtnX = 10.0f + CMain::GetDisplaySideMargin();
        m_pcRewind = CREATE_BTN(fTimeBtnX, CANDLE_CHART_Y + 10.0f, 50.0f, 50.0f, EBTN_CANDLE_PREV, "<<", TRUE); fTimeBtnX += 60.0f;
        m_pcFwd = CREATE_BTN(fTimeBtnX, CANDLE_CHART_Y + 10.0f, 50.0f, 50.0f, EBTN_CANDLE_NEXT, ">>", TRUE);
    }
    
    const FLOAT fScrollerW = 9.0f * 50.0f + 8.0f * 10.0f;
    m_cDurationScroller.SetLocalSize(fScrollerW, BUTTON_H);
    const FLOAT fScrollerX = fW - fScrollerW - 10.0f - CMain::GetDisplaySideMargin();
    m_cDurationScroller.SetLocalPosition(fScrollerX, 10.0f);
    AddChild(m_cDurationScroller);
    
    m_cCandleIntervalScroller.SetLocalSize(fScrollerW, BUTTON_H);
    m_cCandleIntervalScroller.SetLocalPosition(fScrollerX, 70.0f);
    AddChild(m_cCandleIntervalScroller);
    
    AddChild(m_cChart);

    ASSERT((EBTN_DISPLAY_DURATION_CHANGE_END >= (EBTN_DISPLAY_DURATION_CHANGE_START + DURATION_INFO_NUM)));
    m_ppcDurationBtns = new CUIButton*[DURATION_INFO_NUM];
    if (NULL == m_ppcDurationBtns) {
        ASSERT(FALSE);
        return;
    }
    UINT uIndex = 0;
    FLOAT fBtnX = 0.0f;
    for ( ; DURATION_INFO_NUM > uIndex; ++uIndex, fBtnX += (DURATION_BTN_W + 10.0f)) {
        CUIButton* pcBtn = CREATE_BTN(fBtnX, 0.0f, DURATION_BTN_W, BUTTON_H, EBTN_DISPLAY_DURATION_CHANGE_START + uIndex, DURATION_INFO[uIndex].szName, FALSE);
        if (NULL != pcBtn) {
            m_cDurationScroller.AddChild(*pcBtn);
        }
        m_ppcDurationBtns[uIndex] = pcBtn;
    }
    InitializeCandleButtons(); // initialize all candle buttons 1st
    
    uIndex = 0;
    for ( ; MAX_ATTACHED_CHARTS > uIndex; ++uIndex) {
        switch (CPlayerData::GetAttachedChartType(uIndex)) {
            case EATTACH_CHART_NONE:
                break;
            case EATTACH_CHART_MACD:
                AttachChart(EGLOBAL_MACD_CHART, TRUE);
                break;
            case EATTACH_CHART_OBV:
                AttachChart(EGLOBAL_OBV_CHART, TRUE);
                break;
            case EATTACH_CHART_RSI:
                AttachChart(EGLOBAL_RSI_CHART, TRUE);
                break;
            case EATTACH_CHART_CMO:
                AttachChart(EGLOBAL_CMO_CHART, TRUE);
                break;
        }
    }
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_CRYPTO_DOT_COM_MGR, EGLOBAL_EVT_CRYPTO_OHLC_UPDATE);
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_CRYPTO_DOT_COM_MGR, EGLOBAL_EVT_CRYPTO_MARKET_SOCKET_ERROR);

    TryChangeSource(m_eCurrentSource);
}

VOID CTradingMainWindow::InitializeCandleButtons(VOID)
{
    m_uMaxCandleBtnNum = CCryptoDotComMgr::GetCandleTypeNum();
    const UINT uBinanceNum = CBinanceMgr::GetCandleTypeNum();
    if (uBinanceNum > m_uMaxCandleBtnNum) {
        m_uMaxCandleBtnNum = uBinanceNum;
    }
    m_ppcCandleBtns = new CUIButton*[m_uMaxCandleBtnNum];
    if (NULL == m_ppcCandleBtns) {
        ASSERT(FALSE);
        return;
    }
    UINT uIndex = 0;
    for ( ; m_uMaxCandleBtnNum > uIndex; ++uIndex) {
        m_ppcCandleBtns[uIndex] = CREATE_BTN(0, 0, CANDLE_BTN_W, BUTTON_H, EBTN_CANDLE_DURATION_CHANGE_START + uIndex, "", FALSE);
    }
}

VOID CTradingMainWindow::Release(VOID)
{
    m_cMarketClosedLbl.Release();
    m_cMarketClosedLbl.RemoveFromParent();
    UINT uIndex = 0;
    if (NULL != m_ppcDurationBtns) {
        for ( ; DURATION_INFO_NUM > uIndex; ++uIndex) {
            if (NULL != m_ppcDurationBtns[uIndex]) {
                m_ppcDurationBtns[uIndex]->RemoveFromParent();
            }
        }
        SAFE_DELETE_ARRAY(m_ppcDurationBtns);
    }
    m_cDurationScroller.RemoveFromParent();
    m_cDurationScroller.Release();

    if (NULL != m_ppcCandleBtns) {
        uIndex = 0;
        for ( ; m_uMaxCandleBtnNum > uIndex; ++uIndex) {
            if (NULL != m_ppcCandleBtns[uIndex]) {
                m_ppcCandleBtns[uIndex]->RemoveFromParent();
            }
        }
        SAFE_DELETE_ARRAY(m_ppcCandleBtns);
    }
    m_uMaxCandleBtnNum = 0;
    m_cCandleIntervalScroller.RemoveFromParent();
    m_cCandleIntervalScroller.Release();

    m_cBG.RemoveFromParent();
    m_cChart.Release();
    
    ReleaseAttachment();
    
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_CRYPTO_DOT_COM_MGR, EGLOBAL_EVT_CRYPTO_OHLC_UPDATE);
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_CRYPTO_DOT_COM_MGR, EGLOBAL_EVT_CRYPTO_MARKET_SOCKET_ERROR);
    CGameWindow::Release();
}

VOID CTradingMainWindow::OnToggleDuration(UINT uDurationIndex)
{
    if (DURATION_INFO_NUM <= uDurationIndex) {
        ASSERT(FALSE);
        return;
    }
    if (NULL == m_ppcDurationBtns) {
        ASSERT(FALSE);
        return;
    }
    UINT uI = 0;
    for ( ; DURATION_INFO_NUM > uI; ++uI) {
        if (NULL != m_ppcDurationBtns[uI]) {
            CUIImage* pcBG = CUIImage::CastToMe(m_ppcDurationBtns[uI]->GetIdleWidget());
            if (NULL != pcBG) {
                const UINT uColor = (uDurationIndex == uI) ? SELECTED_COLOR : UNSELECTED_COLOR;
                pcBG->SetColor(uColor);
            }
        }
    }
    m_uDurationIndex = uDurationIndex;
    const UINT uTimeSec = DURATION_INFO[uDurationIndex].uIntervalSec;
    m_cChart.SetGridInterval(uTimeSec, 10, 8640 <= uTimeSec);
    OnQueryCandlesEnd(TRUE);
}

VOID CTradingMainWindow::OnToggleSource(ESOURCE_MARKET eSource)
{
    if (eSource != m_eCurrentSource) {
        m_uTradeIndex = 0; // reset the trade type for safety
        m_eCurrentSource = eSource;
    }
    UINT uI = 0;
    for ( ; ESOURCE_MARKET_NUM > uI; ++uI) {
        if (NULL != m_apcSourceBtns[uI]) {
            CUIImage* pcBG = CUIImage::CastToMe(m_apcSourceBtns[uI]->GetIdleWidget());
            if (NULL != pcBG) {
                const UINT uColor = (eSource == uI) ? SELECTED_COLOR : UNSELECTED_COLOR;
                pcBG->SetColor(uColor);
            }
        }
    }
    if (NULL == m_ppcCandleBtns) {
        ASSERT(FALSE);
        return;
    }
    UINT uIndex = 0;
    // remove all buttons 1st
    for ( ; m_uMaxCandleBtnNum > uIndex; ++uIndex) {
        if (NULL != m_ppcCandleBtns[uIndex]) {
            m_ppcCandleBtns[uIndex]->RemoveFromParent();
        }
    }
    UINT uBtnNum = 0;
    const SCandleIntervalDisplayData* psCandleIntervalData = NULL;
    switch (eSource) {
        case ESOURCE_MARKET_CRYPTO_COM:
            uBtnNum = CCryptoDotComMgr::GetCandleTypeNum();
            psCandleIntervalData = CCryptoDotComMgr::GetAllCandleDisplayData();
            break;
        case ESOURCE_MARKET_BINANCE:
            uBtnNum = CBinanceMgr::GetCandleTypeNum();
            psCandleIntervalData = CBinanceMgr::GetAllCandleDisplayData();
            break;
        case ESOURCE_MARKET_OANDA:
            uBtnNum = COandaMgr::GetCandleTypeNum();
            psCandleIntervalData = COandaMgr::GetAllCandleDisplayData();
            break;
        default:
            ASSERT(FALSE);
            return;
    }
    FLOAT fX = 0.0f;
    uIndex = 0;
    for ( ; uBtnNum > uIndex; ++uIndex, fX += (CANDLE_BTN_W + 10.0f)) {
        if (NULL != m_ppcCandleBtns[uIndex]) {
            m_ppcCandleBtns[uIndex]->SetLocalPosition(fX, 0.0f);
            CUITextLabel* pcLbl = CUITextLabel::CastToMe(m_ppcCandleBtns[uIndex]->GetChild());
            if (NULL != pcLbl) {
                const CHAR* szLabel = psCandleIntervalData[uIndex].szDisplayName;
                pcLbl->SetString(szLabel);
            }
            m_cCandleIntervalScroller.AddChild(*m_ppcCandleBtns[uIndex]);
        }
    }
    UpdateTradeTypeBtn();
    
    // safety
    m_uCandleDurationIndex = m_uCandleDurationIndex % uBtnNum;
    OnToggleCandleDuration(m_uCandleDurationIndex, FALSE);
    
    ReattachMoveCandleBtns();
}

VOID CTradingMainWindow::OnToggleCandleDuration(UINT uIndex, BOOLEAN bUserInitiated)
{
    UINT uBtnNum = 0;
    switch (m_eCurrentSource) {
        case ESOURCE_MARKET_CRYPTO_COM:
            uBtnNum = CCryptoDotComMgr::GetCandleTypeNum();
            break;
        case ESOURCE_MARKET_BINANCE:
            uBtnNum = CBinanceMgr::GetCandleTypeNum();
            break;
        case ESOURCE_MARKET_OANDA:
            uBtnNum = COandaMgr::GetCandleTypeNum();
            break;
            
        default:
            break;
    }
    if (uBtnNum <= uIndex) {
        ASSERT(FALSE);
        return;
    }
    m_uCandleDurationIndex = uIndex;
    UINT uI = 0;
    for ( ; uBtnNum > uI; ++uI) {
        if (NULL != m_ppcCandleBtns[uI]) {
            CUIImage* pcBG = CUIImage::CastToMe(m_ppcCandleBtns[uI]->GetIdleWidget());
            if (NULL != pcBG) {
                const UINT uColor = (uIndex == uI) ? SELECTED_COLOR : UNSELECTED_COLOR;
                pcBG->SetColor(uColor);
            }
        }
    }
    m_fRefreshCounter = 30.0f;
    TryQueryCandles(bUserInitiated);
}
VOID CTradingMainWindow::TryQueryCandles(BOOLEAN bRefresh)
{
    switch (m_eCurrentSource) {
        case ESOURCE_MARKET_CRYPTO_COM:
        {
            CCryptoDotComMgr& cMgr = CCryptoDotComMgr::GetInstance();
            CNowLoadingWindow::DisplayWindow(0xFFFFFFFF, "Querying..");
            ASSERT(cMgr.IsMarketSocketConnected());
            CCryptoDotComMgr::GetInstance().QueryOHLC(m_uTradeIndex, m_uCandleDurationIndex);
        }
        return;
        case ESOURCE_MARKET_BINANCE:
        {
            if (!bRefresh && CBinanceMgr::GetInstance().HasOHLCData(m_uTradeIndex, m_uCandleDurationIndex)) {
                OnQueryCandlesEnd(TRUE);
                return;
            }
            CNowLoadingWindow::DisplayWindow(0xFFFFFFFF, "Querying..");
            CEventManager::RegisterForBroadcast(*this, EGLOBAL_BINANCE_MGR, EGLOBAL_EVT_BINANCE_ON_OHLC_QUERY_END, TRUE);
            CBinanceMgr::GetInstance().QueryOHLC(m_uTradeIndex, m_uCandleDurationIndex);
        }
        return;
        case ESOURCE_MARKET_OANDA:
        {
            if (!bRefresh && COandaMgr::GetInstance().HasOHLCData(m_uTradeIndex, m_uCandleDurationIndex)) {
                OnQueryCandlesEnd(TRUE);
                return;
            }
            CNowLoadingWindow::DisplayWindow(0xFFFFFFFF, "Querying..");
            CEventManager::RegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_OHLC_QUERY, TRUE);
            COandaMgr::GetInstance().QueryOHLC(m_uTradeIndex, m_uCandleDurationIndex);
        }
        return;
        default:
            ASSERT(FALSE);
            break;
    }
}

VOID CTradingMainWindow::OnQueryCandlesEnd(BOOLEAN bSuccess)
{
    if (!bSuccess) {
        return;
    }
    const CTradeCandleMap* psCandles = NULL;
    UINT uCandleMins = 0;
    switch (m_eCurrentSource) {
        case ESOURCE_MARKET_CRYPTO_COM:
            psCandles = CCryptoDotComMgr::GetInstance().GetCandleMap(m_uTradeIndex, m_uCandleDurationIndex);
            uCandleMins = CCryptoDotComMgr::GetCandleDurationMins(m_uCandleDurationIndex);
            break;
        case ESOURCE_MARKET_BINANCE:
            psCandles = CBinanceMgr::GetInstance().GetCandleMap(m_uTradeIndex, m_uCandleDurationIndex);
            uCandleMins = CBinanceMgr::GetCandleDurationMins(m_uCandleDurationIndex);
            break;
        case ESOURCE_MARKET_OANDA:
            psCandles = COandaMgr::GetInstance().GetCandleMap(m_uTradeIndex, m_uCandleDurationIndex);
            uCandleMins = COandaMgr::GetCandleDurationMins(m_uCandleDurationIndex);
            break;
        default:
            ASSERT(FALSE);
            return;
    }
    if (NULL == psCandles) {
        return;
    }
    if (m_ePrevCandleSource != m_eCurrentSource || m_uPrevCandleTradeIndex != m_uTradeIndex) {
        m_ePrevCandleSource = m_eCurrentSource;
        m_uPrevCandleTradeIndex = m_uTradeIndex;
        m_cChart.ResetTime();
    }
    m_cChart.OnUpdate(0.0f); // cheat
    m_cChart.SetupChart(*psCandles, uCandleMins);
    
    const UINT uLastCandleTime = m_cChart.GetTradeCandleEndTimeSec();
    const UINT uNowSec = CHLTime::GetTimeSecs();
    m_cMarketClosedLbl.RemoveFromParent();
    if (uNowSec > uLastCandleTime && (0 < psCandles->size())) {
        if ((2 * uCandleMins * 60) < (uNowSec - uLastCandleTime)) {
            AddChild(m_cMarketClosedLbl);
        }
    }
    RefreshAttachedCharts(0xFFFFFFFF);
    m_cTimeSlider.SetupBar(m_cChart.GetTradeCandleStartTimeSec(), uLastCandleTime + uCandleMins * 60, m_cChart.GetNowTimeSec(), m_cChart.GetVisibleTimeSec());
}

VOID CTradingMainWindow::OnOverlayBtnPressed(VOID)
{
    CTradeOverlayWindow* pcWin = new CTradeOverlayWindow(m_cChart);
    if (NULL != pcWin) {
        pcWin->DoModal();
    }
}

VOID CTradingMainWindow::ReleaseAttachment(VOID)
{
    UINT uIndex = 0;
    for ( ; MAX_ATTACHED_CHARTS > uIndex; ++uIndex) {
        if (NULL != m_apcAttachedChart[uIndex]) {
            m_apcAttachedChart[uIndex]->RemoveFromParent();
            m_apcAttachedChart[uIndex]->Release();
            delete m_apcAttachedChart[uIndex];
            m_apcAttachedChart[uIndex] = NULL;
        }
        m_auAttachedChartID[uIndex] = 0;
    }
}

VOID CTradingMainWindow::AttachChart(UINT uSubChartID, BOOLEAN bSkipRecalculateSize)
{
    // check if already attached
    UINT uIndex = 0;
    UINT uEmptyIndex = MAX_ATTACHED_CHARTS;
    for ( ; MAX_ATTACHED_CHARTS > uIndex; ++uIndex) {
        if (0 == m_auAttachedChartID[uIndex] && MAX_ATTACHED_CHARTS == uEmptyIndex) {
            uEmptyIndex = uIndex;
        }
        if (m_auAttachedChartID[uIndex] == uSubChartID) { // already attached before
            return;
        }
    }
    if (MAX_ATTACHED_CHARTS == uEmptyIndex) { // no more empty slots
        return;
    }
    CBaseChart* pcChart = NULL;
    EATTACH_CHART_TYPE eAttachedType = EATTACH_CHART_NONE;
    switch (uSubChartID) {
        case EGLOBAL_CMO_CHART:
            eAttachedType = EATTACH_CHART_CMO;
            pcChart = new CCMOChart(m_cChart);
            break;
        case EGLOBAL_MACD_CHART:
            eAttachedType = EATTACH_CHART_MACD;
            pcChart = new CMACDChart(m_cChart);
            break;
        case EGLOBAL_RSI_CHART:
            eAttachedType = EATTACH_CHART_RSI;
            pcChart = new CRSIChart(m_cChart);
            break;
        case EGLOBAL_OBV_CHART:
            eAttachedType = EATTACH_CHART_OBV;
            pcChart = new COBVChart(m_cChart);
            break;
            
        default:
            ASSERT(FALSE);
            break;
    }
    if (NULL == pcChart) {
        ASSERT(FALSE);
        return;
    }
    pcChart->Initialize();
    AddChild(*pcChart);
    m_apcAttachedChart[uEmptyIndex] = pcChart;
    m_auAttachedChartID[uEmptyIndex] = uSubChartID;
    if (!bSkipRecalculateSize) {
        RefreshAllChartSizes();
        CPlayerData::SetAttachedChartTypes(uEmptyIndex, eAttachedType);
        CSaveData::SaveData(FALSE);
    }
}

VOID CTradingMainWindow::UpdateTradeTypeBtn(VOID)
{
    if (NULL == m_pcChangeTradeBtn) {
        return;
    }
    CUITextLabel* pcTxt = CUITextLabel::CastToMe(m_pcChangeTradeBtn->GetChild());
    if (NULL == pcTxt) {
        return;
    }
    CHAR szBuffer[64];
    switch (m_eCurrentSource) {
        case ESOURCE_MARKET_BINANCE:
            {
                const STradePair* psTrade = CBinanceMgr::GetInstance().GetTradePair(m_uTradeIndex);
                snprintf(szBuffer, 64, "%s-%s", psTrade->szBase, psTrade->szQuote);
            }
            break;
        case ESOURCE_MARKET_CRYPTO_COM:
            {
                const STradePair* psTrade = CCryptoDotComMgr::GetInstance().GetTradePair(m_uTradeIndex);
                snprintf(szBuffer, 64, "%s-%s", psTrade->szBase, psTrade->szQuote);
            }
            break;
        case ESOURCE_MARKET_OANDA:
            {
                const COandaMgr::SOandaPair* psPair = COandaMgr::GetInstance().GetTradePair(m_uTradeIndex);
                snprintf(szBuffer, 64, "%s", psPair->szTradePair);
            }
            break;
        default:
            ASSERT(FALSE);
            return;
    }
    pcTxt->SetString(szBuffer);
}

VOID CTradingMainWindow::OnPressTradeChangeBtn(VOID)
{
    if (NULL == m_pcChangeTradeBtn) {
        ASSERT(FALSE);
    }
    CTradeChangeWindow* pcWin = new CTradeChangeWindow(m_eCurrentSource, m_uTradeIndex);
    if (NULL != pcWin) {
        pcWin->DoModal();
    }
}

VOID CTradingMainWindow::OnTradeChange(UINT uIndex)
{
    m_uTradeIndex = uIndex;
    UpdateTradeTypeBtn();
    m_fRefreshCounter = 30.0f;
    TryQueryCandles(TRUE);
}

VOID CTradingMainWindow::ReattachMoveCandleBtns(VOID)
{
    if (NULL != m_pcFwd) {
        m_pcFwd->RemoveFromParent();
        AddChild((*m_pcFwd));
    }
    if (NULL != m_pcRewind) {
        m_pcRewind->RemoveFromParent();
        AddChild((*m_pcRewind));
    }
}

VOID CTradingMainWindow::AdjustSelectedCandle(INT nAdj)
{
    m_cChart.AdjustToolTip(nAdj);
}

VOID CTradingMainWindow::OnSliderSlided(UINT uSecs)
{
    const CTradeCandleMap* psCandles = NULL;
    UINT uCandleMins = 0;
    switch (m_eCurrentSource) {
        case ESOURCE_MARKET_BINANCE:
            psCandles = CBinanceMgr::GetInstance().GetCandleMap(m_uTradeIndex, m_uCandleDurationIndex);
            uCandleMins = CBinanceMgr::GetCandleDurationMins(m_uCandleDurationIndex);
            break;
        case ESOURCE_MARKET_CRYPTO_COM:
            psCandles = CCryptoDotComMgr::GetInstance().GetCandleMap(m_uTradeIndex, m_uCandleDurationIndex);
            uCandleMins = CCryptoDotComMgr::GetCandleDurationMins(m_uCandleDurationIndex);
            break;
        case ESOURCE_MARKET_OANDA:
            psCandles = COandaMgr::GetInstance().GetCandleMap(m_uTradeIndex, m_uCandleDurationIndex);
            uCandleMins = COandaMgr::GetCandleDurationMins(m_uCandleDurationIndex);
            break;
        default:
            ASSERT(FALSE);
            return;
    }
    if (NULL == psCandles) {
        return;
    }
    m_cChart.ChangeTime(uSecs, *psCandles, uCandleMins);
    RefreshAttachedCharts(0xFFFFFFFF);
}

VOID CTradingMainWindow::RefreshAttachedCharts(UINT uSubChartID)
{
    UINT uIndex = 0;
    for ( ; MAX_ATTACHED_CHARTS > uIndex; ++uIndex) {
        CBaseChart* pcChart = m_apcAttachedChart[uIndex];
        if (NULL != pcChart) {
            if (0xFFFFFFFF == uSubChartID || pcChart->GetGlobalID() == uSubChartID) {
                pcChart->SetupChart();
                pcChart->UpdateTitleString();
            }
        }
    }
}

VOID CTradingMainWindow::ToogleAttachChart(UINT uSubChartID)
{
    UINT uAttachedIndex = MAX_ATTACHED_CHARTS;
    BOOLEAN bHaveSpace = FALSE;
    UINT uIndex = 0;
    for ( ; MAX_ATTACHED_CHARTS > uIndex; ++uIndex) {
        CBaseChart* pcChart = m_apcAttachedChart[uIndex];
        if (NULL != pcChart) {
            if (pcChart->GetGlobalID() == uSubChartID) {
                uAttachedIndex = uIndex;
            }
        }
        else {
            bHaveSpace = TRUE;
        }
    }
    if (MAX_ATTACHED_CHARTS > uAttachedIndex) { // if chart was attached
        m_apcAttachedChart[uAttachedIndex]->RemoveFromParent();
        m_apcAttachedChart[uAttachedIndex]->Release();
        delete m_apcAttachedChart[uAttachedIndex];
        m_apcAttachedChart[uAttachedIndex] = NULL;
        m_auAttachedChartID[uAttachedIndex] = 0;
        RefreshAllChartSizes();
        
        CPlayerData::SetAttachedChartTypes(uAttachedIndex, EATTACH_CHART_NONE);
        CSaveData::SaveData(FALSE);
        return;
    }
    if (bHaveSpace) {
        AttachChart(uSubChartID, FALSE);
    }
    
}
VOID CTradingMainWindow::RefreshAllChartSizes(VOID)
{
    UINT uIndex = 0;
    UINT uAttachedChartNum = 0;
    for ( ; MAX_ATTACHED_CHARTS > uIndex; ++uIndex) {
        if (0 != m_auAttachedChartID[uIndex]) {
            ++uAttachedChartNum;
        }
    }
    const FLOAT fW = CMain::GetScreenWidth();
    const FLOAT fH = CMain::GetScreenHeight();
    const FLOAT fSideMargin = CMain::GetDisplaySideMargin();
    
    const FLOAT afAlternateChartSizes[MAX_ATTACHED_CHARTS+1] = {
        0.0f, 150.0f, 100.0f, 75.0f
    };
    const FLOAT fAlternateChartSize = afAlternateChartSizes[uAttachedChartNum];
    const FLOAT fAlternateChartOffsets = 10.0f * uAttachedChartNum;
    const FLOAT fTotalALternateChartSize = fAlternateChartOffsets + fAlternateChartSize * uAttachedChartNum;
    const FLOAT fMainChartHeight = fH - CANDLE_CHART_Y - SLIDER_BAR_HEIGHT - CMain::GetDisplayBottomMargin() - 10.0f - fTotalALternateChartSize;
    const FLOAT fMainChartX = fSideMargin + 20.0f;
    const FLOAT fMainChartSizeX = fW - 40.0f - 2.0f * fSideMargin - 20.0f;
    
    FLOAT fY = CANDLE_CHART_Y;
    m_cChart.SetLocalPosition(fMainChartX, CANDLE_CHART_Y);
    m_cChart.SetLocalSize(fMainChartSizeX, fMainChartHeight);
    fY += fMainChartHeight;
    fY += 5.0f;
    m_cTimeSlider.SetLocalPosition(fMainChartX, fY);
    m_cTimeSlider.SetLocalSize(fMainChartSizeX - 40.0f, SLIDER_BAR_HEIGHT);
    fY += SLIDER_BAR_HEIGHT + 5.0f;
    uIndex = 0;
    for ( ; MAX_ATTACHED_CHARTS > uIndex; ++uIndex) {
        if (NULL == m_apcAttachedChart[uIndex]) {
            continue;
        }
        m_apcAttachedChart[uIndex]->SetLocalPosition(fMainChartX, fY);
        m_apcAttachedChart[uIndex]->SetLocalSize(fMainChartSizeX, fAlternateChartSize);
        m_apcAttachedChart[uIndex]->OnUpdate(0.0f);
        m_apcAttachedChart[uIndex]->SetupChart();
        fY += fAlternateChartSize + 10.0f;
    }
    OnQueryCandlesEnd(TRUE);
}

VOID CTradingMainWindow::OnUpdate(FLOAT fLapsed)
{
    m_fRefreshCounter -= fLapsed;
    if (0.0f > m_fRefreshCounter) {
        m_fRefreshCounter = 10.0f;
//        TryQueryCandles(TRUE);
    }
    CGameWindow::OnUpdate(fLapsed);
}

VOID CTradingMainWindow::OnToolTip(BOOLEAN bShow, UINT uDisplayCandleIndex)
{
    const CToolTip& cToolTip = m_cChart.GetToolTip();
    FLOAT fY = cToolTip.GetWorldPosition().y + cToolTip.GetWorldSize().y;
    UINT uIndex = 0;
    for ( ; MAX_ATTACHED_CHARTS > uIndex; ++uIndex) {
        CBaseChart* pcChart = m_apcAttachedChart[uIndex];
        if (NULL == pcChart) {
            continue;
        }
        fY += 5.0f;
        pcChart->OnDisplayToolTip(fY, bShow, uDisplayCandleIndex);
    }
}

VOID CTradingMainWindow::TryChangeSource(ESOURCE_MARKET eSource)
{
    if (ESOURCE_MARKET_BINANCE == eSource) {
        CBinanceMgr& cMgr = CBinanceMgr::GetInstance();
        if (cMgr.AreTradePairsQueried()) {
            OnBinanceTradePairsQuery(TRUE);
            return;
        }
        CEventManager::RegisterForBroadcast(*this, EGLOBAL_BINANCE_MGR, EGLOBAL_EVT_BINANCE_TRADE_PAIR_UPDATE, TRUE);
        CNowLoadingWindow::DisplayWindow(0xFFFFFFFF, "Querying Trade Pairs");
        cMgr.QueryTradePairs();
    }
    else if (ESOURCE_MARKET_OANDA == eSource) {
        COandaMgr& cMgr = COandaMgr::GetInstance();
        if (cMgr.IsSignedIn()) {
            OnOandaSignInReply(TRUE);
            return;
        }
        CEventManager::RegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_SIGN_IN, TRUE);
        CNowLoadingWindow::DisplayWindow(0xFFFFFFFF, "Signing in...");
        cMgr.TrySignIn();
    }
    else if (ESOURCE_MARKET_CRYPTO_COM == eSource) { // if crypto.com
        CCryptoDotComMgr& cMgr = CCryptoDotComMgr::GetInstance();
        if (!cMgr.IsMarketSocketConnected()) { // market socket not yet connected
            CEventManager::RegisterForBroadcast(*this, EGLOBAL_CRYPTO_DOT_COM_MGR, EGLOBAL_EVT_CRYPTO_MARKET_SOCKET_CONNECT_REPLY, TRUE);
            CNowLoadingWindow::DisplayWindow(0xFFFFFFFF, "Connecting to Crypto.com");
            cMgr.ConnectMarketSocket();
            return;
        }
        if (cMgr.AreTradePairsQueried()) { // trading pairs not yet queried
            OnCryptoDotComTradePairUpdate(TRUE);
            return;
        }
        CEventManager::RegisterForBroadcast(*this, EGLOBAL_CRYPTO_DOT_COM_MGR, EGLOBAL_EVT_CRYPTO_TRADE_PAIR_UPDATE, TRUE);
        CNowLoadingWindow::DisplayWindow(0xFFFFFFFF, "Querying Trade Pairs");
        cMgr.QueryTradePairs();
    }
}
