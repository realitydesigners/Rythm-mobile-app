#if !defined(MA_UTIL_H)
#define MA_UTIL_H
#include "CryptoUtil.h"
#include "PlayerDataDef.h"
#include <list>

struct SMovingAverage {
    EMA_TYPE            eType;          // the type of MA
    EMA_VALUE_SOURCE    eSource;        // source data
    UINT                uIntervalNum;   // how many intervals
    DOUBLE*             pdPrice;        // array of price
    
    SMovingAverage() : eType(EMA_SMA), eSource(EMA_VALUE_CLOSE), uIntervalNum(0), pdPrice(NULL)
    {
        
    }
    SMovingAverage(EMA_TYPE _eType, EMA_VALUE_SOURCE _eSource, UINT _uIntervalNum) :
    eType(_eType), eSource(_eSource), uIntervalNum(_uIntervalNum), pdPrice(NULL)
    {
        
    }
};

struct SBollingerBands {
    SMovingAverage  sMA;
    DOUBLE*         pdDeviation;
};
struct SMACD {
    SMACDParam  sParams;
    DOUBLE*     pdMACD;
    DOUBLE*     pdSignal;
    DOUBLE*     pdMA1;
    DOUBLE*     pdMA2;
    UINT        uNum;
    SMACD() : pdMACD(NULL), pdSignal(NULL), pdMA1(NULL), pdMA2(NULL), uNum(0)
    {
    }
    SMACD(const SMACDParam& _sParams) : sParams(_sParams), pdMACD(NULL), pdSignal(NULL), pdMA1(NULL), pdMA2(NULL), uNum(0)
    {
        
    }
};
#define MCAD_COLOR_1    RGBA(122,   175,    255,    0xFF)
#define MCAD_COLOR_2    RGBA(0xFF,  0x1F,   0x1F,   0xFF)

#define MCAD_COLOR_RED  RGBA(0xFF,  0x0,    0x0,    0xFF)
#define MCAD_COLOR_BLUE RGBA(0x0,   0x0,    0xFF,   0xFF)

namespace CMAUtil {
    DOUBLE GetSourceValueFromCandle(const STradeCandle& sTradeCandle, EMA_VALUE_SOURCE eSource);

    BOOLEAN GenerateMA(SMovingAverage& sMA, const DOUBLE* pdSrcValues, UINT uSrcValuesNum, UINT uVisibleFirstIndex, UINT uVisibleValueNum);
    BOOLEAN GenerateMA(SMovingAverage& sMA, const CTradeCandleMap& cCandleMap, UINT uFirstCandleTimeSec, UINT uCandleDurationSec, UINT uVisibleCandleNum, UINT uVisibleCandleStartSec);

    BOOLEAN GenerateMACD(SMACD& sMACD, const DOUBLE* pdSrcValues, UINT uSrcValuesNum, UINT uVisibleFirstIndex, UINT uVisibleValueNum);
    BOOLEAN GenerateMACD(SMACD& sMACD, const CTradeCandleMap& cCandleMap, UINT uFirstCandleTimeSec, UINT uCandleDurationSec, UINT uVisibleCandleNum, UINT uVisibleCandleStartSec);
    DOUBLE* GenerateRSI(UINT uRsiInterval, const CTradeCandleMap& cCandleMap, UINT uCandleDurationSec, UINT uVisibleCandleNum, UINT uVisibleCandleStartSec);

    BOOLEAN GenerateBollingerBands(SBollingerBands& sBand, const DOUBLE* pdSrcValues, UINT uSrcValuesNum, UINT uVisibleFirstIndex, UINT uVisibleValueNum);
    BOOLEAN GenerateBollingerBands(SBollingerBands& sBand, const CTradeCandleMap& cCandleMap, UINT uFirstCandleTimeSec, UINT uCandleDurationSec, UINT uVisibleCandleNum, UINT uVisibleCandleStartSec);
};
#endif // #if !defined(CRYPTO_UTIL_H)
