#include "stdafx.h"
#include "AppMain.h"
#include "BibBox.h"
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

CBibBox::CBibBox() :
CUIContainer(EBaseWidget_Container, EGLOBAL_BIB_BOX_CHART),
m_pcMegaZZ(NULL),
m_psTradelog(NULL),
m_bTradeLogOpen(TRUE),
m_cDepthValues(0, 0, EBTN_UNNUSED),
INITIALIZE_TEXT_LABEL(m_cChangeDepthTxt),
INITIALIZE_TEXT_LABEL(m_cXToggleTxt),
INITIALIZE_TEXT_LABEL(m_cChangeDepthNumTxt),
m_bDisableTap(FALSE)
{

}

CBibBox::~CBibBox()
{

}
    
BOOLEAN CBibBox::Initialize(VOID)
{
    const SHLVector2D& sLocalSize = GetLocalSize();
    m_cBibWidget.Initialize();
    m_cBibWidget.RemoveFromParent();
    AddChild(m_cBibWidget);

    m_cDepthDisplay.SetLocalPosition(5.0f, sLocalSize.y - 57.0f);
    AddChild(m_cDepthDisplay);
    m_cDepthValues.SetLocalPosition(-142.0f, sLocalSize.y - 27.0f);
    m_cDepthValues.SetAdditionalSpacing(6.0f);
    m_cDepthValues.HideNameAndPrice();
    m_cDepthValues.SetDisplay(m_cBibWidget.GetDepthNum(), m_cBibWidget.GetDepthIndex());
    m_cDepthValues.Initialize();
    AddChild(m_cDepthValues);

    if (!m_bDisableTap) {
        m_cBibWidget.SetLocalPosition(0.0f, 30.0f);
        m_cBibWidget.SetLocalSize(sLocalSize.x, sLocalSize.y - 30.0f - 60.0f);

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
        m_cChangeDepthBtn.SetLocalPosition(3.0f, 3.0f);
        m_cChangeDepthBtn.AddChild(m_cChangeDepthTxt);
        AddChild(m_cChangeDepthBtn);
        
        m_cXToggleTxt.SetFont(EGAMEFONT_SIZE_14);
        m_cXToggleTxt.SetEffect(CUITextLabel::EFFECT_4SIDE_SHADOW, 1.0f, 1.0f);
        m_cXToggleTxt.SetAnchor(0.5f, 0.5f);
        m_cXToggleTxt.SetLocalPosition(13.0f, 10.0f);
        m_cXToggleTxt.SetString("X");
        m_cXToggleBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
        m_cXToggleBtn.SetID(EBTN_TOGGLE_HIDE_X);
        m_cXToggleBtn.SetLocalPosition(3.0f + 29.0f, 3.0f);
        m_cXToggleBtn.AddChild(m_cXToggleTxt);
        AddChild(m_cXToggleBtn);
        
        m_cChangeDepthNumTxt.SetFont(EGAMEFONT_SIZE_14);
        m_cChangeDepthNumTxt.SetEffect(CUITextLabel::EFFECT_4SIDE_SHADOW, 1.0f, 1.0f);
        m_cChangeDepthNumTxt.SetAnchor(0.5f, 0.5f);
        m_cChangeDepthNumTxt.SetLocalPosition(13.0f, 10.0f);
        m_cChangeDepthNumBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
        m_cChangeDepthNumBtn.SetID(EBTN_CHANGE_DEPTH_NUM);
        m_cChangeDepthNumBtn.SetAnchor(1.0f, 0.0f);
        m_cChangeDepthNumBtn.SetLocalPosition(sLocalSize.x - 3.0f, 3.0f);
        m_cChangeDepthNumBtn.AddChild(m_cChangeDepthNumTxt);
        AddChild(m_cChangeDepthNumBtn);
        
        m_cUpBG.SetLocalSize(26.0f, 20.0f);
        m_cUpBtn.SetDisplayWidgets(m_cUpBG, m_cUpBG);
        m_cUpBtn.SetID(EBTN_CHANGE_UP_COLOR);
        m_cUpBtn.SetAnchor(1.0f, 0.0f);
        m_cUpBtn.SetLocalPosition(sLocalSize.x - 3.0f - 29.0f - 29.0f, 3.0f);
        m_cUpBtn.RemoveFromParent();
        AddChild(m_cUpBtn);
        
        m_cDownBG.SetLocalSize(26.0f, 20.0f);
        m_cDownBtn.SetDisplayWidgets(m_cDownBG, m_cDownBG);
        m_cDownBtn.SetID(EBTN_CHANGE_DOWN_COLOR);
        m_cDownBtn.SetAnchor(1.0f, 0.0f);
        m_cDownBtn.SetLocalPosition(sLocalSize.x - 3.0f - 29.0f, 3.0f);
        m_cDownBtn.RemoveFromParent();
        AddChild(m_cDownBtn);
        
        UpdateDepthLabel();
        UpdateDepthNumLabel();
        UpdateBtnColors();
    }
    else {
        m_cBibWidget.SetLocalSize(sLocalSize.x, sLocalSize.y);
    }
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_PLAYER_DATA, EGLOBAL_EVT_PLAYER_DATA_BIB_COLOR_UPDATED);
    return TRUE;
}

