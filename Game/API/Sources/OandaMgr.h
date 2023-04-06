#if !defined(OANDA_MGR_H)
#define OANDA_MGR_H
#include "CryptoUtil.h"
#include "EventHandler.h"
#include "Rythm.h"
#include <list>

#if defined(DEBUG)
#if !defined(USE_DEMO_ACCOUNT)
#define USE_DEMO_ACCOUNT
#endif // #if !defined(USE_DEMO_ACCOUNT)
#endif // #if defined(DEBUG)
#if defined(USE_DEMO_ACCOUNT)
#define OANDA_MAIN_URL          "https://api-fxpractice.oanda.com/"
#else // #if defined(USE_DEMO_ACCOUNT)
#define OANDA_MAIN_URL          "https://api-fxtrade.oanda.com/"
#endif // #if defined(USE_DEMO_ACCOUNT)

#define OANDA_CANDLE_URL        OANDA_MAIN_URL "v3/instruments/%s/candles"

class COandaMgr : public CEventHandler {
public:
    enum EACCOUNT_INDEX {
        EACCOUNT_LONG=0,
        EACCOUNT_SHORT,
    };
    enum EORDER_STATE {
        EORDER_PENDING=0,
        EORDER_FILLED,
        EORDER_TRIGGERED,
        EORDER_CANCELLED,
    };
    enum EORDER_TYPE {
        EORDER_OANDA_MARKET=0,
        EORDER_OANDA_STOP_LOSS,
        EORDER_OANDA_TRAILING_STOP_LOSS,
    };
    enum ETRADE_STATE {
        ETRADE_OPEN=0,
        ETRADE_CLOSED,
        ETRADE_CLOSED_WHEN_TRADABLE,
    };
    struct SOandaPair {
        const CHAR* szTradePair;
        UINT        uIndex;
        DOUBLE      dPipMultiplier;     // multiplier to get pip.
        UINT        uDisplayPrecision;  // 5 = 0.00000
        UINT        uMinTradeSize;      // min trade unit size
        UINT        uMaxOrderUnits;     // max order units
        DOUBLE      dMinTrailingStop;   // min trailing stop distance
        DOUBLE      dPrice;             // most recent price
        FLOAT       fAskPrice;          // ask price (LONG price)
        FLOAT       fBidPrice;          // bid price (SHORT price)
    };
    struct SOandaOrder {
        UINT            uOrderID;           // order ID
        UINT            uCreateTimestamp;   // timestamp of creation (seconds)
        EORDER_STATE    eState;             // current state of the order
        EORDER_TYPE     eType;
        UINT            uTradePairIndex;    // which trading pair (only used by market order)
        DOUBLE          dUnits;             // market order only
        DOUBLE          dPrice;             // market order only
        UINT            uTradeID;           // trade id of the trade (for stop loss, trailing stop loss)
    };
    struct SOandaTrade {
        UINT            uTradeID;
        UINT            uTradePairIndex;
        DOUBLE          dPrice;
        UINT            uOpenTime;
        ETRADE_STATE    eState;
        DOUBLE          dInitialUnits;
        DOUBLE          dCurrentUnits;
        DOUBLE          dRealizedPL;
        DOUBLE          dUnRealizedPL;
    };
    typedef std::list<SOandaOrder> COrderList;
    typedef std::list<SOandaTrade> CTradeList;
    struct SOandaAccount {
        CHAR*       szAccountID;
        BOOLEAN     bHegdingEnabled;
        CHAR*       szCurrency;
        DOUBLE      dBalance;
        DOUBLE      dMarginUsed;
        DOUBLE      dMarginAvailable;
        COrderList  cPendingOrderList;    // a list of pending orders
        CTradeList  cOpenTradeList;       // a list of open trades
    };

    static VOID InitInstance(VOID);
    static VOID ReleaseInstance(VOID);
    static COandaMgr& GetInstance(VOID);
    
    static UINT GetCandleTypeNum(VOID);
    static const SCandleIntervalDisplayData* GetAllCandleDisplayData(VOID);
    static UINT GetCandleDurationMins(UINT uIndex);
    static const CHAR* GetCandleTypeDisplayName(UINT uIndex);
    static UINT GetCandleIndexFromDisplayName(const CHAR* szName);
    
