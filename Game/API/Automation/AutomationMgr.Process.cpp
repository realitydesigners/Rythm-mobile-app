#include "stdafx.h"
#include "AutoLogicData.h"
#include "AutomationMgr.h"
#include "EventID.h"
#include "EventManager.h"
#include "FractalData.h"
#include "MegaZZ.h"
#include "MessageLog.h"
#include "OandaMgr.h"
#include "PatternData.h"
#include "TradeLog.h"
#include <map>

typedef std::map<UINT, INT> CCategoryResultMap;
typedef std::pair<UINT, INT> CCategoryResultPair;


//    > if exists open entry, check exit requirements, and exit accordingly (target/stop loss)
//      -> if no open SHORT orders, process all Reversal combinations
//             -> if entry condition matches, ENTER
//      -> if no open LONG orders, process all Trend combinations
//             -> if entry condition matches, ENTER
VOID CAutomationMgr::OnTradePairQueried(UINT uTradePairIndex)
{
    COandaMgr& cMgr = COandaMgr::GetInstance();
    const COandaMgr::SOandaPair* psTradePair = cMgr.GetTradePair(uTradePairIndex);
    if (NULL == psTradePair) {
        ASSERT(FALSE);
        return;
    }
    // update entry
    if (!UpdateMegaZZ(*psTradePair)) { // if failed to update, skip
        return;
    }
    if (!m_bStarted) {
        return;
    }
    const BOOLEAN bCanProcessNewTrades = ((psTradePair->fAskPrice - psTradePair->fBidPrice) * psTradePair->dPipMultiplier) < 3.0f;
    ProcessExistingEntry(*psTradePair, TRUE);   // process LONG entries
    ProcessExistingEntry(*psTradePair, FALSE);  // process SSHORT entries
    
    if (!m_bCanEnterNewTrades || m_bMarketClosed || !bCanProcessNewTrades) {
        return;
    }
    Process(uTradePairIndex);
}

