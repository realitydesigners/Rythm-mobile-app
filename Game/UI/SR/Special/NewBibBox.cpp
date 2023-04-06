#include "stdafx.h"
#include "AppMain.h"
#include "Event.h"
#include "EventManager.h"
#include "GameRenderer.h"
#include "MegaZZ.h"
#include "NewBibBox.h"
#include "PlayerData.h"
#include "TradeLog.h"
#include "TextSelectorWindow.h"

VOID CNewBibBox::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    CEventManager::PostEvent(*this, cEvent, fDelay);
}

CNewBibBox::CNewBibBox() :
CUIContainer(EBaseWidget_Container, EGLOBAL_NEW_BIB_BOX_CHART),
m_uUsedBoxNum(0),
m_uUsedLineNum(0),
m_pcMegaZZ(NULL),
m_psTradelog(NULL),
m_bTradeLogOpen(TRUE),
m_cDepthValues(0, 0, 0),
INITIALIZE_TEXT_LABEL(m_cOuterDepthIndex),
INITIALIZE_TEXT_LABEL(m_cOuterDepthNum),
INITIALIZE_TEXT_LABEL(m_cInnerDepthIndex),
INITIALIZE_TEXT_LABEL(m_cInnerDepthNum),
m_uOuterDepthIndex(0),
m_uOuterDepthNum(5)
{
    memset(m_apcDepthhLbls, 0, sizeof(m_apcDepthhLbls));
}

CNewBibBox::~CNewBibBox()
{

}
    
