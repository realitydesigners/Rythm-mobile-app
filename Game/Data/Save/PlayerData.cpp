#include "stdafx.h"
#include "PlayerData.h"
#include "PlayerDataDef.h"
#include "DataHasher.h"
#include "SaveData.h"
#include "System.h"
#include "ZZUtil.h"
#include <inttypes.h>
#include <string.h>
#include <map>
#include <math.h>

#define PLAYER_DATA_VERSION_1   (1)
#define PLAYER_DATA_VERSION_2   (2)
#define PLAYER_DATA_VERSION_3   (3)
#define PLAYER_DATA_VERSION     (4)

static SPlayerData PLAYER_DATA = { 0 };
static CDataHasher HASHCHECK((const BYTE*)&PLAYER_DATA, sizeof(SPlayerData));

VOID CPlayerData::NewGame(VOID)
{
    ASSERT(0 == (sizeof(SPlayerData)%8));
    memset(&PLAYER_DATA, 0, sizeof(SPlayerData));
    PLAYER_DATA.uVersion = PLAYER_DATA_VERSION;
    
    PLAYER_DATA.asMA[0].bySource = EMA_VALUE_CLOSE;
    PLAYER_DATA.asMA[0].byType = EMA_EMA;
    PLAYER_DATA.asMA[0].ushInterval = 20;
    
    PLAYER_DATA.asMA[1].bySource = EMA_VALUE_CLOSE;
    PLAYER_DATA.asMA[1].byType = EMA_NONE;
    PLAYER_DATA.asMA[1].ushInterval = 10;

    PLAYER_DATA.asMA[2].bySource = EMA_VALUE_CLOSE;
    PLAYER_DATA.asMA[2].byType = EMA_EMA;
    PLAYER_DATA.asMA[2].ushInterval = 50;

    PLAYER_DATA.uCMOValue = 20;
    
    PLAYER_DATA.sBollingerParams.uInterval = 15;
    PLAYER_DATA.sBollingerParams.fDeviation = 2.0f;
    
    PLAYER_DATA.sMACD.sMA1.bySource = EMA_VALUE_CLOSE;
    PLAYER_DATA.sMACD.sMA1.byType = EMA_EMA;
    PLAYER_DATA.sMACD.sMA1.ushInterval = 12;
    
    PLAYER_DATA.sMACD.sMA2.bySource = EMA_VALUE_CLOSE;
    PLAYER_DATA.sMACD.sMA2.byType = EMA_EMA;
    PLAYER_DATA.sMACD.sMA2.ushInterval = 26;

    PLAYER_DATA.sMACD.sSignal.bySource = EMA_VALUE_CLOSE;
    PLAYER_DATA.sMACD.sSignal.byType = EMA_EMA;
    PLAYER_DATA.sMACD.sSignal.ushInterval = 9;
    
    PLAYER_DATA.byRSI_Interval = 20;
    
    PLAYER_DATA.byOBV_UsePriceVolume = 0;
    PLAYER_DATA.asOBVMA[0].bySource = EMA_VALUE_CLOSE;
    PLAYER_DATA.asOBVMA[0].byType = EMA_NONE;
    PLAYER_DATA.asOBVMA[0].ushInterval = 12;
    PLAYER_DATA.asOBVMA[1].bySource = EMA_VALUE_CLOSE;
    PLAYER_DATA.asOBVMA[1].byType = EMA_NONE;
    PLAYER_DATA.asOBVMA[1].ushInterval = 12;
    
    PLAYER_DATA.aushChannelRatio[0] = 144;
    PLAYER_DATA.aushChannelRatio[1] = 89;
    PLAYER_DATA.aushChannelRatio[2] = 55;
    PLAYER_DATA.aushChannelRatio[3] = 34;
    PLAYER_DATA.aushChannelRatio[4] = 21;
    PLAYER_DATA.aushChannelRatio[5] = 13;
    PLAYER_DATA.aushChannelRatio[6] = 8;
    PLAYER_DATA.aushChannelRatio[7] = 5;
    PLAYER_DATA.aushChannelRatio[8] = 3;
    
    PLAYER_DATA.auSemaColor[0] = RGBA(255, 255, 255, 0xFF);
    PLAYER_DATA.auSemaColor[1] = RGBA(255,   0,   0, 0xFF);
    PLAYER_DATA.auSemaColor[2] = RGBA(255, 255,   0, 0xFF);
    PLAYER_DATA.auSemaColor[3] = RGBA(0,   255, 255, 0xFF);
    PLAYER_DATA.aushSema[0] = 236;
    PLAYER_DATA.aushSema[1] = 236;
    PLAYER_DATA.aushSema[2] = 382;
    PLAYER_DATA.aushSema[3] = 618;
    
    PLAYER_DATA.auBoxInBoxColor[0] = RGBA(0x7F, 0xFF, 0x7F, 0xFF);
    PLAYER_DATA.auBoxInBoxColor[1] = RGBA(0xFF, 0x7F, 0x7F, 0xFF);
    HASHCHECK.Hash();
}