VOID CBibBox::HideSideBars(BOOLEAN bHide)
{
    m_cBibWidget.SetHideBars(bHide);
}
    
VOID CBibBox::Release(VOID)
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
    
    m_cXToggleTxt.Release();
    m_cXToggleTxt.RemoveFromParent();
    m_cXToggleBtn.RemoveFromParent();
    
    m_cChangeDepthNumTxt.Release();
    m_cChangeDepthNumTxt.RemoveFromParent();
    m_cChangeDepthNumBtn.RemoveFromParent();
    
    m_cUpBtn.RemoveFromParent();
    m_cDownBtn.RemoveFromParent();
    
    CUIContainer::Release();
}

VOID CBibBox::ClearChart(VOID)
{
    m_cBibWidget.ClearChart();
}

VOID CBibBox::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    CEventManager::PostEvent(*this, cEvent, fDelay);
}

VOID CBibBox::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEvent = cEvent.GetIntParam(0);
    switch (uEvent) {
        case EBTN_CHANGE_UP_COLOR:
            OnTapChangeColor(TRUE);
            break;
        case EBTN_CHANGE_DOWN_COLOR:
            OnTapChangeColor(FALSE);
            break;
        case EBTN_TOGGLE_HIDE_X:
            OnTapToggleX();
            break;
        case EEVENT_ON_CHANGE_UP_COLOR:
            OnChangeColor(TRUE, cEvent.GetIntParam(1));
            break;
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
        case EEVENT_ON_CHANGE_DOWN_COLOR:
            OnChangeColor(FALSE, cEvent.GetIntParam(1));
            break;
        case EGLOBAL_EVT_PLAYER_DATA_BIB_COLOR_UPDATED:
            OnBibColorUpdated();
            break;
    }
}

VOID CBibBox::OnTapChangeColor(BOOLEAN bUp)
{
    const UINT uEventID = bUp ? EEVENT_ON_CHANGE_UP_COLOR : EEVENT_ON_CHANGE_DOWN_COLOR;
    const UINT uColor = bUp ? CPlayerData::GetBiBColor(0) : CPlayerData::GetBiBColor(1);
    CColorPickerWindow* pcWin = new CColorPickerWindow(EGLOBAL_BIB_BOX_CHART, uEventID, uColor);
    if (NULL != pcWin) {
        pcWin->DoModal();
    }
}

VOID CBibBox::OnChangeColor(BOOLEAN bUp, UINT uColor)
{
    const UINT uIndex = bUp ? 0 : 1;
    CPlayerData::SetBiBColor(uIndex, uColor);
    CSaveData::SaveData(FALSE);
    CEventManager::BroadcastEvent(EGLOBAL_PLAYER_DATA, CEvent(EGLOBAL_EVT_PLAYER_DATA_BIB_COLOR_UPDATED));
}
VOID CBibBox::OnBibColorUpdated(VOID)
{
    UpdateBtnColors();
    ReloadChart();
}

