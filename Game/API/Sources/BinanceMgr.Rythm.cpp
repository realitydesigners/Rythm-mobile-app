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
#include "PlayerData.h"
#include "SimpleHttpClient.h"
#include <math.h>
#include <memory.h>

#define CANDLE_GRANULARITY_5_MINUTE "5m"
#define CANDLE_GRANULARITY_1_MINUTE "1m"

#define MAX_CANDLE_SECS_PER_FETCH_FOR_5_MIN_CANDLE      (800 * 5 * 60)
#define MAX_CANDLE_SECS_PER_FETCH_FOR_1_MIN_CANDLE      (800 * 60)

// 5 days back
#define FIRST_FETCH_BACKDATED_SECONDS (2*MAX_CANDLE_SECS_PER_FETCH_FOR_5_MIN_CANDLE + MAX_CANDLE_SECS_PER_FETCH_FOR_1_MIN_CANDLE)

static BOOLEAN GetRythmFromJson(UINT& uLastCandleTimeSec, SRythm& sRythm, const Json::Value& cCandle, const SRythm& sPrev)
{
    if (!cCandle.isArray()) {
        ASSERT(FALSE);
        return FALSE;
    }
    
    const Json::Value& cTime    = cCandle[0];
    const Json::Value& cOpen    = cCandle[1];
    const Json::Value& cHigh    = cCandle[2];
    const Json::Value& cLow     = cCandle[3];
    const Json::Value& cClose   = cCandle[4];
    const Json::Value& cVol     = cCandle[5];
    const Json::Value& cCloseTime           = cCandle[6];
    const Json::Value& cQuoteAssetVolume    = cCandle[7];
    const Json::Value& cCnt                 = cCandle[8];
    if (!cTime.isUInt64() || !cOpen.isString() || !cHigh.isString() || !cLow.isString() || !cClose.isString()
        || !cCloseTime.isUInt64() || !cQuoteAssetVolume.isString() || !cVol.isString() || !cCnt.isUInt()) {
        ASSERT(FALSE);
        return FALSE;
    }
    uLastCandleTimeSec = (UINT)(cTime.asUInt64() / 1000);

    const DOUBLE dClosePrice = std::stod(cClose.asString());
    const DOUBLE dVolume = std::stod(cVol.asString());
    const DOUBLE dQuoteAssetVolume = std::stod(cQuoteAssetVolume.asString());
    DOUBLE dAvgPrice;
    if (0.0 == dVolume) {
        if (0 == sPrev.ullTimeMilliSec) {
            dAvgPrice = dClosePrice;
        }
        else {
            dAvgPrice = sPrev.dAvgPrice;
        }
    }
    else {
        dAvgPrice = dQuoteAssetVolume / dVolume;
    }
    DOUBLE dPrevAvgPrice = 0.0;
    DOUBLE dPrevOBV = 0.0;
    if (0 != sPrev.ullTimeMilliSec) {
        dPrevAvgPrice = sPrev.dAvgPrice;
        dPrevOBV = sPrev.dOBV;
    }
    if (0.0 == dPrevAvgPrice) {
        dPrevAvgPrice = dAvgPrice;
    }
    const DOUBLE dPriceDiff = dAvgPrice - dPrevAvgPrice;
    const DOUBLE dOBVDiff = dPriceDiff * dVolume;
    const DOUBLE dOBV = dPrevOBV + dOBVDiff;

    sRythm.dCLosePrice = dClosePrice;
    sRythm.dHighPrice = std::stod(cHigh.asString());
    sRythm.dLowPrice = std::stod(cLow.asString());
    sRythm.dAvgPrice = dAvgPrice;
    sRythm.dActualVolume = dVolume;
    sRythm.dTotalVolume = dVolume;
    sRythm.dTotalQuoteVolume = dQuoteAssetVolume;
    sRythm.dOBV = dOBV;
    sRythm.dOBVDiff = dOBVDiff;
    sRythm.dCLosePrice = std::stod(cClose.asString());
    const UINT64 ullClose = cCloseTime.asUInt64();
    const UINT64 ullNow = CHLTime::GetTimeMilliSecs();
    // close second is candle related.. so it could be 59 seconds after now.
    sRythm.ullTimeMilliSec = ullNow < ullClose ? ullNow : ullClose;
    return TRUE;
}

const SRythmListener* CBinanceMgr::GetRythmListener(UINT uTradeIndex)
{
    for (auto& sRythm : m_cRythmListeners) {
        if (sRythm.uTradePairIndex == uTradeIndex) {
            return &sRythm;
        }
    }
    return NULL;
}
VOID CBinanceMgr::UpdateRythms(FLOAT fLapsed)
{
    for (auto& sRythm : m_cRythmListeners) {
        if (sRythm.bFetching) {
            continue;
        }
        sRythm.fTimeToNextFetch -= fLapsed;
        if (0.0f < sRythm.fTimeToNextFetch) {
            continue;
        }
        sRythm.fTimeToNextFetch = CPlayerData::GetRythmFetchIntervalSecs();
        FetchRythm(sRythm);
    }
}

