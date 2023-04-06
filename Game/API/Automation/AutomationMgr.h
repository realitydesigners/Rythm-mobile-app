#if !defined(AUTOMATION_MGR_H)
#define AUTOMATION_MGR_H
#include "EventHandler.h"
#include "AutomationMgrDefs.h"
#include "OandaMgr.h"

// note, this automation manager only usable for Oanda
class CMegaZZ;
class CAutomationMgr : CEventHandler {
public:
    static BOOLEAN InitializeStatic(VOID);
    static VOID ReleaseStatic(VOID);
    static CAutomationMgr& GetInstance(VOID);
        
    VOID Start(VOID);
    VOID Stop(VOID);
    VOID OnTradePairQueried(UINT uTradePairIndex);
    
    BOOLEAN IsMarketClosedForWeekend(VOID) const    { return m_bMarketClosed; }
    BOOLEAN HasStarted(VOID) const                  { return m_bStarted; }
    BOOLEAN CanEnterNewTrades(VOID) const           { return m_bCanEnterNewTrades; }
    UINT    GetSecondsToOpen(VOID) const            { return m_uSecondsToOpenTime; }
    UINT    GetSecondsToStopEntry(VOID) const       { return m_uSecondsToStopNewEntryTime; }
    UINT    GetSecondsToCloseAllTrade(VOID) const   { return m_uSecondsToCloseAllTrades; }
    UINT    GetFractalIndex(VOID) const             { return m_uFractalIndex; }
    
    const CMegaZZ* FindMegaZZ(UINT uTradePairIndex, UINT uFractalIndex) const;
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID ChangeFractalIndex(UINT uIndex);
    VOID ExportBibLog(VOID);
private:
    enum EEVENT_ID {
        EEVENT_DELAYED_REFRESH=0,
    };
    typedef std::map<UINT, BOOLEAN> CTradeStatusMap;
    typedef std::pair<UINT, BOOLEAN> CTradeStatusPair;
    typedef std::map<UINT, CTradeStatusMap> CTradePairStatusMap;
    typedef std::pair<UINT, CTradeStatusMap> CTradePairStatusPair;
    
    CAutoRunningLogicList   m_cRunningLogicList;
    CTradePairFractalMap    m_cTradePairMap;
    CTradePairEntryMap      m_cShortEntries;
    CTradePairEntryMap      m_cLongEntries;
    CPendingEntryMap        m_cPendingMsgIDEntries;
    CClosingTradeMap        m_cClosingTradeMap;
    CTradePairStatusMap     m_cTradeStatusMap;
    BOOLEAN                 m_bMarketClosed;
    BOOLEAN                 m_bStarted;
    BOOLEAN                 m_bCanEnterNewTrades; // whether the time allows us to enter new trades
    UINT                    m_uSecondsToOpenTime;
    UINT                    m_uSecondsToStopNewEntryTime;
    UINT                    m_uSecondsToCloseAllTrades;
    BOOLEAN                 m_abRefreshAccounts[2];
    
    CBibLogMap              m_cBibLog;
    UINT                    m_uFractalIndex;
    
    CAutomationMgr();
    virtual ~CAutomationMgr();
    
    BOOLEAN Initialize(VOID);
    VOID Release(VOID);
    VOID Release(CTradePairEntryMap& cMap);
    
    VOID ProcessExistingEntry(const COandaMgr::SOandaPair& sPair, BOOLEAN bLong);
    
    CMegaZZ* FindMegaZZ(UINT uTradePairIndex, UINT uFractalIndex, BOOLEAN bCreateIfNotExist);
    
    VOID Process(UINT uTradePairIndex);
    
    BOOLEAN UpdateMegaZZ(const COandaMgr::SOandaPair& sPair);
    
    // close a trade,
    BOOLEAN CloseTrade(UINT uTradeID, BOOLEAN bWasLong, UINT uTradePairIndex, FLOAT fHighestPipGain);
    VOID IncrementStopLossCounter(SAutoRunningLogic& sLogic);
    
    // try open a trade
    VOID TryOpenTrade(const CMegaZZ& cZZ,
                      UINT uEntrySequenceID,
                      UINT uLogicIndex,
                      UINT uPreTradePatternIndex,
                      UINT uCatPatternIndex,
                      BOOLEAN bLong,
                      UINT uStopLossTPIndex,
                      FLOAT fStopLossSafetyPip,
                      UINT uTPIndex,                // which TP entry is this in the logic
                      UINT uLotNum);
    
    // reply handler
    VOID OnCreateMarketOrderReply(UINT uMsgID, UINT uAccountIndex, UINT uTradeID, FLOAT fPrice);
    VOID OnCloseTradeReply(UINT uMsgID, UINT uAccountIndex, UINT uTradeID, BOOLEAN bSuccess, FLOAT fPrice);
    
    BOOLEAN CloseAllAccountTrades(VOID); // returns TRUE if there are open trades
    VOID CloseAllOpenEntries(VOID);
    VOID CloseAll(BOOLEAN bLong);
    
    VOID DelayRefreshAccount(VOID);
    
    VOID UpdateTime(VOID);
    
    VOID OnHeartBeat(VOID);
    
    BOOLEAN CanOpenTrade(UINT uTradePairIndex, BOOLEAN bLong, UINT uLotSize);
    VOID OnOpenedTrade(UINT uTradePairIndex, BOOLEAN bLong, UINT uLotSize);
    VOID OnClosedTrade(UINT uTradePairIndex, BOOLEAN bLong, UINT uLotSize);
    
    VOID UpdateSafetyTP(CTradeEntryList& cList, UINT uEntrySequenceID, FLOAT fSafetyPip);
    
    VOID CLearBibLogs(VOID);
    VOID ClearTradeFractalMap(VOID);
    VOID ProcessBibLog(CMegaZZ& cZZ);
};
#endif // #if !defined(AUTOMATION_MGR_H)