VOID CAutomationMgr::ProcessExistingEntry(const COandaMgr::SOandaPair& sPair, BOOLEAN bLong)
{
    // check for existing entries
    CTradePairEntryMap& cMap = bLong ? m_cLongEntries : m_cShortEntries;
    CTradePairEntryMap::iterator itFound = cMap.find(sPair.uIndex);
    if (cMap.end() == itFound) { // if not found
        return;
    }
    
    CTradeEntryList* pcList = itFound->second;
    if (NULL == pcList) {
        ASSERT(FALSE);
        return;
    }
    CHAR szBuffer[2048];
    CTradeEntryList::iterator itEntry = pcList->begin();
    const CTradeEntryList::iterator itEnd = pcList->end();
    for ( ; itEnd != itEntry; ) {
        SAutomationEntry* psEntry = *(itEntry);
        SAutomationEntry& sEntry = *psEntry;
        if (0 != sEntry.uPendingMsgID) { // waiting for reply from server
            ++itEntry;
            continue;
        }
        ASSERT(0.0f < sEntry.fTransactPrice);
        const SLogic& sLogic = CAutoLogicDataMgr::GetLogic(sEntry.uLogicIndex);
        ASSERT(0 != sEntry.uTradeID);
        CMegaZZ* pcZZ = FindMegaZZ(sEntry.byTradePairIndex, GetFractalIndex(), FALSE);
        if (NULL == pcZZ) {
            ASSERT(FALSE);
            ++itEntry;
            continue;
        }
        const BOOLEAN bLong = (0 != sEntry.byLong);
        const FLOAT fCurrentPrice = pcZZ->GetCurrentPrice();
        FLOAT fPipGain;
        if (bLong) {
            fPipGain = (FLOAT)((fCurrentPrice - sEntry.fTransactPrice) * sPair.dPipMultiplier);
        }
        else {
            fPipGain = (FLOAT)((sEntry.fTransactPrice - fCurrentPrice) * sPair.dPipMultiplier);
        }
        BOOLEAN bUpdateTrade = FALSE;
        if (sEntry.fHighestPipGain < fPipGain) {
            sEntry.fHighestPipGain = fPipGain;
            bUpdateTrade = TRUE;
        }
        if (sEntry.fLowestPipLoss > fPipGain) {
            sEntry.fLowestPipLoss = fPipGain;
            bUpdateTrade = TRUE;
        }
        if (bUpdateTrade) {
            CTradeLog::UpdateTrade(bLong, psEntry->uTradeID, sEntry.fHighestPipGain, sEntry.fLowestPipLoss);
        }
        // check for stop loss
        if (0.0f > fPipGain) {
            const BOOLEAN bDoStopLoss = (0.0f >= ((sEntry.fStopLossSafetyPip + sEntry.fSlippage) + fPipGain));
            if (bDoStopLoss) {
                if (CloseTrade(sEntry.uTradeID, bLong, sPair.uIndex, sEntry.fHighestPipGain)) {
                    OnClosedTrade(sPair.uIndex, bLong, sEntry.uLotSize);
                    const SLogic& sLogic = CAutoLogicDataMgr::GetLogic(sEntry.uLogicIndex);
                    snprintf(szBuffer, 2048, "[%s]=>[%s] SLPattern [%s] %.1f", sLogic.szName, sPair.szTradePair, CPatternDataMgr::GetPattern(sEntry.uStopLossTPIndex).szName, fPipGain);
                    CMessageLog::AddLog(szBuffer);
                    CTradeEntryList::iterator itTmp = itEntry;
                    ++itEntry;
                    delete psEntry;
                    pcList->erase(itTmp);
                    continue;
                }
            }
            ++itEntry;
            continue;
        }
        BOOLEAN bDoTakeProfit = FALSE;
        BOOLEAN bSafetyTP = FALSE;
        // check for TP
        if (0 == sEntry.byPipLevelCrossed) { // target pip level not yet crossed
            if (sEntry.fTP_PipCheck <= fPipGain) {
                sEntry.byPipLevelCrossed =  1;
                bDoTakeProfit = TRUE; // just take profit. skip trailing stop
            }
        }
        else {
            const FLOAT fTrailStop = sEntry.fHighestPipGain - fPipGain;
            if (sEntry.fTP_PipTrailCheck <= fTrailStop) {
                bDoTakeProfit = TRUE;
            }
        }
        if (!bDoTakeProfit) {
            if (0.0f < sEntry.fSafetyTP_PipCheck) { // if safety pip exist
                if (fPipGain <= 1.0f) {             // changed to 1 pip safety instead
                    bDoTakeProfit = TRUE;
                    bSafetyTP = TRUE;
                }
            }
        }
        if (bDoTakeProfit) {
            if (CloseTrade(sEntry.uTradeID, bLong, sPair.uIndex, sEntry.fHighestPipGain)) {
                const UINT uEntrySequenceID = sEntry.uEntrySequence;
                const FLOAT fSafetyPip = sEntry.fTP_PipCheck;
                OnClosedTrade(sPair.uIndex, bLong, sEntry.uLotSize);
                const SLogic& sLogic = CAutoLogicDataMgr::GetLogic(sEntry.uLogicIndex);
                if (bSafetyTP) {
                    snprintf(szBuffer, 2048, "[%s] => Safety TP [%s]", sLogic.szName, sPair.szTradePair);
                }
                else {
                    snprintf(szBuffer, 2048, "[%s] => TP [%s]", sLogic.szName, sPair.szTradePair);
                }
                CMessageLog::AddLog(szBuffer);
                CTradeEntryList::iterator itTmp = itEntry;
                ++itEntry;
                delete psEntry;
                pcList->erase(itTmp);
                if (!bSafetyTP) {
                    UpdateSafetyTP(*pcList, uEntrySequenceID, fSafetyPip);
                }
                continue;
            }
        }
        ++itEntry;
    }
}

