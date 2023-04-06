#include "stdafx.h"
#include "AppMain.h"
#include "StatusBox.h"
#include "ColorPickerWindow.h"
#include "ConfirmationWindow.h"
#include "Event.h"
#include "EventManager.h"
#include "GameRenderer.h"
#include "MegaZZ.h"
#include "PlayerData.h"
#include "SaveData.h"
#include "TextSelectorWindow.h"
#include "TradeLog.h"

CStatusBox::CStatusBox() :
CUIContainer(EBaseWidget_Container, EGLOBAL_BIB_BOX_CHART),
m_pcMegaZZ(NULL),
m_bTradeLogOpen(TRUE),
m_cDepthValues(0, 0, EBTN_UNNUSED),
INITIALIZE_TEXT_LABEL(m_cChangeDepthTxt),
INITIALIZE_TEXT_LABEL(m_cChangeDepthNumTxt),
m_bDisableTap(FALSE)
{

}

CStatusBox::~CStatusBox()
{

}
    
BOOLEAN CStatusBox::Initialize(VOID)
{
    const SHLVector2D& sLocalSize = GetLocalSize();
    m_cBibWidget.Initialize();
    m_cBibWidget.RemoveFromParent();
    m_cBibWidget.SetDepthNum(9);
    AddChild(m_cBibWidget);

    m_cDepthDisplay.SetLocalPosition(5.0f, sLocalSize.y - 57.0f);
    AddChild(m_cDepthDisplay);
    m_cDepthValues.SetLocalPosition(-142.0f, sLocalSize.y - 27.0f);
    m_cDepthValues.SetAdditionalSpacing(6.0f);
    m_cDepthValues.HideNameAndPrice();
    m_cDepthValues.SetDisplay(m_cBibWidget.GetDepthNum(), m_cBibWidget.GetDepthIndex());
    m_cDepthValues.Initialize();
    AddChild(m_cDepthValues);

    m_cBibWidget.SetLocalPosition(0.0f, 0.0f);
    m_cBibWidget.SetLocalSize(sLocalSize.x, sLocalSize.y - 60.0f);

    m_cBtnBG.SetLocalSize(26.0f, 20.0f);
    m_cBtnBG.SetColor(RGBA(0x3F, 0x3F, 0x3F, 0xFF));
    m_cSelectedBtnBG.SetLocalSize(26.0f, 20.0f);
    m_cSelectedBtnBG.SetColor(RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    
    m_cChangeDepthTxt.SetFont(EGAMEFONT_SIZE_14);
    m_cChangeDepthTxt.SetEffect(CUITextLabel::EFFECT_4SIDE_SHADOW, 1.0f, 1.0f);
    m_cChangeDepthTxt.SetAnchor(0.5f, 0.5f);
    m_cChangeDepthTxt.SetLocalPosition(13.0f, 10.0f);
    m_cChangeDepthBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
    m_cChangeDepthBtn.SetID(EBTN_CHANGE_DEPTH);
    m_cChangeDepthBtn.SetAnchor(1.0f, 1.0f);
    m_cChangeDepthBtn.SetLocalPosition(sLocalSize.x - 5.0f - 29.0f, sLocalSize.y - 5.0f);
    m_cChangeDepthBtn.AddChild(m_cChangeDepthTxt);
    AddChild(m_cChangeDepthBtn);
    
    m_cChangeDepthNumTxt.SetFont(EGAMEFONT_SIZE_14);
    m_cChangeDepthNumTxt.SetEffect(CUITextLabel::EFFECT_4SIDE_SHADOW, 1.0f, 1.0f);
    m_cChangeDepthNumTxt.SetAnchor(0.5f, 0.5f);
    m_cChangeDepthNumTxt.SetLocalPosition(13.0f, 10.0f);
    m_cChangeDepthNumBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
    m_cChangeDepthNumBtn.SetID(EBTN_CHANGE_DEPTH_NUM);
    m_cChangeDepthNumBtn.SetAnchor(1.0f, 1.0f);
    m_cChangeDepthNumBtn.SetLocalPosition(sLocalSize.x - 5.0f, sLocalSize.y - 5.0f);
    m_cChangeDepthNumBtn.AddChild(m_cChangeDepthNumTxt);
    AddChild(m_cChangeDepthNumBtn);
    
    UpdateDepthLabel();
    UpdateDepthNumLabel();

    CEventManager::RegisterForBroadcast(*this, EGLOBAL_PLAYER_DATA, EGLOBAL_EVT_PLAYER_DATA_BIB_COLOR_UPDATED);
    return TRUE;
}

VOID CStatusBox::Release(VOID)
{
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_PLAYER_DATA, EGLOBAL_EVT_PLAYER_DATA_BIB_COLOR_UPDATED);
    m_cBibWidget.Release();
    m_cBibWidget.RemoveFromParent();
    
    m_cDepthDisplay.Release();
    m_cDepthDisplay.RemoveFromParent();
    m_cDepthValues.Release();
    m_cDepthValues.RemoveFromParent();
    
    m_cChangeDepthTxt.Release();
    m_cChangeDepthTxt.RemoveFromParent();
    m_cChangeDepthBtn.RemoveFromParent();
    
    m_cChangeDepthNumTxt.Release();
    m_cChangeDepthNumTxt.RemoveFromParent();
    m_cChangeDepthNumBtn.RemoveFromParent();
    
    CUIContainer::Release();
}

