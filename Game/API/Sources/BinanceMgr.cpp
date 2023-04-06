#include "stdafx.h"
#include "AutoPtr.h"
#include "BinanceMgr.h"
#include "ConfirmationWindow.h"
#include "CryptoUtil.h"
#include "Event.h"
#include "EventID.h"
#include "EventManager.h"
#include "HLTime.h"
#include "json.h"
#include "SimpleHttpClient.h"
#include "SoundManager.h"
// WS API https://github.com/binance-exchange/binance-official-api-docs/blob/master/web-socket-streams.md

// API // https://github.com/binance-exchange/binance-official-api-docs/blob/master/rest-api.md#general-api-information
// tradable pairs
// https://api.binance.com/api/v3/exchangeInfo
// ohlc
// https://api.binance.com/api/v3/klines?symbol=BTCUSDT&interval=15m
// https://api.binance.com/api/v3/klines?symbol=BTCUSDT&interval=3m

static CBinanceMgr* s_pcInst = NULL;

VOID CBinanceMgr::InitInstance(VOID)
{
    ASSERT(NULL == s_pcInst);
    s_pcInst = new CBinanceMgr();
}

VOID CBinanceMgr::ReleaseInstance(VOID)
{
    SAFE_DELETE(s_pcInst);
}

CBinanceMgr& CBinanceMgr::GetInstance(VOID)
{
    ASSERT(NULL != s_pcInst);
    return *s_pcInst;
}

static const SCandleIntervalDisplayData s_asCandles[] = {
    { 1,        "1m"    },
    { 3,        "3m"    },
    { 5,        "5m"    },
    { 15,       "15m"   },
    { 30,       "30m"   },
    { 60,       "1h"    },
    { 120,      "2h"    },
    { 240,      "4h"    },
    { 60*6,     "6h"    },
    { 60*8,     "8h"    },
    { 60*12,    "12h"   },
    { 1440,     "1d"    },
    { 1440*3,   "3d"    },
    { 10080,    "1w"    }
};
static const UINT s_uCandleIntervalNum = sizeof(s_asCandles)/sizeof(SCandleIntervalDisplayData);

UINT CBinanceMgr::GetCandleTypeNum(VOID)
{
    return s_uCandleIntervalNum;
}
const SCandleIntervalDisplayData* CBinanceMgr::GetAllCandleDisplayData(VOID)
{
    return s_asCandles;
}


UINT CBinanceMgr::GetCandleDurationMins(UINT uIndex)
{
    return s_asCandles[uIndex % s_uCandleIntervalNum].uTimeMins;
}

const CHAR* CBinanceMgr::GetCandleTypeDisplayName(UINT uIndex)
{
    return s_asCandles[uIndex % s_uCandleIntervalNum].szDisplayName;
}
UINT CBinanceMgr::GetCandleDurationIndexFromDisplayDurationName(const CHAR* szName)
{
    UINT uIndex = 0;
    for ( ; s_uCandleIntervalNum > uIndex; ++uIndex) {
        if (0 == strcmp(s_asCandles[uIndex].szDisplayName, szName)) {
            return uIndex;
        }
    }
    ASSERT(FALSE);
    return 0;
}
CBinanceMgr::CBinanceMgr() :
CEventHandler(EGLOBAL_BINANCE_MGR),
m_psTradePairs(NULL),
m_uTradePairNum(0)
{
    
}
CBinanceMgr::~CBinanceMgr()
{
    for (auto& sTick : m_cTickListenerList) {
        sTick.cMinuteList.clear();
        sTick.cSecondList.clear();
        SProcessedTick& sProcessed = sTick.sProcessed;
        SAFE_DELETE_ARRAY(sProcessed.pdMACD);
        SAFE_DELETE_ARRAY(sProcessed.pdSignal);
    }
    m_cTickListenerList.clear();
    ClearCandleMap();
    ClearTradePairs();
}

