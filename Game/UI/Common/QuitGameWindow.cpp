#include "stdafx.h"
#include "QuitGameWindow.h"
#include "Event.h"
#include "CMain.h"


#define WINDOW_WIDTH (300.0f)
#define WINDOW_HEIGHT (140.0f)

CQuitGameWindow::CQuitGameWindow() :
CGameWindow()
{
    m_bRenderBlackOverlay = TRUE;
    m_bDestroyOnExitModal = TRUE;
    m_eDoModalEffect = EPOP_SLIDE_VERTICAL;
}

CQuitGameWindow::~CQuitGameWindow()
{
}

VOID CQuitGameWindow::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();
    
    // setup background of window
    SetDefaultWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Quit App?");
    SetAnchor(0.5f, 0.5f);
    SetLocalPosition(CMain::GetScreenWidth() * 0.5f, CMain::GetScreenHeight() * 0.5f);
    const FLOAT fX = 40.0f;
    CreateColorButton(fX,           60.0f, 100.0f, 60.0, EBTN_QUIT, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "Quit", TRUE);
    CreateColorButton(fX + 120.0f,  60.0f, 100.0f, 60.0, EBTN_EXIT, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "Back", TRUE);
}

VOID CQuitGameWindow::Release(VOID)
{
    CGameWindow::Release();
}

VOID CQuitGameWindow::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_EXIT:
            ExitModal();
            break;
        case EBTN_QUIT:
            CMain::ShutdownApp();
            break;
        default:
            break;
    }
}

