#include "stdafx.h"
#include "EventID.h"
#include "EventManager.h"
#include "TradeLogList.h"
#include "TradeLogWidget.h"
#include "TradeLog.h"

CTradeLogList::CTradeLogList() :
CUIContainer()
{
    
}
    
CTradeLogList::~CTradeLogList()
{
}
 
BOOLEAN CTradeLogList::SelectWidget(const CTradeLogWidget& cWidget)
{
    CTradeLogWidget* pcWidget = (CTradeLogWidget*)m_cScroller.GetChild();
    BOOLEAN bResult = FALSE;
    while (NULL != pcWidget) {
        if (pcWidget == &cWidget) {
            pcWidget->SetSelected(TRUE);
            bResult = TRUE;
        }
        else {
            pcWidget->SetSelected(FALSE);
        }
        pcWidget = (CTradeLogWidget*)pcWidget->GetNextSibling();
    }
    return bResult;
}

VOID CTradeLogList::Initialize(UINT uEventID)
{
    Release(); // safety
    const SHLVector2D& sSize = GetLocalSize();
    m_cScroller.SetLocalSize(sSize.x, sSize.y);
    AddChild(m_cScroller);
    const CTradeLogs& cLogs = CTradeLog::GetLogs();
    CTradeLogs::const_iterator itr = cLogs.begin();
    const CTradeLogs::const_iterator itrEnd = cLogs.end();
    FLOAT fY = 0.0f;
    CTradeLogWidget* pcLast = NULL;
    while (itrEnd != itr) {
        const STradeLog& sLog = *(itr);
        CTradeLogWidget* pcWidget = new CTradeLogWidget(sLog);
        if (NULL == pcWidget) {
            ASSERT(FALSE);
            return;
        }
        pcWidget->SetLocalPosition(1.0f, fY);
        pcWidget->Initialize(uEventID);
        m_cScroller.AddChild(*pcWidget);
        fY += (TRADE_LOG_WIDGET_H);
        ++itr;
        pcLast = pcWidget;
    }
    const CActiveTradeLogMap& cActives = CTradeLog::GetActiveLogs();
    for (auto itr : cActives) {
        const STradeLog* psLog = itr.second;
        CTradeLogWidget* pcWidget = new CTradeLogWidget(*psLog);
        if (NULL == pcWidget) {
            ASSERT(FALSE);
            return;
        }
        pcWidget->SetLocalPosition(1.0f, fY);
        pcWidget->Initialize(uEventID);
        m_cScroller.AddChild(*pcWidget);
        fY += (TRADE_LOG_WIDGET_H);
        pcLast = pcWidget;
    }
        
    m_cScroller.IncreaseScrollableTail(5.0f);
    m_cScroller.Seek(CUIScroller::EPOS_END, 0.0f);
    if (NULL != pcLast) {
        CUIContainer* pcParent = GetParent();
        if (NULL != pcParent) {
            CEvent cSelectedEvent(uEventID);
            cSelectedEvent.SetPtrParam(1, pcLast);
            pcParent->PostEvent(cSelectedEvent, 0.0f);
        }
        
    }
}

VOID CTradeLogList::Release(VOID)
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