VOID CBinanceMgr::ClearCandleMap(VOID)
{
    for (auto itMap : m_cCandleMap) {
        itMap.second.clear();
    }
    m_cCandleMap.clear();
}
const CTradeCandleMap* CBinanceMgr::GetCandleMap(UINT uTradePairIndex, UINT uCandleIndex)
{
    const STradePair* psPair = GetTradePair(uTradePairIndex);
    if (NULL == psPair) {
        ASSERT(FALSE);
        return NULL;
    }
    const UINT uMins = s_asCandles[uCandleIndex%s_uCandleIntervalNum].uTimeMins;
    SCandleIntervalData* psData = CCryptoUtil::FindCandleInterval(m_cCandleMap, psPair->szTradeKey, uMins, TRUE);
    if (NULL == psData) {
        return NULL;
    }
    return &(psData->cCandleMap);
}

BOOLEAN CBinanceMgr::HasOHLCData(UINT uTradePairIndex, UINT uCandleIndex)
{
    const STradePair* psPair = GetTradePair(uTradePairIndex);
    if (NULL == psPair) {
        ASSERT(FALSE);
        return FALSE;
    }
    const UINT uMins = s_asCandles[uCandleIndex%s_uCandleIntervalNum].uTimeMins;
    // try find candle data for that interval. if it exists, query newer data
    const SCandleIntervalData* psData = CCryptoUtil::FindCandleInterval(m_cCandleMap, psPair->szTradeKey, uMins);
    if (NULL == psData) {
        return FALSE;
    }
    return (0 < psData->cCandleMap.size());
}
VOID CBinanceMgr::QueryOHLC(UINT uTradePairIndex, UINT uCandleIndex, BOOLEAN bSkipTimeCheck)
{
    const STradePair* psPair = GetTradePair(uTradePairIndex);
    if (NULL == psPair) {
        ASSERT(FALSE);
        return;
    }

    const SCandleIntervalDisplayData& sCandleInterval = s_asCandles[uCandleIndex%s_uCandleIntervalNum];
    const UINT uMins = sCandleInterval.uTimeMins;
    // try find candle data for that interval. if it exists, query newer data
    SCandleIntervalData* psData = CCryptoUtil::FindCandleInterval(m_cCandleMap, psPair->szTradeKey, uMins);
    if (NULL == psData) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_BINANCE_MGR, CEvent(EGLOBAL_EVT_BINANCE_ON_OHLC_QUERY_END, FALSE));
        return;
    }
    if (!bSkipTimeCheck) {
        const UINT uNow = CHLTime::GetTimeSecs();
        if (uNow < (psData->uLastQueryLocalTimeSecs + 5)) {
            // use cached result
            CEventManager::BroadcastEvent(EGLOBAL_BINANCE_MGR, CEvent(EGLOBAL_EVT_BINANCE_ON_OHLC_QUERY_END, TRUE));
            return;
        }
    }    
    Json::Value* pcParams = new Json::Value();
    (*pcParams)["interval"] = sCandleInterval.szDisplayName;
    (*pcParams)["symbol"] = psPair->szTradeKey;
    CSimpleHttpClient::DoGet("https://api.binance.com/api/v3/klines", *pcParams, EGLOBAL_BINANCE_MGR, EGLOBAL_EVT_BINANCE_QUERY_OHLC);
}
VOID CBinanceMgr::OnQueryOHLC(const Json::Value* pcParams, INT nErrCode, const CHAR* szResult)
{
    if (CSimpleHttpClient::ESuccess != nErrCode) {
        TRACE("CBinanceMgr OHLC Failure!\n");
        CEventManager::BroadcastEvent(EGLOBAL_BINANCE_MGR, CEvent(EGLOBAL_EVT_BINANCE_ON_OHLC_QUERY_END, FALSE));
        return;
    }
    const CHAR* szInterval = (*pcParams)["interval"].asCString();
    const UINT uMins = GetCandleDurationMins(GetCandleDurationIndexFromDisplayDurationName(szInterval));
    const std::string cKey = (*pcParams)["symbol"].asString();
    SCandleIntervalData* psData = CCryptoUtil::FindCandleInterval(m_cCandleMap, cKey, uMins);
    if (NULL == psData) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_BINANCE_MGR, CEvent(EGLOBAL_EVT_BINANCE_ON_OHLC_QUERY_END, FALSE));
        return;
    }
    Json::Reader cReader;
    Json::Value cValue;
    if (!cReader.parse(szResult, cValue) || !cValue.isArray()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_BINANCE_MGR, CEvent(EGLOBAL_EVT_BINANCE_ON_OHLC_QUERY_END, FALSE));
        return;
    }
    const UINT uCandleNum = cValue.size();
    UINT uLastCandleTimeSec = 0;
    CTradeCandleMap& cMap = psData->cCandleMap;
    cMap.clear(); // clear the map
    UINT uIndex = 0;
    for ( ; uCandleNum > uIndex; ++uIndex) {
        const Json::Value& cCandle = cValue[uIndex];
        if (!cCandle.isArray()) {
            TRACE("Error: %s[%d] result not array format\n", cKey.c_str(), uIndex);
            ASSERT(FALSE);
            continue;
        }
        const Json::Value& cTime    = cCandle[0];
        const Json::Value& cOpen    = cCandle[1];
        const Json::Value& cHigh    = cCandle[2];
        const Json::Value& cLow     = cCandle[3];
        const Json::Value& cClose   = cCandle[4];
        const Json::Value& cVol     = cCandle[5];
        const Json::Value& cQuoteAssetVolume    = cCandle[7];
        const Json::Value& cCnt     = cCandle[8];
        if (!cTime.isUInt64() || !cOpen.isString() || !cHigh.isString() || !cLow.isString() || !cClose.isString()
            || !cQuoteAssetVolume.isString() || !cVol.isString() || !cCnt.isUInt()) {
            TRACE("Error in Data at index %d\n", uIndex);
            ASSERT(FALSE);
            continue;
        }
        uLastCandleTimeSec = (UINT)(cTime.asUInt64() / 1000);
        const DOUBLE dVolume = std::stod(cVol.asString());
        const DOUBLE dQuoteAssetVolume = std::stod(cQuoteAssetVolume.asString());
        const DOUBLE dAvgPrice = dQuoteAssetVolume / dVolume;
        const STradeCandle sTmp = {
            uLastCandleTimeSec,             //                UINT    uTimeSec;
            cCnt.asUInt(),                  //                UINT    uTradeNum;  // number of trades
            std::stod(cOpen.asString()),    //                DOUBLE  dOpen;      // opening price
            0,                              //                DOUBLE  dOpenTime;  // opening time
            std::stod(cClose.asString()),   //                DOUBLE  dClose;     // closing price
            0,                              //                DOUBLE  dCloseTime; // closing time
            std::stod(cHigh.asString()),    //                DOUBLE  dHigh;      // highest price
            std::stod(cLow.asString()),     //                DOUBLE  dLow;       // lowest price
            dAvgPrice,                      //                DOUBLE  dVWAP;      // volume weighted average price
            dVolume                         //                DOUBLE  dVolume;    // total volume traded
        };
        CCryptoUtil::InsertCandle(cMap, sTmp);
    }
    psData->uLastQueryLocalTimeSecs = uLastCandleTimeSec;
    CCryptoUtil::OnInsertCandleEnd(cMap, uMins * 60, uLastCandleTimeSec);
    CEventManager::BroadcastEvent(EGLOBAL_BINANCE_MGR, CEvent(EGLOBAL_EVT_BINANCE_ON_OHLC_QUERY_END, TRUE));
}

