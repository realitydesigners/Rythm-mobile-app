#include "stdafx.h"
#include "SRBaseGraph.h"
#include "EventManager.h"
#include <math.h>

CSRBaseGraph::CSRBaseGraph(EType eType, UINT uGlobalHandlerID) :
CUIContainer(EBaseWidget_Container, uGlobalHandlerID),
m_eType(eType),
INITIALIZE_TEXT_LABEL(m_cIndicator),
INITIALIZE_TEXT_LABEL(m_cShowHideTxt),
m_uUsedTL(0)
{
    m_cIndicator.SetFont(EGAMEFONT_SIZE_16);
    m_cIndicator.SetEffect(CUITextLabel::EFFECT_4SIDE_SHADOW, 1.0f, 1.0f);
    m_cIndicator.SetAnchor(0.0f, 1.0f);

}

CSRBaseGraph::~CSRBaseGraph()
{
    
}

BOOLEAN CSRBaseGraph::Initialize(UINT uBoxNum)
{
    return TRUE;
}

VOID CSRBaseGraph::Release(VOID)
{
    m_cIndicator.Release();
    m_cIndicator.RemoveFromParent();

    m_cSnapBtn.RemoveFromParent();
    
    m_cShowHideTxt.Release();
    m_cShowHideTxt.RemoveFromParent();
    m_cShowHideBtn.RemoveFromParent();
    
    ClearTimeLines();
}

VOID CSRBaseGraph::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    // do not post to parent
    CEventManager::PostEvent(*this, cEvent, fDelay);
}


VOID CSRBaseGraph::ClearTimeLines(VOID)
{
    for (UINT uIndex = 0; m_uUsedTL > uIndex; ++uIndex) {
        m_acTime[uIndex].Release();
    }
    m_uUsedTL = 0;
}

BOOLEAN CSRBaseGraph::AddTimeLine(FLOAT fX, FLOAT fY, UINT uTimeSec)
{
    if (MAX_TIMELINE <= m_uUsedTL) {
//        ASSERT(FALSE);
        return FALSE;
    }
    if (0 < m_uUsedTL) { // do overlap check
        const CTimeIndicator& cPrev = m_acTime[m_uUsedTL-1];
        const SHLVector2D& sPos = cPrev.GetLocalPosition();
        const FLOAT fDiffX = abs(fX - sPos.x);
        if (40.0f > fDiffX) {
            return FALSE;
        }
    }
    CTimeIndicator& cTL = m_acTime[m_uUsedTL];
    cTL.SetLocalPosition(fX, fY);
    cTL.SetTimeSec(uTimeSec);
    cTL.OnUpdate(0.0f);
    ++m_uUsedTL;
    return TRUE;
}

VOID CSRBaseGraph::SetIndicatorString(const CHAR* szString)
{
    const SHLVector2D& sSize = GetLocalSize();
    m_cIndicator.SetLocalPosition(3.0f, sSize.y);
    m_cIndicator.SetString(szString);
    m_cIndicator.RemoveFromParent();
    AddChild(m_cIndicator);
}

VOID CSRBaseGraph::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    SHLVector2D sPos = GetWorldPosition();
    sPos.x += fOffsetX;
    sPos.y += fOffsetY;
    for (UINT uIndex = 0; m_uUsedTL > uIndex; ++uIndex) {
        m_acTime[uIndex].OnRender(sPos.x, sPos.y);
    }
    CUIContainer::OnRender(fOffsetX, fOffsetY);
}
VOID CSRBaseGraph::SetupLabel(CUITextLabel& cLabel)
{
    cLabel.SetFont(EGAMEFONT_SIZE_14);
    cLabel.SetEffect(CUITextLabel::EFFECT_4SIDE_SHADOW, 1.0f, 1.0f);
    cLabel.SetAnchor(0.5f, 0.5f);
    cLabel.SetAlignment(CUITextLabel::EALIGNMENT_CENTER);
    cLabel.SetLocalPosition(22.0f, 11.0f);
    cLabel.RemoveFromParent();
}
VOID CSRBaseGraph::AddSnapLatestBtn(UINT uEventID)
{
    m_cBtnBG.SetColor(RGBA(0x7F, 0x7F, 0x7F, 0x7F));
    m_cBtnBG.SetLocalSize(44.0f, 22.0f);
    m_cSnapBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
    m_cSnapBtn.SetLocalPosition(3.0f, 3.0f);
    m_cSnapBtn.SetID(uEventID);
    m_cSnapBtn.RemoveFromParent();
    AddChild(m_cSnapBtn);
}

VOID CSRBaseGraph::AddShowHideBtn(UINT uEventID)
{
    SetupLabel(m_cShowHideTxt);
    
    m_cShowHideBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
    m_cShowHideBtn.SetLocalPosition(3.0f, 3.0f + 26.0f);
    m_cShowHideBtn.SetID(uEventID);
    m_cShowHideBtn.RemoveFromParent();
    AddChild(m_cShowHideBtn);
    m_cShowHideBtn.AddChild(m_cShowHideTxt);
}

VOID CSRBaseGraph::UpdateShowHideStatus(BOOLEAN bHide)
{
    m_cShowHideTxt.SetString(bHide ? "Show" : "Hide");
}
