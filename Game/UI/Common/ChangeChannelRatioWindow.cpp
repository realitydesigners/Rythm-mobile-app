#include "stdafx.h"
#include "ChangeChannelRatioWindow.h"
#include "CMain.h"
#include "ConfirmationWindow.h"
#include "Event.h"
#include "EventManager.h"
#include "PlayerData.h"
#include "SaveData.h"
#include "TextInputWindow.h"

#define BUTTON_W        (60.0f)
#define BUTTON_PADDING  (10.0f)

#define WINDOW_WIDTH  (BUTTON_PADDING * 10 + BUTTON_W * 9)
#define WINDOW_HEIGHT (200.0f)

CChangeChannelRatioWindow::CChangeChannelRatioWindow() :
CGameWindow(),
m_bContentsChanged(FALSE)
{
    m_bRenderBlackOverlay = TRUE;
    m_bDestroyOnExitModal = TRUE;
    m_eDoModalEffect = EPOP_SLIDE_VERTICAL;
    memset(m_apcTitles, 0, sizeof(m_apcTitles));
    memcpy(m_aushRatios, CPlayerData::GetChannelRatios(), sizeof(m_aushRatios));
}

CChangeChannelRatioWindow::~CChangeChannelRatioWindow()
{
}

VOID CChangeChannelRatioWindow::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();
    SetDefaultWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Channel Ratio");
    SetAnchor(0.5f, 0.5f);
    SetLocalPosition(CMain::GetScreenWidth() * 0.5f, CMain::GetScreenHeight() * 0.5f);
    
    FLOAT fX = (WINDOW_WIDTH - (8 * BUTTON_PADDING) - 9 * BUTTON_W) * 0.5f;
    CHAR szBuffer[64];
    for (UINT uIndex = 0; 9 > uIndex; ++uIndex) {
        snprintf(szBuffer, 64, "D%d", 8 - uIndex + 1);
        CUITextLabel* pcLbl = CreateLabel(szBuffer, ELABEL_COMMON_BUTTON_LABEL);
        if (NULL != pcLbl) {
            pcLbl->SetFont(EGAMEFONT_SIZE_16);
            pcLbl->SetLocalPosition(fX + BUTTON_W * 0.5f, 60.0f);
            pcLbl->SetColor(RGBA(0x9F, 0x9F, 0x9F, 0xFF));
            AddChild(*pcLbl);
        }
        m_apcTitles[uIndex] = pcLbl;
        
        snprintf(szBuffer, 64, "%d", m_aushRatios[uIndex]);
        m_apcBtns[uIndex] = CreateColorButton(fX, 75.0f, BUTTON_W, 50.0f, EBTN_CHANGE_00 + uIndex, RGBA(0x1F, 0x1F, 0x1F, 0xFF), szBuffer, TRUE);
        
        fX += (BUTTON_PADDING + BUTTON_W);
    }
    const FLOAT fCloseX = (WINDOW_WIDTH - 100.0f) * 0.5f;
    CreateColorButton(fCloseX, 140.0f, 100.0f, 50.0f, EBTN_EXIT, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "Done", TRUE);
}

VOID CChangeChannelRatioWindow::Release(VOID)
{
    for (UINT uIndex = 0; 9 > uIndex; ++uIndex) {
        if (NULL != m_apcTitles[uIndex]) {
            m_apcTitles[uIndex]->Release();
            m_apcTitles[uIndex]->RemoveFromParent();
        }
    }
    CGameWindow::Release();
}

VOID CChangeChannelRatioWindow::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_EXIT:
            DoExit();
            break;
        case EBTN_CHANGE_00:
        case EBTN_CHANGE_01:
        case EBTN_CHANGE_02:
        case EBTN_CHANGE_03:
        case EBTN_CHANGE_04:
        case EBTN_CHANGE_05:
        case EBTN_CHANGE_06:
        case EBTN_CHANGE_07:
        case EBTN_CHANGE_08:
            OnPressButton(uEventID - EBTN_CHANGE_00);
            break;
        case EEVENT_TEXT_INPUT_REPLY:
            OnTextInputReply(cEvent.GetIntParam(1));
            break;
        default:
            break;
    }
}

VOID CChangeChannelRatioWindow::OnBackBtnPressed(VOID)
{
    CEvent cEvent(EBTN_EXIT);
    OnReceiveEvent(cEvent);
}

VOID CChangeChannelRatioWindow::OnPressButton(UINT uIndex)
{
    snprintf(m_szBuffer, 64, "%d", m_aushRatios[uIndex]);
    CTextInputWindow* pcWin = new CTextInputWindow(m_szBuffer, 64, *this, CEvent(EEVENT_TEXT_INPUT_REPLY, uIndex), EInputType_Numeric);
    if (NULL != pcWin) {
        pcWin->DoModal();
    }
}

VOID CChangeChannelRatioWindow::OnTextInputReply(UINT uIndex)
{
    if (9 <= uIndex) {
        ASSERT(FALSE);
        return;
    }
    const UINT uValue = atoi(m_szBuffer);
    if (0 == uValue || 1000 < uValue) {
        CConfirmationWindow::DisplayErrorMsg("Number should be 1-1000", NULL, 0);
        return;
    }
    if (uValue == m_aushRatios[uIndex]) {
        return;
    }
    m_bContentsChanged = TRUE;
    m_aushRatios[uIndex] = uValue;
    if (NULL != m_apcBtns[uIndex]) {
        CUITextLabel* pcLbl = CUITextLabel::CastToMe(m_apcBtns[uIndex]->GetChild());
        if (NULL != pcLbl) {
            snprintf(m_szBuffer, 64, "%d", uValue);
            pcLbl->SetString(m_szBuffer);
        }
    }
}

VOID CChangeChannelRatioWindow::DoExit(VOID)
{
    if (m_bContentsChanged) {
        CPlayerData::SetChannelRatios(m_aushRatios);
        CSaveData::SaveData(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_CHANGE_CHANNEL_SIZE_WINDOW, CEvent(EGLOABL_EVT_CHANGE_CHANNEL_SIZE_WINDOW_ON_CHANGED));
    }
    ExitModal();
}