VOID CBinanceMgr::OnQueryOHLC(CEvent& cEvent)
{
    const Json::Value* pcJson   = (const Json::Value*)cEvent.GetPtrParam(5);
    const INT nErrCode          = cEvent.GetIntParam(6);
    const CHAR* szResult        = (const CHAR*) cEvent.GetPtrParam(7);
    CAutoPtr<const Json::Value> cJson(pcJson);
    CAutoPtr<const CHAR>        cResult(szResult);
    
    if (CSimpleHttpClient::ESuccess != nErrCode) {
        CHAR szBuffer[1024];
        snprintf(szBuffer, 1024, "Binance Err Code:%d", nErrCode);
        CConfirmationWindow::DisplayErrorMsg(szBuffer, NULL, 0);
    }
    OnQueryOHLC(pcJson, nErrCode, szResult);
}
VOID CBinanceMgr::OnReceiveEvent(CEvent& cEvent)
{
    switch (cEvent.GetIntParam(0)) {
        case EGLOBAL_EVT_BINANCE_ON_TRADE_PAIR_QUERIED:
            OnQueryTradePairs(cEvent);
            break;
        case EGLOBAL_EVT_BINANCE_QUERY_OHLC:
            OnQueryOHLC(cEvent);
            break;
        case EGLOBAL_EVT_BINANCE_QUERY_TICK:
            OnTickQueried(cEvent);
            break;
        case EGLOBAL_EVT_BINANCE_QUERY_RYTHM:
            OnRythmFetched(cEvent);
            break;
        case EGLOBAL_EVT_BINANCE_ALARM_SE:
            {
                const INT nSEID = cEvent.GetIntParam(1);
                const UINT uCount = cEvent.GetIntParam(2) - 1;
                CSoundManager::PlaySE(nSEID);
                if (0 < uCount) {
                    CEventManager::PostGlobalEvent(EGLOBAL_BINANCE_MGR, CEvent(EGLOBAL_EVT_BINANCE_ALARM_SE, nSEID, uCount), 1.0f);
                }
            }
            break;
        default:
            ASSERT(FALSE);
            break;
    }
}