BOOLEAN CPlayerData::LoadData(const BYTE* pbyBuffer)
{
    ASSERT(0 == (((UINT64)pbyBuffer) % 8));
    memcpy(&PLAYER_DATA, pbyBuffer, sizeof(SPlayerData));
    if (PLAYER_DATA_VERSION != PLAYER_DATA.uVersion) {
        ASSERT(FALSE);
        return FALSE;
    }
    // perform error checking
    for (UINT uIndex = 0; 9 > uIndex; ++uIndex) {
        if (0 == PLAYER_DATA.aushChannelRatio[uIndex]) {
            PLAYER_DATA.aushChannelRatio[uIndex] = 3;
        }
    }
    HASHCHECK.Hash();    
    ZZUtil::LoadZZInfo();
    return TRUE;
}

UINT CPlayerData::GetSaveDataLength(VOID)
{
    return sizeof(SPlayerData);
}

BOOLEAN CPlayerData::SaveData(BYTE* pbyBuffer, UINT uLength)
{
    if (!HASHCHECK.IsMatchHash(TRUE)) {
        ASSERT(FALSE);
        return FALSE;
    }
    if (GetSaveDataLength() != uLength) {
        ASSERT(FALSE);
        return FALSE;
    }
    memcpy(pbyBuffer, &PLAYER_DATA, sizeof(SPlayerData));
    return TRUE;
}

BOOLEAN CPlayerData::IsDisplayMovingAverage(VOID)
{
    return PLAYER_DATA.byDisplayMA;
}
const SMovingAverageData& CPlayerData::GetMovingAverageData(UINT uIndex)
{
    return PLAYER_DATA.asMA[uIndex%3];
}

UINT CPlayerData::GetCMOInterval(VOID)
{
    return PLAYER_DATA.uCMOValue;
}

VOID CPlayerData::SetDisplayMovingAverage(BOOLEAN bDisplay)
{
    if (!HASHCHECK.IsMatchHash(TRUE)) {
        ASSERT(FALSE);
        return;
    }
    PLAYER_DATA.byDisplayMA = bDisplay;
    HASHCHECK.Hash();
}
VOID CPlayerData::SetMovingAverageData(UINT uIndex, EMA_VALUE_SOURCE eSource, EMA_TYPE eType, UINT uInterval)
{
    if (!HASHCHECK.IsMatchHash(TRUE)) {
        ASSERT(FALSE);
        return;
    }
    SMovingAverageData& sMA = PLAYER_DATA.asMA[uIndex%3];
    sMA.bySource = eSource;
    sMA.byType = eType;
    sMA.ushInterval = uInterval;
    HASHCHECK.Hash();
}

VOID CPlayerData::SetCMOInterval(UINT uInterval)
{
    if (!HASHCHECK.IsMatchHash(TRUE)) {
        ASSERT(FALSE);
        return;
    }
    PLAYER_DATA.uCMOValue = uInterval;
    HASHCHECK.Hash();
}

BOOLEAN CPlayerData::IsDisplayCandles(VOID)
{
    return !PLAYER_DATA.byHideCandles;
}

VOID CPlayerData::SetDisplayCandles(BOOLEAN bDisplay)
{
    if (!HASHCHECK.IsMatchHash(TRUE)) {
        ASSERT(FALSE);
        return;
    }
    PLAYER_DATA.byHideCandles = !bDisplay;
    HASHCHECK.Hash();
}

BOOLEAN CPlayerData::IsDisplayBollingerBands(VOID)
{
    return PLAYER_DATA.byDisplayBB;
}
VOID CPlayerData::SetDisplayBollingerBands(BOOLEAN bDisplay)
{
    if (!HASHCHECK.IsMatchHash(TRUE)) {
        ASSERT(FALSE);
        return;
    }
    PLAYER_DATA.byDisplayBB = bDisplay;
    HASHCHECK.Hash();
}

const SBollingerBandsParam& CPlayerData::GetBollingerBands(VOID)
{
    return PLAYER_DATA.sBollingerParams;
}
VOID CPlayerData::SetBollingerBands(UINT uInterval, FLOAT fDeviation)
{
    if (!HASHCHECK.IsMatchHash(TRUE)) {
        ASSERT(FALSE);
        return;
    }
    PLAYER_DATA.sBollingerParams.uInterval = uInterval;
    PLAYER_DATA.sBollingerParams.fDeviation = fDeviation;
    HASHCHECK.Hash();
}

