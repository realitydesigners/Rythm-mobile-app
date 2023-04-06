#include "stdafx.h"
#include "AppResourceManager.h"
#include "AutoLogicData.h"
#include "AutomationMgr.h"
#include "AutoPtr.h"
#include "EventID.h"
#include "EventManager.h"
#include "HLTime.h"
#include "MegaZZ.h"
#include "OandaMgr.h"
#include "TradeLog.h"

#define TRADE_FILE_NAME       "TradeLog.dat"
#define TRADE_LOG_VERSION     (0x000000012)

struct STradeLogHeader {
    UINT uVersion;
    UINT uLogNum;
};

static CTradeLogs s_cList;

static CActiveTradeLogMap s_cPendingOpenMap;
static CActiveTradeLogMap s_cActiveMap;

static VOID DoSave(VOID)
{
    // determine file size
    const UINT uLogNum = (UINT)s_cList.size() + (UINT)s_cActiveMap.size();
    const UINT uSize = sizeof(STradeLogHeader) + uLogNum * sizeof(STradeLog);
    BYTE* pbyBuffer = new BYTE[uSize];
    if (NULL == pbyBuffer) {
        ASSERT(FALSE);
        return;
    }
    CAutoPtrArray<BYTE> cGuard(pbyBuffer);
    STradeLogHeader* psHeader = (STradeLogHeader*)pbyBuffer;
    psHeader->uVersion = TRADE_LOG_VERSION;
    psHeader->uLogNum = uLogNum;
    STradeLog* psLog = (STradeLog*)(psHeader + 1);
    for (const auto& sLog : s_cList) {
        memcpy(psLog, &sLog, sizeof(STradeLog));
        ++psLog;
    }
    for (auto it : s_cActiveMap) {
        STradeLog* psActiveLog = it.second;
        memcpy(psLog, psActiveLog, sizeof(STradeLog));
        ++psLog;
    }
    CAppResourceManager::WriteToLocalStorage(TRADE_FILE_NAME, pbyBuffer, uSize);
}

BOOLEAN CTradeLog::Initialize(VOID)
{
    Release();
    UINT uDataSize = 0;
    BYTE* pbyBuffer = CAppResourceManager::ReadFromLocalStorage(TRADE_FILE_NAME, uDataSize);
    if (NULL == pbyBuffer) {
        TRACE("Trade Log not found. Blank Log\n");
        return TRUE;
    }
    CAutoPtrArray<BYTE> cGuardBuffer(pbyBuffer);
    const STradeLogHeader* psHeader = (const STradeLogHeader*)pbyBuffer;
    if (TRADE_LOG_VERSION != psHeader->uVersion) {
        TRACE("Trade Log Version Changed, cleared Log\n");
        return TRUE;
    }
    const UINT uLogNum = psHeader->uLogNum;
    const STradeLog* psLogs = (const STradeLog*)(psHeader + 1);
    for (UINT uIndex = 0; uLogNum > uIndex; ++uIndex, ++psLogs) {
        // active logs do not have close time
        if (0 < psLogs->uCloseTimeSec) {
            s_cList.push_back(*psLogs);
        }
        else {
            STradeLog* psNew = new STradeLog;
            if (NULL != psNew) {
                memcpy(psNew, psLogs, sizeof(STradeLog));
                const UINT uIDKey = psNew->uTradeID | (psNew->byLong ? 0x80000000 : 0);
                s_cActiveMap.insert(CActiveTradeLogPair(uIDKey, psNew));
            }
        }
    }
    return TRUE;
}

VOID CTradeLog::Release(VOID)
{
    s_cList.clear();
    
    for (auto it : s_cPendingOpenMap) {
        delete it.second;
    }
    s_cPendingOpenMap.clear();
    for (auto it : s_cActiveMap) {
        delete it.second;
    }
    s_cActiveMap.clear();
}

VOID CTradeLog::Clear(VOID)
{
    s_cList.clear(); // only clear active logs
    CEventManager::BroadcastEvent(EGLOBAL_TRADE_LOG, CEvent(EGLOBAL_EVT_TRADE_LOG_REFRESHED));
    DoSave();
}

VOID CTradeLog::AddLog(const STradeLog& sLog)
{
    s_cList.push_back(sLog);
    while (TRADE_LOG_MAX_LOG < s_cList.size()) {
        s_cList.pop_front();
    }
    CEventManager::BroadcastEvent(EGLOBAL_TRADE_LOG, CEvent(EGLOBAL_EVT_TRADE_LOG_REFRESHED));
    DoSave();
}

const CTradeLogs& CTradeLog::GetLogs(VOID)
{
    return s_cList;
}
const CActiveTradeLogMap& CTradeLog::GetActiveLogs(VOID)
{
    return s_cActiveMap;
}

