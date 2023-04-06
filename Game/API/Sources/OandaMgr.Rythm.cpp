#include "stdafx.h"
#include "AutoPtr.h"
#include "ConfirmationWindow.h"
#include "EventID.h"
#include "EventManager.h"
#include "HLTime.h"
#include "json.h"
#include "MessageLog.h"
#include "OandaMgr.h"
#include "PlayerData.h"
#include "SimpleHttpClient.h"
#include <string.h>

#define CANDLE_GRANULARITY_5_SECOND "M1"
#define CANDLE_GRANULARITY_1_MINUTE "M1"

#define MAX_CANDLE_SECS_PER_FETCH_FOR_5_SECOND_CANDLE   (4500 * 5)
#define MAX_CANDLE_SECS_PER_FETCH_FOR_1_MIN_CANDLE      (4500 * 60)

#define FIRST_FETCH_BACKDATED_SECONDS (3*MAX_CANDLE_SECS_PER_FETCH_FOR_1_MIN_CANDLE + MAX_CANDLE_SECS_PER_FETCH_FOR_5_SECOND_CANDLE)


static BOOLEAN GetRythmFromJson(UINT& uLastCandleTimeSec, BOOLEAN& bLastCandleClosed, SRythm& sRythm, const Json::Value& cCandle, const SRythm& sPrev)
{
    if (!cCandle.isObject()) {
        ASSERT(FALSE);
        return FALSE;
    }
    const Json::Value& cTime = cCandle["time"];
    if (!cTime.isString()) {
        ASSERT(FALSE);
        return FALSE;
    }
    const UINT uCandleTimeSec = atoi(cTime.asCString());
    const Json::Value& cPrices      = cCandle["mid"];
    const Json::Value& cClose       = cPrices["c"];
    const Json::Value& cHigh        = cPrices["h"];
    const Json::Value& cLow         = cPrices["l"];
    const Json::Value& cOpen        = cPrices["o"];
    const Json::Value& cVol         = cCandle["volume"];
    const Json::Value& cComplete    = cCandle["complete"];
    if (!cVol.isUInt() || !cClose.isString() || !cHigh.isString() || !cLow.isString() || !cOpen.isString() || !cComplete.isBool()) {
        ASSERT(FALSE);
        return FALSE;
    }
    const DOUBLE dVolume = cVol.asUInt();
    const DOUBLE dOpen = std::stod(cOpen.asCString());
    const DOUBLE dHigh = std::stod(cHigh.asCString());
    const DOUBLE dLow = std::stod(cLow.asCString());
    const DOUBLE dClose = std::stod(cClose.asCString());
    const DOUBLE dAvgPrice = (dClose + dOpen) * 0.5;

    sRythm.ullTimeMilliSec = (UINT64)uCandleTimeSec * 1000L;
    sRythm.dCLosePrice = dClose;
    sRythm.dHighPrice = dHigh;
    sRythm.dLowPrice = dLow;
    sRythm.dAvgPrice = dAvgPrice;
    sRythm.dActualVolume = dVolume;
    sRythm.dTotalVolume = dVolume;
    sRythm.dTotalQuoteVolume = 0.0;
    sRythm.dOBV = 0.0;
    sRythm.dOBVDiff = 0.0;
    bLastCandleClosed = cComplete.asBool();
    if (bLastCandleClosed) { // if candle completed,
        uLastCandleTimeSec = uCandleTimeSec;
    }
    return TRUE;
}

SRythmListener* COandaMgr::GetRythmListener(UINT uTradeIndex)
{
    for (auto& sRythm : m_cRythmListeners) {
        if (sRythm.uTradePairIndex == uTradeIndex) {
            return &sRythm;
        }
    }
    return NULL;
}
VOID COandaMgr::UpdateRythms(FLOAT fLapsed)
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
        sRythm.bRetryingFromError = FALSE;
        FetchRythm(sRythm);
    }
}

BOOLEAN COandaMgr::IsListeningToRythm(UINT uTradeIndex)
{
    for (auto& sRythm : m_cRythmListeners) {
        if (sRythm.uTradePairIndex == uTradeIndex) { // already listening
            return TRUE;
        }
    }
    return FALSE;
}
VOID COandaMgr::ListenToRythm(UINT uTradeIndex)
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

VOID COandaMgr::StopListeningToRythm(UINT uTradeIndex)
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

