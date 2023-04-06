#include "stdafx.h"
#include "EventID.h"
#include "EventManager.h"
#include "LogList.h"
#include "LogUI.h"

CLogList::CLogList() :
CUIContainer(),
m_bRegistered(FALSE)
{
    
}
    
CLogList::~CLogList()
{
}
 
VOID CLogList::Initialize(VOID)
{
    Release(); // safety
    const SHLVector2D& sSize = GetLocalSize();
    m_cScroller.SetLocalSize(sSize.x, sSize.y);
    AddChild(m_cScroller);
    const CMessageLogList& cMsgs = CMessageLog::GetLogs();
    CMessageLogList::const_reverse_iterator itr = cMsgs.rbegin();
    const CMessageLogList::const_reverse_iterator itrEnd = cMsgs.rend();
    FLOAT fY = 5.0f;
    const FLOAT fW = sSize.x - 10.0f;
    while (itrEnd != itr) {
        const SMessageLog& sLog = *(itr);
        CLogUI* pcWidget = new CLogUI();
        if (NULL == pcWidget) {
            ASSERT(FALSE);
            return;
        }
        pcWidget->SetLocalPosition(5.0f, fY);
        pcWidget->SetLocalSize(fW, 20.0f);
        pcWidget->Initialize(sLog);
        m_cScroller.AddChild(*pcWidget);
        fY += (pcWidget->GetLocalSize().y + 2.0f);
        ++itr;
    }
    m_cScroller.IncreaseScrollableTail(5.0f);
    m_cScroller.Seek(CUIScroller::EPOS_END, 0.0f);
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_MESSAGE_LOG, EGLOBAL_EVT_MSG_LOG_REFRESHED);
    m_bRegistered = TRUE;
}

VOID CLogList::Release(VOID)
{
    if (m_bRegistered) {
        CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_MESSAGE_LOG, EGLOBAL_EVT_MSG_LOG_REFRESHED);
        m_bRegistered = FALSE;
    }
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

VOID CLogList::OnReceiveEvent(CEvent& cEvent)
{
    Initialize();
}
