#include "stdafx.h"
#include "EventID.h"
#include "EventManager.h"
#include "AlertWidget.h"

#define CLOSED_W (100.0f)
#define CLOSED_H (20.0f)

#define BG_W (300.0f)
#define BG_H (400.0f)

#define BG_COLOR RGBA(0x1F, 0x1F, 0x1F, 0xFF)

CAlertWidget::CAlertWidget() :
CUIContainer(),
INITIALIZE_TEXT_LABEL(m_cTitle),
m_bUITouched(FALSE)
{
    
}
    
CAlertWidget::~CAlertWidget()
{
}
 
VOID CAlertWidget::Initialize(VOID)
{
    Release(); // safety
    m_cTitleBG.SetLocalSize(CLOSED_W, CLOSED_H);
    m_cTitleBG.SetColor(RGBA(0x4F, 0x4F, 0x4F, 0xFF));
    AddChild(m_cTitleBG);


    m_cTitle.SetFont(EGAMEFONT_SIZE_14);
    m_cTitle.SetEffect(CUITextLabel::EFFECT_SHADOW, 1.0f, 1.0f);
    m_cTitle.SetAnchor(0.5f, 0.5f);
    m_cTitle.SetLocalPosition(CLOSED_W * 0.5f, CLOSED_H * 0.5f);
    m_cTitle.SetString("Alerts");
    AddChild(m_cTitle);
    
    m_cAlerts.SetLocalSize(300.0f, 500.0f);
    m_cAlerts.SetLocalPosition(CLOSED_W - 300.0f, CLOSED_H + 5.0f);
    m_cAlerts.Initialize();
}

VOID CAlertWidget::Release(VOID)
{
    m_cAlerts.Release();
    m_cAlerts.RemoveFromParent();
    
    m_cTitleBG.RemoveFromParent();
    m_cTitle.Release();
    m_cTitle.RemoveFromParent();
}

VOID CAlertWidget::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    CEventManager::PostEvent(*this, cEvent, fDelay);
}


BOOLEAN CAlertWidget::OnTouchBegin(FLOAT fX, FLOAT fY)
{
    m_bUITouched = CUIContainer::OnTouchBegin(fX, fY);
    if (m_bUITouched) {
        ReattachToParent();
        return TRUE;
    }
    const SHLVector2D sPos = { fX, fY };
    BOOLEAN bHit = m_cTitleBG.HitTest(sPos);
    if (!bHit && NULL != m_cAlerts.GetParent()) {
        bHit = m_cAlerts.HitTest(sPos);
    }
    if (bHit) {
        ReattachToParent();
        return TRUE;
    }
    return FALSE;
}
BOOLEAN CAlertWidget::OnTouchMove(FLOAT fX, FLOAT fY)
{
    if (m_bUITouched) {
        return CUIContainer::OnTouchMove(fX, fY);
    }
    return TRUE;
}

BOOLEAN CAlertWidget::OnTouchEnd(FLOAT fX, FLOAT fY)
{
    if (m_bUITouched) {
        return CUIContainer::OnTouchEnd(fX, fY);
    }
    // Toggle open/close log
    const SHLVector2D sPos = { fX, fY };
    const BOOLEAN bHit = m_cTitleBG.HitTest(sPos);
    if (bHit) {
        Toggle();
    }
    return TRUE;
}


VOID CAlertWidget::Toggle(VOID)
{
    if (NULL == m_cAlerts.GetParent()) {
        AddChild(m_cAlerts);
        return;
    }
    m_cAlerts.RemoveFromParent();
}

VOID CAlertWidget::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
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

VOID CAlertWidget::ReattachToParent(VOID)
{
    CEvent cEvent(EBTN_REATTACH);
    PostEvent(cEvent, 0.0f);
}