VOID COandaMgr::FetchRythm(SRythmListener& sListener)
{
    const SOandaPair* psPair = GetTradePair(sListener.uTradePairIndex);
    if (NULL == psPair) {
        ASSERT(FALSE);
        return;
    }
    CHAR szCredentials[128];
    if (!CPlayerData::GetOandaCredential(szCredentials, 128) || 0 == szCredentials[0]) {
        ASSERT(FALSE);
        return;
    }
    CHAR szBuffer[1024];
    snprintf(szBuffer, 1024, "Bearer %s", szCredentials);
    Json::Value cHeaderFields;
    cHeaderFields["Authorization"] = szBuffer;
    cHeaderFields["Accept-Datetime-Format"] = "UNIX"; // unix time

    CHAR szURL[1024];
    snprintf(szURL, 1024, OANDA_CANDLE_URL, psPair->szTradePair);
    
    const UINT uNowSec = CHLTime::GetTimeSecs();
    UINT uToSec;
    UINT uFromSec;
    if (0 != sListener.uFetchedToSec) {
        uFromSec = sListener.uFetchedToSec;
    }
    else {
        sListener.bPreFetching = TRUE; // lets do pre fetch
        uFromSec = uNowSec - FIRST_FETCH_BACKDATED_SECONDS;
    }
    // determine time difference
    const UINT uTimeDiff = uNowSec - uFromSec;
    const CHAR* szCandleString = CANDLE_GRANULARITY_5_SECOND;
    UINT uDurationToFetch = MAX_CANDLE_SECS_PER_FETCH_FOR_5_SECOND_CANDLE;
    if (MAX_CANDLE_SECS_PER_FETCH_FOR_1_MIN_CANDLE < uTimeDiff) {
        uDurationToFetch = MAX_CANDLE_SECS_PER_FETCH_FOR_1_MIN_CANDLE;
        szCandleString = CANDLE_GRANULARITY_1_MINUTE;
    }
    uToSec = uFromSec + uDurationToFetch;
    if (uNowSec < uToSec) {
        uToSec = uNowSec;
        sListener.bPreFetching = FALSE;
    }
    sListener.bFetching = TRUE;
    sListener.uFetchedToSec = uToSec;
    Json::Value* pcParams = new Json::Value();
    (*pcParams)["from"] = uFromSec;
    (*pcParams)["to"] = uToSec;
    (*pcParams)["price"] = "M";           // midpoint data "A" = ask "B" = bid
    (*pcParams)["granularity"] = szCandleString;
    (*pcParams)["nounce"] = uNowSec;
    (*pcParams)["nounce_index"] = sListener.uTradePairIndex;
//    TRACE("[%s] Fetch From %d To %d\n", psPair->szTradePair, uFromSec, uToSec);
    CSimpleHttpClient::DoGet(szURL, *pcParams, EGLOBAL_OANDA_MGR, EGLOBAL_INTERNAL_OANDA_GET_RYTHM, &cHeaderFields);
}

