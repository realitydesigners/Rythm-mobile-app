#include "stdafx.h"
#include "TextInputWindow.h"
#include "Event.h"
#include "CMain.h"
#include "EventManager.h"

#define WINDOW_WIDTH (600.0f)
#define WINDOW_HEIGHT (270.0f)

CTextInputWindow::CTextInputWindow(CHAR* szStringBuffer, UINT uMaxLen, CEventHandler& cHandler, const CEvent& cEvent, EInputType eType) :
CGameWindow(),
INITIALIZE_TEXT_LABEL(m_cTitleText),
INITIALIZE_TEXT_LABEL(m_cActionText),
m_szStringBuffer(szStringBuffer),
m_uMaxLen(uMaxLen),
m_cHandler(cHandler),
m_eInputType(eType),
m_bAllowQuit(TRUE),
m_pcTextBGWidget(NULL)
{
    m_cEvent = cEvent;
    m_bRenderBlackOverlay = TRUE;
    m_bDestroyOnExitModal = TRUE;
}

CTextInputWindow::~CTextInputWindow()
{
}

VOID CTextInputWindow::SetTitleAndActionText(const CHAR* szTitle, const CHAR* szAction)
{
    m_cTitleText.SetString(szTitle);
    m_cActionText.SetString(szAction);
}

VOID CTextInputWindow::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();
    
    // setup background of window
    SetDefaultWindow(WINDOW_WIDTH, WINDOW_HEIGHT, m_cTitleText.GetString());
    SetAnchor(0.5f, 0.5f);
    SetLocalPosition(CMain::GetScreenWidth() * 0.5f, CMain::GetScreenHeight() * 0.25f);
    if (m_bAllowQuit) {
        CreateColorButton(WINDOW_WIDTH - 80.0f, 10.0f, 60.0f, 60.0f, EBTN_EXIT, RGBA(0x2F, 0x2F, 0x2F, 0xFF), "X", TRUE);
    }
    
    CUIWidget* pcWidget = Create9PartImage(ETEX::ID::ui_9part_filter_off, 400.0f, 80.0f);
    if (NULL != pcWidget) {
        pcWidget->SetColor(RGBA(0, 0, 0, 0xFF));
        pcWidget->SetAnchor(0.5f, 0.0f);
        pcWidget->SetLocalPosition(WINDOW_WIDTH * 0.5f, 70.0f);
        AddChild(*pcWidget);
        m_pcTextBGWidget = pcWidget;
    }
    CUIButton* pcBtn = CreateColorButton(WINDOW_WIDTH * 0.5f, 180.0f, 200.0f, 60.0f, EBTN_DONE, RGBA(0x2F, 0x2F, 0x2F, 0xFF), "Done", TRUE);
    if (NULL != pcBtn) {
        pcBtn->SetAnchor(0.5f, 0.0f);
    }
    UpdateWorldCoordinates(TRUE);
}

VOID CTextInputWindow::PostDoModal(VOID)
{
    if (NULL == m_pcTextBGWidget) {
        ASSERT(FALSE);
        return;
    }
    const STextFontInfo sFont = {
        EGAMEFONT_SIZE_30,  //        EGAMEFONT       eFontSize;
        0xFFFFFFFF,         //        UINT            uFontColor;
        -2.0f,              //        FLOAT           fStrokeSize;
        RGBA(0, 0, 0, 0xFF) //        UINT            uStrokeColor;
    };
    const SHLVector2D& sWorldPos = m_pcTextBGWidget->GetWorldPosition();
    const SHLVector2D& sWorldSize = m_pcTextBGWidget->GetWorldSize();
    CTextInputManager::PromptInputForEdit(sFont, m_szStringBuffer, sWorldPos.x, sWorldPos.y, sWorldSize.x, sWorldSize.y, m_uMaxLen, m_eInputType);
    
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_TEXT_INPUT_MGR, EGLOBAL_EVENT_TEXT_INPUT_COMPLETED);
}

VOID CTextInputWindow::OnBackBtnPressed(VOID)
{
    if (m_bAllowQuit) {
        CEvent cExit(EBTN_EXIT);
        PostEvent(cExit, 0.0f);
    }
}

VOID CTextInputWindow::Release(VOID)
{
    CTextInputManager::CancelKeyboardInput();
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_TEXT_INPUT_MGR, EGLOBAL_EVENT_TEXT_INPUT_COMPLETED);

    m_cTitleText.ClearString();
    m_cActionText.ClearString();
    
    CGameWindow::Release();
}

VOID CTextInputWindow::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_EXIT:
            if (m_bAllowQuit) {
                ExitModal();
            }
            break;
        case EBTN_DONE:
        case EGLOBAL_EVENT_TEXT_INPUT_COMPLETED:
            OnInputCompleted();
            break;
        default:
            break;
    }
}

VOID CTextInputWindow::OnInputCompleted(VOID)
{
    if (NULL == m_szStringBuffer) {
        ASSERT(FALSE);
        return;
    }
    const CHAR* szNewString = CTextInputManager::GetText();
    if (NULL == szNewString || 0 == strlen(szNewString)) {
        ExitModal();
        return;
    }
    const CHAR* szOldString = m_szStringBuffer;
    if (NULL != szOldString && 0 == strcmp(szOldString, szNewString)) {
        ExitModal();
        return;
    }
    strncpy(m_szStringBuffer, szNewString, m_uMaxLen);
    m_cHandler.OnReceiveEvent(m_cEvent);
    ExitModal();
}

