#if !defined(BINANCE_MGR_H)
#define BINANCE_MGR_H
#include "CryptoUtil.h"
#include "EventHandler.h"
#include "json-forwards.h"
#include "Rythm.h"
#include "TickDef.h"

#define MAX_BINANCE_CANDLE_NUM (500)
class CBinanceMgr : public CEventHandler {
public:
    static VOID InitInstance(VOID);
    static VOID ReleaseInstance(VOID);
    static CBinanceMgr& GetInstance(VOID);
    
    static UINT GetCandleTypeNum(VOID);
    static const SCandleIntervalDisplayData* GetAllCandleDisplayData(VOID);
    static UINT GetCandleDurationMins(UINT uIndex);
    static const CHAR* GetCandleTypeDisplayName(UINT uIndex);
    static UINT GetCandleDurationIndexFromDisplayDurationName(const CHAR* szName);

    BOOLEAN AreTradePairsQueried(VOID) const;
    BOOLEAN QueryTradePairs(VOID);
    
    UINT GetTradePairNum(VOID);
    const STradePair* GetTradePair(UINT uIndex);

    BOOLEAN HasOHLCData(UINT uTradePairIndex, UINT uCandleIndex);
    VOID QueryOHLC(UINT uTradePairIndex, UINT uCandleIndex, BOOLEAN bSkipTimeCheck = FALSE);
    const CTradeCandleMap* GetCandleMap(UINT uTradePairIndex, UINT uCandleIndex);
    
    BOOLEAN IsTickListening(UINT uTradeIndex);
    VOID ListenToTicks(UINT uTradeIndex);
    VOID StopListenToTicks(UINT uTradeIndex);
    const STickListener* GetTick(UINT uTradeIndex);
    
    BOOLEAN IsListeningToRythm(UINT uTradeIndex);
    VOID ListenToRythm(UINT uTradeIndex);
    VOID StopListeningToRythm(UINT uTradeIndex);
    const SRythmListener* GetRythmListener(UINT uTradeIndex);
    
    VOID Update(FLOAT fLapsed);
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
private:
    CBinanceMgr();
    virtual ~CBinanceMgr();
    
    VOID ClearTradePairs(VOID);
    VOID ClearCandleMap(VOID);
    
    VOID OnQueryTradePairs(const CEvent& cEvent);
    VOID OnQueryOHLC(CEvent& cEvent);
    VOID OnQueryOHLC(const Json::Value* pcParams, INT nErrCode, const CHAR* szResult);
    VOID ProcessOHLCHeartBeat(const CHAR* szTradeKey, const CTradeCandleMap& cCandleMap);
    
    VOID FetchTick(UINT uTradeIndex, BOOLEAN bFullQuery);
    VOID OnTickQueried(CEvent& cEvent);
    VOID OnTickQueried(const Json::Value& cParams, INT nErrCode, const CHAR* szResult);
    
    VOID UpdateRythms(FLOAT fLapsed);
    VOID FetchRythm(SRythmListener& sListener);
    VOID OnRythmFetched(CEvent& cEvent);
    
    static VOID GenerateSignals(STickListener& sTickListener);
private:
    typedef std::map<std::string, DOUBLE>   CTradeKeyPriceMap;
    typedef std::pair<std::string, DOUBLE>   CTradeKeyPricePair;
    
    STradePair*                 m_psTradePairs;
    UINT                        m_uTradePairNum;
    CCandlePairDataMap          m_cCandleMap;
    CTradeKeyPriceMap           m_cPriceMap;
    CTickListener               m_cTickListenerList;
    CRythmListeners             m_cRythmListeners;
};

#endif // #if !defined(BINANCE_MGR_H)