const SMACDParam& CPlayerData::GetMACDParam(VOID)
{
    return PLAYER_DATA.sMACD;
}
VOID CPlayerData::SetMACDParam(const SMACDParam& sParam)
{
    if (!HASHCHECK.IsMatchHash(TRUE)) {
        ASSERT(FALSE);
        return;
    }
    PLAYER_DATA.sMACD = sParam;
    HASHCHECK.Hash();
}

UINT CPlayerData::GetRsiInterval(VOID)
{
    return PLAYER_DATA.byRSI_Interval;
}

VOID CPlayerData::SetRsiInterval(UINT uInterval)
{
    if (!HASHCHECK.IsMatchHash(TRUE)) {
        ASSERT(FALSE);
        return;
    }
    PLAYER_DATA.byRSI_Interval = uInterval;
    HASHCHECK.Hash();
}

BOOLEAN CPlayerData::IsOBVUsingPriceMultiplier(VOID)
{
    return PLAYER_DATA.byOBV_UsePriceVolume;
}

VOID CPlayerData::SetOBVUsingPriceMultiplier(BOOLEAN bUse)
{
    if (!HASHCHECK.IsMatchHash(TRUE)) {
        ASSERT(FALSE);
        return;
    }
    PLAYER_DATA.byOBV_UsePriceVolume = bUse;
    HASHCHECK.Hash();
}
const SMovingAverageData& CPlayerData::GetOBV_MA(UINT uIndex)
{
    return PLAYER_DATA.asOBVMA[uIndex%2];
}

VOID CPlayerData::SetOBV_MA(UINT uIndex, EMA_VALUE_SOURCE eSource, EMA_TYPE eType, UINT uInterval)
{
    if (!HASHCHECK.IsMatchHash(TRUE)) {
        ASSERT(FALSE);
        return;
    }
    SMovingAverageData& sMA = PLAYER_DATA.asOBVMA[uIndex%2];
    sMA.bySource = eSource;
    sMA.byType = eType;
    sMA.ushInterval = uInterval;
    HASHCHECK.Hash();
}

EATTACH_CHART_TYPE CPlayerData::GetAttachedChartType(UINT uIndex)
{
    return (EATTACH_CHART_TYPE)PLAYER_DATA.abyAttachedChartType[uIndex % 3];
}
VOID CPlayerData::SetAttachedChartTypes(UINT uIndex, EATTACH_CHART_TYPE eType)
{
    if (!HASHCHECK.IsMatchHash(TRUE)) {
        ASSERT(FALSE);
        return;
    }
    PLAYER_DATA.abyAttachedChartType[uIndex%3] = eType;
    HASHCHECK.Hash();
}

BOOLEAN CPlayerData::IsSoundEnabled(VOID)
{
    return PLAYER_DATA.bySoundEnabled;
}

VOID CPlayerData::SetSoundEnabled(BOOLEAN bEnabled)
{
    if (!HASHCHECK.IsMatchHash(TRUE)) {
        ASSERT(FALSE);
        return;
    }
    PLAYER_DATA.bySoundEnabled = bEnabled ? 1 : 0;
    HASHCHECK.Hash();
}

VOID CPlayerData::SetOandaCredential(const CHAR* szCredential)
{
    if (!HASHCHECK.IsMatchHash(TRUE)) {
        ASSERT(FALSE);
        return;
    }
    strcpy(PLAYER_DATA.szOandaCredential, szCredential);
    PLAYER_DATA.byOandaMask = rand() % 0xFF;
    ASSERT(sizeof(PLAYER_DATA.szOandaCredential) == 128);
    for (UINT uIndex = 0; 128 > uIndex; ++uIndex) {
        PLAYER_DATA.szOandaCredential[uIndex] = PLAYER_DATA.szOandaCredential[uIndex] ^ PLAYER_DATA.byOandaMask;
    }
    HASHCHECK.Hash();
}

#if defined(WINDOWS_DEV)
#define CUSTOM_BUILD
#endif // #if defined(WINDOWS_DEV)
#if defined(DEBUG)
#if !defined(USE_DEMO_ACCOUNT)
#define USE_DEMO_ACCOUNT
#endif // #if !defined(USE_DEMO_ACCOUNT)
#endif // #if defined(DEBUG)
BOOLEAN CPlayerData::GetOandaCredential(CHAR* szBuffer, UINT uBufferLen)
{
    if (128 > uBufferLen) {
        ASSERT(FALSE);
        return FALSE;
    }
#if defined(USE_DEMO_ACCOUNT)
#if defined(CUSTOM_BUILD)
    strncpy(szBuffer, "9caa700191f59e94974c2ecc2fcae990-5de17b21172bca644989490feace533d", uBufferLen);
#else // #if defined(CUSTOM_BUILD)
    strncpy(szBuffer, "8f027340c1354d04e9ae34a266dea4f4-7935a3ee0aa74121cd054da55e54ef3a", uBufferLen);
#endif // #if defined(CUSTOM_BUILD)
#else // #if defined(USE_DEMO_ACCOUNT)
#if defined(CUSTOM_BUILD)
    strncpy(szBuffer, "7a79dd68f0c86428a64ac866d81fb121-7578a077a139e042adf81446655b220d", uBufferLen);
#else // #if defined(CUSTOM_BUILD)
    for (UINT uIndex = 0; 128 > uIndex; ++uIndex) {
        szBuffer[uIndex] = PLAYER_DATA.szOandaCredential[uIndex] ^ PLAYER_DATA.byOandaMask;
    }
#endif // #if defined(CUSTOM_BUILD)
#endif // #if defined(USE_DEMO_ACCOUNT)
    return TRUE;
}


