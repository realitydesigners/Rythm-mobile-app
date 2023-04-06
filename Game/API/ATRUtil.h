#if !defined(ATR_UTIL_H)
#define ATR_UTIL_H
#include "ATRDefs.h"


namespace ATR {
    VOID SetupAtr(ATR::SATR_Param& sParam, UINT uPeriod, UINT uDurationSeconds);
    VOID ReleaseAtr(ATR::SATR_Param& sParam);
    VOID UpdateAtr(ATR::SATR_Param& sParam, DOUBLE dPrice, UINT uTimeSec);
};
#endif // #if !defined(ATR_UTIL_H)
