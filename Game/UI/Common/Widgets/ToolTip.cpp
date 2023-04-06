#include "stdafx.h"
#include "ToolTip.h"

CToolTip::CToolTip() :
CUIContainer(),
INITIALIZE_TEXT_LABEL(m_cTxt)
{
    AddChild(m_cBG);
    AddChild(m_cTxt);
    m_cTxt.SetFont(EGAMEFONT_SIZE_16);
    m_cTxt.SetLocalPosition(10.0f, 5.0f);
    m_cTxt.SetAnchor(0.0f, 0.0f);
    m_cTxt.AutoSize();
}
    
CToolTip::~CToolTip()
{
    
}

VOID CToolTip::Initialize(const CHAR* szString)
{
    m_cBG.Set9PartTexture(ETEX::ui_9parts_brown, 12, 12, 12, 12);
    
    m_cTxt.SetString(szString);
    m_cTxt.OnUpdate(0.0f);
    const SHLVector2D& sSize = m_cTxt.GetWorldSize();
    
    SetLocalSize(sSize.x + 20.0f, sSize.y + 10.0f);
    m_cBG.SetLocalSize(sSize.x + 20.0f, sSize.y + 10.0f);
}
    
VOID CToolTip::Release(VOID)
{
    m_cBG.Release();
    m_cBG.RemoveFromParent();
    m_cTxt.Release();
    m_cTxt.RemoveFromParent();
    CUIContainer::Release();
}