BOOLEAN CBinanceMgr::IsListeningToRythm(UINT uTradeIndex)
{
    for (auto& sRythm : m_cRythmListeners) {
        if (sRythm.uTradePairIndex == uTradeIndex) { // already listening
            return TRUE;
        }
    }
    return FALSE;
}
VOID CBinanceMgr::ListenToRythm(UINT uTradeIndex)
{
    for (auto& sRythm : m_cRythmListeners) {
        if (sRythm.uTradePairIndex == uTradeIndex) { // already listening
            ++sRythm.uRefCnt;
            return;
        }
    }
    const SRythmListener sTmp = {
        uTradeIndex,    //        UINT            uTradePairIndex;
        1,              //        UINT            uRefCnt;
        0.0f,           //        FLOAT           fTimeToNextFetch;
        0.0,            //        DOUBLE          dCurrentPrice;
        FALSE,          //        BOOLEAN         bFetching;
        FALSE,          //        BOOLEAN         bPreFetching;
        FALSE,          //        BOOLEAN         bRetryingFromError;
        0,              //        UINT            uLastCandleTimeSec;
        0,              //        UINT            uFetchedToSec;
    };
    // not found, insert new entry
    ASSERT(0 == sTmp.cList.size());
    m_cRythmListeners.push_back(sTmp);
}

VOID CBinanceMgr::StopListeningToRythm(UINT uTradeIndex)
{
    CRythmListeners::iterator itRythm = m_cRythmListeners.begin();
    const CRythmListeners::iterator itEnd = m_cRythmListeners.end();
    for ( ; itEnd != itRythm; ++itRythm) {
        SRythmListener& sListener = *(itRythm);
        if (sListener.uTradePairIndex == uTradeIndex) {
            ASSERT(0 < sListener.uRefCnt);
            --sListener.uRefCnt;
            if (0 == sListener.uRefCnt) {
                sListener.cList.clear();
                m_cRythmListeners.erase(itRythm);
                return;
            }
        }
    }
    TRACE("Warn: unable to stop listening to Rythm [%d]\n", uTradeIndex);
}


VOID CBinanceMgr::FetchRythm(SRythmListener& sListener)
{
    const STradePair* psPair = GetTradePair(sListener.uTradePairIndex);
    if (NULL == psPair) {
        ASSERT(FALSE);
        return;
    }
    const UINT uNowSec = CHLTime::GetTimeSecs();
    UINT uToSec;
    UINT uFromSec;
    if (0 == sListener.uFetchedToSec) {
        sListener.bPreFetching = TRUE;
    }
    if (sListener.bPreFetching) {
        if (0 != sListener.uFetchedToSec) {
            uFromSec = sListener.uFetchedToSec;
        }
        else {
            sListener.bPreFetching = TRUE; // lets do pre fetch
            uFromSec = uNowSec - FIRST_FETCH_BACKDATED_SECONDS;
        }
    }
    else {
        uFromSec = sListener.uLastCandleTimeSec;
    }
    // determine time difference
    const UINT uTimeDiff = uNowSec - uFromSec;
    const CHAR* szCandleString = CANDLE_GRANULARITY_1_MINUTE;
    UINT uDurationToFetch = MAX_CANDLE_SECS_PER_FETCH_FOR_1_MIN_CANDLE;
    if (MAX_CANDLE_SECS_PER_FETCH_FOR_5_MIN_CANDLE < uTimeDiff) {
        uDurationToFetch = MAX_CANDLE_SECS_PER_FETCH_FOR_5_MIN_CANDLE;
        szCandleString = CANDLE_GRANULARITY_5_MINUTE;
    }
    uToSec = uFromSec + uDurationToFetch;
    if (uNowSec < uToSec) {
        uFromSec = sListener.uLastCandleTimeSec;
        uToSec = uNowSec;
        sListener.bPreFetching = FALSE;
    }
    sListener.bFetching = TRUE;
    sListener.uFetchedToSec = uToSec;
    
    const UINT64 ullFromSec = (UINT64)uFromSec * 1000L;
    const UINT64 ullToSec = (UINT64)uToSec * 1000L;
    ASSERT(0 != ullFromSec);
//    TRACE("Fetching %llu => %llu with %s\n", ullFromSec, ullToSec, szCandleString);
    Json::Value* pcParams = new Json::Value();
    (*pcParams)["interval"] = szCandleString;
    (*pcParams)["symbol"] = psPair->szTradeKey;
    (*pcParams)["limit"] = 1000;
    (*pcParams)["startTime"] = ullFromSec;
    (*pcParams)["endTime"] = ullToSec;
    CSimpleHttpClient::DoGet("https://api.binance.com/api/v3/klines", *pcParams, EGLOBAL_BINANCE_MGR, EGLOBAL_EVT_BINANCE_QUERY_RYTHM);
}