VOID CBibBox::UpdateBtnColors(VOID)
{
    UINT uColor = CPlayerData::GetBiBColor(0);
    BYTE byR = GET_RED(uColor) / 2;
    BYTE byG = GET_GREEN(uColor) / 2;
    BYTE byB = GET_BLUE(uColor) / 2;
    m_cUpBG.SetColor(RGBA(byR, byG, byB, 0xFF));
    
    uColor = CPlayerData::GetBiBColor(1);
    byR = GET_RED(uColor) / 2;
    byG = GET_GREEN(uColor) / 2;
    byB = GET_BLUE(uColor) / 2;
    m_cDownBG.SetColor(RGBA(byR, byG, byB, 0xFF));
}

#include "OandaMgr.h"
VOID CBibBox::PrepareChart(const STradeLog& sLog, BOOLEAN bOpen)
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
    const UINT uBiggestIndex = m_cBibWidget.GetDepthIndex();
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
    m_cDepthDisplay.Initialize(afSizes, uBiggestIndex, m_cBibWidget.GetDepthNum(), FALSE);
    m_cBibWidget.PrepareChart(sLog, bOpen);
    //m_cDepthDisplay.UpdateLabels(<#const CMegaZZ &cZZ#>)
    m_cDepthValues.Update(sLog, bOpen);
}

VOID CBibBox::PrepareChart(const CMegaZZ& cMegaZZ) // oldest in back
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

VOID CBibBox::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    SHLVector2D sWorldPos = GetWorldPosition();
    sWorldPos.x += fOffsetX;
    sWorldPos.y += fOffsetY;
    const SHLVector2D& sWorldSize = GetWorldSize();
    CGameRenderer::DrawRectOutline(sWorldPos.x, sWorldPos.y, sWorldSize.x, sWorldSize.y, RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    CUIContainer::OnRender(fOffsetX, fOffsetY);
}

VOID CBibBox::OnTapToggleX(VOID)
{
    m_cBibWidget.SetHideX(!m_cBibWidget.IsHideX());
    ReloadChart();
}

VOID CBibBox::SetSmallestChannelWhite(VOID)
{
    m_cBibWidget.SetSmallestChannelWhite(TRUE);
}

VOID CBibBox::UpdateDepthLabel(VOID)
{
    const UINT uBiggestIndex = m_cBibWidget.GetDepthIndex();
    CHAR szBuffer[64];
    snprintf(szBuffer, 64, "D%d", 9 - uBiggestIndex);
    m_cChangeDepthTxt.SetString(szBuffer);
}

VOID CBibBox::UpdateDepthNumLabel(VOID)
{
    const UINT uDepthNum = m_cBibWidget.GetDepthNum();
    CHAR szBuffer[64];
    snprintf(szBuffer, 64, "N%d", uDepthNum);
    m_cChangeDepthNumTxt.SetString(szBuffer);
}

VOID CBibBox::OnTapChangeDepth(VOID)
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

VOID CBibBox::OnTapDepthBtn(UINT uDepthIndex)
{
    m_cBibWidget.SetDepthIndex(uDepthIndex);
    UpdateDepthLabel();
    m_cDepthValues.Release();
    m_cDepthValues.SetDisplay(m_cBibWidget.GetDepthNum(), m_cBibWidget.GetDepthIndex());
    m_cDepthValues.Initialize();
    ReloadChart();
}

VOID CBibBox::SetDefaultDepthIndex(UINT uDepthIndex)
{
    m_cBibWidget.SetDepthIndex(uDepthIndex);
}
VOID CBibBox::SetDefaultDepthNum(UINT uDepthNum)
{
    m_cBibWidget.SetDepthNum(uDepthNum);
}

VOID CBibBox::OnTapChangeDepthNum(VOID)
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
VOID CBibBox::OnTapDepthNumBtn(UINT uIndex)
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

VOID CBibBox::ReloadChart(VOID)
{
    if (NULL != m_pcMegaZZ) {
        PrepareChart(*m_pcMegaZZ);
        return;
    }
    if (NULL != m_psTradelog) {
        PrepareChart(*m_psTradelog, m_bTradeLogOpen);
    }
}

