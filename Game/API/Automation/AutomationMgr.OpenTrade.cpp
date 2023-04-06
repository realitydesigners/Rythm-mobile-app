#include "stdafx.h"
#include "AutoLogicData.h"
#include "AutomationMgr.h"
#include "HLTime.h"
#include "EventID.h"
#include "MegaZZ.h"
#include "MessageLog.h"
#include "OandaMgr.h"
#include "PatternData.h"
#include "TradeLog.h"

VOID CAutomationMgr::TryOpenTrade(const CMegaZZ& cZZ,
                                  UINT uEntrySequenceID,
                                  UINT uLogicIndex,
                                  UINT uPreTradePatternIndex,
                                  UINT uCatPatternIndex,
                                  BOOLEAN bLong,
                                  UINT uStopLossTPIndex,
                                  FLOAT fStopLossSafetyPip,
                                  UINT uTPIndex,                // which TP entry is this in the TP chart
                                  UINT uLotNum)
{
    const UINT uAccountIndex = bLong ? COandaMgr::EACCOUNT_LONG : COandaMgr::EACCOUNT_SHORT;
    const BYTE byTradePairIndex = (BYTE)cZZ.GetTradePairIndex();
    const COandaMgr::SOandaPair* psPair = COandaMgr::GetInstance().GetTradePair(byTradePairIndex);
    if (NULL == psPair) {
        ASSERT(FALSE);
        return;
    }
    
    const INT nUnits = bLong ? uLotNum : -(INT)(uLotNum);
    const UINT uMsgID = COandaMgr::GetInstance().CreateMarketOrder(uAccountIndex, byTradePairIndex, nUnits);
    if (0 == uMsgID) {
        ASSERT(FALSE);
        CMessageLog::AddLog("OpenTrade Error1");
        return;
    }
    OnOpenedTrade(byTradePairIndex, bLong, uLotNum);
    
    const BYTE byLong = bLong ? 1 : 0;
    const FLOAT fCurrentPrice = cZZ.GetCurrentPrice();
    SAutomationEntry* psEntry = new SAutomationEntry;
    if (NULL == psEntry) {
        ASSERT(FALSE);
        CMessageLog::AddLog("OpenTrade Error2");
        return;
    }
    psEntry->uEntrySequence = uEntrySequenceID;
    psEntry->byTradePairIndex = byTradePairIndex;
    psEntry->byLong = byLong;
    psEntry->byPipLevelCrossed = 0;
    psEntry->fHighestPipGain = 0.0f;
    psEntry->fLowestPipLoss = 0.0f;
    psEntry->uLogicIndex = uLogicIndex;
    psEntry->uStopLossTPIndex = uStopLossTPIndex;
    psEntry->fStopLossSafetyPip = fStopLossSafetyPip;
    psEntry->uTPIndex = uTPIndex;
    const STPEntry& sTPEntry = CAutoLogicDataMgr::GetTPEntry(uTPIndex);
    psEntry->fTP_PipTrailCheck = sTPEntry.fTrailStop;
    if (sTPEntry.eType == ETP_PIP_ONLY) {
        psEntry->fTP_PipCheck = sTPEntry.fPip;
    }
    else {
        ASSERT(sTPEntry.eType == ETP_FRAME_LVL);
        ASSERT(1 <= sTPEntry.byFrameIndex);
        ASSERT(9 >= sTPEntry.byFrameIndex);
        const UINT uDepthIndex = (9 - sTPEntry.byFrameIndex);
        const MegaZZ::SChannel& sChannel = cZZ.GetCurrentChannel(uDepthIndex);
        FLOAT fCurrentPriceOffset;
        if (MegaZZ::EType_Up == sChannel.byType) {
            fCurrentPriceOffset = sChannel.fTop - fCurrentPrice;
        }
        else {
            fCurrentPriceOffset = fCurrentPrice - sChannel.fBottom;
        }
        if (0.0f > fCurrentPriceOffset) {
            ASSERT(FALSE);
            fCurrentPriceOffset = 0.0f;
        }
        const FLOAT fTargetPriceOffset = cZZ.GetChannelSize(uDepthIndex) * sTPEntry.fPercent;
        psEntry->fTP_PipCheck = (fTargetPriceOffset - fCurrentPriceOffset) * psPair->dPipMultiplier;
        TRACE("Entry Pip Check(F%d Percent:%.1f%% => %.1f Trail %.1f\n", sTPEntry.byFrameIndex, sTPEntry.fPercent, psEntry->fTP_PipCheck, psEntry->fTP_PipTrailCheck);
    }    
    psEntry->fSafetyTP_PipCheck = 0.0f; // not yet set
    
    psEntry->uLotSize = uLotNum;
    psEntry->uPendingMsgID = uMsgID;
    psEntry->uTradeID = 0;
    psEntry->fTransactPrice = 0.0f;
    psEntry->fSlippage = fCurrentPrice;
    
    m_cPendingMsgIDEntries.insert(CPendingEntryPair(uMsgID, psEntry));
    CTradePairEntryMap& cMap = bLong ? m_cLongEntries : m_cShortEntries;
    CTradePairEntryMap::iterator itFound = cMap.find(byTradePairIndex);
    if (cMap.end() == itFound) { // if not found
        CTradeEntryList* pcList = new CTradeEntryList();
        if (NULL == pcList) {
            ASSERT(FALSE);
            CMessageLog::AddLog("OpenTrade Error3");
            return;
        }
        cMap.insert(CTradePairEntryPair(byTradePairIndex, pcList));
        itFound = cMap.find(byTradePairIndex);
    }
    if (cMap.end() == itFound) { // if not found
        ASSERT(FALSE);
        CMessageLog::AddLog("OpenTrade Error4");
        return;
    }
    CTradeEntryList* pcList = itFound->second;
    pcList->push_back(psEntry);
    CTradeLog::GenerateNewLog(cZZ, uLogicIndex, uPreTradePatternIndex, uCatPatternIndex, uStopLossTPIndex, uTPIndex, bLong, uMsgID);
}


