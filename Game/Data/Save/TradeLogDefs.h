#if !defined(TRADE_LOG_DEFS_H)
#define TRADE_LOG_DEFS_H
#include "MegaZZDef.h"
#include "AutoLogicDefs.h"
#include <list>
#include <map>

#define TRADE_LOG_MAX_LOG               (2000)

struct STradeLog {
    BYTE                byTradePairIndex;                                   // trade pair
    BYTE                byLong;                                             // long or short
    BYTE                abyPad[2];                                          // padding
    UINT                uTradeID;                                           // trade ID
    
    UINT                uOpenTimeSec;                                       // open time
    FLOAT               fOpenPrice;                                         // open price
    
    FLOAT               fSlippage;                                          // slippage
    UINT                uLogicIndex;                                        // trade order index
    
    UINT                uPreTradePatternIndex;                              // pre trade pattern
    UINT                uCategoryPatternIndex;                              // category pattern index
    UINT                uTPIndex;                                           // which TP index is this (from TP chart)
    
    UINT                uStopLossTPIndex;                              // stop loss pattern
    UINT                uCloseTimeSec;                                      // close time
    
    FLOAT               fCLosePrice;                                        // close price
    FLOAT               fHighestPipReached;                                 // highest pip reached
    FLOAT               fLowestPipReached;                                  // lowest pip reached

    MegaZZ::SChannel    asStart[MEGAZZ_MAX_DEPTH];                          // channel information
    MegaZZ::SChannel    asEnd[MEGAZZ_MAX_DEPTH];                            // channel information
};
typedef std::list<STradeLog>        CTradeLogs;
typedef std::map<UINT,STradeLog*>   CActiveTradeLogMap;
typedef std::pair<UINT,STradeLog*>  CActiveTradeLogPair;
#endif // #if !defined(TRADE_LOG_DEFS_H)