STradeLog* CTradeLog::GenerateNewLog(const CMegaZZ& cMegaZZ,
                                     UINT uLogicIndex,
                                     UINT uPreTradePatternIndex,
                                     UINT uCatPatternIndex,
                                     UINT uSLPatternIndex,
                                     UINT uTPIndex,
                                     BOOLEAN bLong,
                                     UINT uPendingMsgID)
{
    STradeLog* psLog = new STradeLog;
    if (NULL == psLog) {
        ASSERT(FALSE);
        return NULL;
    }
    memset(psLog, 0, sizeof(STradeLog));
    psLog->byTradePairIndex = cMegaZZ.GetTradePairIndex();
    psLog->uOpenTimeSec = CHLTime::GetTimeSecs();
    psLog->fOpenPrice = cMegaZZ.GetCurrentPrice();
    psLog->uLogicIndex = uLogicIndex;
    psLog->uPreTradePatternIndex = uPreTradePatternIndex;
    psLog->uCategoryPatternIndex = uCatPatternIndex;
    psLog->uTPIndex = uTPIndex;
    psLog->uStopLossTPIndex = uSLPatternIndex;

    psLog->byLong = bLong ? 1 : 0;
    for (UINT uDepthIndex = 0; MEGAZZ_MAX_DEPTH > uDepthIndex; ++uDepthIndex) {
        psLog->asStart[uDepthIndex] = cMegaZZ.GetCurrentChannel(uDepthIndex);
    }
    s_cPendingOpenMap.insert(CActiveTradeLogPair(uPendingMsgID, psLog));
    return psLog;
}

VOID CTradeLog::OnTradeOpened(UINT uPendingMsgID, UINT uTradeID, FLOAT fOpenPrice)
{
    CActiveTradeLogMap::iterator itFound = s_cPendingOpenMap.find(uPendingMsgID);
    if (s_cPendingOpenMap.end() == itFound) {
        ASSERT(FALSE);
        return;
    }
    STradeLog* psLog = itFound->second;
    s_cPendingOpenMap.erase(itFound);
    psLog->uTradeID = uTradeID;
    psLog->fSlippage = fOpenPrice - psLog->fOpenPrice;
    if (0 == psLog->byLong) {
        psLog->fSlippage = -psLog->fSlippage;
    }
    psLog->fOpenPrice = fOpenPrice;
    ASSERT(0x80000000 > uTradeID);
    const UINT uIDKey = uTradeID | (psLog->byLong ? 0x80000000 : 0);
    s_cActiveMap.insert(CActiveTradeLogPair(uIDKey, psLog));
    CEventManager::BroadcastEvent(EGLOBAL_TRADE_LOG, CEvent(EGLOBAL_EVT_TRADE_LOG_REFRESHED));
}

VOID CTradeLog::UpdateTrade(BOOLEAN bLong, UINT uTradeID, FLOAT fHighestPipReached, FLOAT fLowestPipReached)
{
    const UINT uIDKey = uTradeID | (bLong ? 0x80000000 : 0);
    CActiveTradeLogMap::iterator itFound = s_cActiveMap.find(uIDKey);
    if (s_cActiveMap.end() == itFound) {
//        ASSERT(FALSE);
        return;
    }
    STradeLog* psLog = itFound->second;
    psLog->fHighestPipReached = fHighestPipReached;
    psLog->fLowestPipReached = fLowestPipReached;
}
VOID CTradeLog::OnCloseLog(BOOLEAN bLong, UINT uTradeID, FLOAT fClosePrice, FLOAT fHighestPipReached)
{
    const UINT uIDKey = uTradeID | (bLong ? 0x80000000 : 0);
    CActiveTradeLogMap::iterator itFound = s_cActiveMap.find(uIDKey);
    if (s_cActiveMap.end() == itFound) {
//        ASSERT(FALSE);
        return;
    }
    STradeLog* psLog = itFound->second;
    s_cActiveMap.erase(itFound);
    const SLogic& sLogic = CAutoLogicDataMgr::GetLogic(psLog->uLogicIndex);
    const CMegaZZ* pcZZ =  CAutomationMgr::GetInstance().FindMegaZZ(psLog->byTradePairIndex, CAutomationMgr::GetInstance().GetFractalIndex());
    if (NULL != pcZZ) {
        for (UINT uDepthIndex = 0; MEGAZZ_MAX_DEPTH > uDepthIndex; ++uDepthIndex) {
            psLog->asEnd[uDepthIndex] = pcZZ->GetCurrentChannel(uDepthIndex);
        }
    }
    
    psLog->uCloseTimeSec = CHLTime::GetTimeSecs();
    psLog->fCLosePrice = fClosePrice;
    psLog->fHighestPipReached = fHighestPipReached;
    
    // move this log to history list
    AddLog(*psLog);
    // compute pip gain
    FLOAT fGain = fClosePrice - psLog->fOpenPrice;
    if (0 == psLog->byLong) {
        fGain = -fGain;
    }
    const COandaMgr::SOandaPair* psPair = COandaMgr::GetInstance().GetTradePair(psLog->byTradePairIndex);
    if (NULL != psPair) {
        fGain = fGain * psPair->dPipMultiplier;
    }
    // delete ptr.
    delete psLog;
}
