#include "stdafx.h"
#include "AutoGraphPanel.h"
#include "EventManager.h"


VOID CAutoGraphPanel::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    // do not post to parent.
    CEventManager::PostEvent(*this, cEvent, fDelay);
}

CAutoGraphPanel::CAutoGraphPanel() :
CUIContainer(),
INITIALIZE_TEXT_LABEL(m_cT),
INITIALIZE_TEXT_LABEL(m_cG),
m_cTop(0, 9),
m_uPatternIndex(0),
m_psRows(NULL),
m_uRowNum(0)
{
    
}

CAutoGraphPanel::~CAutoGraphPanel()
{
    
}

VOID CAutoGraphPanel::Initialize(FLOAT fW, FLOAT fH)
{
    SetLocalSize(fW, fH);
    m_cBG.SetLocalSize(fW, fH);
    m_cBG.SetColor(RGBA(0x1F, 0x1F, 0x1F, 0xFF));
    m_cBG.SetBorderColor(RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    AddChild(m_cBG);
    
    m_cBtnBG.SetLocalSize(20.0f, 20.0f);
    m_cBtnBG.SetColor(RGBA(0x4F, 0x4F, 0x4F, 0xFF));
    
    m_cG.SetFont(EGAMEFONT_SIZE_12);
    m_cG.SetEffect(CUITextLabel::EFFECT_SHADOW, 1.0f, 1.0f);
    m_cG.SetString("G");
    m_cG.SetLocalPosition(10.0f, 10.0f);
    m_cG.SetAnchor(0.5f, 0.5f);
    m_cGBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
    m_cGBtn.SetID(EBTN_TOGGLE_GRID);
    m_cGBtn.AddChild(m_cG);
    m_cGBtn.SetLocalPosition(5.0f, 5.0f);
    AddChild(m_cGBtn);
    
    m_cT.SetFont(EGAMEFONT_SIZE_12);
    m_cT.SetEffect(CUITextLabel::EFFECT_SHADOW, 1.0f, 1.0f);
    m_cT.SetString("T");
    m_cT.SetLocalPosition(10.0f, 10.0f);
    m_cT.SetAnchor(0.5f, 0.5f);
    m_cTBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
    m_cTBtn.SetID(EBTN_TOGGLE_TRANSPARENCY);
    m_cTBtn.AddChild(m_cT);
    m_cTBtn.SetLocalPosition(30.0f, 5.0f);
    AddChild(m_cTBtn);
    
    m_cTop.SetLocalPosition(0.0f, 30.0f);
    m_cTop.SetLocalSize(fW, fW + 30.0f);
    m_cTop.Initialize(TRUE);
    AddChild(m_cTop);
}

VOID CAutoGraphPanel::Release(VOID)
{
    m_cT.Release();
    m_cT.RemoveFromParent();
    m_cTBtn.RemoveFromParent();
    m_cG.Release();
    m_cG.RemoveFromParent();
    m_cGBtn.RemoveFromParent();
    m_cTop.Release();
    m_cTop.RemoveFromParent();
}


VOID CAutoGraphPanel::OnReceiveEvent(CEvent& cEvent)
{
    switch (cEvent.GetIntParam(0)) {
        case EBTN_TOGGLE_GRID:
            m_cTop.ToggleGrid();
            Refresh();
            break;
        case EBTN_TOGGLE_TRANSPARENCY:
            m_cTop.ToggleTransparency();
            Refresh();
            break;
    }
}

VOID CAutoGraphPanel::Refresh(UINT uPatternIndex)
{
    m_uPatternIndex = uPatternIndex;
    m_psRows = NULL;
    m_uRowNum = 0;
    m_cTop.Refresh(uPatternIndex);
}

VOID CAutoGraphPanel::Refresh(const SPatternRow* psRows, UINT uRowNum)
{
    m_psRows = psRows;
    m_uRowNum = uRowNum;
    m_cTop.Refresh(m_psRows, m_uRowNum);
}

VOID CAutoGraphPanel::Refresh(VOID)
{
    if (NULL != m_psRows) {
        Refresh(m_psRows, m_uRowNum);
    }
    else {
        Refresh(m_uPatternIndex);
    }
}
