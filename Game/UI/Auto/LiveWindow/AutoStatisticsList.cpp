#include "stdafx.h"
#include "AutoLogicData.h"
#include "AutoStatisticsList.h"
#include "AutoStatisticsWidget.h"
#include "EventID.h"
#include "EventManager.h"
#include "OandaMgr.h"
#include "PatternData.h"
#include "TradeLog.h"

VOID CAutoStatisticsList::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    // do not post to parent.
    CEventManager::PostEvent(*this, cEvent, fDelay);
}

CAutoStatisticsList::CAutoStatisticsList(UINT uSelectedEventID) :
CUIContainer(EBaseWidget_Container, EGLOBAL_AUTO_LOGIC_STATISTICS_LIST),
m_uSelectedEventID(uSelectedEventID),
m_uSelectedIndex(0),
m_psFilteredData(NULL),
m_uFilteredNum(0),
m_ppsFilteredActiveOnly(NULL),
m_uFilteredActiveOnlyNum(0),
INITIALIZE_TEXT_LABEL(m_cName),
INITIALIZE_TEXT_LABEL(m_cLong),
INITIALIZE_TEXT_LABEL(m_cTargetOpenedNum),
INITIALIZE_TEXT_LABEL(m_cTakeProfitNum),
INITIALIZE_TEXT_LABEL(m_cStopLossNum),
INITIALIZE_TEXT_LABEL(m_cCumulativePip)
{

}
    
CAutoStatisticsList::~CAutoStatisticsList()
{
}
 
VOID CAutoStatisticsList::Initialize(VOID)
{
    m_cName.SetFont(EGAMEFONT_SIZE_12);
    m_cName.SetAnchor(0.0f, 0.5f);
    m_cName.SetLocalPosition(STATISTICS_NAME_X, STATISTICS_HEIGHT * 0.5f);
    m_cName.SetString("Name");
    AddChild(m_cName);
    
    m_cLong.SetFont(EGAMEFONT_SIZE_12);
    m_cLong.SetAnchor(0.5f, 0.5f);
    m_cLong.SetLocalPosition(STATISTICS_LONG_X, STATISTICS_HEIGHT * 0.5f);
    m_cLong.SetString("B/S");
    AddChild(m_cLong);
    
    m_cTargetOpenedNum.SetFont(EGAMEFONT_SIZE_12);
    m_cTargetOpenedNum.SetAnchor(0.5f, 0.5f);
    m_cTargetOpenedNum.SetLocalPosition(STATISTICS_TARGET_X, STATISTICS_HEIGHT * 0.5f);
    m_cTargetOpenedNum.SetString("Tgt");
    AddChild(m_cTargetOpenedNum);

    m_cTakeProfitNum.SetFont(EGAMEFONT_SIZE_12);
    m_cTakeProfitNum.SetAnchor(0.5f, 0.5f);
    m_cTakeProfitNum.SetLocalPosition(STATISTICS_TAKE_PROFIT_X, STATISTICS_HEIGHT * 0.5f);
    m_cTakeProfitNum.SetString("TP");
    AddChild(m_cTakeProfitNum);
    
    m_cStopLossNum.SetFont(EGAMEFONT_SIZE_12);
    m_cStopLossNum.SetAnchor(0.5f, 0.5f);
    m_cStopLossNum.SetLocalPosition(STATISTICS_STOPLOSS_X, STATISTICS_HEIGHT * 0.5f);
    m_cStopLossNum.SetString("SL");
    AddChild(m_cStopLossNum);
    
    m_cCumulativePip.SetFont(EGAMEFONT_SIZE_12);
    m_cCumulativePip.SetAnchor(0.5f, 0.5f);
    m_cCumulativePip.SetLocalPosition(STATISTICS_PIP_X, STATISTICS_HEIGHT * 0.5f);
    m_cCumulativePip.SetString("Pip");
    AddChild(m_cCumulativePip);
    
    const SHLVector2D& sSize = GetLocalSize();
    const FLOAT fY = STATISTICS_HEIGHT + 4.0f;
    m_cScroller.SetLocalPosition(0.0f, fY);
    m_cScroller.SetLocalSize(sSize.x, sSize.y - fY);
    AddChild(m_cScroller);
    
    m_uFilteredNum = CAutoLogicDataMgr::GetLogicNum();
    m_psFilteredData = new SFilteredData[m_uFilteredNum];
    m_ppsFilteredActiveOnly = new SFilteredData*[m_uFilteredNum];
    if (NULL == m_psFilteredData || NULL == m_ppsFilteredActiveOnly) {
        ASSERT(FALSE);
        return;
    }
    memset(m_ppsFilteredActiveOnly, 0, sizeof(SFilteredData*) * m_uFilteredNum);
    for (UINT uIndex = 0; m_uFilteredNum > uIndex; ++uIndex) {
        SFilteredData& sCurrent = m_psFilteredData[uIndex];
        sCurrent.uIndex = uIndex;
        const SLogic& sLogic = CAutoLogicDataMgr::GetLogic(uIndex);
        sCurrent.szName = CPatternDataMgr::GetPattern(sLogic.uCategoryPatternIndex).szName;
    }
    RefreshList();
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_SYSTEM, EGLOBAL_EVENT_SYSTEM_HEARTBEAT);
}

