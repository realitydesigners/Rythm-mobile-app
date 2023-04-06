#include "stdafx.h"
#include "AutoPtr.h"
#include "ConfirmationWindow.h"
#include "EventID.h"
#include "EventManager.h"
#include "HLTime.h"
#include "json.h"
#include "OandaMgr.h"
#include "PlayerData.h"
#include "SimpleHttpClient.h"
#include <string.h>

#define MAX_CANDLE_QUERY_NUM (1000)

VOID COandaMgr::ClearCandleMap(VOID)
{
    for (auto itMap : m_cCandleMap) {
        itMap.second.clear();
    }
    m_cCandleMap.clear();
}

const CTradeCandleMap* COandaMgr::GetCandleMap(UINT uTradePairIndex, UINT uCandleIndex)
{
    const SOandaPair* psPair = GetTradePair(uTradePairIndex);
    if (NULL == psPair) {
        ASSERT(FALSE);
        return NULL;
    }
    const UINT uMins = GetCandleDurationMins(uCandleIndex);
    SCandleIntervalData* psData = CCryptoUtil::FindCandleInterval(m_cCandleMap, psPair->szTradePair, uMins, TRUE);
    if (NULL == psData) {
        return NULL;
    }
    return &(psData->cCandleMap);

}

BOOLEAN COandaMgr::HasOHLCData(UINT uTradePairIndex, UINT uCandleIndex)
{
    const SOandaPair* psPair = GetTradePair(uTradePairIndex);
    if (NULL == psPair) {
        ASSERT(FALSE);
        return FALSE;
    }
    const UINT uMins = GetCandleDurationMins(uCandleIndex);
    // try find candle data for that interval. if it exists, query newer data
    const SCandleIntervalData* psData = CCryptoUtil::FindCandleInterval(m_cCandleMap, psPair->szTradePair, uMins);
    if (NULL == psData) {
        return FALSE;
    }
    return (0 < psData->cCandleMap.size());
}

VOID COandaMgr::QueryOHLC(UINT uTradePairIndex, UINT uCandleIndex, BOOLEAN bSkipTimeCheck)
{
    const SOandaPair* psPair = GetTradePair(uTradePairIndex);
    if (NULL == psPair) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_OHLC_QUERY, FALSE));
        return;
    }
    CHAR szCredentials[128];
    if (!CPlayerData::GetOandaCredential(szCredentials, 128) || 0 == szCredentials[0]) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_OHLC_QUERY, FALSE));
        return;
    }
    CHAR szBuffer[1024];
    snprintf(szBuffer, 1024, "Bearer %s", szCredentials);
    Json::Value cHeaderFields;
    cHeaderFields["Authorization"] = szBuffer;
    cHeaderFields["Accept-Datetime-Format"] = "UNIX"; // unix time

    const CHAR* szCandleString = GetCandleTypeDisplayName(uCandleIndex);
    CHAR szURL[1024];
    snprintf(szURL, 1024, OANDA_CANDLE_URL, psPair->szTradePair);
    Json::Value* pcParams = new Json::Value();
    (*pcParams)["count"] = MAX_CANDLE_QUERY_NUM;    // max to return
    (*pcParams)["price"] = "M";                     // midpoint data "A" = ask "B" = bid
    (*pcParams)["granularity"] = szCandleString;    // candle duration
    CSimpleHttpClient::DoGet(szURL, *pcParams, EGLOBAL_OANDA_MGR, EGLOBAL_INTERNAL_OANDA_GET_OHLC, &cHeaderFields);
}

