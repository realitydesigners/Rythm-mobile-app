#include "stdafx.h"
#include "EventID.h"
#include "EventManager.h"
#include "LogWidget.h"
#include "MessageLog.h"

#define CLOSED_W (100.0f)
#define CLOSED_H (20.0f)

#define LOG_BG_W (600.0f)
#define LOG_BG_H (400.0f)

#define LOG_WIDGET_BG_COLOR RGBA(0x1F, 0x1F, 0x1F, 0xFF)

CLogWidget::CLogWidget() :
CUIContainer(),
m_bRegistered(FALSE),
INITIALIZE_TEXT_LABEL(m_cLogTitle),
INITIALIZE_TEXT_LABEL(m_cClear),
m_bUITouched(FALSE)
{
    
}
    
CLogWidget::~CLogWidget()
{
}
 
VOID CLogWidget::Initialize(VOID)
{
    Release(); // safety
    m_cHeaderBG.SetLocalSize(CLOSED_W, CLOSED_H);
    m_cHeaderBG.SetColor(RGBA(0x4F, 0x4F, 0x4F, 0xFF));
    AddChild(m_cHeaderBG);

    m_cLogTitle.SetFont(EGAMEFONT_SIZE_14);
    m_cLogTitle.SetEffect(CUITextLabel::EFFECT_SHADOW, 1.0f, 1.0f);
    m_cLogTitle.SetAnchor(0.5f, 0.5f);
    m_cLogTitle.SetLocalPosition(CLOSED_W * 0.5f, CLOSED_H * 0.5f);
    AddChild(m_cLogTitle);
    
    m_cLogBG.SetColor(LOG_WIDGET_BG_COLOR);
    m_cLogBG.SetBorderColor(RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    const FLOAT fX = CLOSED_W - LOG_BG_W;
    const FLOAT fY = CLOSED_H + 1.0f;
    m_cLogBG.SetLocalPosition(fX, fY);
    m_cLogBG.SetLocalSize(LOG_BG_W, LOG_BG_H);
    m_cList.SetLocalPosition(fX, fY);
    m_cList.SetLocalSize(LOG_BG_W, LOG_BG_H);
    UpdateTitle();
    
    m_cClear.SetFont(EGAMEFONT_SIZE_14);
    m_cClear.SetAnchor(0.5f, 0.5f);
    m_cClear.SetString("Clear");
    m_cClear.SetLocalPosition(30.0f, 10.0f);
    m_cClearBtnBG.SetColor(RGBA(0x7F, 0x7F, 0x7F, 0x7F));
    m_cClearBtnBG.SetLocalSize(60.0f, 20.0f);
    m_cClearBtn.SetDisplayWidgets(m_cClearBtnBG, m_cClearBtnBG);
    m_cClearBtn.SetAnchor(1.0f, 0.0f);
    m_cClearBtn.SetLocalPosition(fX + LOG_BG_W - 10.0f, fY + 10.0f);
    m_cClearBtn.SetID(EBTN_CLEAR);
    m_cClearBtn.AddChild(m_cClear);
    
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_MESSAGE_LOG, EGLOBAL_EVT_MSG_LOG_REFRESHED);
    m_bRegistered = TRUE;
}

VOID CLogWidget::Release(VOID)
{
    if (m_bRegistered) {
        CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_MESSAGE_LOG, EGLOBAL_EVT_MSG_LOG_REFRESHED);
        m_bRegistered = TRUE;
    }
    m_cHeaderBG.RemoveFromParent();
    m_cLogTitle.Release();
    m_cLogTitle.RemoveFromParent();
    
    m_cLogBG.RemoveFromParent();
    
    m_cList.Release();
    m_cList.RemoveFromParent();
    
    m_cClear.Release();
    m_cClear.RemoveFromParent();
    m_cClearBtn.RemoveFromParent();
}

VOID CLogWidget::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    CEventManager::PostEvent(*this, cEvent, fDelay);
}

VOID CLogWidget::UpdateTitle(VOID)
{
    CHAR szBuffer[128];
    snprintf(szBuffer, 128, "Log(%d)", (UINT)CMessageLog::GetLogs().size());
    m_cLogTitle.SetString(szBuffer);
}


BOOLEAN CLogWidget::OnTouchBegin(FLOAT fX, FLOAT fY)
{
    m_bUITouched = CUIContainer::OnTouchBegin(fX, fY);
    if (m_bUITouched) {
        ReattachToParent();
        return TRUE;
    }
    const SHLVector2D sPos = { fX, fY };
    BOOLEAN bHit = m_cHeaderBG.HitTest(sPos);
    if (!bHit && NULL != m_cList.GetParent()) {
        bHit = m_cList.HitTest(sPos);
    }
    if (bHit) {
        ReattachToParent();
        return TRUE;
    }
    return FALSE;
}
BOOLEAN CLogWidget::OnTouchMove(FLOAT fX, FLOAT fY)
{
    if (m_bUITouched) {
        return CUIContainer::OnTouchMove(fX, fY);
    }
    return TRUE;
}

BOOLEAN CLogWidget::OnTouchEnd(FLOAT fX, FLOAT fY)
{
    if (m_bUITouched) {
        return CUIContainer::OnTouchEnd(fX, fY);
    }
    // Toggle open/close log
    const SHLVector2D sPos = { fX, fY };
    const BOOLEAN bHit = m_cHeaderBG.HitTest(sPos);
    if (bHit) {
        ToggleLog();
    }
    return TRUE;
}


VOID CLogWidget::ToggleLog(VOID)
{
    if (NULL == m_cLogBG.GetParent()) {
        AddChild(m_cLogBG);
        m_cList.Initialize();
        AddChild(m_cList);
        AddChild(m_cClearBtn);
        return;
    }
    m_cLogBG.RemoveFromParent();
    m_cList.Release();
    m_cList.RemoveFromParent();
    m_cClearBtn.RemoveFromParent();
}

VOID CLogWidget::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EGLOBAL_EVT_MSG_LOG_REFRESHED:
            UpdateTitle();
            break;
        case EBTN_CLEAR:
            CMessageLog::Clear();
            break;
        case EBTN_REATTACH:
            {
                CUIContainer* pcParent = GetParent();
                if (NULL != pcParent) {
                    pcParent->ReAttachChild(*this);
                }
            }
            break;
    }    
}

VOID CLogWidget::ReattachToParent(VOID)
{
    CEvent cEvent(EBTN_REATTACH);
    PostEvent(cEvent, 0.0f);
}
