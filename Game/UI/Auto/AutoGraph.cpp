#include "stdafx.h"
#include "AutoGraph.h"
#include "EventManager.h"
#include "GameRenderer.h"
#include "PlayerData.h"
#include "TextSelectorWindow.h"
#include <math.h>

VOID CAutoGraph::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    // do not post to parent.
    CEventManager::PostEvent(*this, cEvent, fDelay);
}

CAutoGraph::CAutoGraph(UINT uStartDepthIndex, UINT uDepthNum) :
CUIContainer(),
m_uStartDepthIndex(uStartDepthIndex),
m_uDepthNum(uDepthNum),
m_uBoxNum(0),
m_uLineNum(0),
m_bDrawGrid(FALSE),
m_bTransparent(FALSE),
INITIALIZE_TEXT_LABEL(m_cDepthIndexTxt),
INITIALIZE_TEXT_LABEL(m_cDepthNumTxt),
m_psPatternRows(NULL),
m_uPatternRowNum(0)
{
    memset(m_abEnabled, 0xFF, sizeof(m_abEnabled));
}
    
CAutoGraph::~CAutoGraph()
{
}
 
VOID CAutoGraph::Initialize(BOOLEAN bOverrideSize)
{
    FLOAT fW = AUTO_GRAPH_WIDTH;
    FLOAT fH = AUTO_GRAPH_HEIGHT;
    if (bOverrideSize) {
        const SHLVector2D& sSize = GetLocalSize();
        fW = sSize.x;
        fH = sSize.y;
    }
    SetLocalSize(fW, fH);
    
    m_cBtnBG.SetLocalSize(20.0f, 20.0f);
    m_cBtnBG.SetColor(RGBA(0x4F, 0x4F, 0x4F, 0xFF));
    
    const FLOAT fBtnY = bOverrideSize ? -20.0f : (fH - 30.0f);
    m_cDepthIndexTxt.SetFont(EGAMEFONT_SIZE_12);
    m_cDepthIndexTxt.SetEffect(CUITextLabel::EFFECT_SHADOW, 1.0f, 1.0f);
    m_cDepthIndexTxt.SetAnchor(0.5f, 0.5f);
    m_cDepthIndexTxt.SetLocalPosition(10.0f, 10.0f);
    m_cDepthBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
    m_cDepthBtn.AddChild(m_cDepthIndexTxt);
    m_cDepthBtn.SetID(EBTN_CHANGE_DEPTH_INDEX);
    m_cDepthBtn.SetLocalPosition(fW - 55.0f, fBtnY);
    AddChild(m_cDepthBtn);
    
    m_cDepthNumTxt.SetFont(EGAMEFONT_SIZE_12);
    m_cDepthNumTxt.SetEffect(CUITextLabel::EFFECT_SHADOW, 1.0f, 1.0f);
    m_cDepthNumTxt.SetAnchor(0.5f, 0.5f);
    m_cDepthNumTxt.SetLocalPosition(10.0f, 10.0f);
    m_cDepthNumBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
    m_cDepthNumBtn.AddChild(m_cDepthNumTxt);
    m_cDepthNumBtn.SetID(EBTN_CHANGE_DEPTH_NUM);
    m_cDepthNumBtn.SetLocalPosition(fW - 30.0f, fBtnY);
    AddChild(m_cDepthNumBtn);
    
    m_cDisplay.SetLocalPosition(5.0f, fW - 5.0f);
    AddChild(m_cDisplay);
    
    const USHORT* pushRatios = CPlayerData::GetChannelRatios();
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        m_afSizes[uIndex] = (FLOAT)pushRatios[uIndex];
    }
    UpdateDepthLabels();
    UpdateDepthDisplay();
}
VOID CAutoGraph::Release(VOID)
{
    m_cDepthIndexTxt.Release();
    m_cDepthIndexTxt.RemoveFromParent();
    m_cDepthBtn.RemoveFromParent();
    m_cDepthNumTxt.Release();
    m_cDepthNumTxt.RemoveFromParent();
    m_cDepthNumBtn.RemoveFromParent();
    
    m_cDisplay.Release();
    m_cDisplay.RemoveFromParent();
    Clear();
}

VOID CAutoGraph::Clear(VOID)
{
    m_uBoxNum = 0;
    m_uLineNum = 0;
}

VOID CAutoGraph::AddBox(FLOAT fX, FLOAT fY, FLOAT fW, FLOAT fH, UINT uColor, UINT uDepthIndex)
{
    if (EBOX_NUM <= m_uBoxNum) {
        ASSERT(FALSE);
        return;
    }
    SBox& sBox = m_asBox[m_uBoxNum];
    ++m_uBoxNum;
    sBox.fX = fX;
    sBox.fY = fY;
    sBox.fW = fW;
    sBox.fH = fH;
    sBox.uColor = uColor;
    sBox.uDepthIndex = uDepthIndex;
}
VOID CAutoGraph::AddLine(FLOAT fX1, FLOAT fY1, FLOAT fX2, FLOAT fY2, UINT uColor, UINT uDepthIndex)
{
    if (ELINE_NUM <= m_uLineNum) {
        ASSERT(FALSE);
        return;
    }
    SLine& sLine = m_asLine[m_uLineNum];
    ++m_uLineNum;
    sLine.fX1 = fX1;
    sLine.fY1 = fY1;
    sLine.fX2 = fX2;
    sLine.fY2 = fY2;
    sLine.uColor = uColor;
    sLine.uDepthIndex = uDepthIndex;
}

