#include "stdafx.h"
#include "CMain.h"
#include "CryptoUtil.h"
#include "EventManager.h"
#include "GameRenderer.h"
#include "SRGraphDef.h"
#include "SRInnerZZ.h"
#include "TextInputWindow.h"
#include <math.h>

CSRInnerZZ::CSRInnerZZ() :
CSRBaseGraph(CSRBaseGraph::EType_Inner),
INITIALIZE_TEXT_LABEL(m_cLineIndicator),
m_pcMegaZZ(NULL),
m_uDepth(0),
m_psZZs(NULL),
m_uMaxZZ(0),
m_uUsedZZ(0),
m_psAreas(NULL),
m_uMaxArea(0),
m_uUsedArea(0),
m_psLines(NULL),
m_uMaxLine(0),
m_uUsedLine(0),
m_fPrevTouchX(0.0f),
m_bTouched(FALSE),
m_fXOffset(0.0f),
m_uHorizontalLineNum(9),
m_fHorizontalLineOffset(0.0f),
m_bHideUI(TRUE),
m_ppsZZFloatValueList(NULL),
m_psZZFloatValues(NULL),
m_uZZValueNum(0)
{

}

CSRInnerZZ::~CSRInnerZZ()
{
    ASSERT(NULL == m_psZZs);
    ASSERT(NULL == m_psAreas);
    ASSERT(NULL == m_psLines);
    ASSERT(NULL == m_ppsZZFloatValueList);
    ASSERT(NULL == m_psZZFloatValues);
}

BOOLEAN CSRInnerZZ::Initialize(UINT uBoxNum)
{
    if (!CSRBaseGraph::Initialize(uBoxNum)) {
        return FALSE;
    }
    {
        const UINT uMaxZZ = 100;
        if (uMaxZZ != m_uMaxZZ) {
            SAFE_DELETE_ARRAY(m_psZZs);
            m_psZZs = new SR_ZZ[uMaxZZ];
        }
        if (NULL == m_psZZs) {
            ASSERT(FALSE);
            return FALSE;
        }
        m_uMaxZZ = uMaxZZ;
        m_uUsedZZ = 0;
    }
    {
        const UINT uMaxArea = 100;
        if (uMaxArea != m_uMaxArea) {
            SAFE_DELETE_ARRAY(m_psAreas);
            m_psAreas = new SR_Area[uMaxArea];
        }
        if (NULL == m_psAreas) {
            ASSERT(FALSE);
            return FALSE;
        }
        m_uMaxArea = uMaxArea;
        m_uUsedArea = 0;
    }
    {
        const UINT uMaxLine = 100;
        if (uMaxLine != m_uMaxLine) {
            SAFE_DELETE_ARRAY(m_psLines);
            m_psLines = new SR_Line[uMaxLine];
        }
        if (NULL == m_psLines) {
            ASSERT(FALSE);
            return FALSE;
        }
        m_uMaxLine = uMaxLine;
        m_uUsedLine = 0;
    }

//    m_cBtnBG.SetColor(RGBA(0x7F, 0x7F, 0x7F, 0x7F));
//    m_cBtnBG.SetLocalSize(44.0f, 22.0f);
    AddSnapLatestBtn(EBTN_SNAP_LATEST);
//
//    m_cLineIndicator.RemoveFromParent();
//    m_cLineBtn.RemoveFromParent();
//
//    if (!m_bHideUI) {
//        m_cLineIndicator.SetFont(EGAMEFONT_SIZE_14);
//        m_cLineIndicator.SetEffect(CUITextLabel::EFFECT_4SIDE_SHADOW, 1.0f, 1.0f);
//        m_cLineIndicator.SetAnchor(0.5f, 0.5f);
//        m_cLineIndicator.SetAlignment(CUITextLabel::EALIGNMENT_CENTER);
//        m_cLineIndicator.SetLocalPosition(22.0f, 11.0f);
//
//        m_cLineBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
//        m_cLineBtn.SetLocalPosition(3.0f + 50.0f, 3.0f);
//        m_cLineBtn.SetID(EBTN_CHANGE_LINE_NUM);
//        AddChild(m_cLineBtn);
//        m_cLineBtn.AddChild(m_cLineIndicator);
//    }
//    AddShowHideBtn(EBTN_X_TOGGLE);

    SetHorizontalLineNum(m_uHorizontalLineNum);
//    UpdateToggleXLabel();
    return TRUE;
}
    
