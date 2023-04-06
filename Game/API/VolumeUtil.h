#if !defined(VOLUME_UTIL_H)
#define VOLUME_UTIL_H
#include "CryptoUtil.h"
#include <list>



namespace CVolumeUtil {
    DOUBLE* GenerateOBV(const DOUBLE* pdSrcPrice, const DOUBLE* pdSrcVolume, UINT uSrcValuesNum, INT nVisibleFirstIndex, UINT uVisibleValueNum, BOOLEAN bUseVWAPVolume);
    DOUBLE* GenerateOBV(UINT& uNumGenerated, const CTradeCandleMap& cCandleMap, UINT uCandleDurationSec, BOOLEAN bUseVWAPVolume);
};
#endif // #if !defined(VOLUME_UTIL_H)
