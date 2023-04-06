#if !defined(CRYPTO_DOT_COM_MGR_H)
#define CRYPTO_DOT_COM_MGR_H
#include "CryptoUtil.h"
#include "EventHandler.h"
#include "json-forwards.h"

class CCryptoDotComMgr : public CEventHandler {
public:
    static VOID InitInstance(VOID);
    static VOID ReleaseInstance(VOID);
    static CCryptoDotComMgr& GetInstance(VOID);

    static UINT GetCandleTypeNum(VOID);
    static const SCandleIntervalDisplayData* GetAllCandleDisplayData(VOID);
    static UINT GetCandleDurationMins(UINT uIndex);
    static const CHAR* GetCandleTypeDisplayName(UINT uIndex);
    static UINT GetCandleIndexFromDisplayName(const CHAR* szName);
    
    BOOLEAN IsMarketSocketConnected(VOID);
    VOID    ConnectMarketSocket(VOID);
    
    BOOLEAN AreTradePairsQueried(VOID) const;
    BOOLEAN QueryTradePairs(VOID);
    
    UINT GetTradePairNum(VOID);
    const STradePair* GetTradePair(UINT uIndex);
    UINT GetTradePairIndex(const CHAR* szTradeKey);
    
    VOID OnPaused(VOID); // used for on app paused
    VOID OnResume(VOID); // used for on app resumed
    
    VOID QueryOHLC(UINT uTradePairIndex, UINT uCandleIndex);
    const CTradeCandleMap* GetCandleMap(UINT uTradePairIndex, UINT uCandleIndex);
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;

private:
    CCryptoDotComMgr();
    ~CCryptoDotComMgr();

    VOID ClearTradePairs(VOID);
    VOID ClearTradeList(VOID);
    VOID ClearCandleMap(VOID);
    VOID OnQueryTradePairs(const CEvent& cEvent);
    VOID OnQueryOHLC(const Json::Value& cData, UINT uCandleIndex, const CHAR* szKey);
    
private:
    CTradePairMap               m_cTradePairMap;
    CTradePairDataList          m_cTradedList;  // list of all queried trades
    std::string                 m_cLastID;      // id to continue query new trades
    
    CCandlePairDataMap          m_cCandleMap;
    CSubscribedChannelMap       m_cSubscribedChannels;  // map of channels that has been subscribed
    
// websocket
public:
    static VOID OnMarketSocketConnected(VOID);
    static VOID OnMarketSocketConnectFail(const CHAR* szReason);
    static VOID OnMarketSocketClosed(const CHAR* szReason);
    
    VOID OnReceiveMarketMessage(const CHAR* szMessage);

private:
    static BOOLEAN  IsMarketSocketConnectedInternal(VOID);
    static VOID     ConnectMarketSocketInternal(const CHAR* szURL);
    static BOOLEAN  SendMessageToMarketInternal(const CHAR* szMessage);
    static VOID     CloseMarketSocketInternal(VOID);
    
    static BOOLEAN SubscribeToChannel(const CHAR* szChannelName);
    static BOOLEAN SendMessageToMarket(const Json::Value& cParams);

    VOID ParseMarketSubscribeResult(UINT64 ullID, const Json::Value& cResult);
    static VOID ProcessCandleResult(UINT64 ullID, const Json::Value& cResult);
};

#endif // #if !defined(CRYPTO_DOT_COM_MGR_H)
