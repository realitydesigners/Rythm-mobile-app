#include "stdafx.h"
#include "Event.h"
#include "OandaMgr.h"
#include "OTradeList.h"
#include "OTradeWidget.h"

COTradeList::COTradeList() :
CUIContainer()
{
    
}
    
COTradeList::~COTradeList()
{
}
 
VOID COTradeList::Initialize(UINT uSelectedEventID, UINT uAccountIndex)
{
    Release(); // safety
    const SHLVector2D& sSize = GetLocalSize();
    m_cScroller.SetLocalSize(sSize.x, sSize.y);
    AddChild(m_cScroller);
    COandaMgr& cMgr = COandaMgr::GetInstance();
    const COandaMgr::CTradeList* pcList = cMgr.GetOpenTrades(uAccountIndex);
    if (NULL == pcList) {
        return;
    }
        
    FLOAT fY = 0.0f;
    for (const auto& sTrades : *pcList) {
        COTradeWidget* pcWidget = new COTradeWidget(sTrades);
        if (NULL == pcWidget) {
            ASSERT(FALSE);
            return;
        }
        if (!pcWidget->Initialize(uSelectedEventID)) {
            ASSERT(FALSE);
            pcWidget->Release();
            delete pcWidget;
            continue;
        }
        pcWidget->SetLocalPosition(0.0f, fY);
        m_cScroller.AddChild(*pcWidget);
        fY += OTRADE_WIDGET_H + 2.0f;
    }
    m_cScroller.IncreaseScrollableTail(5.0f);
}

VOID COTradeList::Release(VOID)
{
    CUIWidget* pcChild = m_cScroller.GetChild();
    while (NULL != pcChild) {
        CUIWidget* pcTmp = pcChild;
        pcChild = pcChild->GetNextSibling();
        pcTmp->RemoveFromParent();
        pcTmp->Release();
        delete pcTmp;
    }
    m_cScroller.RemoveFromParent();
}

const COandaMgr::SOandaTrade* COTradeList::GetTrade(UINT uTradeID) const
{
    COTradeWidget* pcChild = (COTradeWidget*)m_cScroller.GetChild();
    while (NULL != pcChild) {
        const COandaMgr::SOandaTrade& sTrade = pcChild->GetTrade();
        if (sTrade.uTradeID == uTradeID) {
            return &sTrade;
        }
        pcChild = (COTradeWidget*)pcChild->GetNextSibling();
    }
    return NULL;

}
UINT COTradeList::GetTradePairIndex(UINT uTradeID) const
{
    COTradeWidget* pcChild = (COTradeWidget*)m_cScroller.GetChild();
    while (NULL != pcChild) {
        const COandaMgr::SOandaTrade& sTrade = pcChild->GetTrade();
        if (sTrade.uTradeID == uTradeID) {
            return sTrade.uTradePairIndex;
        }
        pcChild = (COTradeWidget*)pcChild->GetNextSibling();
    }
    return 0xFFFFFFFF;
}