VOID CAutomationMgr::UpdateSafetyTP(CTradeEntryList& cList, UINT uEntrySequenceID, FLOAT fSafetyPip)
{
    fSafetyPip *= 0.75f; // reduce 25%
    TRACE("Try set safety pip %d to %f\n", uEntrySequenceID, fSafetyPip);
    
    for (auto psEntry : cList) {
        if (psEntry->uEntrySequence != uEntrySequenceID) {
            continue;
        }
        if (0.0f == psEntry->fSafetyTP_PipCheck) {
            psEntry->fSafetyTP_PipCheck = fSafetyPip;
            continue;
        }
        if (psEntry->fSafetyTP_PipCheck < fSafetyPip) {
            psEntry->fSafetyTP_PipCheck = fSafetyPip;
        }
    }
}
BOOLEAN CAutomationMgr::UpdateMegaZZ(const COandaMgr::SOandaPair& sPair)
{
    COandaMgr& cMgr = COandaMgr::GetInstance();
    const SRythmListener* psRythm = cMgr.GetRythmListener(sPair.uIndex);
    if (NULL == psRythm) {
        ASSERT(FALSE);
        return FALSE;
    }
    if (0 == psRythm->cList.size()) { // no entry
        return FALSE;
    }
    const UINT uFractalIndex = GetFractalIndex();
    CMegaZZ* pcZZ = FindMegaZZ(sPair.uIndex, uFractalIndex, TRUE); // find or create a new megazz
    if (NULL == pcZZ) {
        ASSERT(FALSE);
        return FALSE;
    }
    if (0 == pcZZ->GetFrameUsedNum()) { // not yet initialized
        const SFractal& sFractal = CFractalDataMgr::GetFractal(uFractalIndex);
        FLOAT afSizes[MEGAZZ_MAX_DEPTH];
        for (UINT uDepthIndex = 0; MEGAZZ_MAX_DEPTH > uDepthIndex; ++uDepthIndex) {
            afSizes[uDepthIndex] = sFractal.afPipSize[uDepthIndex] / sPair.dPipMultiplier;
        }
        pcZZ->Initlialize(MEGAZZ_MAX_DEPTH, sFractal.uBoxNum, afSizes);
        
        auto& sRythm =  *(psRythm->cList.begin());
        const BOOLEAN bUp = psRythm->dCurrentPrice >= sRythm.dCLosePrice;
        pcZZ->SetupFirstPrice(sRythm.ullTimeMilliSec, sRythm.dCLosePrice, bUp);
    }
    UINT64 ullLastUpdatedMillisec = pcZZ->GetLastUpdatedTimeMillisec();
    for (const auto& sRythm : psRythm->cList) {
        if (ullLastUpdatedMillisec > sRythm.ullTimeMilliSec) {
            continue;
        }
        ullLastUpdatedMillisec = sRythm.ullTimeMilliSec;
        pcZZ->UpdatePrice(ullLastUpdatedMillisec, (FLOAT)sRythm.dCLosePrice);
    }
    pcZZ->EndUpdate();
    ProcessBibLog(*pcZZ);
    CEventManager::BroadcastEvent(EGLOBAL_AUTOMATION_MGR, CEvent(EGLOBAL_EVT_AUTOMATION_MEGAZZ_UPDATED, sPair.uIndex));
    return TRUE;
}

