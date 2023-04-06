#include "stdafx.h"
#include "ColorPickerWindow.h"
#include "CMain.h"
#include "Event.h"
#include "EventManager.h"

#define BUTTON_W        (60.0f)
#define BUTTON_PADDING  (10.0f)

#define WINDOW_WIDTH  (400.0f)
#define WINDOW_HEIGHT (340.0f)

CColorPickerWindow::CColorPickerWindow(UINT uHandlerID, UINT uEventID, UINT uColor) :
CGameWindow(),
m_uHandlerID(uHandlerID),
m_uEventID(uEventID),
m_uColor(uColor)
{
    m_bRenderBlackOverlay = TRUE;
    m_bDestroyOnExitModal = TRUE;
    m_eDoModalEffect = EPOP_SLIDE_VERTICAL;
}

CColorPickerWindow::~CColorPickerWindow()
{
}

VOID CColorPickerWindow::SetGrayscaleMode(VOID)
{
    m_cPicker.SetGrayscaleMode();
}
VOID CColorPickerWindow::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();
    SetDefaultWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Change Color");
    SetAnchor(0.5f, 0.5f);
    SetLocalPosition(CMain::GetScreenWidth() * 0.5f, CMain::GetScreenHeight() * 0.5f);
    const FLOAT fCloseX = (WINDOW_WIDTH - 200.0f) * 0.5f;
    CreateColorButton(fCloseX, WINDOW_HEIGHT - 10.0f - 50.0f, 200.0f, 50.0f, EBTN_EXIT, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "Done", TRUE);
    
    m_cPicker.SetAnchor(0.5f, 0.0f);
    m_cPicker.SetLocalPosition(WINDOW_WIDTH * 0.5f, 40.0f);
    m_cPicker.Initialize(m_uColor);
    AddChild(m_cPicker);
}

VOID CColorPickerWindow::Release(VOID)
{
    m_cPicker.Release();
    m_cPicker.RemoveFromParent();
    CGameWindow::Release();
}

VOID CColorPickerWindow::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_EXIT:
            DoExit();
            break;
        default:
            break;
    }
}

VOID CColorPickerWindow::OnBackBtnPressed(VOID)
{
    CEvent cEvent(EBTN_EXIT);
    OnReceiveEvent(cEvent);
}


VOID CColorPickerWindow::DoExit(VOID)
{
    CEventManager::PostGlobalEvent(m_uHandlerID, CEvent(m_uEventID, m_cPicker.GetColor()));
    ExitModal();
}
