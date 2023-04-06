#include "stdafx.h"
#include "AutomationMgr.h"
#include "EventID.h"
#include "EventManager.h"
#include "MegaZZ.h"
#include "MessageLog.h"
#include "NowLoadingWindow.h"
#include "OandaMgr.h"
#include "TradeLog.h"

// callback when reply from a close trade is received
VOID CAutomationMgr::OnCloseTradeReply(UINT uMsgID, UINT uAccountIndex, UINT uTradeID, BOOLEAN bSuccess, FLOAT fPrice)
{
    CHAR szBuffer[1024];
    if (!bSuccess) { // if close fail, skip
        snprintf(szBuffer, 1024, "Auto Close Trade[%d] Error!", uTradeID);
        CMessageLog::AddLog(szBuffer);
        return;
    }
    const BOOLEAN bLong = (uAccountIndex == COandaMgr::EACCOUNT_LONG);
    m_abRefreshAccounts[uAccountIndex % 2] = TRUE;

    CClosingTradeMap::iterator itFound = m_cClosingTradeMap.find(uMsgID);
    if (m_cClosingTradeMap.end() == itFound) {
        TRACE("Warn: Trade not found (MsgID:%d, AcctID:%d TradeID:%d\n", uMsgID, uAccountIndex, uTradeID);
        CTradeLog::OnCloseLog(bLong, uTradeID, fPrice, 0.0f);
        return;
    }
    SAutoPendingCloseTrade* psTrade = itFound->second;
    if (NULL == psTrade) {
        ASSERT(FALSE);
        return;
    }
    CTradeLog::OnCloseLog(bLong, uTradeID, fPrice, psTrade->fHighestPipGain);
    const COandaMgr::SOandaPair* psPair = COandaMgr::GetInstance().GetTradePair(psTrade->uTradePairIndex);
    if (NULL == psPair) {
        ASSERT(FALSE);
        return;
    }
    delete psTrade;
    m_cClosingTradeMap.erase(itFound);
}

// try to close a trade,
BOOLEAN CAutomationMgr::CloseTrade(UINT uTradeID, BOOLEAN bWasLong, UINT uTradePairIndex, FLOAT fHighestPipGain)
{
    const UINT uAccountIndex = bWasLong ? COandaMgr::EACCOUNT_LONG : COandaMgr::EACCOUNT_SHORT;
    COandaMgr& cMgr = COandaMgr::GetInstance();
    const COandaMgr::SOandaTrade* psTrade = cMgr.GetTrade(uAccountIndex, uTradeID);
    if (NULL == psTrade) {
        TRACE("Trade Missing at:%d (%d)\n", uAccountIndex, uTradeID);
        //ASSERT(FALSE);
        return FALSE;
    }
    SAutoPendingCloseTrade* psPendingClose = new SAutoPendingCloseTrade;
    if (NULL == psPendingClose) {
        ASSERT(FALSE);
        return FALSE;
    }
    psPendingClose->bLong = bWasLong;
    psPendingClose->uTradePairIndex = uTradePairIndex;
    psPendingClose->uPendingMsgID = cMgr.CloseTrade(uAccountIndex, uTradeID);
    psPendingClose->fHighestPipGain = fHighestPipGain;
    if (0 == psPendingClose->uPendingMsgID) {
        ASSERT(FALSE);
        delete psPendingClose;
        return FALSE;
    }
    m_cClosingTradeMap.insert(CClosingTradePair(psPendingClose->uPendingMsgID, psPendingClose));
    return TRUE;
}

BOOLEAN CAutomationMgr::CloseAllAccountTrades(VOID)
{
    BOOLEAN bHasClosedTrade = FALSE;
    COandaMgr& cMgr = COandaMgr::GetInstance();
    for (UINT uAccountIndex = 0; 2 > uAccountIndex; ++uAccountIndex) {
        const COandaMgr::CTradeList* pcList = cMgr.GetOpenTrades(uAccountIndex);
        if (NULL == pcList) {
            continue;
        }
        for (auto& sTrade : *pcList) {
            cMgr.CloseTrade(uAccountIndex, sTrade.uTradeID);
            bHasClosedTrade = TRUE;
        }
    }
    return bHasClosedTrade;
}

VOID CAutomationMgr::CloseAllOpenEntries(VOID)
{
    CloseAll(TRUE);
    CloseAll(FALSE);
}
VOID CAutomationMgr::CloseAll(BOOLEAN bLong)
{
    CTradePairEntryMap& cMap = bLong ? m_cLongEntries : m_cShortEntries;
    for (auto it : cMap) {
        const UINT uTradePairIndex = it.first;
        CTradeEntryList* pcList = it.second;
        for (auto psEntry : *pcList) {
            if (CloseTrade(psEntry->uTradeID, bLong, uTradePairIndex, psEntry->fHighestPipGain)) {
                OnClosedTrade(uTradePairIndex, bLong, psEntry->uLotSize);
            }
        }
    }
}