VOID CStatusBox::ClearChart(VOID)
{
    m_cBibWidget.ClearChart();
}

VOID CStatusBox::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    CEventManager::PostEvent(*this, cEvent, fDelay);
}

VOID CStatusBox::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEvent = cEvent.GetIntParam(0);
    switch (uEvent) {
        case EBTN_CHANGE_DEPTH:
            OnTapChangeDepth();
            break;
        case EEVENT_ON_CHANGE_DEPTH_INDEX:
            OnTapDepthBtn(cEvent.GetIntParam(2));
            break;
        case EBTN_CHANGE_DEPTH_NUM:
            OnTapChangeDepthNum();
            break;
        case EEVENT_ON_CHANGE_DEPTH_NUM:
            OnTapDepthNumBtn(cEvent.GetIntParam(2));
            break;
        case EGLOBAL_EVT_PLAYER_DATA_BIB_COLOR_UPDATED:
            OnBibColorUpdated();
            break;
    }
}

VOID CStatusBox::OnBibColorUpdated(VOID)
{
    ReloadChart();
}

#include "OandaMgr.h"

VOID CStatusBox::PrepareChart(const CMegaZZ& cMegaZZ) // oldest in back
{
    ClearChart();
    m_pcMegaZZ = &cMegaZZ;
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        const BOOLEAN bEnabled = 0.0f < cMegaZZ.GetCurrentChannel(uIndex).fTop;
        m_cDepthDisplay.SetEnabled(uIndex, bEnabled);
    }
    const UINT uBiggestIndex = m_cBibWidget.GetDepthIndex();
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
    m_cDepthDisplay.Initialize(afSizes, uBiggestIndex, m_cBibWidget.GetDepthNum(), bShowPercent);
    m_cBibWidget.PrepareChart(cMegaZZ);
    m_cDepthDisplay.UpdateLabels(cMegaZZ);
    m_cDepthValues.Update(cMegaZZ);
}

VOID CStatusBox::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    SHLVector2D sWorldPos = GetWorldPosition();
    sWorldPos.x += fOffsetX;
    sWorldPos.y += fOffsetY;
    const SHLVector2D& sWorldSize = GetWorldSize();
    CGameRenderer::DrawRectOutline(sWorldPos.x, sWorldPos.y, sWorldSize.x, sWorldSize.y, RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    CUIContainer::OnRender(fOffsetX, fOffsetY);
}