BOOLEAN CNewBibBox::Initialize(VOID)
{
    const SHLVector2D& sLocalSize = GetLocalSize();
    m_cBibWidget.Initialize();
    m_cBibWidget.SetDepthNum(4);
    m_cBibWidget.SetDepthIndex(5);
    m_cBibWidget.RemoveFromParent();
    AddChild(m_cBibWidget);

    m_cDepthDisplay.SetLocalPosition(5.0f, sLocalSize.y - 57.0f);
    AddChild(m_cDepthDisplay);
    m_cDepthValues.SetLocalPosition(-142.0f, sLocalSize.y - 27.0f);
    m_cDepthValues.SetAdditionalSpacing(6.0f);
    m_cDepthValues.HideNameAndPrice();
    m_cDepthValues.SetDisplay(9, 0);
    m_cDepthValues.Initialize();
    AddChild(m_cDepthValues);
    
    CHAR szBuffer[32];
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        CUITextLabel* pcLbl = NEW_TEXT_LABEL;
        if (NULL == pcLbl) {
            ASSERT(FALSE);
            return FALSE;
        }
        pcLbl->SetAnchor(0.5f, 0.5f);
        pcLbl->SetFont(EGAMEFONT_SIZE_14);
        snprintf(szBuffer, 32, "F%d", 9 - uIndex);
        pcLbl->SetString(szBuffer);
        pcLbl->SetEffect(CUITextLabel::EFFECT_8SIDE, 1.0f, 1.0f);
        pcLbl->SetVisibility(FALSE);
        AddChild(*pcLbl);
        m_apcDepthhLbls[uIndex] = pcLbl;
    }
    m_cBtnImg.SetLocalSize(30.0f, 20.0f);
    m_cBtnImg.SetColor(RGBA(0x4F, 0x4F, 0x4F, 0xFF));
    
    m_cOuterDepthIndex.SetFont(EGAMEFONT_SIZE_12);
    m_cOuterDepthIndex.SetLocalPosition(15.0f, 10.0f);
    m_cOuterDepthIndex.SetAnchor(0.5f, 0.5f);
    m_cOuterDepthIndexBtn.SetLocalPosition(5.0f, 5.0f);
    m_cOuterDepthIndexBtn.SetID(EBTN_OUTER_DEPTH_INDEX);
    m_cOuterDepthIndexBtn.SetDisplayWidgets(m_cBtnImg, m_cBtnImg);
    m_cOuterDepthIndexBtn.AddChild(m_cOuterDepthIndex);
    AddChild(m_cOuterDepthIndexBtn);

    m_cOuterDepthNum.SetFont(EGAMEFONT_SIZE_12);
    m_cOuterDepthNum.SetLocalPosition(15.0f, 10.0f);
    m_cOuterDepthNum.SetAnchor(0.5f, 0.5f);
    m_cOuterDepthNumBtn.SetLocalPosition(40.0f, 5.0f);
    m_cOuterDepthNumBtn.SetID(EBTN_OUTER_DEPTH_NUM);
    m_cOuterDepthNumBtn.SetDisplayWidgets(m_cBtnImg, m_cBtnImg);
    m_cOuterDepthNumBtn.AddChild(m_cOuterDepthNum);
    AddChild(m_cOuterDepthNumBtn);
        
    m_cInnerDepthIndex.SetFont(EGAMEFONT_SIZE_12);
    m_cInnerDepthIndex.SetLocalPosition(15.0f, 10.0f);
    m_cInnerDepthIndex.SetAnchor(0.5f, 0.5f);
    m_cInnerDepthIndexBtn.SetID(EBTN_INNER_DEPTH_INDEX);
    m_cInnerDepthIndexBtn.SetDisplayWidgets(m_cBtnImg, m_cBtnImg);
    m_cInnerDepthIndexBtn.AddChild(m_cInnerDepthIndex);
    AddChild(m_cInnerDepthIndexBtn);

    m_cInnerDepthNum.SetFont(EGAMEFONT_SIZE_12);
    m_cInnerDepthNum.SetLocalPosition(15.0f, 10.0f);
    m_cInnerDepthNum.SetAnchor(0.5f, 0.5f);
    m_cInnerDepthNumBtn.SetID(EBTN_INNER_DEPTH_NUM);
    m_cInnerDepthNumBtn.SetDisplayWidgets(m_cBtnImg, m_cBtnImg);
    m_cInnerDepthNumBtn.AddChild(m_cInnerDepthNum);
    AddChild(m_cInnerDepthNumBtn);

    UpdateInnerLabels();
    UpdateOuterLabels();
    UpdateInnerBtnPositions();
    return TRUE;
}
VOID CNewBibBox::Release(VOID)
{
    m_cBibWidget.Release();
    m_cBibWidget.RemoveFromParent();
    
    m_cDepthDisplay.Release();
    m_cDepthDisplay.RemoveFromParent();
    m_cDepthValues.Release();
    m_cDepthValues.RemoveFromParent();
    
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        SAFE_REMOVE_RELEASE_DELETE(m_apcDepthhLbls[uIndex]);
    }
    
    m_cOuterDepthIndex.Release();
    m_cOuterDepthIndex.RemoveFromParent();
    m_cOuterDepthIndexBtn.RemoveFromParent();
    
    m_cOuterDepthNum.Release();
    m_cOuterDepthNum.RemoveFromParent();
    m_cOuterDepthNumBtn.RemoveFromParent();
    
    m_cInnerDepthIndex.Release();
    m_cInnerDepthIndex.RemoveFromParent();
    m_cInnerDepthIndexBtn.RemoveFromParent();
    
    m_cInnerDepthNum.Release();
    m_cInnerDepthNum.RemoveFromParent();
    m_cInnerDepthNumBtn.RemoveFromParent();
    
    CUIContainer::Release();
}

VOID CNewBibBox::ClearChart(VOID)
{
    m_cBibWidget.ClearChart();
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        if (NULL != m_apcDepthhLbls[uIndex]) {
            m_apcDepthhLbls[uIndex]->SetVisibility(FALSE);
        }
    }
}


