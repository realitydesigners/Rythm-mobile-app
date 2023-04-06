#include "stdafx.h"
#include "CMOChart.h"
#include "Event.h"
#include "EventID.h"
#include "EventManager.h"
#include "GameRenderer.h"
#include "GameWidgetDefs.h"
#include "PlayerData.h"
#include "TradeChart.h"

#define CHART_Y_PADDING (20.0f)

CCMOChart::CCMOChart(CTradeChart& cChart) :
CBaseChart(EGLOBAL_CMO_CHART, cChart),
m_pfPosY(NULL),
m_pdValues(NULL)
{
    memset(m_pcLabels, 0, sizeof(m_pcLabels));
}
    
CCMOChart::~CCMOChart()
{
    
}

VOID CCMOChart::Initialize(VOID)
{
    CBaseChart::Initialize();
    const CHAR* aszStrings[3] = {
        "50.0", "0.0", "-50.0",
    };
    UINT uIndex = 0;
    for ( ; 3 > uIndex; ++uIndex) {
        CUITextLabel* pcLabel = NEW_TEXT_LABEL;
        if (NULL == pcLabel) {
            ASSERT(FALSE);
            return;
        }
        pcLabel->SetColor(RGBA(0x7F, 0x7F, 0x7F, 0xFF));
        pcLabel->SetFont(EGAMEFONT_SIZE_16);
        pcLabel->SetAnchor(0.0f, 0.5f);
        pcLabel->SetString(aszStrings[uIndex]);
        m_pcLabels[uIndex] = pcLabel;
    }
}
    
VOID CCMOChart::Release(VOID)
{
    UINT uIndex = 0;
    for ( ; 3 > uIndex; ++uIndex) {
        if (NULL != m_pcLabels[uIndex]) {
            m_pcLabels[uIndex]->RemoveFromParent();
            m_pcLabels[uIndex]->Release();
            delete m_pcLabels[uIndex];
            m_pcLabels[uIndex] = NULL;
        }
    }
    SAFE_DELETE_ARRAY(m_pfPosY);
    SAFE_DELETE_ARRAY(m_pdValues);
    CBaseChart::Release();
}

#define GRID_COLOR_1  GRID_COLOR
#define GRID_COLOR_2  RGBA(0x7F,0x7F,0x7F,0x7F)
VOID CCMOChart::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    // render time lines
    RenderTimeLines();
    const SHLVector2D& sWorldPos = GetWorldPosition();
    const SHLVector2D& sWorldSize = GetWorldSize();
    
    // render labels
    const UINT auColors[3] = {
        GRID_COLOR_1, GRID_COLOR_2, GRID_COLOR_1
    };
    const FLOAT fX1 = sWorldPos.x;
    const FLOAT fX2 = fX1 + sWorldSize.x - 40.0f;
    UINT uIndex = 0;
    for ( ; 3 > uIndex; ++uIndex) {
        if (NULL != m_pcLabels[uIndex] && NULL != m_pcLabels[uIndex]->GetParent()) {
            const FLOAT fY = m_pcLabels[uIndex]->GetLocalPosition().y + sWorldPos.y;
            CGameRenderer::DrawLine(fX1, fY, fX2, fY, auColors[uIndex]);
        }
    }
    CGameRenderer::DrawLine(fX1, sWorldPos.y, fX2, sWorldPos.y, GRID_COLOR);
    CGameRenderer::DrawLine(fX1, sWorldPos.y + sWorldSize.y, fX2, sWorldPos.y + sWorldSize.y, GRID_COLOR);
    if (NULL != m_pfPosY) {
        FLOAT fX = m_fStartX;
        FLOAT fPrevY = m_pfPosY[0];
        uIndex = 1;
        for ( ; m_uIntervalNum > uIndex; ++uIndex) {
            CGameRenderer::DrawLine(fX, fPrevY, fX + m_fCandleWidth, m_pfPosY[uIndex], RGBA(0x9F, 0x9F, 0x9F, 0xFF));
            fX += m_fCandleWidth;
            fPrevY = m_pfPosY[uIndex];
        }
    }
    CBaseChart::OnRender(fOffsetX, fOffsetY);
}

VOID CCMOChart::GetToolTipString(CHAR* szBuffer, UINT uBufferLen, UINT uCandleIndexSelected) const
{
    DOUBLE dValue = 0.0f;
    if (NULL != m_pdValues) {
        dValue = m_pdValues[uCandleIndexSelected % m_uIntervalNum];
    }
    snprintf(szBuffer, uBufferLen, "CMO\n%.1f", dValue);
}
FLOAT CCMOChart::GetToolTipY(UINT uCandleIndexSelected) const
{
    if (NULL == m_pfPosY) {
        ASSERT(FALSE);
        return 0.0f;
    }
    return m_pfPosY[uCandleIndexSelected % m_uIntervalNum];
}

VOID CCMOChart::AdjustLabels(VOID)
{
    const FLOAT fTextYOffset = (GetLocalSize().y - CHART_Y_PADDING) / 4.0f;
    const FLOAT fStartY = CHART_Y_PADDING * 0.5f;
    const FLOAT fX = GetLocalSize().x - CHART_OFFSET_FROM_RIGHT + 5.0f;
    UINT uIndex = 0;
    for ( ; 3 > uIndex; ++uIndex) {
        if (NULL != m_pcLabels[uIndex]) {
            m_pcLabels[uIndex]->SetLocalPosition(fX, fStartY + (uIndex + 1) * fTextYOffset);
            m_pcLabels[uIndex]->RemoveFromParent();
            AddChild(*m_pcLabels[uIndex]);
        }
    }
}

VOID CCMOChart::GetTitleString(CHAR* szBuffer, UINT uBufferLen) const
{
    snprintf(szBuffer, uBufferLen, "CMO(%d)", CPlayerData::GetCMOInterval());
}
