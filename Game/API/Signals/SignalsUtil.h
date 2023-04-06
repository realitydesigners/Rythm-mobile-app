#if !defined(SIGNALS_UTIL_H)
#define SIGNALS_UTIL_H

#include "CryptoUtil.h"
#include "SignalsDef.h"

namespace CSignalsUtil {
    VOID GenerateSignals(CSignalList& cList, DOUBLE dAmountAllocated,
                         const CTradeCandleMap& cCandles, UINT uCandleStartTimeSec, UINT uCandleEndTimeSec, UINT uCandleDurationSec);

    VOID GenerateFOMO(CSignalList& cList, DOUBLE dAmountAllocated,
                      const DOUBLE* pdPrices, const DOUBLE* pdVolumes, UINT uPriceNum);
    VOID GenerateSpecial(CSignalList& cList, DOUBLE dAmountAllocated,
                         const DOUBLE* pdPrices, const DOUBLE* pdVolumes, UINT uPriceNum,
                         UINT uCandleStartTimeSec, UINT uCandleDurationSec,
                         const CTradeCandleMap& cCandles);
};
#endif // #if !defined(SIGNALS_UTIL_H)
