#include "stdafx.h"
#include "TradeChart.h"
#include "CMain.h"
#include "Event.h"
#include "EventID.h"
#include "EventManager.h"
#include "MACDDetails.h"
#include "MADetails.h"
#include "PlayerData.h"
#include "SaveData.h"
#include "TextInputWindow.h"
#include "TradeOverlayWindow.h"
#include "TradeSubChartWindow.h"

#define SELECTED_COLOR      RGBA(0x1F,0x1F,0xFF,0xFF)
#define UNSELECTED_COLOR    RGBA(0x7F, 0x7F, 0x7F, 0xFF)

#define CREATE_BTN(x, y, w, h, id, text) CreateColorButton(x, y, w, h, id, UNSELECTED_COLOR, text, TRUE)

#define WIN_H (130.0f) // 10 + 50 + 50 + 10 + 10

CTradeOverlayWindow::CTradeOverlayWindow(CTradeChart& cChart) :
CGameWindow(),
m_cChart(cChart)
{
    m_bDestroyOnExitModal = TRUE;
}

CTradeOverlayWindow::~CTradeOverlayWindow()
{
}
VOID CTradeOverlayWindow::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();
    const FLOAT fW = CMain::GetScreenWidth() - 2.0f * CMain::GetDisplaySideMargin();
    SetLocalPosition(CMain::GetDisplaySideMargin(), 0.0f);
    SetLocalSize(fW, WIN_H);
    m_cBG.Set9PartTexture(ETEX::ID::ui_9part_filter_off, 3.0f, 3.0f, 3.0f, 3.0f);
    m_cBG.SetLocalSize(fW, WIN_H);
    AddChild(m_cBG);
    
    CREATE_BTN((fW - 60.0f), 10.0f, 50.0f, 50.0f, EBTN_EXIT, "Quit");
    
    FLOAT fX = 10.0f;
    FLOAT fY = 10.0f;
    
    CREATE_BTN(fX, fY, 70.0f, 50.0f, EBTN_TOGGLE_CANDLE, "Candle Toggle");      fX += 80.0f;
    CREATE_BTN(fX, fY, 70.0f, 50.0f, EBTN_SUB_CHART_UI,  "Sub Charts");         fX += 80.0f;
    
    fX += 10.0f;
    CREATE_BTN(fX, fY, 70.0f, 50.0f, EBTN_TOGGLE_BB,            "BB Toggle");           fX += 80.0f;
    CREATE_BTN(fX, fY, 70.0f, 50.0f, EBTN_BB_CHANGE_INTERVAL,   "BB\nInterval");        fX += 80.0f;
    CREATE_BTN(fX, fY, 70.0f, 50.0f, EBTN_BB_CHANGE_MULTIPLIER, "BB\nMultiplier");      fX += 80.0f;

    // end line
    fY += 60.0f;
    fX = 10.0f;
    CREATE_BTN(fX, fY, 70.0f, 50.0f, EBTN_TOGGLE_MA, "MA Toggle");      fX += 80.0f;
    CREATE_BTN(fX, fY, 70.0f, 50.0f, EBTN_MA_DETAILS, "Details");       fX += 80.0f;
}

VOID CTradeOverlayWindow::Release(VOID)
{
    m_cBG.RemoveFromParent();
    CGameWindow::Release();
}

VOID CTradeOverlayWindow::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_EXIT:
            ExitModal();
            return;
        case EBTN_TOGGLE_CANDLE:
            OnToggleCandle();
            break;
        case EBTN_SUB_CHART_UI:
            OnPressSubChartUI();
            return;
            
        case EBTN_TOGGLE_MA:
            OnToggleMA();
            break;
        case EBTN_MA_DETAILS:
            OnPressMADetails();
            return;

        case EBTN_TOGGLE_BB:
            OnToggleBB();
            break;
        case EBTN_BB_CHANGE_INTERVAL:
            OnPressBBInterval();
            break;
        case EBTN_BB_CHANGE_MULTIPLIER:
            OnPressBBMultiplier();
            break;
        case EEVENT_ON_TEXT_INPUT_COMPLETE:
            OnTextInputComplete(cEvent.GetIntParam(1), cEvent.GetIntParam(2));
            break;
        
            
        default:
            break;
    }
}

