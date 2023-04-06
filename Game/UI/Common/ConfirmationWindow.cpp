#include "stdafx.h"
#include "ConfirmationWindow.h"
#include "Event.h"
#include "CMain.h"
#include "EventManager.h"
#include "MessageData.h"

VOID CConfirmationWindow::DisplayErrorMsg(const CHAR* szMessage, CEventHandler* pcHandler, UINT uCloseEventID)
{
    CConfirmationWindow* pcWin = new CConfirmationWindow(pcHandler);
    if (NULL == pcWin) {
        ASSERT(FALSE);
        return;
    }
    pcWin->SetTitle(CMessageData::GetMsgID(MSG_ERROR_OCCURRED));
    pcWin->SetMessage(szMessage);
    pcWin->SetOKButton(CMessageData::GetMsgID(MSG_OK), uCloseEventID);
    pcWin->HideCancelButton();
    pcWin->DoModal();
}
#define WINDOW_WIDTH (600.0f)
#define WINDOW_HEIGHT (400.0f)

CConfirmationWindow::CConfirmationWindow(CEventHandler* pcHandler) :
CGameWindow(),
INITIALIZE_TEXT_LABEL(m_cTitle),
INITIALIZE_TEXT_LABEL(m_cMessage),
INITIALIZE_TEXT_LABEL(m_cOK),
INITIALIZE_TEXT_LABEL(m_cCancel),
m_bHideCancel(FALSE),
m_bDisplayUpperRightCancel(FALSE),
m_nOKID(-1),
m_nCancelID(-1),
m_pcHandler(pcHandler)
{
    m_bRenderBlackOverlay = TRUE;
    m_bDestroyOnExitModal = TRUE;
    m_eDoModalEffect = EPOP_SLIDE_VERTICAL;
}

CConfirmationWindow::~CConfirmationWindow()
{
}

VOID CConfirmationWindow::SetTitle(const CHAR* szTitle)
{
    m_cTitle.SetString(szTitle);
}
VOID CConfirmationWindow::SetMessage(const CHAR* szMessage)
{
    m_cMessage.SetString(szMessage);
}
VOID CConfirmationWindow::SetOKButton(const CHAR* szOK, INT nEventID)
{
    m_cOK.SetString(szOK);
    m_nOKID = nEventID;
}
VOID CConfirmationWindow::SetCancelButton(const CHAR* szCancel, INT nEventID)
{
    m_cCancel.SetString(szCancel);
    m_nCancelID = nEventID;
}