VOID CAutomationMgr::Process(UINT uTradePairIndex)
{
    const COandaMgr::SOandaPair* psPair = COandaMgr::GetInstance().GetTradePair(uTradePairIndex);
    if (NULL == psPair) {
        return;
    }
    const CMegaZZ* pcZZ = FindMegaZZ(uTradePairIndex, GetFractalIndex());
    if (NULL == pcZZ) {
        ASSERT(FALSE);
        return;
    }
    for (const auto& psRunningLogic : m_cRunningLogicList) { // for each logic
        if (psRunningLogic->bStopped) { // skip stopped logic
            continue;
        }
        const SLogic& sLogic = psRunningLogic->sLogic;
        // first we try check category pattern
        if (!CPatternDataMgr::CanPassPattern(*pcZZ, sLogic.uCategoryPatternIndex, FALSE)) {
            // failed main category, skip
            continue;
        }
        const STPEntry& sStopLoss = CAutoLogicDataMgr::GetTPEntry(sLogic.uStopLossTPIndex);
        ASSERT(9 >= sStopLoss.byFrameIndex);
        const FLOAT fStopLossPip = pcZZ->GetChannelSize(9 - sStopLoss.byFrameIndex) * sStopLoss.fPercent * psPair->dPipMultiplier;
        
        BOOLEAN bStarted = FALSE;
        const UINT uCategoryLotSize = sLogic.uLotSize;
        for (UINT uIndex = 0; TRADE_ORDER_TARGET_NUM > uIndex; ++uIndex) { // for each of the TP
            const UINT uTPIndex = sLogic.anTargetIndex[uIndex];
            if (TRADE_ORDER_UNUSED_TARGET == uTPIndex) {
                continue;
            }
            const UINT uLotSize = uCategoryLotSize + uIndex;
            // if a trade is already opened with this lot size. skip all next trades
            if (!CanOpenTrade(uTradePairIndex, sLogic.bLong, uLotSize)) {
                break;
            }
            static UINT s_uEntrySequenceID = 0;
            if (!bStarted) {
                ++s_uEntrySequenceID;
                bStarted = TRUE;
            }
            TryOpenTrade(*pcZZ,
                         s_uEntrySequenceID,
                         psRunningLogic->uLogicIndex,
                         0,
                         sLogic.uCategoryPatternIndex,
                         sLogic.bLong,
                         sLogic.uStopLossTPIndex,
                         fStopLossPip,
                         uTPIndex,
                         uLotSize);
        }
    }
}

BOOLEAN CAutomationMgr::CanOpenTrade(UINT uTradePairIndex, BOOLEAN bLong, UINT uLotSize)
{
    ASSERT(0x80000000 > uTradePairIndex);
    ASSERT(0xFFFF > uLotSize);
    if (bLong) {
        uTradePairIndex |= 0x80000000;
    }
    CTradePairStatusMap::iterator itFound = m_cTradeStatusMap.find(uTradePairIndex);
    if (m_cTradeStatusMap.end() == itFound) { // if not found,
        return TRUE;
    }
    CTradeStatusMap& cMap = itFound->second;
    CTradeStatusMap::iterator itTrade = cMap.find(uLotSize);
    if (cMap.end() == itTrade) {
        return TRUE;
    }
    return FALSE;
}
VOID CAutomationMgr::OnOpenedTrade(UINT uTradePairIndex, BOOLEAN bLong, UINT uLotSize)
{
    ASSERT(0x80000000 > uTradePairIndex);
    ASSERT(0xFFFF > uLotSize);
    if (bLong) {
        uTradePairIndex |= 0x80000000;
    }
    CTradePairStatusMap::iterator itFound = m_cTradeStatusMap.find(uTradePairIndex);
    if (m_cTradeStatusMap.end() == itFound) { // if not found,
        CTradeStatusMap cTmp;
        m_cTradeStatusMap.insert(CTradePairStatusPair(uTradePairIndex, cTmp));
        itFound = m_cTradeStatusMap.find(uTradePairIndex);
    }
    if (m_cTradeStatusMap.end() == itFound) { // if not found,
        ASSERT(FALSE);
        return;
    }
    CTradeStatusMap& cMap = itFound->second;
    CTradeStatusMap::iterator itTrade = cMap.find(uLotSize);
    if (cMap.end() == itTrade) {
        cMap.insert(CTradeStatusPair(uLotSize, TRUE));
        return;
    }
    ASSERT(FALSE);
}

VOID CAutomationMgr::OnClosedTrade(UINT uTradePairIndex, BOOLEAN bLong, UINT uLotSize)
{
    ASSERT(0x80000000 > uTradePairIndex);
    ASSERT(0xFFFF > uLotSize);
    if (bLong) {
        uTradePairIndex |= 0x80000000;
    }
    CTradePairStatusMap::iterator itFound = m_cTradeStatusMap.find(uTradePairIndex);
    if (m_cTradeStatusMap.end() == itFound) { // if not found,
        ASSERT(FALSE);
        return;
    }
    CTradeStatusMap& cMap = itFound->second;
    CTradeStatusMap::iterator itTrade = cMap.find(uLotSize);
    if (cMap.end() == itTrade) {
        ASSERT(FALSE);
        return;
    }
    cMap.erase(itTrade);
}