VOID CBinanceMgr::ProcessOHLCHeartBeat(const CHAR* szTradeKey, const CTradeCandleMap& cCandleMap)
{
    if (0 == cCandleMap.size()) {
        return;
    }
    UINT uNewestCandleTimeSec = 0;
    DOUBLE dPrice = 0.0;
    for (auto it : cCandleMap) {
        const UINT uTimeSec = it.first;
        if (uNewestCandleTimeSec < uTimeSec) {
            uNewestCandleTimeSec = uTimeSec;
            dPrice = it.second.dClose;
        }
    }
    const std::string cTradeKey = szTradeKey;
    auto itFound = m_cPriceMap.find(cTradeKey);
    if (m_cPriceMap.end() == itFound) {
        TRACE("[%s] First : %f\n", szTradeKey, dPrice);
        m_cPriceMap.insert(CTradeKeyPricePair(cTradeKey, dPrice));
        return;
    }
    const DOUBLE dPrevPrice = itFound->second;
    const DOUBLE dPriceDiff = dPrice - dPrevPrice;
    const FLOAT fPercentageChange = (FLOAT)((dPriceDiff * 100.0) / dPrice);
    
    TRACE("[%s] Price Change : %.3f%%\n", szTradeKey, fPercentageChange);
    m_cPriceMap.insert(CTradeKeyPricePair(cTradeKey, dPrice));
    if (0.2 < fPercentageChange || -0.2 > fPercentageChange) {
        TRACE("Firing Alarm for %s [%f] => [%f]\n", szTradeKey, dPrevPrice, dPrice);
        CEventManager::PostGlobalEvent(EGLOBAL_BINANCE_MGR, CEvent(EGLOBAL_EVT_BINANCE_ALARM_SE, SE_SFX_PING, 10));
    }
}