#define CONFIRMATION_BORDER_SIZE (30.0f)
#define CONFIRMATION_BTN_SPACING (20.0f)
VOID CConfirmationWindow::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();
    
    FLOAT fW = 0.0f;
    FLOAT fH = CONFIRMATION_BORDER_SIZE;
    if (NULL != m_cTitle.GetString()) {
        m_cTitle.SetFont(EGAMEFONT_SIZE_24);
        m_cTitle.Commit(); // call commit will get the size of the textlabel from the string
        const SHLVector2D& sSize = m_cTitle.GetLocalSize();
        fW = sSize.x;
        fH += 10.0f + sSize.y;
    }
    if (NULL != m_cMessage.GetString()) {
        m_cMessage.SetFont(EGAMEFONT_SIZE_20);
        m_cMessage.Commit(); // call commit will get the size of the textlabel from the string
        const SHLVector2D& sSize = m_cMessage.GetLocalSize();
        if (sSize.x > fW) {
            fW = sSize.x;
        }
        fH += 10.0f + sSize.y;
    }
    if (NULL == m_cOK.GetString()) {
        m_cOK.SetString("OK");
    }
    m_cOK.SetFont(EGAMEFONT_SIZE_20);
    m_cOK.SetAnchor(0.5f, 0.5f);
    m_cOK.Commit();
    SHLVector2D sBtnSize = m_cOK.GetLocalSize();
    if (!m_bHideCancel) {
        if (NULL == m_cCancel.GetString()) {
            m_cCancel.SetString("Cancel");
        }
        m_cCancel.SetFont(EGAMEFONT_SIZE_20);
        m_cCancel.SetAnchor(0.5f, 0.5f);
        m_cCancel.Commit();
        const SHLVector2D& sCancelSize = m_cCancel.GetLocalSize();
        if (sBtnSize.x < sCancelSize.x) {
            sBtnSize.x = sCancelSize.x;
        }
        if (sBtnSize.y < sCancelSize.y) {
            sBtnSize.y = sCancelSize.y;
        }
    }
    if (sBtnSize.x < 50.0f) {
        sBtnSize.x = 50.0f;
    }
    if (sBtnSize.y < 50.0f) {
        sBtnSize.y = 50.0f;
    }
    FLOAT fBtnTotalW = sBtnSize.x;
    if (!m_bHideCancel) {
        fBtnTotalW += sBtnSize.x + CONFIRMATION_BTN_SPACING;
    }
    if (fW < fBtnTotalW) {
        fW = fBtnTotalW;
    }
    fW += 2.0f * CONFIRMATION_BORDER_SIZE;
    fH += sBtnSize.y + CONFIRMATION_BORDER_SIZE;
        
    SetLocalSize(fW, fH);
    SetAnchor(0.5f, 0.5f);
    SetLocalPosition(CMain::GetScreenWidth() * 0.5f, CMain::GetScreenHeight() * 0.5f);

    m_cBG.Set9PartTexture(ETEX::ID::ui_9part_filter_off);
    m_cBG.SetColor(RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    m_cBG.SetLocalSize(fW, fH);
    AddChild(m_cBG);

    FLOAT fY = CONFIRMATION_BORDER_SIZE;
    if (NULL != m_cTitle.GetString()) {
        m_cTitle.SetAnchor(0.5f, 0.0f);
        m_cTitle.SetLocalPosition(fW * 0.5f, fY);
        AddChild(m_cTitle);
        fY += m_cTitle.GetLocalSize().y + 10.0f;
    }
    if (NULL != m_cMessage.GetString()) {
        m_cMessage.SetAnchor(0.5f, 0.0f);
        m_cMessage.SetLocalPosition(fW * 0.5f, fY);
        AddChild(m_cMessage);
        fY += m_cMessage.GetLocalSize().y + 10.0f;
    }
    FLOAT fBtnX = (fW - fBtnTotalW) * 0.5f;
    CUIWidget* pcBtnWidget = CreateColorImage(RGBA(0x7F, 0x7F, 0x7F, 0xFF), sBtnSize.x, sBtnSize.y);
    if (NULL == pcBtnWidget) {
        ASSERT(FALSE);
        return;
    }
    if (!m_bHideCancel) {
        CUIButton* pcBtn = CreateButton(fBtnX, fY, EBTN_CANCEL, *pcBtnWidget, *pcBtnWidget, TRUE);
        if (NULL == pcBtn) {
            ASSERT(FALSE);
            return;
        }
        m_cCancel.SetLocalPosition(sBtnSize.x * 0.5f, sBtnSize.y * 0.5f);
        pcBtn->AddChild(m_cCancel);
        fBtnX += sBtnSize.x + CONFIRMATION_BTN_SPACING;
    }
    CUIButton* pcBtn = CreateButton(fBtnX, fY, EBTN_OK, *pcBtnWidget, *pcBtnWidget, TRUE);
    if (NULL == pcBtn) {
        ASSERT(FALSE);
        return;
    }
    m_cOK.SetLocalPosition(sBtnSize.x * 0.5f, sBtnSize.y * 0.5f);
    pcBtn->AddChild(m_cOK);
    
    if (m_bDisplayUpperRightCancel) {
        pcBtn = CreateButton(fW - sBtnSize.x - 10.0f, 10.0f, EBTN_DEFAULT_CANCEL, *pcBtnWidget, *pcBtnWidget, TRUE);
        if (NULL == pcBtn) {
            ASSERT(FALSE);
            return;
        }
    }
}

VOID CConfirmationWindow::Release(VOID)
{
    m_cOK.ClearString();
    m_cOK.RemoveFromParent();
    m_cCancel.ClearString();
    m_cCancel.RemoveFromParent();
    
    m_cMessage.ClearString();
    m_cMessage.RemoveFromParent();
    
    m_cTitle.ClearString();
    m_cTitle.RemoveFromParent();
    
    m_cBG.RemoveFromParent();
    
    CGameWindow::Release();
}

VOID CConfirmationWindow::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_OK:
            if (NULL != m_pcHandler && -1 != m_nOKID) {
                CEventManager::PostEvent(*m_pcHandler, CEvent(m_nOKID));
            }
            break;
        case EBTN_CANCEL:
            if (NULL != m_pcHandler && -1 != m_nCancelID) {
                CEventManager::PostEvent(*m_pcHandler, CEvent(m_nCancelID));
            }
            break;
        case EBTN_DEFAULT_CANCEL:
            break;
        default:
            break;
    }
    ExitModal();
}