#include "OandaMgr.h"
VOID CNewBibBox::PrepareChart(const STradeLog& sLog, BOOLEAN bOpen)
{
    if (m_psTradelog != &sLog) {
        m_sTmp = sLog;
        m_psTradelog = &sLog;
    }
    m_bTradeLogOpen = bOpen;
    ClearChart();
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        const BOOLEAN bEnabled = 0.0f < sLog.asStart[uIndex].fTop;
        m_cDepthDisplay.SetEnabled(uIndex, bEnabled);
    }
    FLOAT afSizes[MEGAZZ_MAX_DEPTH];
    const COandaMgr::SOandaPair* psPair = COandaMgr::GetInstance().GetTradePair(sLog.byTradePairIndex);
    if (NULL == psPair) {
        ASSERT(FALSE);
        return;
    }
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        const MegaZZ::SChannel& sChannel = sLog.asStart[uIndex];
        afSizes[uIndex] = (sChannel.fTop - sChannel.fBottom) * psPair->dPipMultiplier;
    }
    m_cDepthDisplay.Initialize(afSizes, 0, MEGAZZ_MAX_DEPTH, FALSE);
    m_cBibWidget.PrepareChart(sLog, bOpen);
    m_cDepthValues.Update(sLog, bOpen);
}

#define RETRACE_UP_COLOR        RGBA(110,210,230,0xFF)
#define RETRACE_DOWN_COLOR      RGBA(255,110,110,0xFF)

VOID CNewBibBox::PrepareChart(const CMegaZZ& cMegaZZ) // oldest in back
{
    ClearChart();
    m_pcMegaZZ = &cMegaZZ;
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        const BOOLEAN bEnabled = 0.0f < cMegaZZ.GetCurrentChannel(uIndex).fTop;
        m_cDepthDisplay.SetEnabled(uIndex, bEnabled);
        if (NULL != m_apcDepthhLbls[uIndex]) {
            m_apcDepthhLbls[uIndex]->SetVisibility(FALSE);
        }
    }
    FLOAT afSizes[MEGAZZ_MAX_DEPTH];
    BOOLEAN bShowPercent = FALSE;
    if (ESOURCE_MARKET_OANDA == cMegaZZ.GetSource()) {
        const COandaMgr::SOandaPair* psPair = COandaMgr::GetInstance().GetTradePair(cMegaZZ.GetTradePairIndex());
        if (NULL == psPair) {
            ASSERT(FALSE);
            return;
        }
        for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
            const MegaZZ::SChannel& sChannel = cMegaZZ.GetCurrentChannel(uIndex);
            afSizes[uIndex] = (sChannel.fTop - sChannel.fBottom) * psPair->dPipMultiplier;
        }
    }
    else {
        bShowPercent = TRUE;
        const FLOAT fPrice = cMegaZZ.GetCurrentPrice();
        for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
            const MegaZZ::SChannel& sChannel = cMegaZZ.GetCurrentChannel(uIndex);
            afSizes[uIndex] = (sChannel.fTop - sChannel.fBottom) * 100 / fPrice;
        }
    }
    m_cDepthDisplay.Initialize(afSizes, 0, MEGAZZ_MAX_DEPTH, bShowPercent);
    m_cDepthDisplay.UpdateLabels(cMegaZZ);
    m_cDepthValues.Update(cMegaZZ);
    
    m_uUsedLineNum = 0;
    m_uUsedBoxNum = 0;
    
    const SHLVector2D& sSize = GetLocalSize();
    const UINT uUpColor = CPlayerData::GetBiBColor(0);
    const UINT uDnColor = CPlayerData::GetBiBColor(1);
    FLOAT fStartX = 0.0f;
    FLOAT fStartY = 0.0f;
    FLOAT fEndX = sSize.x;
    FLOAT fEndY = sSize.y - NEW_BIB_BOX_HEIGHT_OFFSET;
    for (UINT uIndex = 0; m_uOuterDepthNum > uIndex; ++uIndex) {
        const UINT uDepthIndex = m_uOuterDepthIndex + uIndex;
        const MegaZZ::SChannel& sChannel = cMegaZZ.GetCurrentChannel(uDepthIndex);
        const BOOLEAN bUp = (sChannel.byType == MegaZZ::EType_Up);
        const BOOLEAN bRetrace = (sChannel.byDisplayNumber > 6);
        if (bUp) {
            if (NULL != m_apcDepthhLbls[uDepthIndex]) {
                m_apcDepthhLbls[uDepthIndex]->SetLocalPosition(fStartX + HIGHER_DEPTH_THICKNESS * 0.5f, fEndY - HIGHER_DEPTH_THICKNESS * 0.5f);
                m_apcDepthhLbls[uDepthIndex]->SetVisibility(TRUE);
            }
                
            const UINT uColor = bRetrace ? RETRACE_UP_COLOR : uUpColor;
            AddBox(fStartX, fStartY, HIGHER_DEPTH_THICKNESS, fEndY - fStartY, uColor);
            const FLOAT fX2 = fStartX + HIGHER_DEPTH_THICKNESS;
            AddLine(fX2, fStartY, fX2, fEndY - HIGHER_DEPTH_THICKNESS, RGBA(0,0,0,0xFF));

            AddBox(fX2, fEndY - HIGHER_DEPTH_THICKNESS, fEndX - fX2, HIGHER_DEPTH_THICKNESS, uColor);
            AddLine(fX2, fEndY - HIGHER_DEPTH_THICKNESS, fEndX, fEndY - HIGHER_DEPTH_THICKNESS, RGBA(0,0,0,0xFF));
            fStartX += HIGHER_DEPTH_THICKNESS;
            fEndY -= HIGHER_DEPTH_THICKNESS;
        }
        else {
            if (NULL != m_apcDepthhLbls[uDepthIndex]) {
                m_apcDepthhLbls[uDepthIndex]->SetLocalPosition(fEndX - HIGHER_DEPTH_THICKNESS * 0.5f, fStartY + HIGHER_DEPTH_THICKNESS * 0.5f);
                m_apcDepthhLbls[uDepthIndex]->SetVisibility(TRUE);
            }

            const UINT uColor = bRetrace ? RETRACE_DOWN_COLOR : uDnColor;
            AddBox(fStartX, fStartY, fEndX - fStartX, HIGHER_DEPTH_THICKNESS, uColor);
            const FLOAT fX2 = fEndX - HIGHER_DEPTH_THICKNESS;
            const FLOAT fY2 = fStartY + HIGHER_DEPTH_THICKNESS;
            AddBox(fX2, fY2, HIGHER_DEPTH_THICKNESS, fEndY - fY2, uColor);

            AddLine(fStartX, fY2, fX2, fY2, RGBA(0,0,0,0xFF));
            AddLine(fX2, fY2, fX2, fEndY, RGBA(0,0,0,0xFF));

            fEndX -= HIGHER_DEPTH_THICKNESS;
            fStartY += HIGHER_DEPTH_THICKNESS;
        }
    }
    m_cBibWidget.SetLocalPosition(fStartX, fStartY);
    m_cBibWidget.PrepareChart(cMegaZZ);
    UpdateInnerBtnPositions();
}

