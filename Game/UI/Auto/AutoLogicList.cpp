#include "stdafx.h"
#include "AutoLogicList.h"
#include "AutoLogicWidget.h"
//#include "EntryData.h"
#include "EventID.h"
#include "EventManager.h"
#include "PatternData.h"

VOID CAutoLogicList::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    // do not post to parent.
    CEventManager::PostEvent(*this, cEvent, fDelay);
}

CAutoLogicList::CAutoLogicList() :
CUIContainer(EBaseWidget_Container, EGLOBAL_AUTO_LOGIC_LIST),
m_uSelectedEventID(0),
m_uSelectedPatternIndex(0)
{
    
}
    
CAutoLogicList::~CAutoLogicList()
{
}
 
VOID CAutoLogicList::Initialize(UINT uSelectedEventID)
{
    m_uSelectedEventID = uSelectedEventID;
    const SHLVector2D& sSize = GetLocalSize();
    m_cBG.SetLocalSize(sSize.x, sSize.y);
    m_cBG.SetColor(RGBA(0x1F, 0x1F, 0x1F, 0xFF));
    m_cBG.SetBorderColor(RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    AddChild(m_cBG);
    
    m_cScroller.SetLocalSize(sSize.x, sSize.y);
    AddChild(m_cScroller);
}

VOID CAutoLogicList::Release(VOID)
{
    m_cBG.RemoveFromParent();
    ClearList();
    m_cScroller.RemoveFromParent();
    CUIContainer::Release();
}
VOID CAutoLogicList::ClearList(VOID)
{
    CUIWidget* pcChild = m_cScroller.GetChild();
    while (NULL != pcChild) {
        CUIWidget* pcTmp = pcChild;
        pcChild = pcChild->GetNextSibling();
        pcTmp->RemoveFromParent();
        pcTmp->Release();
        delete pcTmp;
    }
}
VOID CAutoLogicList::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EEVENT_SELECTED_LOGIC:
            OnSelectedWidget(cEvent.GetIntParam(1));
            break;
        default:
            break;
    }
}

VOID CAutoLogicList::FilterPatternByCategory(UINT uEntryCategoryIndex)
{
//    ClearList();
//    if (UNUSED_CATEGORY == uEntryCategoryIndex) {
//        return;
//    }
//    const SCategoryIndex& sCategory = CEntryDataMgr::GetCategory(uEntryCategoryIndex);
//    const SHLVector2D& sSize = GetLocalSize();
//    const UINT uNum = CPatternDataMgr::GetPatternNum();
//    const UINT uIndexNum = CEntryDataMgr::GetPatternIndexNum();
//    const UINT* puIndexes = CEntryDataMgr::GetPatternIndex();
//    FLOAT fY = 5.0f;
//    for (UINT uIndex = 0; sCategory.uEntryNum > uIndex; ++uIndex) {
//        const UINT uActualIndex = sCategory.uEntryIndex + uIndex;
//        if (uIndexNum <= uActualIndex) {
//            ASSERT(FALSE);
//            continue;
//        }
//        const UINT uPatternIndex = puIndexes[uActualIndex];
//        if (uNum <= uPatternIndex) {
//            ASSERT(FALSE);
//            continue;
//        }
//        CAutoLogicWidget* pcWidget = new CAutoLogicWidget(uPatternIndex);
//         if (NULL == pcWidget) {
//             ASSERT(FALSE);
//             return;
//         }
//         pcWidget->SetLocalSize(sSize.x - 6.0f, 25.0f);
//         pcWidget->SetLocalPosition(3.0f, fY);
//         pcWidget->Initialize(EEVENT_SELECTED_LOGIC);
//         m_cScroller.AddChild(*pcWidget);
//         fY += 27.0f;
//    }
}
VOID CAutoLogicList::RefreshPatternList(VOID)
{
    ClearList();
    const SHLVector2D& sSize = GetLocalSize();
    const UINT uNum = CPatternDataMgr::GetPatternNum();
    FLOAT fY = 5.0f;
    for (UINT uPatternIndex = 0; uNum > uPatternIndex; ++uPatternIndex) {
       CAutoLogicWidget* pcWidget = new CAutoLogicWidget(uPatternIndex);
        if (NULL == pcWidget) {
            ASSERT(FALSE);
            return;
        }
        pcWidget->SetLocalSize(sSize.x - 6.0f, 25.0f);
        pcWidget->SetLocalPosition(3.0f, fY);
        pcWidget->Initialize(EEVENT_SELECTED_LOGIC);
        m_cScroller.AddChild(*pcWidget);
        fY += 27.0f;
    }
}


VOID CAutoLogicList::OnSelectedWidget(UINT uPatternIndex)
{
    CAutoLogicWidget* pcChild = (CAutoLogicWidget*)m_cScroller.GetChild();
    while (NULL != pcChild) {
        pcChild->SetSelected(pcChild->GetPatternIndex() == uPatternIndex);
        pcChild = (CAutoLogicWidget*)pcChild->GetNextSibling();
    }
    CUIContainer* pcParent = GetParent();
    if (NULL != pcParent) {
        CEvent cEvent(m_uSelectedEventID, uPatternIndex);
        pcParent->PostEvent(cEvent, 0.0f);
    }
}