VOID CAutoGraph::ToggleGrid(VOID)
{
    m_bDrawGrid = !m_bDrawGrid;
}
VOID CAutoGraph::ToggleTransparency(VOID)
{
    m_bTransparent = !m_bTransparent;
}
VOID CAutoGraph::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    SHLVector2D sPos = GetWorldPosition();
    sPos.x += fOffsetX;
    sPos.y += fOffsetY;
    UINT uEndDepthIndex = m_uStartDepthIndex + m_uDepthNum;
    if (9 < uEndDepthIndex) {
        uEndDepthIndex = 9;
    }
    for (UINT uIndex = m_uStartDepthIndex; uEndDepthIndex > uIndex; ++uIndex) {
        RenderBoxes(sPos.x, sPos.y, uIndex);
        RenderLines(sPos.x, sPos.y, uIndex);
    }
    CUIContainer::OnRender(fOffsetX, fOffsetY);
}
VOID CAutoGraph::RenderBoxes(FLOAT fOffsetX, FLOAT fOffsetY, UINT uDepthIndex) const
{
    for (UINT uIndex = 0; m_uBoxNum > uIndex; ++uIndex) {
        const SBox& sBox = m_asBox[uIndex];
        if (sBox.uDepthIndex != uDepthIndex) {
            continue;
        }
        CGameRenderer::DrawRectNoTex(fOffsetX + sBox.fX, fOffsetY + sBox.fY, sBox.fW, sBox.fH, sBox.uColor);
    }
}

VOID CAutoGraph::RenderLines(FLOAT fOffsetX, FLOAT fOffsetY, UINT uDepthIndex) const
{
    for (UINT uIndex = 0; m_uLineNum > uIndex; ++uIndex) {
        const SLine& sLine = m_asLine[uIndex];
        if (sLine.uDepthIndex != uDepthIndex) {
            continue;
        }
        CGameRenderer::DrawLine(fOffsetX + sLine.fX1, fOffsetY + sLine.fY1,
                                fOffsetX + sLine.fX2, fOffsetY + sLine.fY2,
                                sLine.uColor);
    }
}

VOID CAutoGraph::OnReceiveEvent(CEvent& cEvent)
{
    switch (cEvent.GetIntParam(0)) {
        case EBTN_CHANGE_DEPTH_NUM:
            OnTapDepthNumBtn();
            break;
        case EBTN_CHANGE_DEPTH_INDEX:
            OnTapDepthIndexBtn();
            break;
        case EEVENT_USER_REPLY_DEPTH_INDEX:
            OnUserReplyDepthIndex(cEvent.GetIntParam(2));
            break;
        case EEVENT_USER_REPLY_DEPTH_NUM:
            OnUserReplyNum(cEvent.GetIntParam(2));
            break;
    }
}

VOID CAutoGraph::UpdateDepthLabels(VOID)
{
    CHAR szBuffer[64];
    snprintf(szBuffer, 64, "D%d", 9 - m_uStartDepthIndex);
    m_cDepthIndexTxt.SetString(szBuffer);
    
    snprintf(szBuffer, 64, "N%d", m_uDepthNum);
    m_cDepthNumTxt.SetString(szBuffer);
}

VOID CAutoGraph::OnTapDepthIndexBtn(VOID)
{
    CTextSelectorWindow* pcWin = new CTextSelectorWindow(*this, EEVENT_USER_REPLY_DEPTH_INDEX, 0);
    if (NULL == pcWin) {
        return;
    }
    const SHLVector2D& sWorldPos = m_cDepthBtn.GetWorldPosition();
    pcWin->SetLocalPosition(sWorldPos.x + 5.0f, sWorldPos.y + 5.0f);
    const CHAR* aszSelections[7] = {
        "D9", "D8", "D7", "D6", "D5", "D4", "D3"
    };
    if (!pcWin->Initialize(aszSelections, 7, m_uStartDepthIndex)) {
        ASSERT(FALSE);
        pcWin->Release();
        delete pcWin;
        return;
    }
    pcWin->DoModal();

}

VOID CAutoGraph::OnTapDepthNumBtn(VOID)
{
    CTextSelectorWindow* pcWin = new CTextSelectorWindow(*this, EEVENT_USER_REPLY_DEPTH_NUM, 0);
    if (NULL == pcWin) {
        return;
    }
    const SHLVector2D& sWorldPos = m_cDepthNumBtn.GetWorldPosition();
    pcWin->SetLocalPosition(sWorldPos.x + 5.0f, sWorldPos.y + 5.0f);
    const CHAR* aszSelections[6] = {
        "N4", "N5", "N6", "N7", "N8", "N9"
    };
    const UINT uSelectedIndex = m_uDepthNum - 4;
    if (!pcWin->Initialize(aszSelections, 6, uSelectedIndex)) {
        ASSERT(FALSE);
        pcWin->Release();
        delete pcWin;
        return;
    }
    pcWin->DoModal();

}

VOID CAutoGraph::OnUserReplyDepthIndex(UINT uSelectedIndex)
{
    m_uStartDepthIndex = uSelectedIndex;
    UpdateDepthLabels();
    UpdateDepthDisplay();
    if (NULL != m_psPatternRows) {
        Refresh(m_psPatternRows, m_uPatternRowNum);
    }
}
VOID CAutoGraph::OnUserReplyNum(UINT uSelectedIndex)
{
    m_uDepthNum = 4 + uSelectedIndex;
    UpdateDepthLabels();
    UpdateDepthDisplay();
    if (NULL != m_psPatternRows) {
        Refresh(m_psPatternRows, m_uPatternRowNum);
    }
}

VOID CAutoGraph::UpdateDepthDisplay(VOID)
{
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex ;++uIndex) {
        m_cDisplay.SetEnabled(uIndex, m_abEnabled[uIndex]);
    }
    m_cDisplay.Initialize(m_afSizes, m_uStartDepthIndex, m_uDepthNum);
}
