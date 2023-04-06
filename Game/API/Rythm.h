#if !defined(RYTHM_H)
#define RYTHM_H
#include <list>

struct SRythm {
    UINT64  ullTimeMilliSec;    // time of latest update
    DOUBLE  dCLosePrice;        // closing price
    DOUBLE  dHighPrice;         // closing price
    DOUBLE  dLowPrice;          // closing price
    DOUBLE  dAvgPrice;          // average price in this tick
    DOUBLE  dActualVolume;      // actual volume of this tick
    DOUBLE  dTotalVolume;       // total volume for candle
    DOUBLE  dTotalQuoteVolume;  // total quote volume for candle
    
    DOUBLE  dOBV;               // OBV
    DOUBLE  dOBVDiff;           // OBV Diff
};
typedef std::list<SRythm> CRythms;

struct SRythmListener {
    UINT            uTradePairIndex;
    UINT            uRefCnt;
    FLOAT           fTimeToNextFetch;
    DOUBLE          dCurrentPrice;
    BOOLEAN         bFetching;
    BOOLEAN         bPreFetching;
    BOOLEAN         bRetryingFromError;
    UINT            uLastCandleTimeSec;
    UINT            uFetchedToSec;
    CRythms         cList;
};
typedef std::list<SRythmListener> CRythmListeners;

#endif // #if !defined(RYTHM_H)