    VOID Update(FLOAT fLapsed);
    
    BOOLEAN IsSignedIn(VOID) const;
    const CHAR* GetAccountID(UINT uAccountIndex) const;
    VOID TrySignIn(VOID);
    
    BOOLEAN AreTradePairsQueried(VOID) const;
    VOID QueryTradePairs(VOID);
    UINT GetTradePairNum(VOID);
    const SOandaPair* GetTradePair(UINT uIndex);
    const SOandaPair* GetTradePairByName(const CHAR* szName);
    
    BOOLEAN HasOHLCData(UINT uTradePairIndex, UINT uCandleIndex);
    VOID QueryOHLC(UINT uTradePairIndex, UINT uCandleIndex, BOOLEAN bSkipTimeCheck = FALSE);
    const CTradeCandleMap* GetCandleMap(UINT uTradePairIndex, UINT uCandleIndex);
    
    BOOLEAN IsListeningToRythm(UINT uTradeIndex);
    VOID ListenToRythm(UINT uTradeIndex);
    VOID StopListeningToRythm(UINT uTradeIndex);
    SRythmListener* GetRythmListener(UINT uTradeIndex);
    
    // accounts related
    VOID QueryAccountDetails(UINT uAccountIndex);
    UINT GetAccountNum(VOID) const;
    SOandaAccount* GetAccount(UINT uAccountIndex);
    
    // orders related
    VOID QueryPendingOrders(UINT uAccountIndex);
    UINT CreateMarketOrder(UINT uAccountIndex, UINT uTradePairIndex, INT nUnits);
//    VOID CancelOrder(UINT uAccountIndex, UINT uOrderID);
    
    // trade related
    const CTradeList* GetOpenTrades(UINT uAccountIndex) const;
    const SOandaTrade* GetTrade(UINT uAccountIndex, UINT uTradeID) const;
    VOID QueryOpenTrades(UINT uAccountIndex);
    UINT CloseTrade(UINT uAccountIndex, UINT uTradeID, UINT uUnits = 0);
    
    // spread related
    VOID QueryLatestSpreads(const UINT* puTradePairIndex, UINT uTradePairNum);
    
    // display related
    VOID GetPriceDisplayString(CHAR* szBuffer, UINT uBufferLen, UINT uTradePairIndex, DOUBLE dPrice);
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
private:
    SOandaAccount*              m_psAccounts;
    UINT                        m_uAccountNum; // number of accounts
    SOandaPair*                 m_psTradePairs;
    UINT                        m_uTradePairNum;
    
    CCandlePairDataMap          m_cCandleMap;
    CRythmListeners             m_cRythmListeners;
    
    COandaMgr();
    virtual ~COandaMgr();
    
    VOID Release(VOID);
    VOID ReleaseAccounts(VOID);
    VOID ClearCandleMap(VOID);
    
    VOID UpdateTradePairPrice(UINT uTradePairIndex, DOUBLE dPrice);
    
    VOID OnCredentialInput(VOID);
    VOID OnGetAccountReply(const CEvent& cEvent);
    VOID OnGetAccountDetailsReply(const CEvent& cEvent);
    VOID OnGetPendingOrdersReply(const CEvent& cEvent);
    VOID OnCreateMarketOrderReply(const CEvent& cEvent);
//    VOID OnCancelOrderReply(const CEvent& cEvent);
    VOID OnGetOpenTradesReply(const CEvent& cEvent);
    VOID OnCloseTradeReply(const CEvent& cEvent);
    VOID OnGetTradePairsReply(const CEvent& cEvent);
    VOID OnGetOHLCReply(const CEvent& cEvent);
    VOID OnGetSpreadsReply(const CEvent& cEvent);
    
    VOID UpdateRythms(FLOAT fLapsed);
    VOID FetchRythm(SRythmListener& sListener);
    VOID OnRythmFetched(CEvent& cEvent);
};

#endif // #if !defined(OANDA_MGR_H)
