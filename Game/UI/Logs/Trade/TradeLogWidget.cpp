#include "stdafx.h"
#include "AutoLogicData.h"
#include "Event.h"
#include "GameRenderer.h"
#include "HLTime.h"
#include "OandaMgr.h"
#include "PatternData.h"
#include "TradeLogWidget.h"

#define SELECTED_COLOR RGBA(0x4F,0x4F,0x4F,0xFF)

#define SELECTED_TEXT_COLOR     (0xFFFFFFFF)
#define NOT_SELECTED_TEXT_COLOR RGBA(0x7F,0x7F,0x7F,0xFF)

CTradeLogWidget::CTradeLogWidget(const STradeLog& sLog) :
CUIButton(),
m_sLog(sLog),
INITIALIZE_TEXT_LABEL(m_cDate),
INITIALIZE_TEXT_LABEL(m_cID),
INITIALIZE_TEXT_LABEL(m_cPip)
{
    
}
    
CTradeLogWidget::~CTradeLogWidget()
{
}
 
static const CHAR* s_aszMths[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

VOID CTradeLogWidget::GetTimeString(UINT uGmtTimeSec, CHAR* szBuffer, UINT uBufferLen)
{
    const UINT uLocalTimeSec = CHLTime::GetLocalTimeSecs(uGmtTimeSec);
    UINT uMth, uDay;
    CHLTime::GetMonthAndDay(uLocalTimeSec, uMth, uDay);
    UINT uHr, uMin, uSec;
    CHLTime::GetHHMMSS(uLocalTimeSec, uHr, uMin, uSec);
    snprintf(szBuffer, uBufferLen, "%d %s %d:%02d:%02d", uDay, s_aszMths[uMth%12], uHr, uMin, uSec);
}
BOOLEAN CTradeLogWidget::Initialize(UINT uEventID)
{
    SetPressedOffset(1.0f);
    m_cBG.SetLocalSize(TRADE_LOG_WIDGET_W, TRADE_LOG_WIDGET_H);
    m_cBG.SetColor(SELECTED_COLOR);
    m_cBG.SetVisibility(FALSE);
    SetDisplayWidgets(m_cBG, m_cBG);
    SetID(uEventID);
    
    m_cTradeName.SetLocalSize(200.0f, 20.0f);
    m_cTradeName.SetLocalPosition(5.0f, 0.0f);
    m_cTradeName.SetName(ESOURCE_MARKET_OANDA, m_sLog.byTradePairIndex);
    AddChild(m_cTradeName);
    
    FLOAT fPipMultiplier = 1.0f;
    const COandaMgr::SOandaPair* psPair = COandaMgr::GetInstance().GetTradePair(m_sLog.byTradePairIndex);
    if (NULL != psPair) {
        fPipMultiplier = psPair->dPipMultiplier;
    }
    
    CHAR szBuffer[1024];
    {
        GetTimeString(m_sLog.uOpenTimeSec, szBuffer, 1024);
        m_cDate.SetFont(EGAMEFONT_SIZE_12);
        m_cDate.SetColor(NOT_SELECTED_TEXT_COLOR);
        m_cDate.SetAnchor(1.0f, 0.5f);
        m_cDate.SetString(szBuffer);
        m_cDate.SetLocalPosition(TRADE_LOG_WIDGET_W, 10.0f);
        AddChild(m_cDate);
    }
    {   // pip
        FLOAT fPip;
        if (0 < m_sLog.uCloseTimeSec) { // if already closed
            fPip = (m_sLog.fCLosePrice - m_sLog.fOpenPrice) * fPipMultiplier;
            if (0 == m_sLog.byLong) { // if short
                fPip = -fPip;
            }
            if (0.0f <= fPip) {
                snprintf(szBuffer, 1024, "Closed +%.1fp", fPip);
                m_cPip.SetColor(RGBA(0x4F, 0xFF, 0x4F, 0xFF));
            }
            else {
                snprintf(szBuffer, 1024, "Closed %.1fp", fPip);
                m_cPip.SetColor(RGBA(0xFF, 0x4F, 0x4F, 0xFF));
            }
        }
        else {
            fPip = (psPair->dPrice - m_sLog.fOpenPrice) * fPipMultiplier;
            if (0 == m_sLog.byLong) { // if short
                fPip = -fPip;
            }
            if (0.0f <= fPip) {
                snprintf(szBuffer, 1024, "Still Open +%.1fp", fPip);
                m_cPip.SetColor(RGBA(0x4F, 0xFF, 0x4F, 0xFF));
            }
            else {
                snprintf(szBuffer, 1024, "Still Open %.1fp", fPip);
                m_cPip.SetColor(RGBA(0xFF, 0x4F, 0x4F, 0xFF));
            }
        }
        m_cPip.SetFont(EGAMEFONT_SIZE_12);
        m_cPip.SetAnchor(1.0f, 1.0f);
        m_cPip.SetLocalPosition(TRADE_LOG_WIDGET_W, TRADE_LOG_WIDGET_H - 3.0f);
        m_cPip.SetString(szBuffer);
        AddChild(m_cPip);
    }
    {   // ID
        const CHAR* szCatName = CPatternDataMgr::GetPattern(m_sLog.uCategoryPatternIndex).szName;
        snprintf(szBuffer, 1024, "%s-%s", szCatName, m_sLog.byLong ? "Long" : "Short");
        m_cID.SetString(szBuffer);
        m_cID.SetFont(EGAMEFONT_SIZE_12);
        m_cID.SetAnchor(0.0f, 1.0f);
        m_cID.SetColor(NOT_SELECTED_TEXT_COLOR);
        m_cID.SetLocalPosition(5.0f, TRADE_LOG_WIDGET_H - 3.0f);
        AddChild(m_cID);
    }
    return TRUE;
}
VOID CTradeLogWidget::SetSelected(BOOLEAN bSelected)
{
    m_cBG.SetVisibility(bSelected);
    m_cTradeName.SetSelected(bSelected);
    m_cDate.SetColor(bSelected ? SELECTED_TEXT_COLOR : NOT_SELECTED_TEXT_COLOR);
    m_cID.SetColor(bSelected ? SELECTED_TEXT_COLOR : NOT_SELECTED_TEXT_COLOR);
}

VOID CTradeLogWidget::Release(VOID)
{
    m_cTradeName.Release();
    m_cTradeName.RemoveFromParent();
    
    m_cDate.Release();
    m_cDate.RemoveFromParent();
    
    m_cID.Release();
    m_cID.RemoveFromParent();
    
    m_cPip.Release();
    m_cPip.RemoveFromParent();
}

BOOLEAN CTradeLogWidget::CreateButtonEvent(CEvent& cEvent)
{
    cEvent.SetIntParam(0, GetID());
    cEvent.SetPtrParam(1, this);
    return TRUE;
}

VOID CTradeLogWidget::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    CUIButton::OnRender(fOffsetX, fOffsetY);
    const SHLVector2D& sWorldPos = GetWorldPosition();
    fOffsetX += sWorldPos.x;
    fOffsetY += sWorldPos.y;
    const SHLVector2D& sWorldSize = GetWorldSize();
    CGameRenderer::DrawRectOutline(fOffsetX, fOffsetY, sWorldSize.x, sWorldSize.y, RGBA(0x7F,0x7F,0x7F,0xFF));
}