VOID CStatusBox::UpdateDepthLabel(VOID)
{
    const UINT uBiggestIndex = m_cBibWidget.GetDepthIndex();
    CHAR szBuffer[64];
    snprintf(szBuffer, 64, "D%d", 9 - uBiggestIndex);
    m_cChangeDepthTxt.SetString(szBuffer);
}

VOID CStatusBox::UpdateDepthNumLabel(VOID)
{
    const UINT uDepthNum = m_cBibWidget.GetDepthNum();
    CHAR szBuffer[64];
    snprintf(szBuffer, 64, "N%d", uDepthNum);
    m_cChangeDepthNumTxt.SetString(szBuffer);
}

VOID CStatusBox::OnTapChangeDepth(VOID)
{
    CTextSelectorWindow* pcWin = new CTextSelectorWindow(*this, EEVENT_ON_CHANGE_DEPTH_INDEX, 0);
    if (NULL == pcWin) {
        return;
    }
    const SHLVector2D& sWorldPos = m_cChangeDepthBtn.GetWorldPosition();
    pcWin->SetLocalPosition(sWorldPos.x + 5.0f, sWorldPos.y + 5.0f);
    const CHAR* aszSelections[8] = {
        "D9", "D8", "D7", "D6", "D5", "D4", "D3", "D2"
    };
    if (!pcWin->Initialize(aszSelections, 8, m_cBibWidget.GetDepthIndex())) {
        ASSERT(FALSE);
        pcWin->Release();
        delete pcWin;
        return;
    }
    pcWin->DoModal();
}

VOID CStatusBox::OnTapDepthBtn(UINT uDepthIndex)
{
    m_cBibWidget.SetDepthIndex(uDepthIndex);
    UpdateDepthLabel();
    m_cDepthValues.Release();
    m_cDepthValues.SetDisplay(m_cBibWidget.GetDepthNum(), m_cBibWidget.GetDepthIndex());
    m_cDepthValues.Initialize();
    ReloadChart();
}

VOID CStatusBox::SetDefaultDepthIndex(UINT uDepthIndex)
{
    m_cBibWidget.SetDepthIndex(uDepthIndex);
}
VOID CStatusBox::SetDefaultDepthNum(UINT uDepthNum)
{
    m_cBibWidget.SetDepthNum(uDepthNum);
}

VOID CStatusBox::OnTapChangeDepthNum(VOID)
{
    CTextSelectorWindow* pcWin = new CTextSelectorWindow(*this, EEVENT_ON_CHANGE_DEPTH_NUM, 0);
    if (NULL == pcWin) {
        return;
    }
    const SHLVector2D& sWorldPos = m_cChangeDepthNumBtn.GetWorldPosition();
    pcWin->SetLocalPosition(sWorldPos.x + 5.0f, sWorldPos.y + 5.0f);
    const CHAR* aszSelections[8] = {
        "N2", "N3", "N4", "N5", "N6", "N7", "N8", "N9"
    };
    const UINT uSelectedIndex = m_cBibWidget.GetDepthNum() - 2;
    if (!pcWin->Initialize(aszSelections, 8, uSelectedIndex)) {
        ASSERT(FALSE);
        pcWin->Release();
        delete pcWin;
        return;
    }
    pcWin->DoModal();
}
VOID CStatusBox::OnTapDepthNumBtn(UINT uIndex)
{
    const UINT uDepthNum = 2 + uIndex;
    ASSERT(9 >= uDepthNum);
    m_cBibWidget.SetDepthNum(uDepthNum);
    UpdateDepthNumLabel();
    m_cDepthValues.Release();
    m_cDepthValues.SetDisplay(m_cBibWidget.GetDepthNum(), m_cBibWidget.GetDepthIndex());
    m_cDepthValues.Initialize();
    ReloadChart();
}

VOID CStatusBox::ReloadChart(VOID)
{
    if (NULL != m_pcMegaZZ) {
        PrepareChart(*m_pcMegaZZ);
        return;
    }
}

