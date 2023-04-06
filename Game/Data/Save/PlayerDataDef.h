#if !defined(PLAYER_DATA_DEF_H)
#define PLAYER_DATA_DEF_H

enum EMA_TYPE {
    EMA_NONE=0,
    EMA_SMA,
    EMA_EMA,
};
enum EMA_VALUE_SOURCE {
    EMA_VALUE_OPEN=0,
    EMA_VALUE_AVG,
    EMA_VALUE_CLOSE,
    EMA_VALUE_SOURCE_NUM
};
struct SMovingAverageData {
    BYTE    byType;
    BYTE    bySource;
    USHORT  ushInterval;
    UINT    uPad;
};
struct SBollingerBandsParam {
    UINT    uInterval;
    FLOAT   fDeviation;
};
struct SMACDParam {
    SMovingAverageData sMA1;
    SMovingAverageData sMA2;
    SMovingAverageData sSignal;
};
enum EATTACH_CHART_TYPE {
    EATTACH_CHART_NONE=0,
    EATTACH_CHART_MACD,
    EATTACH_CHART_OBV,
    EATTACH_CHART_CMO,
    EATTACH_CHART_RSI,
};

enum ERythmInterval {
    ERythmInterval_Per_2_Seconds=0,
    ERythmInterval_Per_5_Seconds,
    ERythmInterval_Per_10_Seconds,
    ERythmInterval_Per_15_Seconds,
    ERythmInterval_Per_30_Seconds,
    ERythmInterval_Num
};

struct SRythmGraphInfo {
    BYTE byGraphType;   // 0 = channel, 1 = straight, 2 = inner, 3 = p&f, 4 = multi
    BYTE byDepth;       // (1-8) locked to fixed depth, 0 = follow main ui depth
};
struct SPlayerData {
    UINT                    uVersion;
    SMovingAverageData      asMA[3];                // moving average data
    UINT                    uCMOValue;              // Chande Momentum Oscillator
    SBollingerBandsParam    sBollingerParams;       // bollinger bands
    SMACDParam              sMACD;                  // MACD (default to 12,26)
    
    BYTE                    byDisplayMA;
    BYTE                    byHideCandles;
    BYTE                    byDisplayBB;
    BYTE                    byRSI_Interval;         // RSI interval (default 14)
    
    BYTE                    byOBV_UsePriceVolume;
    BYTE                    abyAttachedChartType[3];
    
    SMovingAverageData      asOBVMA[2];              // moving average data for OBV (0 = main data, 1 = difference)
    
    CHAR                    szOandaCredential[128];
    BYTE                    byOandaMask;
    BYTE                    bySoundEnabled;           // whether we going to have alarm when something is triggering
    BYTE                    byRythmQueryInterval;     // time interval for querying rythms
    BYTE                    byTimelineEnabled;        // timeline enabled or not
    BYTE                    byPad[4];
    
    USHORT                  aushChannelRatio[9];
    USHORT                  aushPad[3];
    UINT                    auSemaColor[4];
    USHORT                  aushSema[4];              // new sema data (for decimal point)
    UINT                    auBoxInBoxColor[2];
    UINT                    uPad[10];
};

#endif // #if !defined(PLAYER_DATA_DEF_H)
