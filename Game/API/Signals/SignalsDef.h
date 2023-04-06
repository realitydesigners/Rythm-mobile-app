#if !defined(SIGNALS_DEF_H)
#define SIGNALS_DEF_H
#include <list>

enum ESIG_TYPE {
    ESIG_BUY=0,         // buy signal
    ESIG_TAKE_PROFIT,   // take profit signal
    ESIG_END_PROFIT,    // end profit
    ESIG_STOP_LOSS,     // stop loss signal
};

struct SSignal {
    UINT        uCandleIndex;   // exact candle index
    ESIG_TYPE   eType;
    DOUBLE      dPrice;         // the price to perform action
    DOUBLE      dVolume;        // volume bought/sold
    DOUBLE      dAmount;        // amount used/earned
};
typedef std::list<SSignal> CSignalList;

#endif // #if !defined(SIGNALS_DEF_H)