VOID CNewBibBox::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    SHLVector2D sWorldPos = GetWorldPosition();
    sWorldPos.x += fOffsetX;
    sWorldPos.y += fOffsetY;
    const SHLVector2D& sWorldSize = GetWorldSize();
    CGameRenderer::DrawRectOutline(sWorldPos.x, sWorldPos.y, sWorldSize.x, sWorldSize.y, RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    
    for (UINT uIndex = 0; m_uUsedBoxNum > uIndex; ++uIndex) {
        const SBox2D& sBox = m_asBox[uIndex];
        CGameRenderer::DrawRectNoTex(sWorldPos.x + sBox.sPos.x, sWorldPos.y + sBox.sPos.y, sBox.sSize.x, sBox.sSize.y, sBox.uColor);
    }
    for (UINT uIndex = 0; m_uUsedLineNum > uIndex; ++uIndex) {
        const SLine2D& sLine = m_asLine[uIndex];
        CGameRenderer::DrawLine(sWorldPos.x + sLine.sPt1.x, sWorldPos.y + sLine.sPt1.y, sWorldPos.x + sLine.sPt2.x, sWorldPos.y + sLine.sPt2.y, sLine.uColor);
    }
    CUIContainer::OnRender(fOffsetX, fOffsetY);
}


VOID CNewBibBox::AddBox(FLOAT fX, FLOAT fY, FLOAT fW, FLOAT fH, UINT uColor)
{
    if (EBOX_NUM <= m_uUsedBoxNum) {
        ASSERT(FALSE);
        return;
    }
    SBox2D& sBox = m_asBox[m_uUsedBoxNum];
    ++m_uUsedBoxNum;
    sBox.sPos.x = fX;
    sBox.sPos.y = fY;
    sBox.sSize.x = fW;
    sBox.sSize.y = fH;
    sBox.uColor = uColor;
}

