#include "stdafx.h"
#include "ConfirmationWindow.h"
#include "CryptoDotComMgr.h"
#include "CryptoUtil.h"
#include "Event.h"
#include "EventID.h"
#include "EventManager.h"
#include "HLTime.h"
#include "json.h"
#include "SimpleHttpClient.h"
#include "CryptoDotComMgr.JNI.h"

// API // https://exchange-docs.crypto.com/#introduction

static CCryptoDotComMgr* s_pcInst = NULL;

VOID CCryptoDotComMgr::InitInstance(VOID)
{
    ASSERT(NULL == s_pcInst);
    s_pcInst = new CCryptoDotComMgr();
#if defined(ANDROID_NDK)
    JNI_Init_CryptoDotComMgr();
#endif // #if defined(ANDROID_NDK)
}

VOID CCryptoDotComMgr::ReleaseInstance(VOID)
{
    SAFE_DELETE(s_pcInst);
#if defined(ANDROID_NDK)
    JNI_Release_CryptoDotComMgr();
#endif // #if defined(ANDROID_NDK)
}

CCryptoDotComMgr& CCryptoDotComMgr::GetInstance(VOID)
{
    ASSERT(NULL != s_pcInst);
    return *s_pcInst;
}

static const SCandleIntervalDisplayData s_asCandles[] = {
    { 1,        "1m"    },
    { 5,        "5m"    },
    { 15,       "15m"   },
    { 30,       "30m"   },
    { 60,       "1h"    },
    { 240,      "4h"    },
    { 360,      "6h"    },
    { 720,      "12h"   },
    { 1440,     "1D"    },
    { 10080,    "7D"    },
    { 20160,    "14D"   }
};
static const UINT s_uCandleIntervalNum = sizeof(s_asCandles)/sizeof(SCandleIntervalDisplayData);

UINT CCryptoDotComMgr::GetCandleTypeNum(VOID)
{
    return s_uCandleIntervalNum;
}
const SCandleIntervalDisplayData* CCryptoDotComMgr::GetAllCandleDisplayData(VOID)
{
    return s_asCandles;
}
UINT CCryptoDotComMgr::GetCandleDurationMins(UINT uIndex)
{
    return s_asCandles[uIndex % s_uCandleIntervalNum].uTimeMins;
}
const CHAR* CCryptoDotComMgr::GetCandleTypeDisplayName(UINT uIndex)
{
    return s_asCandles[uIndex % s_uCandleIntervalNum].szDisplayName;
}
UINT CCryptoDotComMgr::GetCandleIndexFromDisplayName(const CHAR* szName)
{
    UINT uIndex = 0;
    for ( ; s_uCandleIntervalNum > uIndex; ++uIndex) {
        if (0 == strcmp(s_asCandles[uIndex].szDisplayName, szName)) {
            return uIndex;
        }
    }
    ASSERT(FALSE);
    return 0; // safety return
}

CCryptoDotComMgr::CCryptoDotComMgr() : CEventHandler(EGLOBAL_CRYPTO_DOT_COM_MGR)
{
    
}
CCryptoDotComMgr::~CCryptoDotComMgr()
{
    ClearTradePairs();
    m_cSubscribedChannels.clear();
    ClearTradeList();
    ClearCandleMap();
}

VOID CCryptoDotComMgr::ClearTradeList(VOID)
{
    for (auto& sList : m_cTradedList) {
        sList.cList.clear();
    }
    m_cTradedList.clear();
    m_cLastID.clear();
}

VOID CCryptoDotComMgr::ClearCandleMap(VOID)
{
    for (auto itMap : m_cCandleMap) {
        itMap.second.clear();
    }
    m_cCandleMap.clear();
}

const CTradeCandleMap* CCryptoDotComMgr::GetCandleMap(UINT uTradePairIndex, UINT uCandleIndex)
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

VOID CCryptoDotComMgr::OnPaused(VOID)
{
#if !defined(MAC_PORT)
    TRACE("CCryptoDotComMgr OnPaused\n");
    CloseMarketSocketInternal();
    m_cSubscribedChannels.clear(); // cleaer all subscribed sockets
#endif // #if !defined(MAC_PORT)
}

VOID CCryptoDotComMgr::OnResume(VOID)
{
}

BOOLEAN CCryptoDotComMgr::SubscribeToChannel(const CHAR* szChannelName)
{
    Json::Value cParam;
    cParam["id"] = 1;
    cParam["method"] = "subscribe";
    cParam["params"]["channels"][0] = szChannelName;
    cParam["nonce"] = CHLTime::GetTimeMilliSecs();
    if (!SendMessageToMarket(cParam)) {
        return FALSE;
    }
    return TRUE;
}