VOID CBinanceMgr::OnRythmFetched(CEvent& cEvent)
{
    const Json::Value* pcJson   = (const Json::Value*)cEvent.GetPtrParam(5);
    const INT nErrCode          = cEvent.GetIntParam(6);
    const CHAR* szResult        = (const CHAR*) cEvent.GetPtrParam(7);
    
    CAutoPtr<const Json::Value> cJson(pcJson);
    CAutoPtrArray<const CHAR>   cResult(szResult);
    
    const Json::Value& cParams = *pcJson;
    const std::string cKey = cParams["symbol"].asString();
    UINT uTradeIndex = 0;
    {
        UINT uIndex = 0;
        const UINT uTradeNum = GetTradePairNum();
        const CHAR* szKey = cKey.c_str();
        for ( ; uTradeNum > uIndex; ++uIndex) {
            const STradePair* psPair = GetTradePair(uIndex);
            if (NULL != psPair) {
                if (0 == strcmp(psPair->szTradeKey, szKey)) {
                    uTradeIndex = uIndex;
                    break;
                }
            }
        }
    }
    SRythmListener* psListener = NULL;
    for (auto& sRythm : m_cRythmListeners) {
        if (uTradeIndex == sRythm.uTradePairIndex) { // found
            psListener = &sRythm;
            break;
        }
    }
    if (NULL == psListener) {
        TRACE("Warn Rythm: %s NOT Found!\n", cKey.c_str());
        return;
    }
    psListener->bFetching = FALSE;
    CHAR szErrorString[1024];
    if (CSimpleHttpClient::ESuccess != nErrCode) {
        TRACE("Binance Rythm Err Code: %d\n", nErrCode);
        if (NULL != szResult) {
            CConfirmationWindow::DisplayErrorMsg(szResult, NULL, 0);
        }
        return;
    }
    Json::Reader cReader;
    Json::Value cValue;
    if (!cReader.parse(szResult, cValue) || !cValue.isArray()) {
        ASSERT(FALSE);
        psListener->bRetryingFromError = TRUE;
        psListener->fTimeToNextFetch = 30.0f;
        snprintf(szErrorString, 1024, "[%d] Json Err\n", uTradeIndex);
        CConfirmationWindow::DisplayErrorMsg(szErrorString, NULL, 0);
        return;
    }
    SRythm sPrev = { 0 };
    if (0 < psListener->cList.size()) {
        sPrev = (*(psListener->cList.rbegin())); // do a copy
    }
    const UINT uCandleNum = cValue.size();
    UINT uLastCandleTimeSec = (UINT)(sPrev.ullTimeMilliSec / 1000);
    for (UINT uIndex = 0; uCandleNum > uIndex; ++uIndex) {
        const Json::Value& cCandle = cValue[uIndex];
        SRythm sTmp;
        if (!GetRythmFromJson(uLastCandleTimeSec, sTmp, cCandle, sPrev)) {
            TRACE("Error in json reply!\n");
            ASSERT(FALSE);
            psListener->bRetryingFromError = TRUE;
            psListener->fTimeToNextFetch = 30.0f;
            snprintf(szErrorString, 1024, "[%d]!GetRythmFromJson()\n", uTradeIndex);
            CConfirmationWindow::DisplayErrorMsg(szErrorString, NULL, 0);
            return;
        }
        if (sPrev.ullTimeMilliSec < sTmp.ullTimeMilliSec) {
            psListener->cList.push_back(sTmp);
        }
        else {
            if (sPrev.ullTimeMilliSec == sTmp.ullTimeMilliSec) {
                if (0 != memcmp(&sTmp, &sPrev, sizeof(SRythm))) {
                    // replace
                    psListener->cList.pop_back();
                    psListener->cList.push_back(sTmp);
                }
            }
            else {
                psListener->cList.push_back(sTmp);
            }
        }
        sPrev = sTmp;
    }
    ASSERT(0 != uLastCandleTimeSec);
    psListener->uLastCandleTimeSec = uLastCandleTimeSec;
    psListener->dCurrentPrice = sPrev.dCLosePrice;
//    TRACE("%d Candles Fetched\n", uCandleNum);
//    TRACE("Last Candle Sec %d\n", uLastCandleTimeSec);
    if (psListener->bPreFetching) {
//        TRACE("Continue Prefetching Trade Pair: %d\n", psListener->uTradePairIndex);
        FetchRythm(*psListener);
        return;
    }

    CEventManager::BroadcastEvent(EGLOBAL_BINANCE_MGR, CEvent(EGLOBAL_EVT_BINANCE_RYTHM_UPDATE, uTradeIndex));
}
