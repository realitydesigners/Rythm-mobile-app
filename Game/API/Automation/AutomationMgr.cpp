#include "stdafx.h"
#include "AutomationMgr.h"
#include "Event.h"
#include "EventID.h"
#include "EventManager.h"
#include "FractalData.h"
#include "AutoLogicData.h"
#include "MegaZZ.h"
#include "MessageLog.h"
#include "NowLoadingWindow.h"
#include "PatternData.h"
#include "HLTime.h"

static CAutomationMgr* s_pcMgr = NULL;

BOOLEAN CAutomationMgr::InitializeStatic(VOID)
{
    if (NULL != s_pcMgr) {
        ASSERT(FALSE);
        return FALSE;
    }
    s_pcMgr = new CAutomationMgr();
    if (NULL != s_pcMgr) {
        s_pcMgr->Initialize();
    }
    return (NULL != s_pcMgr);
}

VOID CAutomationMgr::ReleaseStatic(VOID)
{
    SAFE_RELEASE_DELETE(s_pcMgr);
}

CAutomationMgr& CAutomationMgr::GetInstance(VOID)
{
    return *s_pcMgr;
}


CAutomationMgr::CAutomationMgr() :
CEventHandler(EGLOBAL_AUTOMATION_MGR),
m_bMarketClosed(FALSE),
m_bStarted(FALSE),
m_bCanEnterNewTrades(FALSE),
m_uSecondsToOpenTime(0),
m_uSecondsToStopNewEntryTime(0),
m_uSecondsToCloseAllTrades(0),
m_uFractalIndex(0)
{
    m_abRefreshAccounts[0] = m_abRefreshAccounts[1] = FALSE;
}

CAutomationMgr::~CAutomationMgr()
{
    
}
BOOLEAN CAutomationMgr::Initialize(VOID)
{
    Release(); // sasfety
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_SYSTEM, EGLOBAL_EVENT_SYSTEM_HEARTBEAT);
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_RYTHM_UPDATE);
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_CLOSE_TRADE_REPLY);
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_CREATE_MARKET_ORDER_REPLY);
    return TRUE;
}

VOID CAutomationMgr::Release(VOID)
{
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_RYTHM_UPDATE);
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_SYSTEM, EGLOBAL_EVENT_SYSTEM_HEARTBEAT);
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_CLOSE_TRADE_REPLY);
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_CREATE_MARKET_ORDER_REPLY);
    for (auto& psRunningLogic : m_cRunningLogicList) {
        delete psRunningLogic;
    }
    m_cRunningLogicList.clear();
    
    ClearTradeFractalMap();
    Release(m_cLongEntries);
    Release(m_cShortEntries);
    m_cPendingMsgIDEntries.clear(); // they share the same ptr as long/short entries
    m_cTradeStatusMap.clear();

    m_bStarted = FALSE;
    
    for (auto& itTrade : m_cClosingTradeMap) {
        SAutoPendingCloseTrade* psTrade = itTrade.second;
        delete psTrade;
    }
    m_cClosingTradeMap.clear();
    m_abRefreshAccounts[0] = m_abRefreshAccounts[1] = FALSE;
    CLearBibLogs();
}

VOID CAutomationMgr::CLearBibLogs(VOID)
{
    for (auto it : m_cBibLog) {
        CBibLogList* pcList = it.second;
        for (auto psLog : *pcList) {
            delete psLog;
        }
        delete pcList;
    }
    m_cBibLog.clear();
}

VOID CAutomationMgr::Release(CTradePairEntryMap& cMap)
{
    for (auto& it : cMap ) {
        CTradeEntryList* pcList = it.second;
        for (auto psEntry : *pcList) {
            delete psEntry;
        }
        pcList->clear();
        SAFE_DELETE(pcList);
    }
    cMap.clear();
}

const CMegaZZ* CAutomationMgr::FindMegaZZ(UINT uTradePairIndex, UINT uFractalIndex) const
{
    return const_cast<CAutomationMgr*>(this)->FindMegaZZ(uTradePairIndex, uFractalIndex, FALSE);
}
CMegaZZ* CAutomationMgr::FindMegaZZ(UINT uTradePairIndex, UINT uFractalIndex, BOOLEAN bCreateIfNotExist)
{
    CFractalMegaZZMap* pcMap = NULL;
    {
        CTradePairFractalMap::iterator itFoundMap = m_cTradePairMap.find(uTradePairIndex);
        if (m_cTradePairMap.end() == itFoundMap) {
            if (!bCreateIfNotExist) {
                return NULL;
            }
            pcMap = new CFractalMegaZZMap;
            if (NULL == pcMap) {
                ASSERT(FALSE);
                return NULL;
            }
            m_cTradePairMap.insert(CTradePairFractalPair(uTradePairIndex, pcMap));
        }
        else {
            pcMap = itFoundMap->second;
        }
    }
    if (NULL == pcMap) {
        ASSERT(FALSE);
        return NULL;
    }
    CFractalMegaZZMap::iterator itZZ = pcMap->find(uFractalIndex);
    if (pcMap->end() == itZZ) {
        if (!bCreateIfNotExist) {
            return NULL;
        }
        CMegaZZ* pcZZ = new CMegaZZ();
        if (NULL == pcZZ) {
            ASSERT(FALSE);
            return NULL;
        }
        pcZZ->SetSource(ESOURCE_MARKET_OANDA, uTradePairIndex);
        pcMap->insert(CFractalMegaZZPair(uFractalIndex, pcZZ));        
        return pcZZ;
    }
    return itZZ->second;
}

