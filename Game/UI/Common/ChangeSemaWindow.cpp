#include "stdafx.h"
#include "ChangeSemaWindow.h"
#include "CMain.h"
#include "ConfirmationWindow.h"
#include "Event.h"
#include "EventManager.h"
#include "PlayerData.h"
#include "SaveData.h"
#include "TextInputWindow.h"
#include "ZZUtil.h"

#define BUTTON_W        (60.0f)
#define BUTTON_PADDING  (10.0f)

#define WINDOW_WIDTH  (800.0f)
#define WINDOW_HEIGHT (340.0f)

CChangeSemaWindow::CChangeSemaWindow(UINT uHandlerID, UINT uEventID) :
CGameWindow(EGLOBAL_CHANGE_SEMA_WINDOW),
m_uHandlerID(uHandlerID),
m_uEventID(uEventID)
{
    m_bRenderBlackOverlay = TRUE;
    m_bDestroyOnExitModal = TRUE;
    m_eDoModalEffect = EPOP_SLIDE_VERTICAL;
}

CChangeSemaWindow::~CChangeSemaWindow()
{
}

VOID CChangeSemaWindow::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();
    SetDefaultWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Semaphor");
    SetAnchor(0.5f, 0.5f);
    SetLocalPosition(CMain::GetScreenWidth() * 0.5f, CMain::GetScreenHeight() * 0.5f);
    const FLOAT fCloseX = (WINDOW_WIDTH - 200.0f) * 0.5f;
    CreateColorButton(fCloseX, WINDOW_HEIGHT - 10.0f - 50.0f, 200.0f, 50.0f, EBTN_EXIT, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "Done", TRUE);
    
    CHAR szBuffer[64];
    for (UINT uIndex = 0; 4 > uIndex; ++uIndex) {
        CColorPicker& cPicker = m_acPicker[uIndex];
        cPicker.SetLocalPosition(10.0f + uIndex * 200.0f, 50.0f);
        cPicker.Initialize(CPlayerData::GetSemaColor(uIndex));
        AddChild(cPicker);
        m_afSema[uIndex] = CPlayerData::GetSemaPercent(uIndex);
        snprintf(szBuffer, 64, "%.1f%%", m_afSema[uIndex]);
        m_apcBtns[uIndex] = CreateColorButton(60.0f + uIndex * 200.0f, 55.0f, 80.0f, 60.0f, EBTN_CHANGE_00 + uIndex, 0, szBuffer, TRUE);
    }
}

VOID CChangeSemaWindow::Release(VOID)
{
    for (UINT uIndex = 0; 4 > uIndex; ++uIndex) {
        m_acPicker[uIndex].Release();
        m_acPicker[uIndex].RemoveFromParent();
    }
    CGameWindow::Release();
}

VOID CChangeSemaWindow::OnReceiveEvent(CEvent& cEvent)
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
            OnPressButton(uEventID - EBTN_CHANGE_00);
            break;
        case EEVENT_TEXT_INPUT_REPLY:
            OnTextInputReply(cEvent.GetIntParam(1));
            break;
        default:
            break;
    }
}

VOID CChangeSemaWindow::OnBackBtnPressed(VOID)
{
    CEvent cEvent(EBTN_EXIT);
    OnReceiveEvent(cEvent);
}

VOID CChangeSemaWindow::OnPressButton(UINT uIndex)
{
    if (4 <= uIndex) {
        ASSERT(FALSE);
        return;
    }
    snprintf(m_szBuffer, 64, "%.1f", m_afSema[uIndex]);
    CTextInputWindow* pcWin = new CTextInputWindow(m_szBuffer, 64, *this, CEvent(EEVENT_TEXT_INPUT_REPLY, uIndex), EInputType_Decimal);
    if (NULL != pcWin) {
        pcWin->DoModal();
    }
}

VOID CChangeSemaWindow::OnTextInputReply(UINT uIndex)
{
    if (4 <= uIndex) {
        ASSERT(FALSE);
        return;
    }
    const FLOAT fValue = (FLOAT)atof(m_szBuffer);
    if (1.0f > fValue || 100.0f < fValue) {
        CConfirmationWindow::DisplayErrorMsg("Number should be 1-100", NULL, 0);
        return;
    }
    m_afSema[uIndex] = fValue;
    if (NULL != m_apcBtns[uIndex]) {
        CUITextLabel* pcLbl = CUITextLabel::CastToMe(m_apcBtns[uIndex]->GetChild());
        if (NULL != pcLbl) {
            snprintf(m_szBuffer, 64, "%.1f%%", fValue);
            pcLbl->SetString(m_szBuffer);
        }
    }
}

VOID CChangeSemaWindow::DoExit(VOID)
{
    for (UINT uIndex = 0; 4 > uIndex; ++uIndex) {
        CPlayerData::SetSemaPercent(uIndex, m_afSema[uIndex]);
        CPlayerData::SetSemaColor(uIndex, m_acPicker[uIndex].GetColor());
    }
    CSaveData::SaveData(FALSE);
    ZZUtil::LoadZZInfo();
    CEventManager::PostGlobalEvent(m_uHandlerID, CEvent(m_uEventID));
    ExitModal();
}