VOID CAutoStatisticsList::Release(VOID)
{
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_SYSTEM, EGLOBAL_EVENT_SYSTEM_HEARTBEAT);
    m_cName.Release();
    m_cName.RemoveFromParent();
    m_cLong.Release();
    m_cLong.RemoveFromParent();
    
    
    m_cTargetOpenedNum.Release();
    m_cTargetOpenedNum.RemoveFromParent();
    
    m_cTakeProfitNum.Release();
    m_cTakeProfitNum.RemoveFromParent();
    
    m_cStopLossNum.Release();
    m_cStopLossNum.RemoveFromParent();
    
    m_cCumulativePip.Release();
    m_cCumulativePip.RemoveFromParent();
    
    ClearList();
    m_cScroller.RemoveFromParent();
    
    SAFE_DELETE_ARRAY(m_psFilteredData);
    m_uFilteredNum = 0;
    SAFE_DELETE_ARRAY(m_ppsFilteredActiveOnly);
    m_uFilteredActiveOnlyNum = 0;
    
    CUIContainer::Release();
}
VOID CAutoStatisticsList::ClearList(VOID)
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
VOID CAutoStatisticsList::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EEVENT_SELECTED_ENTRY_INDEX:
            OnSelectedEntryIndex(cEvent.GetIntParam(1), TRUE);
            break;
        case EGLOBAL_EVENT_SYSTEM_HEARTBEAT:
            RefreshList();
            break;
        default:
            break;
    }
}


