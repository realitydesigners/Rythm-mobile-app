#if !defined(CRYPTO_UTIL_H)
#define CRYPTO_UTIL_H
#include <list>
#include <map>
#include <string>

#define NO_DATA_TIME_INTERVAL_MINS      (60)
#define NO_DATA_TIME_INTERVAL_SECS      (NO_DATA_TIME_INTERVAL_MINS * 60)
#define NO_DATA_TIME_INTERVAL_MILLISECS (NO_DATA_TIME_INTERVAL_SECS * 1000)

enum ESOURCE_MARKET {
    ESOURCE_MARKET_CRYPTO_COM=0,
    ESOURCE_MARKET_BINANCE,
    ESOURCE_MARKET_OANDA,
    ESOURCE_MARKET_NUM
};

enum ETRADE_TYPE {
    ETRADE_BUY=0,
    ETRADE_SELL
};
enum EORDER_TYPE {
    EORDER_MARKET=0,
    EORDER_LIMIT,
};

struct STradePair {
    const CHAR* szTradeKey;
    const CHAR* szBase;
    const CHAR* szQuote;
    UINT uPriceDecimals;
    UINT uQuantityDecimals;
};
struct STrade {
    std::string cPrice;
    std::string cVolume;
    DOUBLE      dTimeSecs;
    ETRADE_TYPE eTrade;
    EORDER_TYPE eOrder;
};

struct STradeCandle {
    UINT    uTimeSec;
    UINT    uTradeNum;  // number of trades
    DOUBLE  dOpen;      // opening price
    DOUBLE  dOpenTime;  // opening time
    DOUBLE  dClose;     // closing price
    DOUBLE  dCloseTime; // closing time
    DOUBLE  dHigh;      // highest price
    DOUBLE  dLow;       // lowest price
    DOUBLE  dVWAP;      // volume weighted average price
    DOUBLE  dVolume;    // total volume traded
};

struct SCandleIntervalDisplayData {
    UINT        uTimeMins;
    const CHAR* szDisplayName;
};

typedef std::map<UINT, STradePair>      CTradePairMap;
typedef std::pair<UINT, STradePair>     CTradePairPair;

typedef std::list<STrade>               CTradeList;
typedef std::map<UINT, STradeCandle>    CTradeCandleMap;
typedef std::list<STradeCandle>         CTradeCandleList;

struct STradePairData {
    std::string cKey;      // trade key
    CTradeList  cList;
};
typedef std::list<STradePairData> CTradePairDataList;


struct SCandleIntervalData {
    UINT                uInterval;
    UINT                uLastQueryLocalTimeSecs;    // time since last query
    std::string         m_cLastID;                  // ID to continue querying new data
    CTradeCandleMap     cCandleMap;                 // map of the candles
};
typedef std::map<UINT, SCandleIntervalData>         CCandleIntervalMap;

typedef std::map<std::string, CCandleIntervalMap>   CCandlePairDataMap;

// for websockets
typedef std::map<std::string,UINT>                  CSubscribedChannelMap;


struct SDisplayCandle {
    STradeCandle    sData;
    FLOAT           afBox[4];   // 4 point box
    FLOAT           afLine[3];  // 3 point line [y1, y2, x]
    UINT            uColor;     // color to render
};

namespace CCryptoUtil {

    VOID GeneratePriceDisplayString(ESOURCE_MARKET eMarket, UINT uTradePairIndex,DOUBLE dPrice, CHAR* szBuffer, UINT uBufferLen);
    VOID GenerateString(DOUBLE dValue, CHAR* szBuffer, UINT uBufferLen);

    VOID InsertTrade(CTradePairDataList& cList, const std::string& cTradeKey, const STrade& sTrade);
    
    SCandleIntervalData* FindCandleInterval(CCandlePairDataMap& cMap, const std::string& cKey, UINT uInterval, BOOLEAN bSkipInsert = FALSE);
    VOID InsertCandle(CTradeCandleMap& cMap, const STradeCandle& sCandle);
    VOID OnInsertCandleEnd(CTradeCandleMap& cMap, UINT uCandleDurationSec, UINT uLastCandleTimeSec);

    BOOLEAN IsChannelSubscribed(const CSubscribedChannelMap& cMap, const std::string& cChannel);
    VOID SetSubscribedStatus(CSubscribedChannelMap& cMap, const std::string& cChannel, BOOLEAN bSubscribed);

    UINT FindFirstCandleTimeSec(const CTradeCandleMap& cCandleMap);
    BOOLEAN FindHighLowClosing(DOUBLE& dHigh, DOUBLE dLow, const CTradeCandleMap& cCandleMap);

    const CHAR* GetTradePairName(ESOURCE_MARKET eSource, UINT uTradeIndex, BOOLEAN bShortForm = FALSE);
};


#endif // #if !defined(CRYPTO_UTIL_H)
