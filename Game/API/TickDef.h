#if !defined(SPIKES_DEF_H)
#define SPIKES_DEF_H
#include <list>

struct STick {
    DOUBLE  dCLosePrice;        // closing price
    DOUBLE  dAvgPrice;          // average price in this tick
    DOUBLE  dActualVolume;      // actual volume of this tick
    DOUBLE  dTotalVolume;       // total volume for candle
    DOUBLE  dTotalQuoteVolume;  // total quote volume for candle
    
    DOUBLE  dOBV;               // OBV
    DOUBLE  dOBVDiff;           // OBV Diff
    UINT    uOpenSec;           // time when tick opened
    UINT    uCloseSec;          // time of latest update
};
typedef std::list<STick> CTickList;

enum EOBV_SIG {
    EOBV_SIG_NEUTRAL=0,
    EOBV_SIG_UP,
    EOBV_SIG_DOWN,
    EOBV_SIG_NUM
};

enum EOBV_TYPE {
    EOBV_TYPE_4HR=0,
    EOBV_TYPE_2HR,
    EOBV_TYPE_1HR,
    EOBV_TYPE_30_MINS,
    EOBV_TYPE_NUM
};

enum EMACD_SIG {
    EMACD_SIG_NEUTRAL=0,
    EMACD_SIG_BUY,
    EMACD_SIG_SELL,
    EMACD_SIG_NUM
};

enum ESECOND_TICK_SIG {
    ESECOND_TICK_NEUTRAL=0,
    ESECOND_TICK_PLUNGING_SELL,
    ESECOND_TICK_CRAZY_BUY,
};

struct SProcessedTick {
    UINT                uNum;           // number of entries
    UINT                uMax;           // maximum accepted number of entries
    DOUBLE*             pdMACD;         // MACD
    DOUBLE*             pdSignal;       // MACD signal
    DOUBLE*             pdWorkBuffer1;  // Work Buffer;
    DOUBLE*             pdWorkBuffer2;  // Work Buffer;
    DOUBLE*             pdWorkBuffer3;  // Work Buffer;
    
    EOBV_SIG            aeUpperSignalPrice[EOBV_TYPE_NUM];
    UINT                aauUpperIndexPrice[EOBV_TYPE_NUM][2];
    
    EOBV_SIG            aeLowerSignalPrice[EOBV_TYPE_NUM];
    UINT                aauLowerIndexPrice[EOBV_TYPE_NUM][2];
    
    EOBV_SIG            aeUpperSignalOBV[EOBV_TYPE_NUM];
    UINT                aauUpperIndexOBV[EOBV_TYPE_NUM][2];
    
    EOBV_SIG            aeLowerSignalOBV[EOBV_TYPE_NUM];
    UINT                aauLowerIndexOBV[EOBV_TYPE_NUM][2];
    
    EMACD_SIG           eMACD;
    ESECOND_TICK_SIG    eSecondTick;
};

struct STickListener {
    UINT            uTradeIndex;
    UINT            uRefCnt;
    FLOAT           fTimeToNextFetch;
    BOOLEAN         bFetching;
    CTickList       cMinuteList;    // the minute tick list
    CTickList       cSecondList;    // the second list
    SProcessedTick  sProcessed;     // processed tick
};
typedef std::list<STickListener> CTickListener;


#define TICKS_PER_MINUTE        (60)
#define TICK_LISTENER_DURATION  (60.0f/TICKS_PER_MINUTE)

#define TICK_HISTORY_MINS       (240)


#define TICKER_WIDTH_FOR_LABEL (60.0f)
#define TICKER_HEIGHT_FOR_LABEL (24.0f)

namespace CTickDef {
    BOOLEAN GenerateMACDSignal(STickListener& sTickListener);
};
#endif // #if !defined(SPIKES_DEF_H)