VOID CAutomationMgr::OnReceiveEvent(CEvent& cEvent)
{
    switch (cEvent.GetIntParam(0)) {
        case EGLOBAL_EVT_OANDA_RYTHM_UPDATE:
            OnTradePairQueried(cEvent.GetIntParam(1));
            break;
        case EGLOBAL_EVT_OANDA_CLOSE_TRADE_REPLY:
            OnCloseTradeReply(cEvent.GetIntParam(1), cEvent.GetIntParam(2), cEvent.GetIntParam(3), cEvent.GetIntParam(4), cEvent.GetFloatParam(5));
            break;
        case EGLOBAL_EVT_OANDA_CREATE_MARKET_ORDER_REPLY:
            OnCreateMarketOrderReply(cEvent.GetIntParam(1), cEvent.GetIntParam(2), cEvent.GetIntParam(3), cEvent.GetFloatParam(4));
            break;
        case EGLOBAL_EVENT_SYSTEM_HEARTBEAT:
            OnHeartBeat();
            break;
        case EEVENT_DELAYED_REFRESH:
            CNowLoadingWindow::HideWindow();
            DelayRefreshAccount();
            break;
        default:
            ASSERT(FALSE);
            break;
    }
}


VOID CAutomationMgr::Start(VOID)
{
    if (m_bStarted) {
        ASSERT(FALSE);
        return;
    }
    ASSERT(0 == m_cRunningLogicList.size());
    const SFractal& sFractal = CFractalDataMgr::GetFractal(GetFractalIndex());
    const UINT uLogicNum = CAutoLogicDataMgr::GetLogicNum();
    for (UINT uLogicIndex = 0; uLogicNum > uLogicIndex; ++uLogicIndex) {
        const SLogic& sLogic = CAutoLogicDataMgr::GetLogic(uLogicIndex);
        SAutoRunningLogic* psRunning = new SAutoRunningLogic(sFractal, sLogic, uLogicIndex);
        if (NULL == psRunning) {
            ASSERT(FALSE);
            return;
        }
        m_cRunningLogicList.push_back(psRunning);
    }
    if (CloseAllAccountTrades()) {
        CNowLoadingWindow::DisplayWindow();
        CEventManager::PostGlobalEvent(EGLOBAL_AUTOMATION_MGR, CEvent(EEVENT_DELAYED_REFRESH), 2.0f);
        return;
    }
    m_bStarted = TRUE;
}
VOID CAutomationMgr::DelayRefreshAccount(VOID)
{
    for (UINT uAccountIndex = 0; 2 > uAccountIndex; ++uAccountIndex) {
        m_abRefreshAccounts[0] = TRUE;
    }
}

VOID CAutomationMgr::Stop(VOID)
{
    m_bStarted = FALSE;
    for (auto& psRunningLogic : m_cRunningLogicList) {
        delete psRunningLogic;
    }
    m_cRunningLogicList.clear();
    
    CloseAllOpenEntries();
}

VOID CAutomationMgr::IncrementStopLossCounter(SAutoRunningLogic& sLogic)
{
    CHAR szBuffer[1024];
    ++sLogic.uStopLossCount;
    if (5 <= sLogic.uStopLossCount) {
        sLogic.bStopped = TRUE;
    }
    const CHAR* szName = CPatternDataMgr::GetPattern(sLogic.sLogic.uCategoryPatternIndex).szName;
    if (sLogic.bStopped) {
        snprintf(szBuffer, 1024, "SL [%s] %d=>%d [Terminated]", szName, sLogic.uStopLossCount - 1, sLogic.uStopLossCount);
    }
    else {
        snprintf(szBuffer, 1024, "SL [%s] %d=>%d", szName, sLogic.uStopLossCount - 1, sLogic.uStopLossCount);
    }
    CMessageLog::AddLog(szBuffer);
}

VOID CAutomationMgr::OnHeartBeat(VOID)
{
    for (UINT uIndex = 0; 2 > uIndex; ++uIndex) {
        if (m_abRefreshAccounts[uIndex]) {
            m_abRefreshAccounts[uIndex] = FALSE;
            COandaMgr::GetInstance().QueryOpenTrades(uIndex);
            COandaMgr::GetInstance().QueryAccountDetails(uIndex);
        }
    }
    UpdateTime();
}