VOID COandaMgr::OnRythmFetched(CEvent& cEvent)
{
    const Json::Value* pcJson   = (const Json::Value*)cEvent.GetPtrParam(5);
    const INT nErrCode          = cEvent.GetIntParam(6);
    const CHAR* szResult        = (const CHAR*) cEvent.GetPtrParam(7);
    
    CAutoPtr<const Json::Value> cJson(pcJson);
    CAutoPtrArray<const CHAR>   cResult(szResult);
    const UINT uTradePairIndex  = (*pcJson)["nounce_index"].asUInt();
    SRythmListener* psListener = GetRythmListener(uTradePairIndex);
    if (NULL == psListener) { // its possible destroyed.
        TRACE("Warn: Listener was destroyed (%d)\n", uTradePairIndex);
        return;
    }
    psListener->bFetching = FALSE;
    CHAR szErrorString[1024];
    if (CSimpleHttpClient::ESuccess != nErrCode) {
        snprintf(szErrorString, 1024, "Fetch[%d] Err:%d", uTradePairIndex, nErrCode);
        CMessageLog::AddLog(szErrorString);
        if (NULL != szResult) {
            CMessageLog::AddLog(szResult);
        }
        psListener->bRetryingFromError = TRUE;
        psListener->fTimeToNextFetch = 30.0f;
        return;
    }
    Json::Reader cReader;
    Json::Value cData;
    if (!cReader.parse(szResult, cData) || !cData.isObject()) {
        ASSERT(FALSE);
        psListener->bRetryingFromError = TRUE;
        psListener->fTimeToNextFetch = 30.0f;
        snprintf(szErrorString, 1024, "[%d] Json Err\n", uTradePairIndex);
        CConfirmationWindow::DisplayErrorMsg(szErrorString, NULL, 0);
        return;
    }
#if defined(DEBUG)
    const Json::Value& cInstrument = cData["instrument"];
    if (!cInstrument.isString()) {
        ASSERT(FALSE);
        return;
    }
    const COandaMgr::SOandaPair* psPair = GetTradePairByName(cInstrument.asCString());
    if (NULL == psPair) {
        ASSERT(FALSE);
        return;
    }
    ASSERT(psPair->uIndex == uTradePairIndex);
#endif // #if defined(DEBUG)

    SRythm sPrev = { 0 };
    if (0 < psListener->cList.size()) {
        sPrev = (*(psListener->cList.rbegin())); // do a copy
    }

    const Json::Value& cCandles = cData["candles"];
    if (!cCandles.isArray()) {
        ASSERT(FALSE);
        psListener->bRetryingFromError = TRUE;
        psListener->fTimeToNextFetch = 30.0f;
        snprintf(szErrorString, 1024, "[%d]!cCandles.isArray()\n", uTradePairIndex);
        CConfirmationWindow::DisplayErrorMsg(szErrorString, NULL, 0);
        std::string cResult(szResult);
        std::string cCut = cResult.substr(0, 512);
        CConfirmationWindow::DisplayErrorMsg(cCut.c_str(), NULL, 0);
        return;
    }
    const UINT uCandleNum = cCandles.size();
    UINT uLastCandleTimeSec = (UINT)(sPrev.ullTimeMilliSec / 1000);
    BOOLEAN bLastCandleClosed = FALSE;
    for (UINT uIndex = 0; uCandleNum > uIndex; ++uIndex) {
        const Json::Value& cCandle = cCandles[uIndex];
        SRythm sTmp;
        if (!GetRythmFromJson(uLastCandleTimeSec, bLastCandleClosed, sTmp, cCandle, sPrev)) {
            TRACE("Error in json reply!\n");
            ASSERT(FALSE);
            psListener->bRetryingFromError = TRUE;
            psListener->fTimeToNextFetch = 30.0f;
            snprintf(szErrorString, 1024, "[%d]!GetRythmFromJson()\n", uTradePairIndex);
            CConfirmationWindow::DisplayErrorMsg(szErrorString, NULL, 0);
            return;
        }
        if (sPrev.ullTimeMilliSec < sTmp.ullTimeMilliSec) {
//            TRACE("Add: %llu\n", sTmp.ullTimeMilliSec);
            psListener->cList.push_back(sTmp);
        }
        else {
            if (sPrev.ullTimeMilliSec == sTmp.ullTimeMilliSec) {
                if (0 != memcmp(&sTmp, &sPrev, sizeof(SRythm))) {
                    // replace
//                    TRACE("Replace: %llu\n", sTmp.ullTimeMilliSec);
                    psListener->cList.pop_back();
                    psListener->cList.push_back(sTmp);
                }
            }
            else {
//                TRACE("Add: %llu\n", sTmp.ullTimeMilliSec);
                psListener->cList.push_back(sTmp);
            }
        }
        sPrev = sTmp;
    }
    const UINT uNowSec = CHLTime::GetTimeSecs();
    UINT uSecDiff = 0;
    UINT uMinDiff = 0;
    UINT uHrDiff = 0;
    if (uNowSec > uLastCandleTimeSec) {
        uSecDiff = uNowSec - uLastCandleTimeSec;
        uMinDiff = (uSecDiff / 60) % 60;
        uHrDiff = uSecDiff / 3600;
        uSecDiff = uSecDiff % 60;
    }
//    TRACE("[%s] Last Candle Time %d  Diff %dH %dM %dS\n", psPair->szTradePair, uLastCandleTimeSec, uHrDiff, uMinDiff, uSecDiff);
    ASSERT(0 != uLastCandleTimeSec);
    psListener->uLastCandleTimeSec = uLastCandleTimeSec;
    psListener->dCurrentPrice = sPrev.dCLosePrice;
    UpdateTradePairPrice(uTradePairIndex, sPrev.dCLosePrice);
//    TRACE("[%s] %d Candles Fetched\n", psPair->szTradePair, uCandleNum);
    if (psListener->bPreFetching) {
//        TRACE("[%s] Continue Prefetching Trade Pair\n", psPair->szTradePair);
        FetchRythm(*psListener);
        return;
    }
    CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_RYTHM_UPDATE, uTradePairIndex));
}
