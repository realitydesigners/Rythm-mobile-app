#include "stdafx.h"
#include "CMain.h"
#include "Event.h"
#include "EventID.h"
#include "EventManager.h"
#include "MACDDetails.h"
#include "OBVDetails.h"
#include "PlayerData.h"
#include "SaveData.h"
#include "TextInputWindow.h"
#include "TradeSubChartWindow.h"

#define UNSELECTED_COLOR    RGBA(0x7F, 0x7F, 0x7F, 0xFF)

#define CREATE_BTN(x, y, w, h, id, text) CreateColorButton(x, y, w, h, id, UNSELECTED_COLOR, text, TRUE)

#define WIN_H (130.0f) // 10 + 50 + 50 + 10 + 10

CTradeSubChartWindow::CTradeSubChartWindow() :
CGameWindow()
{
    m_bDestroyOnExitModal = TRUE;
}

CTradeSubChartWindow::~CTradeSubChartWindow()
{
}
VOID CTradeSubChartWindow::InitializeInternals(VOID)
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
    CREATE_BTN(fX, fY, 70.0f, 50.0f, EBTN_TOGGLE_MACD,      "Toggle MACD");        fY += 60.0f;
    CREATE_BTN(fX, fY, 70.0f, 50.0f, EBTN_MACD_DETAILS,     "Details");
    
    fY = 10.0f;
    fX += 80.0f;
    CREATE_BTN(fX, fY, 70.0f, 50.0f, EBTN_TOGGLE_OBV,       "Toggle OBV");         fY += 60.0f;
    CREATE_BTN(fX, fY, 70.0f, 50.0f, EBTN_OBV_DETAILS,      "Details");

    fY = 10.0f;
    fX += 80.0f;
    CREATE_BTN(fX, fY, 70.0f, 50.0f, EBTN_TOGGLE_CMO,       "Toggle CMO");         fY += 60.0f;
    CREATE_BTN(fX, fY, 70.0f, 50.0f, EBTN_CHANGE_CMO_VALUE, "CMO Change");
 
    fY = 10.0f;
    fX += 80.0f;
    CREATE_BTN(fX, fY, 70.0f, 50.0f, EBTN_TOGGLE_RSI,       "Toggle RSI");         fY += 60.0f;
    CREATE_BTN(fX, fY, 70.0f, 50.0f, EBTN_CHANGE_RSI_VALUE, "RSI Change");

}

VOID CTradeSubChartWindow::Release(VOID)
{
    m_cBG.RemoveFromParent();
    CGameWindow::Release();
}

VOID CTradeSubChartWindow::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_EXIT:
            ExitModal();
            return;
        case EBTN_TOGGLE_CMO:
            CEventManager::PostGlobalEvent(EGLOBAL_TRADE_MAIN_WINDOW, CEvent(EGLOBAL_EVT_TRADE_MAIN_WINDOW_TOGGLE_ATTACH_CHART, EGLOBAL_CMO_CHART));
            break;
        case EBTN_TOGGLE_MACD:
            CEventManager::PostGlobalEvent(EGLOBAL_TRADE_MAIN_WINDOW, CEvent(EGLOBAL_EVT_TRADE_MAIN_WINDOW_TOGGLE_ATTACH_CHART, EGLOBAL_MACD_CHART));
            break;
        case EBTN_TOGGLE_RSI:
            CEventManager::PostGlobalEvent(EGLOBAL_TRADE_MAIN_WINDOW, CEvent(EGLOBAL_EVT_TRADE_MAIN_WINDOW_TOGGLE_ATTACH_CHART, EGLOBAL_RSI_CHART));
            break;
        case EBTN_TOGGLE_OBV:
            CEventManager::PostGlobalEvent(EGLOBAL_TRADE_MAIN_WINDOW, CEvent(EGLOBAL_EVT_TRADE_MAIN_WINDOW_TOGGLE_ATTACH_CHART, EGLOBAL_OBV_CHART));
            break;
        case EBTN_CHANGE_CMO_VALUE:
            OnCMOValueChange();
            break;
        case EBTN_MACD_DETAILS:
            OnPressMACDDetails();
            break;
        case EBTN_CHANGE_RSI_VALUE:
            OnRSIValueChange();
            break;
        case EBTN_OBV_DETAILS:
            OnPressOBVDetails();
            break;
            
        case EEVENT_ON_TEXT_INPUT_COMPLETE:
            OnTextInputComplete(cEvent.GetIntParam(1), cEvent.GetIntParam(2));
            break;
        
        default:
            break;
    }
}

VOID CTradeSubChartWindow::OnCMOValueChange(VOID)
{
    snprintf(m_szBuffer, 64, "%d", CPlayerData::GetCMOInterval());
    CEvent cEvent(EEVENT_ON_TEXT_INPUT_COMPLETE, EBTN_CHANGE_CMO_VALUE);
    CTextInputWindow* pcWin = new CTextInputWindow(m_szBuffer, 64, *this, cEvent, EInputType_Numeric);
    if (NULL != pcWin) {
        pcWin->DoModal();
    }
}

VOID CTradeSubChartWindow::OnTextInputComplete(UINT uPrevEventID, UINT uParam1)
{
    if (0 == strlen(m_szBuffer)) {
        return;
    }
    switch (uPrevEventID) {
        case EBTN_CHANGE_CMO_VALUE:
            {
                const INT nInterval = atoi(m_szBuffer);
                if (0 >= nInterval || 5000 < nInterval) { // safety
                    return;
                }
                CPlayerData::SetCMOInterval(nInterval);
                CSaveData::SaveData(FALSE);
                CEventManager::PostGlobalEvent(EGLOBAL_TRADE_MAIN_WINDOW, CEvent(EGLOBAL_EVT_TRADE_MAIN_WINDOW_REFRESH_ATTACH_CHART, EGLOBAL_CMO_CHART));
            }
            break;
        case EBTN_CHANGE_RSI_VALUE:
            {
                const INT nInterval = atoi(m_szBuffer);
                if (0 >= nInterval || 5000 < nInterval) { // safety
                    return;
                }
                CPlayerData::SetRsiInterval(nInterval);
                CSaveData::SaveData(FALSE);
                CEventManager::PostGlobalEvent(EGLOBAL_TRADE_MAIN_WINDOW, CEvent(EGLOBAL_EVT_TRADE_MAIN_WINDOW_REFRESH_ATTACH_CHART, EGLOBAL_RSI_CHART));
            }
            break;

        default:
            ASSERT(FALSE);
            break;
    }
}

VOID CTradeSubChartWindow::OnPressMACDDetails(VOID)
{
    CMACDDetails* pcWin = new CMACDDetails();
    if (NULL != pcWin) {
        pcWin->DoModal();
        ExitModal();
    }
}
VOID CTradeSubChartWindow::OnRSIValueChange(VOID)
{
    snprintf(m_szBuffer, 64, "%d", CPlayerData::GetRsiInterval());
    CEvent cEvent(EEVENT_ON_TEXT_INPUT_COMPLETE, EBTN_CHANGE_RSI_VALUE);
    CTextInputWindow* pcWin = new CTextInputWindow(m_szBuffer, 64, *this, cEvent, EInputType_Numeric);
    if (NULL != pcWin) {
        pcWin->DoModal();
    }
}
VOID CTradeSubChartWindow::OnPressOBVDetails(VOID)
{
    COBVDetails* pcDetails = new COBVDetails();
    if (NULL != pcDetails) {
        pcDetails->DoModal();
        ExitModal();
    }
}
