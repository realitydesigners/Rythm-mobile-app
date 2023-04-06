#include "stdafx.h"
#include "AutoDashboardList.h"
#include "AutomationMgr.h"
#include "CryptoUtil.h"
#include "EventID.h"
#include "EventManager.h"
#include "MegaZZ.h"
#include "RythmDashboardWidget.h"

VOID CAutoDashboardList::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    // do not post to parent.
    CEventManager::PostEvent(*this, cEvent, fDelay);
}

CAutoDashboardList::CAutoDashboardList(UINT uSelectedEventID) :
CUIContainer(EBaseWidget_Container),
m_uSelectedEventID(uSelectedEventID),
INITIALIZE_TEXT_LABEL(m_cNameLbl),
INITIALIZE_TEXT_LABEL(m_cPriceLbl),
m_ppcWidgets(NULL),
m_uTradePairNum(0)
{
    memset(m_apcBarLbls, 0, sizeof(m_apcBarLbls));
    memset(m_abToggleOn, 1, sizeof(m_abToggleOn));
}
    
CAutoDashboardList::~CAutoDashboardList()
{
}
 
VOID CAutoDashboardList::Initialize(ESOURCE_MARKET eSource, UINT uTradePairNum)
{
    const FLOAT fWidth = (RYTHM_DASHBOARD_WIDGET_BTN_START_X + 9 * RYTHM_DASHBOARD_WIDGET_HEIGHT + 5.0f);
    const FLOAT fHeight = uTradePairNum * RYTHM_DASHBOARD_WIDGET_HEIGHT + 25.0f;// top buffer
    SetLocalSize(fWidth, fHeight);

    m_cNameLbl.SetFont(EGAMEFONT_SIZE_12);
    m_cNameLbl.SetAnchor(0.0f, 0.5f);
    m_cNameLbl.SetLocalPosition(RYTHM_DASHBOARD_WIDGET_NAME_X, 10.0f);
    m_cNameLbl.SetString("chart");
    AddChild(m_cNameLbl);

    m_cPriceLbl.SetFont(EGAMEFONT_SIZE_12);
    m_cPriceLbl.SetAnchor(0.0f, 0.5f);
    m_cPriceLbl.SetLocalPosition(RYTHM_DASHBOARD_WIDGET_PRICE_X, 10.0f);
    m_cPriceLbl.SetString("price");
    AddChild(m_cPriceLbl);

    m_cBtnImg.SetLocalSize(RYTHM_DASHBOARD_WIDGET_HEIGHT, RYTHM_DASHBOARD_WIDGET_HEIGHT);
    m_cBtnImg.SetVisibility(FALSE);
    CHAR szBuffer[64];
    FLOAT fBarX = RYTHM_DASHBOARD_WIDGET_BTN_START_X;
    for (UINT uIndex = 0; 9 > uIndex; ++uIndex) {
        CUITextLabel* pcLbl = NEW_TEXT_LABEL;
        if (NULL == pcLbl) {
            ASSERT(FALSE);
            return;
        }
        pcLbl->SetFont(EGAMEFONT_SIZE_10);
        pcLbl->SetAnchor(0.5f, 0.5f);
        pcLbl->SetLocalPosition(RYTHM_DASHBOARD_WIDGET_HEIGHT * 0.5f, RYTHM_DASHBOARD_WIDGET_HEIGHT * 0.5f);
        snprintf(szBuffer, 64, "F%d", 9 - uIndex);
        pcLbl->SetString(szBuffer);
        m_apcBarLbls[uIndex] = pcLbl;
        m_acBtns[uIndex].SetLocalPosition(fBarX, -2.0f);
        m_acBtns[uIndex].SetDisplayWidgets(m_cBtnImg, m_cBtnImg);
        m_acBtns[uIndex].AddChild(*pcLbl);
        m_acBtns[uIndex].SetID(EBTN_TAP_BOX_00 + uIndex);
        AddChild(m_acBtns[uIndex]);
        fBarX += RYTHM_DASHBOARD_WIDGET_HEIGHT;
    }
    m_ppcWidgets = new CRythmDashboardWidget*[uTradePairNum];
    if (NULL == m_ppcWidgets) {
        ASSERT(FALSE);
        return;
    }
    m_eSource = eSource;
    m_uTradePairNum = uTradePairNum;
    FLOAT fWidgetY = 22.0f;
    for (UINT uIndex = 0; m_uTradePairNum > uIndex; ++uIndex) {
        CRythmDashboardWidget* pcWidget = new CRythmDashboardWidget(uIndex, 0, EEVENT_SELECTED_PAIR);
        if (NULL == pcWidget) {
            ASSERT(FALSE);
            return;
        }
        pcWidget->SetLocalPosition(0.0f, fWidgetY);
        pcWidget->Initialize();
        pcWidget->UpdateTradePairName(CCryptoUtil::GetTradePairName(m_eSource, uIndex));
        if (0 == uIndex) {
            pcWidget->SetSelected();
        }
        AddChild(*pcWidget);
        m_ppcWidgets[uIndex] = pcWidget;
        fWidgetY += RYTHM_DASHBOARD_WIDGET_HEIGHT;
    }
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_AUTOMATION_MGR, EGLOBAL_EVT_AUTOMATION_MEGAZZ_UPDATED);
}

