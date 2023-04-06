#include "stdafx.h"
#include "CMain.h"
#include "CBaseChart.h"
#include "Event.h"
#include "EventID.h"
#include "EventManager.h"
#include "GameRenderer.h"
#include "GameWidgetDefs.h"
#include "PlayerData.h"
#include "TradeChart.h"

#define CHART_Y_PADDING (20.0f)

CBaseChart::CBaseChart(UINT uGlobalEventID, CTradeChart& cChart) :
CUIContainer(EBaseWidget_Container, uGlobalEventID),
m_cMainChart(cChart),
INITIALIZE_TEXT_LABEL(m_cLabel),
m_fStartX(0.0),
m_fCandleWidth(0.0f),
m_uIntervalNum(0),
m_bDisplaySelectedValue(FALSE),
m_fSelectedX(0.0f),
m_fSelectedY(0.0f)
{

}
    
CBaseChart::~CBaseChart()
{
    
}

VOID CBaseChart::Initialize(VOID)
{
    m_cLabel.SetFont(EGAMEFONT_SIZE_20);
    m_cLabel.SetLocalPosition(5.0f, 5.0f);
    
    AddChild(m_cLabel);
    UpdateTitleString();
}

VOID CBaseChart::UpdateTitleString(VOID)
{
    CHAR szBuffer[128];
    GetTitleString(szBuffer, 128);
    m_cLabel.SetString(szBuffer);
}
    
VOID CBaseChart::Release(VOID)
{
    m_cLabel.Release();
    m_cLabel.RemoveFromParent();
    
    m_cToolTip.Release();
    CUIContainer::Release();
}

VOID CBaseChart::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    if (m_bDisplaySelectedValue) {
        const SHLVector2D& sWorldPos = GetWorldPosition();
        const SHLVector2D& sWorldSize = GetWorldSize();
        CGameRenderer::DrawLine(sWorldPos.x, m_fSelectedY, sWorldPos.x + sWorldSize.x - 40.0f, m_fSelectedY, 0xFFFFFFFF);
        CGameRenderer::DrawLine(m_fSelectedX , sWorldPos.y, m_fSelectedX, sWorldPos.y + sWorldSize.y, 0xFFFFFFFF);

        FLOAT fX = m_fSelectedX + FINGER_FATNESS;
        const SHLVector2D& sTooltipSize = m_cToolTip.GetWorldSize();
        if ((fX + sTooltipSize.x) > sWorldSize.x) {
            fX = m_fSelectedX - FINGER_FATNESS - sTooltipSize.x;
        }
        FLOAT fY = m_fSelectedY + FINGER_FATNESS;
        if ((CMain::GetScreenHeight() - CMain::GetDisplayBottomMargin()) < fY + (sTooltipSize.y)) {
            fY = m_fSelectedY - FINGER_FATNESS - sTooltipSize.y;
        }
        m_cToolTip.OnRender(fX, 0.0); // Y is adjusted by mainwindow
    }
    CUIContainer::OnRender(fOffsetX, fOffsetY);
}

VOID CBaseChart::OnDisplayToolTip(FLOAT& fY, BOOLEAN bShow, UINT uDisplayedCandleIndex)
{
    m_bDisplaySelectedValue = bShow;
    if (bShow) {
        uDisplayedCandleIndex = uDisplayedCandleIndex % m_uIntervalNum;
        m_fSelectedX = m_fStartX + uDisplayedCandleIndex * m_fCandleWidth;
        m_fSelectedY = GetToolTipY(uDisplayedCandleIndex);
    
        CHAR szBuffer[1024];
        GetToolTipString(szBuffer, 1024, uDisplayedCandleIndex);
        m_cToolTip.SetLocalPosition(0.0f, fY);
        m_cToolTip.Initialize(szBuffer);
        m_cToolTip.OnUpdate(0.0f);
        fY += m_cToolTip.GetLocalSize().y;
    }
}


VOID CBaseChart::RenderTimeLines(VOID) const
{
    const SHLVector2D& sWorldPos = GetWorldPosition();
    const SHLVector2D& sWorldSize = GetWorldSize();
//    CGameRenderer::DrawRectNoTex(sWorldPos.x, sWorldPos.y, sWorldSize.x, sWorldSize.y, RGBA(0x7F, 0, 0, 0x7F));
    const CLabelList& cLbls = m_cMainChart.GetTimeLabels();
    const FLOAT fY1 = sWorldPos.y;
    const FLOAT fY2 = fY1 + sWorldSize.y;
    for (auto pcLbl : cLbls) {
        if (NULL != pcLbl) {
            const FLOAT fX = pcLbl->GetLocalPosition().x;
            CGameRenderer::DrawLine(fX, fY1, fX, fY2, GRID_COLOR);
        }
    }

}