VOID CSRInnerZZ::Release(VOID)
{
    m_pcMegaZZ = NULL;
    m_uDepth = 0;
    
    m_cBtnBG.RemoveFromParent();
    m_cLineIndicator.Release();
    m_cLineIndicator.RemoveFromParent();
    m_cLineBtn.RemoveFromParent();
    
    SAFE_DELETE_ARRAY(m_psZZs);
    m_uMaxZZ = 0;
    SAFE_DELETE_ARRAY(m_psAreas);
    m_uMaxArea = 0;
    SAFE_DELETE_ARRAY(m_psLines);
    m_uMaxLine = 0;

    SAFE_DELETE_ARRAY(m_ppsZZFloatValueList);
    SAFE_DELETE_ARRAY(m_psZZFloatValues);
    m_uZZValueNum = 0;
    
    CSRBaseGraph::Release();
}

VOID CSRInnerZZ::SetHorizontalLineNum(UINT uNum)
{
    const FLOAT fHeightOffset = m_bHideUI ? 0.0f : SR_INNER_ZZ_HEIGHT_OFFSET;
 
    m_uHorizontalLineNum = uNum;
    m_fHorizontalLineOffset = (GetLocalSize().y - fHeightOffset - 2.0f * SR_INNER_ZZ_PADDING) / (uNum + 1);
    CHAR szBuffer[32];
    snprintf(szBuffer, 32, "L:%d", uNum);
    m_cLineIndicator.SetString(szBuffer);
}
VOID CSRInnerZZ::UpdateToggleXLabel(VOID)
{
    UpdateShowHideStatus(m_bHideUI);
}
VOID CSRInnerZZ::UpdateIndicator(FLOAT fChannelSize)
{
    CHAR szBuffer[64];
    snprintf(szBuffer, 64, "D%d", (9 - m_uDepth));
    SetIndicatorString(szBuffer);
}
VOID CSRInnerZZ::ClearChart(VOID)
{
    m_uUsedArea = 0;
    m_uUsedZZ = 0;
    m_uUsedLine = 0;
}
VOID CSRInnerZZ::AddZZ(FLOAT fX, FLOAT fY, FLOAT fSize, UINT uZZLevel, UINT uColor)
{
    if (m_uMaxZZ <= m_uUsedZZ) {
        ASSERT(FALSE);
        return;
    }
    SR_ZZ& sZZ = m_psZZs[m_uUsedZZ];
    sZZ.fX = fX;
    sZZ.fY = fY;
    sZZ.fSize = fSize;
    sZZ.uZZLevel = uZZLevel;
    sZZ.uColor = uColor;
    ++m_uUsedZZ;
}
VOID CSRInnerZZ::AddArea(FLOAT fX1, FLOAT fY1, FLOAT fX2, FLOAT fY2, FLOAT fY3, UINT uColor)
{
    if (m_uMaxArea <= m_uUsedArea) {
        ASSERT(FALSE);
        return;
    }
    SR_Area& sArea = m_psAreas[m_uUsedArea];
    sArea.fX1 = fX1;
    sArea.fY1 = fY1;
    sArea.fX2 = fX2;
    sArea.fY2 = fY2;
    sArea.fY3 = fY3;
    sArea.uColor = uColor;
    ++m_uUsedArea;
}
VOID CSRInnerZZ::AddLine(FLOAT fX1, FLOAT fY1, FLOAT fX2, FLOAT fY2, UINT uColor, BOOLEAN bTimeLine)
{
    if (m_uMaxLine <= m_uUsedLine) {
        ASSERT(FALSE);
        return;
    }
    SR_Line& sLine =  m_psLines[m_uUsedLine];
    sLine.fX1 = fX1;
    sLine.fY1 = fY1;
    sLine.fX2 = fX2;
    sLine.fY2 = fY2;
    if (!bTimeLine) {
        const FLOAT fDiffX = fX2 - fX1;
        const FLOAT fDiffY = fY2 - fY1;
        const FLOAT fDist = (FLOAT)sqrt(fDiffX * fDiffX + fDiffY * fDiffY);
        // rotate 90 degrees (0,1) => (1,0) => (0,-1) => (-1,0)
        sLine.fTangentX = fDiffY / fDist;
        sLine.fTangentY = -fDiffX / fDist;
    }
    else {
        sLine.fTangentX = sLine.fTangentY = 0.0f;
    }

    sLine.uColor = uColor;
    ++m_uUsedLine;
}
static VOID RenderThickLine(FLOAT fX1, FLOAT fY1, FLOAT fX2, FLOAT fY2, FLOAT fTangentX, FLOAT fTangentY, UINT uColor)
{
    FLOAT afPos[8] = { 0.0f };
    fX1 -= fTangentX;
    fY1 -= fTangentY;
    
    fX2 -= fTangentX;
    fY2 -= fTangentY;
    
    const FLOAT fX3 = fX1 + fTangentX * 2.0f;
    const FLOAT fY3 = fY1 + fTangentY * 2.0f;
    const FLOAT fX4 = fX2 + fTangentX * 2.0f;
    const FLOAT fY4 = fY2 + fTangentY * 2.0f;
    afPos[0] = fX1; afPos[1] = fY1;
    afPos[2] = fX2; afPos[3] = fY2;
    afPos[4] = fX3; afPos[5] = fY3;
    afPos[6] = fX4; afPos[7] = fY4;
    CGameRenderer::DrawFilledPolygon(afPos, 4, uColor);
}
VOID CSRInnerZZ::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    const SHLVector2D& sPos = GetWorldPosition();
    const SHLVector2D& sSize = GetWorldSize();
    CGameRenderer::DrawRectOutline(sPos.x-1 + fOffsetX, sPos.y-1 + fOffsetY, sSize.x+2, sSize.y+2, RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    CMain::SetScissorTest(sPos.x-1 + fOffsetX, sPos.y-1 + fOffsetY, sSize.x+1, sSize.y+1);
    const FLOAT fNewOffsetX = fOffsetX + sPos.x;
    const FLOAT fNewOffsetY = fOffsetY + sPos.y;
    FLOAT afPos[8] = { 0.0f };
    if (NULL != m_psAreas) {
        for (UINT uIndex = 0; m_uUsedArea > uIndex; ++uIndex) {
            const SR_Area& sArea = m_psAreas[uIndex];
            afPos[0] = fNewOffsetX + sArea.fX1;
            afPos[1] = fNewOffsetY + sArea.fY1;
            
            afPos[2] = fNewOffsetX + sArea.fX2;
            afPos[3] = fNewOffsetY + sArea.fY2;
            
            afPos[4] = afPos[0];
            afPos[5] = fNewOffsetY + sArea.fY3;
            
            afPos[6] = afPos[2];
            afPos[7] = afPos[5];
            CGameRenderer::DrawFilledPolygon(afPos, 4, sArea.uColor);
            CGameRenderer::DrawLine(afPos[0], afPos[1], afPos[2], afPos[3], RGBA(0x7F, 0x7F, 0x7F, 0xFF));
        }
    }
    if (NULL != m_psLines) {
        for (UINT uIndex = 0; m_uUsedLine > uIndex; ++uIndex) {
            const SR_Line& sLine = m_psLines[uIndex];
            const FLOAT fX1 = fNewOffsetX + sLine.fX1;
            const FLOAT fY1 = fNewOffsetY + sLine.fY1;
            const FLOAT fX2 = fNewOffsetX + sLine.fX2;
            const FLOAT fY2 = fNewOffsetY + sLine.fY2;
            if (0.0f == sLine.fTangentX && 0.0f == sLine.fTangentY) {
                CGameRenderer::DrawLine(fX1, fY1, fX2, fY2, sLine.uColor);
            }
            else {
                RenderThickLine(fX1, fY1, fX2, fY2, sLine.fTangentX, sLine.fTangentY, sLine.uColor);
            }
        }
    }
    if (NULL != m_psZZs) {
        for (UINT uIndex = 0; m_uUsedZZ > uIndex; ++uIndex) {
            const SR_ZZ& sZZ = m_psZZs[uIndex];
            const FLOAT fX = fNewOffsetX + sZZ.fX;
            const FLOAT fY = fNewOffsetY + sZZ.fY;
            const FLOAT fX1 = fX - sZZ.fSize;
            const FLOAT fY1 = fY;
            
            const FLOAT fX2 = fX;
            const FLOAT fY2 = fY - sZZ.fSize;
            
            const FLOAT fX3 = fX;
            const FLOAT fY3 = fY + sZZ.fSize;
            const FLOAT fX4 = fX + sZZ.fSize;
            const FLOAT fY4 = fY;
            afPos[0] = fX1; afPos[1] = fY1;
            afPos[2] = fX2; afPos[3] = fY2;
            afPos[4] = fX3; afPos[5] = fY3;
            afPos[6] = fX4; afPos[7] = fY4;
            CGameRenderer::DrawFilledPolygon(afPos, 4, sZZ.uColor);
        }
    }

    CMain::ClearScissorTest();
    const FLOAT fHeightOffset = m_bHideUI ? 0.0f : SR_INNER_ZZ_HEIGHT_OFFSET;
    const FLOAT fEndX = fNewOffsetX + sSize.x;
    FLOAT fNewY = fNewOffsetY + fHeightOffset + SR_INNER_ZZ_PADDING + m_fHorizontalLineOffset;
    for (UINT uIndex = 0; m_uHorizontalLineNum > uIndex; ++uIndex) {
        CGameRenderer::DrawLine(fNewOffsetX, fNewY, fEndX, fNewY, RGBA(0x7f, 0x7f, 0x7f, 0xff));
        fNewY += m_fHorizontalLineOffset;
    }
    CSRBaseGraph::OnRender(fOffsetX, fOffsetY);
}