VOID CCryptoDotComMgr::QueryOHLC(UINT uTradePairIndex, UINT uCandleIndex)
{
    const STradePair* psPair = GetTradePair(uTradePairIndex);
    if (NULL == psPair) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_CRYPTO_DOT_COM_MGR, CEvent(EGLOBAL_EVT_CRYPTO_OHLC_UPDATE, uTradePairIndex, uCandleIndex, FALSE));
        return;
    }
    CHAR szChannelName[1024];
    snprintf(szChannelName, 1024, "candlestick.%s.%s", GetCandleTypeDisplayName(uCandleIndex), psPair->szTradeKey);

    if (CCryptoUtil::IsChannelSubscribed(m_cSubscribedChannels, szChannelName)) {
        // do nothing, since already subscribed
        CEventManager::BroadcastEvent(EGLOBAL_CRYPTO_DOT_COM_MGR, CEvent(EGLOBAL_EVT_CRYPTO_OHLC_UPDATE, uTradePairIndex, uCandleIndex, TRUE));
        return;
    }
    // try find candle data for that interval. if it exists, query newer data
    SCandleIntervalData* psData = CCryptoUtil::FindCandleInterval(m_cCandleMap, psPair->szTradeKey, GetCandleDurationMins(uCandleIndex));
    if (NULL == psData) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_CRYPTO_DOT_COM_MGR, CEvent(EGLOBAL_EVT_CRYPTO_OHLC_UPDATE, uTradePairIndex, uCandleIndex, FALSE));
        return;
    }
    if (!SubscribeToChannel(szChannelName)) {
        CEventManager::BroadcastEvent(EGLOBAL_CRYPTO_DOT_COM_MGR, CEvent(EGLOBAL_EVT_CRYPTO_OHLC_UPDATE, uTradePairIndex, uCandleIndex, FALSE));
        return;
    }
    // do nothing here, we wait for reply
}
VOID CCryptoDotComMgr::OnQueryOHLC(const Json::Value& cData, UINT uCandleIndex, const CHAR* szKey)
{
    
    const UINT uMins = GetCandleDurationMins(uCandleIndex);
    const UINT uTradePairIndex = GetTradePairIndex(szKey);
    SCandleIntervalData* psData = CCryptoUtil::FindCandleInterval(m_cCandleMap, szKey, uMins);
    if (NULL == psData) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_CRYPTO_DOT_COM_MGR, CEvent(EGLOBAL_EVT_CRYPTO_OHLC_UPDATE, uTradePairIndex, uCandleIndex, FALSE));
        return;
    }
    if (!cData.isArray()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_CRYPTO_DOT_COM_MGR, CEvent(EGLOBAL_EVT_CRYPTO_OHLC_UPDATE, uTradePairIndex, uCandleIndex, FALSE));
        return;
    }
    psData->uLastQueryLocalTimeSecs = CHLTime::GetTimeSecs();
//    TRACE("OLHC Update: %s.%s\n", szKey, GetCandleTypeDisplayName(uCandleIndex));
    
    CTradeCandleMap& cMap = psData->cCandleMap;
    const UINT uDataNum = cData.size();
    UINT uIndex = 0;
    UINT uLastCandleTimeSec = 0;
    for ( ; uDataNum > uIndex; ++uIndex) {
        const Json::Value& cCandle = cData[uIndex];
        if (!cCandle.isObject()) {
            TRACE("Error: %s[%d] result not object format\n", szKey, uIndex);
            ASSERT(FALSE);
            continue;
        }
        const Json::Value& cTime    = cCandle["t"];
        const Json::Value& cOpen    = cCandle["o"];
        const Json::Value& cHigh    = cCandle["h"];
        const Json::Value& cLow     = cCandle["l"];
        const Json::Value& cClose   = cCandle["c"];
        const Json::Value& cVol     = cCandle["v"];
        if (!cTime.isUInt64() || !cOpen.isDouble() || !cHigh.isDouble() || !cLow.isDouble() || !cClose.isDouble() || !cVol.isDouble()) {
            TRACE("Error in Data at index %d\n", uIndex);
            ASSERT(FALSE);
            continue;
        }
        uLastCandleTimeSec = (UINT)(cTime.asUInt64() / 1000);
        const DOUBLE dOpen = cOpen.asDouble();
        const DOUBLE dClose = cClose.asDouble();
        const DOUBLE dHigh = cHigh.asDouble();
        const DOUBLE dLow = cLow.asDouble();
        const DOUBLE dVol = cVol.asDouble();
        CTradeCandleMap::iterator itFound = cMap.find(uLastCandleTimeSec);
        if (cMap.end() == itFound) { // if not found, insert
            const DOUBLE dVWAP = (dOpen + dClose) * 0.5;
            const STradeCandle sTmp = {
                uLastCandleTimeSec,             //                UINT    uTimeSec;
                0,                              //                UINT    uTradeNum;  // number of trades
                dOpen,                          //                DOUBLE  dOpen;      // opening price
                0,                              //                DOUBLE  dOpenTime;  // opening time
                dClose,                         //                DOUBLE  dClose;     // closing price
                0,                              //                DOUBLE  dCloseTime; // closing time
                dHigh,                          //                DOUBLE  dHigh;      // highest price
                dLow,                           //                DOUBLE  dLow;       // lowest price
                dVWAP,                          //                DOUBLE  dVWAP;      // volume weighted average price
                dVol                            //                DOUBLE  dVolume;    // total volume traded
            };
            CCryptoUtil::InsertCandle(cMap, sTmp);
        }
        else {
            // append to the data..
            STradeCandle& sCandle = itFound->second;
            const DOUBLE dNewVol = sCandle.dVolume + dVol;
            const DOUBLE dRatio = dVol / dNewVol;
            const DOUBLE dNewVWAP = (dOpen + dClose) * 0.5;
            const DOUBLE dFinalVWAP = dRatio * dNewVWAP + (1.0 - dRatio) * sCandle.dVWAP;
            sCandle.dClose = dClose;
            if (sCandle.dHigh < dHigh) {
                sCandle.dHigh = dHigh;
            }
            if (sCandle.dLow > dLow) {
                sCandle.dLow = dLow;
            }
            sCandle.dVolume = dNewVol;
            sCandle.dVWAP = dFinalVWAP;
        }
    }
//    TRACE("Count:%d Time Diff: %d\n", uDataNum, CHLTime::GetTimeSecs() - uNewestTime);
    CCryptoUtil::OnInsertCandleEnd(cMap, uMins * 60, uLastCandleTimeSec);
    CEventManager::BroadcastEvent(EGLOBAL_CRYPTO_DOT_COM_MGR, CEvent(EGLOBAL_EVT_CRYPTO_OHLC_UPDATE, uTradePairIndex, uCandleIndex, TRUE));
}
