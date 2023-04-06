#include "stdafx.h"
#include "Event.h"
#include "EventID.h"
#include "EventManager.h"
#include "OTradeWidget.h"

COTradeWidget::COTradeWidget(const COandaMgr::SOandaTrade& sTrade) :
CUIButton(),
m_sTrade(sTrade),
INITIALIZE_TEXT_LABEL(m_cTradePairAndUnits),
INITIALIZE_TEXT_LABEL(m_cPositionAndPrice),
INITIALIZE_TEXT_LABEL(m_cCurrentPrice),
INITIALIZE_TEXT_LABEL(m_cPLAndPip),
m_dPipMultiplier(0.0)
{
    
}
    
COTradeWidget::~COTradeWidget()
{
}
 
BOOLEAN COTradeWidget::Initialize(UINT uSelectedEventID)
{
    m_cBG.SetLocalSize(OTRADE_WIDGET_W-1, OTRADE_WIDGET_H);
    m_cBG.SetColor(RGBA(30, 41, 54, 0xFF));
    SetDisplayWidgets(m_cBG, m_cBG);
    SetID(uSelectedEventID);
    
    COandaMgr& cMgr = COandaMgr::GetInstance();
    const COandaMgr::SOandaPair* psPair = cMgr.GetTradePair(m_sTrade.uTradePairIndex);
    if (NULL == psPair) {
        ASSERT(FALSE);
        return FALSE;
    }
    m_dPipMultiplier = psPair->dPipMultiplier;
    CHAR szBuffer[256];
    snprintf(szBuffer, 256, "Trade[%d] %s", m_sTrade.uTradeID, psPair->szTradePair);
    m_cTradePairAndUnits.SetFont(EGAMEFONT_SIZE_14);
    m_cTradePairAndUnits.SetString(szBuffer);
    m_cTradePairAndUnits.SetAnchor(0.0f, 0.5f);
    m_cTradePairAndUnits.SetLocalPosition(5.0f, 10.0f);
    AddChild(m_cTradePairAndUnits);
    
    CHAR szPriceBuffer[64];
    cMgr.GetPriceDisplayString(szPriceBuffer, 64, m_sTrade.uTradePairIndex, m_sTrade.dPrice);
    m_cPositionAndPrice.SetFont(EGAMEFONT_SIZE_12);
    m_cPositionAndPrice.SetString(szPriceBuffer);
    m_cPositionAndPrice.SetAnchor(0.0f, 0.5f);
    m_cPositionAndPrice.SetLocalPosition(5.0f, 30.0f);
    AddChild(m_cPositionAndPrice);
    m_cPositionAndPrice.OnUpdate(0.0f); // force update
    const SHLVector2D& sPriceSize = m_cPositionAndPrice.GetLocalSize();
    
    m_cArrow.SetTexture(ETEX::icon_arrow, TRUE);
    const SHLVector2D& sArrowSize = m_cArrow.GetLocalSize();
    m_cArrow.SetLocalSize(sArrowSize.x * 0.5f, sArrowSize.y * 0.5f);
    m_cArrow.SetAnchor(0.0f, 0.5f);
    const FLOAT fArrowX = 5.0f + sPriceSize.x + 5.0f;
    m_cArrow.SetLocalPosition(5.0f + sPriceSize.x + 5.0f, 30.0f);
    AddChild(m_cArrow);
    
    m_cCurrentPrice.SetFont(EGAMEFONT_SIZE_12);
    if (0.0 != psPair->dPrice) {
        cMgr.GetPriceDisplayString(szPriceBuffer, 64, m_sTrade.uTradePairIndex, psPair->dPrice);
        m_cCurrentPrice.SetString(szPriceBuffer);
        const DOUBLE dDiff = psPair->dPrice - m_sTrade.dPrice;
        UpdatePip(dDiff);
    }
    else {
        m_cCurrentPrice.SetString("----");
        m_cPLAndPip.SetString("----");
        m_cPLAndPip.SetColor(0xFFFFFFFF);
    }
    m_cCurrentPrice.SetAnchor(0.0f, 0.5f);
    m_cCurrentPrice.SetLocalPosition(fArrowX + sArrowSize.x + 5.0f, 30.0f);
    AddChild(m_cCurrentPrice);

    m_cPLAndPip.SetFont(EGAMEFONT_SIZE_12);
    m_cPLAndPip.SetAnchor(1.0f, 0.5f);
    m_cPLAndPip.SetLocalPosition(OTRADE_WIDGET_W - 4.0f, 30.0f);
    AddChild(m_cPLAndPip);

    CEventManager::RegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_RYTHM_UPDATE);
    return TRUE;
}

VOID COTradeWidget::Release(VOID)
{
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_RYTHM_UPDATE);
    m_cTradePairAndUnits.Release();
    m_cTradePairAndUnits.RemoveFromParent();
    m_cArrow.RemoveFromParent();
    
    m_cCurrentPrice.Release();
    m_cCurrentPrice.RemoveFromParent();
    
    m_cPLAndPip.Release();
    m_cPLAndPip.RemoveFromParent();
    
    CUIButton::Release();
}

VOID COTradeWidget::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EGLOBAL_EVT_OANDA_RYTHM_UPDATE:
            OnPriceUpdate(cEvent.GetIntParam(1));
            break;
        default:
            break;
    }
}

VOID COTradeWidget::OnPriceUpdate(UINT uTradePairIndex)
{
    if (m_sTrade.uTradePairIndex != uTradePairIndex) {
        return;
    }
    COandaMgr& cMgr = COandaMgr::GetInstance();
    const SRythmListener* psListener = cMgr.GetRythmListener(m_sTrade.uTradePairIndex);
    if (NULL == psListener) {
        ASSERT(FALSE);
        return;
    }
    CHAR szPriceBuffer[64];
    cMgr.GetPriceDisplayString(szPriceBuffer, 64, m_sTrade.uTradePairIndex, psListener->dCurrentPrice);
    m_cCurrentPrice.SetString(szPriceBuffer);
    const DOUBLE dDiff = psListener->dCurrentPrice - m_sTrade.dPrice;
    UpdatePip(dDiff);
}

VOID COTradeWidget::UpdatePip(DOUBLE dDiff)
{
    CHAR szBuffer[64];
    dDiff *= m_dPipMultiplier;
    snprintf(szBuffer, 64, "%.1f pip", dDiff);
    m_cPLAndPip.SetString(szBuffer);
    UINT uColor;
    const BOOLEAN bLong = 0.0 < m_sTrade.dCurrentUnits;
    if (bLong) {
        uColor = (0.0 <= dDiff) ? RGBA(0, 0xFF, 0, 0xFF) : RGBA(0xFF, 0, 0, 0xFF);
    }
    else {
        uColor = (0.0 >= dDiff) ? RGBA(0, 0xFF, 0, 0xFF) : RGBA(0xFF, 0, 0, 0xFF);
    }
    m_cPLAndPip.SetColor(uColor);
}

VOID COTradeWidget::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    cEvent.SetIntParam(1, m_sTrade.uTradeID);
    CUIContainer::PostEvent(cEvent, fDelay);
}

BOOLEAN COTradeWidget::CreateButtonEvent(CEvent& cEvent)
{
    cEvent.SetIntParam(0, GetID());
    cEvent.SetIntParam(1, m_sTrade.uTradePairIndex);
    return TRUE;
}