VOID CNewBibBox::AddLine(FLOAT fX1, FLOAT fY1, FLOAT fX2, FLOAT fY2, UINT uColor)
{
    if (ELINE_NUM <= m_uUsedLineNum) {
        ASSERT(FALSE);
        return;
    }
    SLine2D& sLine = m_asLine[m_uUsedLineNum];
    ++m_uUsedLineNum;
    sLine.sPt1.x = fX1;
    sLine.sPt1.y = fY1;
    sLine.sPt2.x = fX2;
    sLine.sPt2.y = fY2;
    sLine.uColor = uColor;
}

VOID CNewBibBox::UpdateOuterLabels(VOID)
{
    const SHLVector2D& sLocalSize = GetLocalSize();
    const FLOAT fBoxW = sLocalSize.x - HIGHER_DEPTH_THICKNESS * m_uOuterDepthNum;
    const FLOAT fBoxH = sLocalSize.y - NEW_BIB_BOX_HEIGHT_OFFSET - HIGHER_DEPTH_THICKNESS * m_uOuterDepthNum;
    m_cBibWidget.SetLocalSize(fBoxW, fBoxH);
    
    CHAR szBuffer[32];
    snprintf(szBuffer, 32, "F%d", 9 - m_uOuterDepthIndex);
    m_cOuterDepthIndex.SetString(szBuffer);
    
    snprintf(szBuffer, 32, "N%d", m_uOuterDepthNum);
    m_cOuterDepthNum.SetString(szBuffer);
    
    if (NULL != m_pcMegaZZ) {
        PrepareChart(*m_pcMegaZZ);
    }
}

VOID CNewBibBox::UpdateInnerLabels(VOID)
{
    CHAR szBuffer[32];
    snprintf(szBuffer, 32, "F%d", 9 - m_cBibWidget.GetDepthIndex());
    m_cInnerDepthIndex.SetString(szBuffer);
    
    snprintf(szBuffer, 32, "N%d", m_cBibWidget.GetDepthNum());
    m_cInnerDepthNum.SetString(szBuffer);
}

VOID CNewBibBox::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_OUTER_DEPTH_INDEX:
            OnTapOuterDepthIndex();
            break;
        case EBTN_OUTER_DEPTH_NUM:
            OnTapOuterDepthNum();
            break;
        case EEVENT_OUTER_DEPTH_INDEX_REPLY:
            OnReplyOuterDepthIndex(cEvent.GetIntParam(2));
            break;
        case EEVENT_OUTER_DEPTH_NUM_REPLY:
            OnReplyOuterDepthNum(cEvent.GetIntParam(2));
            break;
        case EBTN_INNER_DEPTH_INDEX:
            OnTapInnerDepthIndex();
            break;;
        case EEVENT_INNER_DEPTH_INDEX_REPLY:
            OnReplyInnerDepthIndex(cEvent.GetIntParam(2));
            break;
        case EBTN_INNER_DEPTH_NUM:
            OnTapInnerDepthNum();
            break;
        case EEVENT_INNER_DEPTH_NUM_REPLY:
            OnReplyInnerDepthNum(cEvent.GetIntParam(2));
            break;
        default:
            break;
    }
}


VOID CNewBibBox::OnTapOuterDepthIndex(VOID)
{
    CTextSelectorWindow* pcWin = new CTextSelectorWindow(*this, EEVENT_OUTER_DEPTH_INDEX_REPLY, 0);
    if (NULL == pcWin) {
        ASSERT(FALSE);
        return;
    }
    const CHAR* asValues[5] = { "F9", "F8", "F7", "F6", "F5" };
    pcWin->Initialize(asValues, 5, m_uOuterDepthIndex);
    const SHLVector2D& sWorldPos = m_cOuterDepthIndexBtn.GetWorldPosition();
    pcWin->SetLocalPosition(sWorldPos.x + 20.0f, sWorldPos.y + 20.0f);
    pcWin->DoModal();
}