VOID CAutoStatisticsList::RefreshList(VOID)
{
    ClearList();
    if (NULL == m_psFilteredData) {
        ASSERT(FALSE);
        return;
    }
    for (UINT uIndex = 0; m_uFilteredNum > uIndex; ++uIndex) {
        SFilteredData& sCurrent = m_psFilteredData[uIndex];
        sCurrent.fPipGain = 0.0f;
        sCurrent.uSLNum = 0;
        sCurrent.uTPNum = 0;
        sCurrent.uTargetNum = 0;
    }
    const CTradeLogs& sOldLogs = CTradeLog::GetLogs();
    COandaMgr& cMgr = COandaMgr::GetInstance();
    for (auto& sLog : sOldLogs) {
        if (m_uFilteredNum <= sLog.uLogicIndex) {
            ASSERT(FALSE);
            continue;
        }
        const COandaMgr::SOandaPair* psPair = cMgr.GetTradePair(sLog.byTradePairIndex);
        if (NULL == psPair) {
            ASSERT(FALSE);
            continue;
        }
        SFilteredData& sCurrent = m_psFilteredData[sLog.uLogicIndex];
        sCurrent.bLong = sLog.byLong;
        ++sCurrent.uTargetNum;
        if (0.0f >= sLog.fCLosePrice) {
            continue;
        }
        FLOAT fGain = sLog.fCLosePrice - sLog.fOpenPrice;
        if (0 == sLog.byLong) {
            fGain = -fGain;
        }
        sCurrent.fPipGain += (FLOAT)(fGain * psPair->dPipMultiplier);
        if (0.0f > fGain) {
            ++sCurrent.uSLNum;
        }
        else {
            ++sCurrent.uTPNum;
        }
    }
    const CActiveTradeLogMap& cActives = CTradeLog::GetActiveLogs();
    for (auto itr : cActives) {
        const STradeLog* psLog =  itr.second;
        if (m_uFilteredNum <= psLog->uLogicIndex) {
            ASSERT(FALSE);
            continue;
        }
        const COandaMgr::SOandaPair* psPair = cMgr.GetTradePair(psLog->byTradePairIndex);
        if (NULL == psPair) {
            ASSERT(FALSE);
            continue;
        }
        SFilteredData& sCurrent = m_psFilteredData[psLog->uLogicIndex];
        sCurrent.bLong = psLog->byLong;
        ++sCurrent.uTargetNum;
        FLOAT fGain = (FLOAT)(psPair->dPrice - psLog->fOpenPrice);
        if (0 == psLog->byLong) {
            fGain = -fGain;
        }
        sCurrent.fPipGain += (FLOAT)(fGain * psPair->dPipMultiplier);
    }
    
    m_uFilteredActiveOnlyNum = 0;
    for (UINT uIndex = 0; m_uFilteredNum > uIndex; ++uIndex) {
        SFilteredData& sCurrent = m_psFilteredData[uIndex];
        if (0.0f != sCurrent.fPipGain || 0 != sCurrent.uTargetNum) {
            m_ppsFilteredActiveOnly[m_uFilteredActiveOnlyNum] = &sCurrent;
            ++m_uFilteredActiveOnlyNum;
        }
    }
    FLOAT fY = 0.0f;
    for (UINT uIndex = 0; m_uFilteredActiveOnlyNum > uIndex; ++uIndex) {
        CAutoStatisticsWidget* pcWidget = new CAutoStatisticsWidget(uIndex, *m_ppsFilteredActiveOnly[uIndex]);
        if (NULL == pcWidget) {
            ASSERT(FALSE);
            return;
        }
        pcWidget->SetLocalPosition(0.0f, fY);
        pcWidget->Initialize(EEVENT_SELECTED_ENTRY_INDEX);
        m_cScroller.AddChild(*pcWidget);
        fY += STATISTICS_HEIGHT + 2.0f;
    }
    OnSelectedEntryIndex(m_uSelectedIndex, FALSE);
}

VOID CAutoStatisticsList::OnSelectedEntryIndex(UINT uIndex, BOOLEAN bFireEvent)
{
    if (NULL == m_ppsFilteredActiveOnly || m_uFilteredActiveOnlyNum <= uIndex) {
        return;
    }
    m_uSelectedIndex = uIndex;
    if (bFireEvent) {
        CUIContainer* pcParent = GetParent();
        if (NULL != pcParent) {
            CEvent cEvent(m_uSelectedEventID, m_ppsFilteredActiveOnly[uIndex]->uIndex);
            pcParent->PostEvent(cEvent, 0.0f);
        }
    }
    CAutoStatisticsWidget* pcChild = (CAutoStatisticsWidget*)m_cScroller.GetChild();
    while (NULL != pcChild) {
        pcChild->SetSelected(pcChild->GetIndex() == uIndex);
        pcChild = (CAutoStatisticsWidget*)pcChild->GetNextSibling();
    }
}

#include "GameRenderer.h"
VOID CAutoStatisticsList::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    CUIContainer::OnRender(fOffsetX, fOffsetY);
    SHLVector2D sWorldPos = GetWorldPosition();
    sWorldPos.x += fOffsetX;
    sWorldPos.y += fOffsetY;
    const SHLVector2D& sWorldSize = GetWorldSize();
    CGameRenderer::DrawRectOutline(sWorldPos.x, sWorldPos.y, sWorldSize.x, sWorldSize.y, RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    const FLOAT fY = sWorldPos.y + STATISTICS_HEIGHT + 2.0f;
    CGameRenderer::DrawLine(sWorldPos.x, fY, sWorldPos.x + sWorldSize.x, fY, RGBA(0x7F, 0x7F, 0x7F, 0xFF));
}
