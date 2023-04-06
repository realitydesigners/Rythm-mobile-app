#include "stdafx.h"
#if defined(DEBUG)
#include "MainDebug.h"
#include "CMain.h"
#include "Event.h"
#include "AppMain.h"
#include "QuitGameWindow.h"

#define DEBUG_WIDTH (800.0f)
#define DEBUG_HEIGHT (620.0f)

CMainDebug::CMainDebug() :
CGameWindow()
{
    m_bDestroyOnExitModal = TRUE;
}

CMainDebug::~CMainDebug()
{
}

#define CREATE_DEBUG_BTN(x, y, id, text) CreateColorButton(x, y, 100.0f, 50.0f, id, RGBA(0x7F, 0x7F, 0x7F, 0x7F), text, TRUE)
VOID CMainDebug::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();
    SetDefaultWindow(DEBUG_WIDTH, DEBUG_HEIGHT, "Main Debug");
    SetAnchor(0.5f, 0.5f);
    SetLocalPosition(CMain::GetScreenWidth() * 0.5f, CMain::GetScreenHeight() * 0.5f);
    CREATE_DEBUG_BTN(DEBUG_WIDTH - 110.0f, 10.0f, EBTN_EXIT, "Quit");

    CREATE_DEBUG_BTN(10.0f, 10.0f,  EBTN_TOGGLE_FPS,    "FPS Toggle");
    CREATE_DEBUG_BTN(120.0f, 10.0f, EBTN_PING,          "Kraken Ping");
}

VOID CMainDebug::Release(VOID)
{
    CGameWindow::Release();
}

VOID CMainDebug::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    CUIWindow* pcWin = NULL;
    switch (uEventID) {
        case EBTN_TOGGLE_FPS:
            CAppMain::ToggleFPS();
            break;
        case EBTN_EXIT:
            {
                CUIWindow* pcWin = new CQuitGameWindow();
                if (NULL != pcWin) {
                    pcWin->DoModal();
                }
            }
            break;
        case EBTN_PING:
            break;
            
        default:
            break;
    }
    if (NULL != pcWin) {
        pcWin->DoModal();
    }
}

VOID CMainDebug::OnBackBtnPressed(VOID)
{
    CEvent cEvent(EBTN_EXIT);
    OnReceiveEvent(cEvent);
}

#endif // #if defined(DEBUG)