VOID CNewBibBox::OnReplyOuterDepthIndex(UINT uDepthIndex)
{
    m_uOuterDepthIndex = uDepthIndex;
    UpdateOuterLabels();
}
VOID CNewBibBox::OnTapOuterDepthNum(VOID)
{
    CTextSelectorWindow* pcWin = new CTextSelectorWindow(*this, EEVENT_OUTER_DEPTH_NUM_REPLY, 0);
    if (NULL == pcWin) {
        ASSERT(FALSE);
        return;
    }
    const CHAR* asValues[5] = { "5", "4", "3", "2", "1" };
    pcWin->Initialize(asValues, 5, 5 - m_uOuterDepthNum);
    const SHLVector2D& sWorldPos = m_cOuterDepthIndexBtn.GetWorldPosition();
    pcWin->SetLocalPosition(sWorldPos.x + 20.0f, sWorldPos.y + 20.0f);
    pcWin->DoModal();
}

VOID CNewBibBox::OnReplyOuterDepthNum(UINT uDepthNum)
{
    m_uOuterDepthNum = 5 - uDepthNum;
    UpdateOuterLabels();
}

VOID CNewBibBox::UpdateInnerBtnPositions(VOID)
{
    const SHLVector2D& sPos = m_cBibWidget.GetLocalPosition();
    const SHLVector2D& sSize = m_cBibWidget.GetLocalSize();
    const FLOAT fX = sPos.x + sSize.x - 70.0f;
    const FLOAT fY = sPos.y + sSize.y - 25.0f;
    m_cInnerDepthIndexBtn.SetLocalPosition(fX, fY);
    m_cInnerDepthNumBtn.SetLocalPosition(fX + 35.0f, fY);
}

VOID CNewBibBox::OnTapInnerDepthIndex(VOID)
{
    CTextSelectorWindow* pcWin = new CTextSelectorWindow(*this, EEVENT_INNER_DEPTH_INDEX_REPLY, 0);
    if (NULL == pcWin) {
        ASSERT(FALSE);
        return;
    }
    const CHAR* asValues[7] = { "F9", "F8", "F7", "F6", "F5", "F4", "F3" };
    pcWin->Initialize(asValues, 7, m_cBibWidget.GetDepthIndex());
    const SHLVector2D& sWorldPos = m_cInnerDepthIndexBtn.GetWorldPosition();
    pcWin->SetLocalPosition(sWorldPos.x + 20.0f, sWorldPos.y + 20.0f);
    pcWin->DoModal();

}

VOID CNewBibBox::OnReplyInnerDepthIndex(UINT uDepthIndex)
{
    m_cBibWidget.SetDepthIndex(uDepthIndex);
    UpdateInnerLabels();
    if (NULL != m_pcMegaZZ) {
        PrepareChart(*m_pcMegaZZ);
    }
}

VOID CNewBibBox::OnTapInnerDepthNum(VOID)
{
    CTextSelectorWindow* pcWin = new CTextSelectorWindow(*this, EEVENT_INNER_DEPTH_NUM_REPLY, 0);
    if (NULL == pcWin) {
        ASSERT(FALSE);
        return;
    }
    const CHAR* asValues[9] = { "9", "8", "7", "6", "5", "4", "3", "2", "1" };
    pcWin->Initialize(asValues, 9, 9 - m_cBibWidget.GetDepthNum());
    const SHLVector2D& sWorldPos = m_cInnerDepthNumBtn.GetWorldPosition();
    pcWin->SetLocalPosition(sWorldPos.x + 20.0f, sWorldPos.y + 20.0f);
    pcWin->DoModal();
}

VOID CNewBibBox::OnReplyInnerDepthNum(UINT uSelectedIndex)
{
    m_cBibWidget.SetDepthNum(9 - uSelectedIndex);
    UpdateInnerLabels();
    if (NULL != m_pcMegaZZ) {
        PrepareChart(*m_pcMegaZZ);
    }
}
