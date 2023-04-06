#if !defined(ATR_DEFS_H)
#define ATR_DEFS_H
#include <list>

namespace ATR {
    struct SATR_Tick {
        UINT uTimeSec;
        DOUBLE dHigh;
        DOUBLE dLow;
        DOUBLE dClose;
    };
    typedef std::list<SATR_Tick> CTickList;

    struct SATR_Param {
        UINT        uPeriod;
        UINT        uDurationSeconds; // duration of each tick in seconds
        DOUBLE      dPrevATR;
        DOUBLE      dATR;             // calculated ATR, 0 for unable to calculate yet
        CTickList   cTicks;
    };
};
#endif // #if !defined(ATR_DEFS_H)
