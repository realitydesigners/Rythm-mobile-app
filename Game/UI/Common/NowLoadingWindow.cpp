#include "stdafx.h"
#include "NowLoadingWindow.h"
#include "Event.h"
#include "CMain.h"
#include "MessageData.h"

#define FULL_ROTATION (360.0f)

CNowLoadingWindow::CNowLoadingWindow(UINT uMsgID, const CHAR* szMsg) :
CUIWindow(),
m_fRotationInterval(0.0f),
m_uMSGID(uMsgID),
INITIALIZE_TEXT_LABEL(m_cText)
{
    m_bDestroyOnExitModal = TRUE;
    if (0xFFFFFFFF == uMsgID && NULL != szMsg) {
        m_cText.SetString(szMsg);
    }
    else if (0xFFFFFFFF != uMsgID) {
        m_cText.SetString(CMessageData::GetMsgID(m_uMSGID));
    }
}

CNowLoadingWindow::~CNowLoadingWindow()
{
    
}


VOID CNowLoadingWindow::InitializeInternals(VOID)
{
    const UINT uScreenHeight = CMain::GetScreenHeight();
    const UINT uScreenWidth = CMain::GetScreenWidth();
    SetLocalSize((FLOAT)uScreenWidth, (FLOAT)uScreenHeight);
    
    m_cBG.Set9PartTexture(ETEX::ui_9part_filter_off);
    m_cBG.SetColor(RGBA(0xAF, 0xAF, 0xAF, 0xAF));
    m_cBG.SetLocalSize(200.0f, 200.0f);
    m_cBG.SetLocalPosition((uScreenWidth - 200.0f) * 0.5f, (uScreenHeight - 200.0f) * 0.5f);
    AddChild(m_cBG);
    
    if (NULL != m_cText.GetString()) {
        m_cText.SetFont(EGAMEFONT_SIZE_18);
        m_cText.SetAnchor(0.5f, 0.5f);
        m_cText.SetAlignment(CUITextLabel::EALIGNMENT_CENTER);
        m_cText.SetLocalPosition(uScreenWidth * 0.5f, uScreenHeight * 0.5f + 85.0f);
        AddChild(m_cText);
    }
    m_cLoadingIcon.SetTexture(ETEX::icon_spinner, TRUE);
    m_cLoadingIcon.SetAnchor(0.5f, 0.5f);
    m_cLoadingIcon.SetLocalPosition(uScreenWidth * 0.5f, uScreenHeight * 0.5f);
    AddChild(m_cLoadingIcon);
}

VOID CNowLoadingWindow::Release(VOID)
{
    m_cBG.RemoveFromParent();
    m_cText.ClearString();
    m_cText.RemoveFromParent();
    m_cLoadingIcon.RemoveFromParent();
}

// update function
VOID CNowLoadingWindow::OnUpdate(FLOAT fLapsed)
{
    m_fRotationInterval += fLapsed * FULL_ROTATION;
    if (FULL_ROTATION < m_fRotationInterval) {
        m_fRotationInterval -= FULL_ROTATION;
    }
    m_cLoadingIcon.SetRotation(m_fRotationInterval);
    CUIWindow::OnUpdate(fLapsed);
}

VOID CNowLoadingWindow::OnReceiveEvent(CEvent& cEvent)
{
    
}

VOID CNowLoadingWindow::OnBackBtnPressed(VOID)
{

}

static CNowLoadingWindow* s_pcLoadingWindow = NULL;

BOOLEAN CNowLoadingWindow::IsDisplayed(VOID)
{
    return (NULL != s_pcLoadingWindow);
}
VOID CNowLoadingWindow::DisplayWindow(UINT uMsgID, const CHAR* szMsg)
{
    if (NULL != s_pcLoadingWindow) {
        ASSERT(FALSE);
        return;
    }
    s_pcLoadingWindow = new CNowLoadingWindow(uMsgID, szMsg);
    if (NULL == s_pcLoadingWindow) {
        ASSERT(FALSE);
        return;
    }
    s_pcLoadingWindow->DoModal();
}

VOID CNowLoadingWindow::HideWindow(VOID)
{
    if (NULL == s_pcLoadingWindow) {
        TRACE("warn: already hidden loading window\n");
        return;
    }
    s_pcLoadingWindow->ExitModal();
    s_pcLoadingWindow = NULL;
}