static VOID InsertLog(CBibLogList& cList, CMegaZZ& cZZ)
{
    SBibLog sTmp;
    for (UINT uIndex = 0; 8 > uIndex; ++uIndex) {
        const UINT uDepthIndex = uIndex;
        const MegaZZ::SChannel& sChannel = cZZ.GetCurrentChannel(uDepthIndex);
        BYTE byWas = sChannel.byRetracementNumber;
        BYTE byIs = sChannel.byDisplayNumber;
        if (MegaZZ::EType_Up == sChannel.byType) {
            byWas += 10;
            byIs += 10;
        }
        sTmp.abyWAS[uIndex] = byWas;
        sTmp.abyIS[uIndex] = byIs;
        sTmp.afTop[uIndex] = sChannel.fTop;
        sTmp.afBottom[uIndex] = sChannel.fBottom;
    }
    BOOLEAN bInsert = (0 == cList.size());
    if (!bInsert) {
        const SBibLog* psPrev = *cList.begin();
        if (0 == memcmp(psPrev->abyWAS, sTmp.abyWAS, 8)) {
            if (0 == memcmp(psPrev->abyIS, sTmp.abyIS, 8)) {
                if (0 == memcmp(psPrev->afBottom, sTmp.afBottom, sizeof(FLOAT)*8)) {
                    if (0 == memcmp(psPrev->afTop, sTmp.afTop, sizeof(FLOAT)*8)) {                        
                        return;
                    }
                }
            }
        }
    }
    SBibLog* psLog = new SBibLog;
    if (NULL == psLog) {
        ASSERT(FALSE);
        return;
    }
    memcpy(psLog, &sTmp, sizeof(SBibLog));
    psLog->uTime = CHLTime::GetTimeSecs();
    psLog->fPrice = cZZ.GetCurrentPrice();
    cList.push_front(psLog);
}
VOID CAutomationMgr::ProcessBibLog(CMegaZZ& cZZ)
{
#if defined(MAC_PORT)
    const UINT uTradePairIndex = cZZ.GetTradePairIndex();
    if (0 != uTradePairIndex) {
        return;
    }
    CBibLogMap::iterator itFound = m_cBibLog.find(uTradePairIndex);
    if (m_cBibLog.end() == itFound) {
        CBibLogList* pcList = new CBibLogList;
        if (NULL == pcList) {
            ASSERT(FALSE);
            return;
        }
        m_cBibLog.insert(CBibLogPair(uTradePairIndex, pcList));
        itFound = m_cBibLog.find(uTradePairIndex);
    }
    if (m_cBibLog.end() == itFound) {
        ASSERT(FALSE);
        return;
    }
    CBibLogList* pcList = itFound->second;
    InsertLog(*pcList, cZZ);
#endif // #if defined(MAC_PORT)
}

#include "AppResourceManager.h"
#include "OandaMgr.h"
#include "TradeLogWidget.h"
VOID CAutomationMgr::ExportBibLog(VOID)
{
    CHAR szFileName[128];
    CHAR szFilePath[1024];
    for (auto it : m_cBibLog) {
        CBibLogList* pcList = it.second;
        const UINT uTradePair = it.first;
        const COandaMgr::SOandaPair* psPair = COandaMgr::GetInstance().GetTradePair(uTradePair);
        if (NULL == psPair) {
            ASSERT(FALSE);
            continue;
        }
        snprintf(szFileName, 128, "/Log/%s.log", psPair->szTradePair);
        CAppResourceManager::GetLocalStoragePath(szFileName, szFilePath, 1024);
        FILE* phFile = fopen(szFilePath, "w");
        if (NULL == phFile) {
            ASSERT(FALSE);
            return;
        }
        CBibLogList::reverse_iterator itr = pcList->rbegin();
        const CBibLogList::reverse_iterator itEnd = pcList->rend();
        for (; itEnd != itr; ++itr) {
            const SBibLog* psLog = *(itr);
            fwrite(psLog, 1, sizeof(SBibLog), phFile);
        }
        fclose(phFile);
    }
}

VOID CAutomationMgr::ClearTradeFractalMap(VOID)
{
    
    for (auto& it : m_cTradePairMap ) {
        CFractalMegaZZMap* pcMap = it.second;
        ASSERT(NULL != pcMap);
        for (auto& it2 : *pcMap) {
            CMegaZZ* pcMegaZZ = it2.second;
            ASSERT(NULL != pcMegaZZ);
            pcMegaZZ->Release();
            delete pcMegaZZ;
        }
        pcMap->clear();
        delete pcMap;
    }
    m_cTradePairMap.clear();
}
VOID CAutomationMgr::ChangeFractalIndex(UINT uIndex)
{
    m_uFractalIndex = uIndex;
    ClearTradeFractalMap();
}