VOID CAutomationMgr::OnCreateMarketOrderReply(UINT uMsgID, UINT uAccountIndex, UINT uTradeID, FLOAT fPrice)
{
    CTradeLog::OnTradeOpened(uMsgID, uTradeID, fPrice);
    CPendingEntryMap::iterator itFound = m_cPendingMsgIDEntries.find(uMsgID);
    if (m_cPendingMsgIDEntries.end() == itFound) {
        TRACE("Warn: entry not found for reply from create market order");
        return;
    }
    SAutomationEntry* psEntry = itFound->second;
    ASSERT(uMsgID == psEntry->uPendingMsgID);
    
    psEntry->uPendingMsgID = 0;
    psEntry->uTradeID = uTradeID;
    psEntry->fTransactPrice = fPrice;
    FLOAT fSlippage = fPrice - psEntry->fSlippage;
    if (!psEntry->byLong) {
        fSlippage = -fSlippage;
    }
    m_abRefreshAccounts[uAccountIndex % 2] = TRUE;
    m_cPendingMsgIDEntries.erase(itFound);

    const COandaMgr::SOandaPair* psPair = COandaMgr::GetInstance().GetTradePair(psEntry->byTradePairIndex);
    if (NULL == psPair) {
        ASSERT(FALSE);
        return;
    }
    psEntry->fSlippage = fSlippage * psPair->dPipMultiplier;
    if (2.0f < (psEntry->fTP_PipCheck - psEntry->fSlippage)) {
        psEntry->fTP_PipCheck -= psEntry->fSlippage;
    }
    const CHAR* szLogicName =CAutoLogicDataMgr::GetLogicName(psEntry->uLogicIndex);
    CHAR szBuffer[2048];
    snprintf(szBuffer, 2048, "Entry %s [%d] %s from %s Slippage %.1fpips", psEntry->byLong ? "LONG" : "SHORT", psEntry->uLotSize, psPair->szTradePair, szLogicName, psEntry->fSlippage);
    CMessageLog::AddLog(szBuffer);
}