VOID COandaMgr::OnGetOHLCReply(const CEvent& cEvent)
{
    const Json::Value* pcJson   = (const Json::Value*)cEvent.GetPtrParam(5);
    const INT nErrCode          = cEvent.GetIntParam(6);
    const CHAR* szResult        = (const CHAR*) cEvent.GetPtrParam(7);
    CAutoPtr<const Json::Value> cJson(pcJson);
    CAutoPtrArray<const CHAR> cResult(szResult);

    if (CSimpleHttpClient::ESuccess != nErrCode) {
        CHAR szBuffer[1024];
        snprintf(szBuffer, 1024, "Oanda Err Code:%d", nErrCode);
        CConfirmationWindow::DisplayErrorMsg(szBuffer, NULL, 0);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_OHLC_QUERY, FALSE));
        return;
    }
    Json::Reader cReader;
    Json::Value cData;
    if (!cReader.parse(szResult, cData) || !cData.isObject()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_OHLC_QUERY, FALSE));
        return;
    }
    const Json::Value& cInstrument = cData["instrument"];
    if (!cInstrument.isString()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_OHLC_QUERY, FALSE));
        return;
    }
    ASSERT((*pcJson)["granularity"].isString());
    const UINT uCandleIndex = GetCandleIndexFromDisplayName((*pcJson)["granularity"].asCString());
    const UINT uCandleMins = GetCandleDurationMins(uCandleIndex);
    SCandleIntervalData* psData = CCryptoUtil::FindCandleInterval(m_cCandleMap, cInstrument.asCString(), uCandleMins);
    if (NULL == psData) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_OHLC_QUERY, FALSE));
        return;
    }
    const Json::Value& cCandles = cData["candles"];
    if (!cCandles.isArray()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_OHLC_QUERY, FALSE));
        return;
    }
    CTradeCandleMap& cMap = psData->cCandleMap;
    cMap.clear();
    UINT uLastCandleTimeSec = 0;
    const UINT uCandleNum = (UINT)cCandles.size();
    for (UINT uIndex = 0; uCandleNum > uIndex; ++uIndex) {
        const Json::Value& cCandle = cCandles[uIndex];
        if (!cCandle.isObject()) {
            ASSERT(FALSE);
            continue;
        }
        const Json::Value& cTime = cCandle["time"];
        if (!cTime.isString()) {
            ASSERT(FALSE);
            continue;
        }
        uLastCandleTimeSec = atoi(cTime.asCString());
        const Json::Value& cPrices  = cCandle["mid"];
        const Json::Value& cClose   = cPrices["c"];
        const Json::Value& cHigh    = cPrices["h"];
        const Json::Value& cLow     = cPrices["l"];
        const Json::Value& cOpen    = cPrices["o"];
        const Json::Value& cVol     = cCandle["volume"];
        if (!cVol.isUInt() || !cClose.isString() || !cHigh.isString() || !cLow.isString() || !cOpen.isString()) {
            ASSERT(FALSE);
            continue;
        }
        const DOUBLE dVolume = cVol.asUInt();
        const DOUBLE dOpen = std::stod(cOpen.asCString());
        const DOUBLE dHigh = std::stod(cHigh.asCString());
        const DOUBLE dLow = std::stod(cLow.asCString());
        const DOUBLE dClose = std::stod(cClose.asCString());
        const DOUBLE dAvgPrice = (dClose + dOpen) * 0.5;
        const STradeCandle sTmp = {
            uLastCandleTimeSec,             //                UINT    uTimeSec;
            0,                              //                UINT    uTradeNum;  // number of trades
            dOpen,                          //                DOUBLE  dOpen;      // opening price
            0,                              //                DOUBLE  dOpenTime;  // opening time
            dClose,                         //                DOUBLE  dClose;     // closing price
            0,                              //                DOUBLE  dCloseTime; // closing time
            dHigh,                          //                DOUBLE  dHigh;      // highest price
            dLow,                           //                DOUBLE  dLow;       // lowest price
            dAvgPrice,                      //                DOUBLE  dVWAP;      // volume weighted average price
            dVolume                         //                DOUBLE  dVolume;    // total volume traded
        };
        CCryptoUtil::InsertCandle(cMap, sTmp);
    }
    psData->uLastQueryLocalTimeSecs = uLastCandleTimeSec;
    CCryptoUtil::OnInsertCandleEnd(cMap, uCandleMins * 60, uLastCandleTimeSec);
    CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_OHLC_QUERY, TRUE));
}