VOID CAutoDashboardList::Release(VOID)
{
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_AUTOMATION_MGR, EGLOBAL_EVT_AUTOMATION_MEGAZZ_UPDATED);
    m_cNameLbl.Release();
    m_cNameLbl.RemoveFromParent();
    
    m_cPriceLbl.Release();
    m_cPriceLbl.RemoveFromParent();
    
    for (UINT uIndex = 0; 9 > uIndex; ++uIndex) {
        if (NULL != m_apcBarLbls[uIndex]) {
            m_apcBarLbls[uIndex]->Release();
            m_apcBarLbls[uIndex]->RemoveFromParent();
            delete m_apcBarLbls[uIndex];
            m_apcBarLbls[uIndex] = NULL;
        }
        m_acBtns[uIndex].RemoveFromParent();        
    }
    if (NULL != m_ppcWidgets) {
        for (UINT uIndex = 0; m_uTradePairNum > uIndex; ++uIndex) {
            if (NULL != m_ppcWidgets[uIndex]) {
                m_ppcWidgets[uIndex]->Release();
                m_ppcWidgets[uIndex]->RemoveFromParent();
                delete m_ppcWidgets[uIndex];
            }
        }
        SAFE_DELETE_ARRAY(m_ppcWidgets);
    }
    m_uTradePairNum = 0;
    CUIContainer::Release();
}

VOID CAutoDashboardList::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EEVENT_SELECTED_PAIR:
            OnTradePairSelected(cEvent.GetIntParam(1));
            break;
        case EGLOBAL_EVT_AUTOMATION_MEGAZZ_UPDATED:
            OnMegaZZUpdated(cEvent.GetIntParam(1));
            break;
        case EBTN_TAP_BOX_00:
        case EBTN_TAP_BOX_01:
        case EBTN_TAP_BOX_02:
        case EBTN_TAP_BOX_03:
        case EBTN_TAP_BOX_04:
        case EBTN_TAP_BOX_05:
        case EBTN_TAP_BOX_06:
        case EBTN_TAP_BOX_07:
        case EBTN_TAP_BOX_08:
            OnToggleDepthIndex(uEventID - EBTN_TAP_BOX_00);
            break;
        default:
            break;
    }
}

VOID CAutoDashboardList::UpdateFractalIndex(VOID)
{
    CAutomationMgr& cMgr = CAutomationMgr::GetInstance();
    const UINT uFractalIndex = cMgr.GetFractalIndex();
    CHAR szBuffer[128];
    for (UINT uIndex = 0; m_uTradePairNum > uIndex; ++uIndex) {
        const CMegaZZ* pcZZ = cMgr.FindMegaZZ(uIndex, uFractalIndex);
        if (NULL != m_ppcWidgets[uIndex]) {
            if (NULL == pcZZ) {
                m_ppcWidgets[uIndex]->Clear();
            }
            else {
                m_ppcWidgets[uIndex]->Update(*pcZZ);
                CCryptoUtil::GeneratePriceDisplayString(m_eSource, uIndex, pcZZ->GetCurrentPrice(), szBuffer, 128);
                m_ppcWidgets[uIndex]->DisplayMessage(szBuffer);
            }
        }        
    }
}

VOID CAutoDashboardList::OnMegaZZUpdated(UINT uTradePairIndex)
{
    if (m_uTradePairNum <= uTradePairIndex) { // might happen.
        return;
    }
    if (NULL == m_ppcWidgets[uTradePairIndex]) {
        return;
    }
    CHAR szBuffer[128];
    const CMegaZZ* pcZZ = CAutomationMgr::GetInstance().FindMegaZZ(uTradePairIndex, CAutomationMgr::GetInstance().GetFractalIndex());
    if (NULL == pcZZ) {
        m_ppcWidgets[uTradePairIndex]->Clear();
    }
    else {
        CCryptoUtil::GeneratePriceDisplayString(m_eSource, uTradePairIndex, pcZZ->GetCurrentPrice(), szBuffer, 128);
        m_ppcWidgets[uTradePairIndex]->Update(*pcZZ);
        m_ppcWidgets[uTradePairIndex]->DisplayMessage(szBuffer);
    }
}

VOID CAutoDashboardList::OnTradePairSelected(UINT uTradePairIndex)
{
    if (NULL != m_ppcWidgets) {
        for (UINT uIndex = 0; m_uTradePairNum > uIndex; ++uIndex) {
            if (uTradePairIndex == uIndex) { // skip the selected index
                if (NULL != m_ppcWidgets[uIndex]) {
                    m_ppcWidgets[uIndex]->SetSelected();
                }
                continue;
            }
            if (NULL != m_ppcWidgets[uIndex]) {
                m_ppcWidgets[uIndex]->ClearSelection();
            }
        }
    }
    CUIContainer* pcParent = GetParent();
    if (NULL != pcParent) {
        CEvent cEvent(m_uSelectedEventID, uTradePairIndex);
        pcParent->PostEvent(cEvent, 0.0f);
    }
}
#include "GameRenderer.h"
VOID CAutoDashboardList::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    CUIContainer::OnRender(fOffsetX, fOffsetY);
    SHLVector2D sWorldPos = GetWorldPosition();
    sWorldPos.x += fOffsetX;
    sWorldPos.y += fOffsetY;
    const SHLVector2D& sWorldSize = GetWorldSize();
    CGameRenderer::DrawRectOutline(sWorldPos.x, sWorldPos.y, sWorldSize.x, sWorldSize.y, RGBA(0x7F, 0x7F, 0x7F, 0xFF));
}

VOID CAutoDashboardList::OnToggleDepthIndex(UINT uDepthIndex)
{
    ASSERT(9 > uDepthIndex);
    m_abToggleOn[uDepthIndex] = !m_abToggleOn[uDepthIndex];
    for (UINT uIndex = 0; m_uTradePairNum > uIndex; ++uIndex) {
        if (NULL != m_ppcWidgets[uIndex]) {
            m_ppcWidgets[uIndex]->ToggleVisibility(m_abToggleOn);
        }        
    }
}