BOOLEAN CSRInnerZZ::OnTouchBegin(FLOAT fX, FLOAT fY)
{
    if (CSRBaseGraph::OnTouchBegin(fX, fY)) {
        m_bTouched = FALSE;
        return TRUE;
    }
    const SHLVector2D& sPos = { fX, fY };
    m_bTouched = HitTest(sPos);
    m_fPrevTouchX = fX;
    return m_bTouched;
}

BOOLEAN CSRInnerZZ::OnTouchMove(FLOAT fX, FLOAT fY)
{
    if (!m_bTouched) {
        return CSRBaseGraph::OnTouchMove(fX, fY);
    }
    const FLOAT fDiffX = fX - m_fPrevTouchX;
    m_fPrevTouchX = fX;
    m_fXOffset += fDiffX;
    if (NULL != m_pcMegaZZ) {
        PrepareChart(*m_pcMegaZZ, m_uDepth);
    }
    return TRUE;
}
BOOLEAN CSRInnerZZ::OnTouchEnd(FLOAT fX, FLOAT fY)
{
    if (!m_bTouched) {
        return CSRBaseGraph::OnTouchEnd(fX, fY);
    }
    
    return OnTouchMove(fX, fY);
}

VOID CSRInnerZZ::OnReceiveEvent(CEvent& cEvent)
{
    switch (cEvent.GetIntParam(0)) {
        case EBTN_X_TOGGLE:
            m_bHideUI = !m_bHideUI;
            Initialize(0);
            if (NULL != m_pcMegaZZ) {
                PrepareChart(*m_pcMegaZZ, m_uDepth);
            }
            break;

        case EBTN_SNAP_LATEST:
            m_fXOffset = 0.0f;
            if (NULL != m_pcMegaZZ) {
                PrepareChart(*m_pcMegaZZ, m_uDepth);
            }
            break;
        case EBTN_CHANGE_LINE_NUM:
            OnPressChangeLineBtn();
            break;
        case EEVENT_ON_TEXT_REPLY:
            OnUserInputReply();
            break;
        default:
            break;
    }
}

VOID CSRInnerZZ::OnPressChangeLineBtn(VOID)
{
    CEvent cReply(EEVENT_ON_TEXT_REPLY);
    snprintf(m_szBuffer, 64, "%d", m_uHorizontalLineNum);
    CTextInputWindow* pcWin = new CTextInputWindow(m_szBuffer, 64, *this, cReply, EInputType_Numeric);
    if (NULL != pcWin) {
        pcWin->DoModal();
    }
}

VOID CSRInnerZZ::OnUserInputReply(VOID)
{
    const UINT uNum = atoi(m_szBuffer);
    if (10 < uNum) {
        return;
    }
    SetHorizontalLineNum(uNum);
}