VOID CTradeOverlayWindow::OnToggleCandle(VOID)
{
    m_cChart.ToggleCandle();
}

VOID CTradeOverlayWindow::OnPressSubChartUI(VOID)
{
    CTradeSubChartWindow* pcWin = new CTradeSubChartWindow();
    if (NULL != pcWin) {
        pcWin->DoModal();
        ExitModal();
    }
}

VOID CTradeOverlayWindow::OnToggleMA(VOID)
{
    m_cChart.ToggleMAChart();
}
VOID CTradeOverlayWindow::OnPressMADetails(VOID)
{
    CMADetails* pcWin = new CMADetails(m_cChart);
    if (NULL != pcWin) {
        pcWin->DoModal();
        ExitModal();
    }
}

VOID CTradeOverlayWindow::OnToggleBB(VOID)
{
    CPlayerData::SetDisplayBollingerBands(!CPlayerData::IsDisplayBollingerBands());
    CSaveData::SaveData(FALSE);
    CEventManager::PostGlobalEvent(EGLOBAL_TRADE_MAIN_CHART, CEvent(EGLOBAL_EVT_TRADE_CHART_REFRESH_BB));
}

VOID CTradeOverlayWindow::OnPressBBInterval(VOID)
{
    snprintf(m_szBuffer, 64, "%d", CPlayerData::GetBollingerBands().uInterval);
    CEvent cEvent(EEVENT_ON_TEXT_INPUT_COMPLETE, EBTN_BB_CHANGE_INTERVAL);
    CTextInputWindow* pcWin = new CTextInputWindow(m_szBuffer, 64, *this, cEvent, EInputType_Numeric);
    if (NULL != pcWin) {
        pcWin->DoModal();
    }
}
VOID CTradeOverlayWindow::OnPressBBMultiplier(VOID)
{
    snprintf(m_szBuffer, 64, "%.02f", CPlayerData::GetBollingerBands().fDeviation);
    CEvent cEvent(EEVENT_ON_TEXT_INPUT_COMPLETE, EBTN_BB_CHANGE_MULTIPLIER);
    CTextInputWindow* pcWin = new CTextInputWindow(m_szBuffer, 64, *this, cEvent, EInputType_Decimal);
    if (NULL != pcWin) {
        pcWin->DoModal();
    }
}


VOID CTradeOverlayWindow::OnTextInputComplete(UINT uPrevEventID, UINT uParam1)
{
    if (0 == strlen(m_szBuffer)) {
        return;
    }
    switch (uPrevEventID) {
        case EBTN_BB_CHANGE_INTERVAL:
            {
                const INT nInterval = atoi(m_szBuffer);
                if (0 >= nInterval || 5000 < nInterval) { // safety
                    return;
                }
                CPlayerData::SetBollingerBands(nInterval, CPlayerData::GetBollingerBands().fDeviation);
                CSaveData::SaveData(FALSE);
                CEventManager::PostGlobalEvent(EGLOBAL_TRADE_MAIN_CHART, CEvent(EGLOBAL_EVT_TRADE_CHART_REFRESH_BB));
            }
            break;
        case EBTN_BB_CHANGE_MULTIPLIER:
            {
                const FLOAT fDeviation = atof(m_szBuffer);
                if (0.0f == fDeviation) {
                    return;
                }
                CPlayerData::SetBollingerBands(CPlayerData::GetBollingerBands().uInterval, fDeviation);
                CSaveData::SaveData(FALSE);
                CEventManager::PostGlobalEvent(EGLOBAL_TRADE_MAIN_CHART, CEvent(EGLOBAL_EVT_TRADE_CHART_REFRESH_BB));
            }
            break;
        default:
            ASSERT(FALSE);
            break;
    }
}
