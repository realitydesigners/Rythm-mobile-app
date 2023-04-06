#include "stdafx.h"
#include "AutoLogicWidget.h"
#include "Event.h"
#include "PatternData.h"

CAutoLogicWidget::CAutoLogicWidget(UINT uPatternIndex) :
CUIButton(),
m_uPatternIndex(uPatternIndex),
INITIALIZE_TEXT_LABEL(m_cName)
{
    
}
    
CAutoLogicWidget::~CAutoLogicWidget()
{
}
 
VOID CAutoLogicWidget::Initialize(UINT uEventID)
{
    const SHLVector2D& sSize = GetLocalSize();
    m_cBG.SetLocalSize(sSize.x, sSize.y);
    m_cBG.SetColor(RGBA(0x4F, 0x4F, 0x4F, 0xFF));
    SetDisplayWidgets(m_cBG, m_cBG);
    SetID(uEventID);
    
    m_cName.SetFont(EGAMEFONT_SIZE_14);
    m_cName.SetEffect(CUITextLabel::EFFECT_SHADOW, 1.0f, 1.0f);
    m_cName.SetAnchor(0.0f, 0.5f);
    m_cName.SetLocalPosition(5.0f, sSize.y * 0.5f);
    AddChild(m_cName);

    CHAR szBuffer[64];
    snprintf(szBuffer, 64, "%s", CPatternDataMgr::GetPattern(m_uPatternIndex).szName);
    m_cName.SetString(szBuffer);
}

VOID CAutoLogicWidget::SetSelected(BOOLEAN bSelected)
{
    m_cBG.SetColor(bSelected ? RGBA(0x7F, 0x7F, 0x7F, 0xFF) : RGBA(0x4F, 0x4F, 0x4F, 0xFF));
}

VOID CAutoLogicWidget::Release(VOID)
{
    m_cName.Release();
    m_cName.RemoveFromParent();
    CUIButton::Release();
}

BOOLEAN CAutoLogicWidget::CreateButtonEvent(CEvent& cEvent)
{
    cEvent.SetIntParam(0, GetID());
    cEvent.SetIntParam(1, m_uPatternIndex);
    return TRUE;
}