ERythmInterval CPlayerData::GetRythmFetchInterval(VOID)
{
    return (ERythmInterval)PLAYER_DATA.byRythmQueryInterval;
}

FLOAT CPlayerData::GetRythmFetchIntervalSecs(VOID)
{
    const FLOAT afFetchIntervals[ERythmInterval_Num] = {
        2.0f,   //        ERythmInterval_Per_2_Seconds,
        5.0f,   //        ERythmInterval_Per_5_Seconds,
        10.0f,  //        ERythmInterval_Per_10_Seconds,
        15.0f,  //        ERythmInterval_Per_15_Seconds,
        30.0f,  //        ERythmInterval_Per_30_Seconds,
    };
    return afFetchIntervals[PLAYER_DATA.byRythmQueryInterval%ERythmInterval_Num];
}
VOID CPlayerData::SetRythmFetchInterval(ERythmInterval eInterval)
{
    if (!HASHCHECK.IsMatchHash(TRUE)) {
        ASSERT(FALSE);
        return;
    }
    PLAYER_DATA.byRythmQueryInterval = eInterval;
    HASHCHECK.Hash();
}

const USHORT* CPlayerData::GetChannelRatios(VOID)
{
    return PLAYER_DATA.aushChannelRatio;
}

VOID CPlayerData::SetChannelRatios(const USHORT* pushRatios)
{
    if (!HASHCHECK.IsMatchHash(TRUE)) {
        ASSERT(FALSE);
        return;
    }
    ASSERT(MAX_DEPTH == 9);
    memcpy(PLAYER_DATA.aushChannelRatio, pushRatios, sizeof(USHORT) * 9);
    HASHCHECK.Hash();
}

FLOAT CPlayerData::GetSemaPercent(UINT uIndex)
{
    return PLAYER_DATA.aushSema[uIndex%4] * 0.1f;
}

UINT CPlayerData::GetSemaColor(UINT uIndex)
{
    return PLAYER_DATA.auSemaColor[uIndex%4];
}

VOID CPlayerData::SetSemaPercent(UINT uIndex, FLOAT fPercent)
{
    if (4 <= uIndex) {
        ASSERT(FALSE);
        return;
    }
    if (!HASHCHECK.IsMatchHash(TRUE)) {
        ASSERT(FALSE);
        return;
    }
    PLAYER_DATA.aushSema[uIndex] = (USHORT)round(fPercent * 10);
    HASHCHECK.Hash();
}

VOID CPlayerData::SetSemaColor(UINT uIndex, UINT uColor)
{
    if (4 <= uIndex) {
        ASSERT(FALSE);
        return;
    }
    if (!HASHCHECK.IsMatchHash(TRUE)) {
        ASSERT(FALSE);
        return;
    }
    PLAYER_DATA.auSemaColor[uIndex] = uColor;
    HASHCHECK.Hash();
}

BYTE CPlayerData::IsTimelineEnabled(VOID)
{
    return (0 != PLAYER_DATA.byTimelineEnabled);
}
VOID CPlayerData::SetTimelineEnabled(BOOLEAN bEnabled)
{
    if (!HASHCHECK.IsMatchHash(TRUE)) {
        ASSERT(FALSE);
        return;
    }
    PLAYER_DATA.byTimelineEnabled = bEnabled ? 1 : 0;
    HASHCHECK.Hash();
}

UINT CPlayerData::GetBiBColor(UINT uIndex)
{
    ASSERT(2 > uIndex);
    return PLAYER_DATA.auBoxInBoxColor[uIndex];
}

VOID CPlayerData::SetBiBColor(UINT uIndex, UINT uColor)
{
    ASSERT(2 > uIndex);
    if (!HASHCHECK.IsMatchHash(TRUE)) {
        ASSERT(FALSE);
        return;
    }
    PLAYER_DATA.auBoxInBoxColor[uIndex] = uColor;
    HASHCHECK.Hash();
}
