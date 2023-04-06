#if !defined(PLAYER_DATA_H)
#define PLAYER_DATA_H
#include "FontDefs.h"
#include "PlayerDataDef.h"

class CPlayerData
{
    friend class CSaveData;
public:
    static BOOLEAN                      IsDisplayCandles(VOID);
    static VOID                         SetDisplayCandles(BOOLEAN bDisplay);
    
    static BOOLEAN                      IsDisplayMovingAverage(VOID);
    static const SMovingAverageData&    GetMovingAverageData(UINT uIndex);
    static VOID                         SetDisplayMovingAverage(BOOLEAN bDisplay);
    static VOID                         SetMovingAverageData(UINT uIndex, EMA_VALUE_SOURCE eSource, EMA_TYPE eType, UINT uInterval);
    
    static EATTACH_CHART_TYPE           GetAttachedChartType(UINT uIndex);
    static VOID                         SetAttachedChartTypes(UINT uIndex, EATTACH_CHART_TYPE eType);
    static UINT                         GetCMOInterval(VOID);
    static VOID                         SetCMOInterval(UINT uInterval);
    
    static BOOLEAN                      IsDisplayBollingerBands(VOID);
    static VOID                         SetDisplayBollingerBands(BOOLEAN bDisplay);
    static const SBollingerBandsParam&  GetBollingerBands(VOID);
    static VOID                         SetBollingerBands(UINT uInterval, FLOAT fDeviation);
    
    static const SMACDParam&            GetMACDParam(VOID);
    static VOID                         SetMACDParam(const SMACDParam& sParam);
    
    static UINT                         GetRsiInterval(VOID);
    static VOID                         SetRsiInterval(UINT uInterval);
    
    static BOOLEAN                      IsOBVUsingPriceMultiplier(VOID);
    static VOID                         SetOBVUsingPriceMultiplier(BOOLEAN bUse);
    static const SMovingAverageData&    GetOBV_MA(UINT uIndex);
    static VOID                         SetOBV_MA(UINT uIndex, EMA_VALUE_SOURCE eSource, EMA_TYPE eType, UINT uInterval);
    
    static BOOLEAN                      IsSoundEnabled(VOID);
    static VOID                         SetSoundEnabled(BOOLEAN bEnabled);
    
    static VOID                         SetOandaCredential(const CHAR* szCredential);
    static BOOLEAN                      GetOandaCredential(CHAR* szBuffer, UINT uBufferLen);
    
    static ERythmInterval               GetRythmFetchInterval(VOID);
    static FLOAT                        GetRythmFetchIntervalSecs(VOID);
    static VOID                         SetRythmFetchInterval(ERythmInterval eInterval);
    
    static const USHORT*                GetChannelRatios(VOID);
    static VOID                         SetChannelRatios(const USHORT* pushRatios);
    
    static FLOAT                        GetSemaPercent(UINT uIndex);
    static UINT                         GetSemaColor(UINT uIndex);
    static VOID                         SetSemaPercent(UINT uIndex, FLOAT fPercent);
    static VOID                         SetSemaColor(UINT uIndex, UINT uColor);
    
    static BYTE                         IsTimelineEnabled(VOID);
    static VOID                         SetTimelineEnabled(BOOLEAN bEnabled);
    
    static UINT                         GetBiBColor(UINT uIndex);
    static VOID                         SetBiBColor(UINT uIndex, UINT uColor);
private:
    static VOID             NewGame(VOID);
    static BOOLEAN          LoadData(const BYTE* pbyBuffer);
    
    static UINT             GetSaveDataLength(VOID);
    static BOOLEAN          SaveData(BYTE* pbyBuffer, UINT uLength);
};

#endif // #if !defined(PLAYER_DATA_H)
