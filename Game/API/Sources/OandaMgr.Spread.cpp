#include "stdafx.h"
#include "AutoPtr.h"
#include "ConfirmationWindow.h"
#include "EventID.h"
#include "EventManager.h"
#include "GameUtil.h"
#include "json.h"
#include "MessageLog.h"
#include "OandaMgr.h"
#include "PlayerData.h"
#include "SimpleHttpClient.h"
#include "System.h"


#define OANDA_FETCH_SPREADS_URL OANDA_MAIN_URL "v3/accounts/%s/pricing"
VOID COandaMgr::QueryLatestSpreads(const UINT* puTradePairIndex, UINT uTradePairNum)
{
    ASSERT(0 < uTradePairNum);
    const CHAR* szAccountID = GetAccountID(0);
    if (NULL == szAccountID) { // must be already signed in
        ASSERT(FALSE);
        return;
    }
    CHAR szCredentials[128];
    if (!CPlayerData::GetOandaCredential(szCredentials, 128)) {
        ASSERT(FALSE);
        return;
    }
    CHAR szTradePairString[2048] = { 0 };
    for (UINT uIndex = 0; uTradePairNum > uIndex; ++uIndex) {
        const COandaMgr::SOandaPair* psPair = GetTradePair(uIndex);
        if (NULL == psPair) {
            ASSERT(FALSE);
            return;
        }
        if (0 < uIndex) {
#if defined(ANDROID_NDK)
            strlcat(szTradePairString, ",", 2048);
#else // #if defined(ANDROID_NDK)
            strlcat(szTradePairString, "%2C", 2048);
#endif // #if defined(ANDROID_NDK)
        }
        strlcat(szTradePairString, psPair->szTradePair, 2048);
    }
    
    CHAR szBuffer[1024];
    snprintf(szBuffer, 1024, "Bearer %s", szCredentials);
    Json::Value cHeaderFields;
    cHeaderFields["Authorization"] = szBuffer;
    cHeaderFields["Accept-Datetime-Format"] = "UNIX"; // unix time
    CHAR szURL[2048];
    snprintf(szURL, 2048, OANDA_FETCH_SPREADS_URL, szAccountID);
    Json::Value* pcParams = new Json::Value();
    (*pcParams)["instruments"] = szTradePairString;
    CSimpleHttpClient::DoGet(szURL, *pcParams, EGLOBAL_OANDA_MGR, EGLOBAL_INTERNAL_OANDA_GET_SPREADS, &cHeaderFields);
}

VOID COandaMgr::OnGetSpreadsReply(const CEvent& cEvent)
{
    const Json::Value* pcJson   = (const Json::Value*)cEvent.GetPtrParam(5);
    const INT nErrCode          = cEvent.GetIntParam(6);
    const CHAR* szResult        = (const CHAR*) cEvent.GetPtrParam(7);
    CAutoPtr<const Json::Value> cJson(pcJson);
    CAutoPtrArray<const CHAR> cResult(szResult);

    if (CSimpleHttpClient::ESuccess != nErrCode) {
        CMessageLog::AddLog("Fetch Spread Err");
        if (NULL != szResult) {
            CMessageLog::AddLog(szResult);
        }
        return;
    }
    Json::Reader cReader;
    Json::Value cData;
    if (!cReader.parse(szResult, cData) || !cData.isObject()) {
        ASSERT(FALSE);
        CMessageLog::AddLog("FS Err: fail make json");
        return;
    }
    const Json::Value& cPrices = cData["prices"];
    if (!cPrices.isArray()) {
        ASSERT(FALSE);
        return;
    }
    const UINT uNum = (UINT)cPrices.size();
    for (UINT uIndex = 0; uNum > uIndex; ++uIndex) {
        const Json::Value& cPrice = cPrices[uIndex];
        if (!cPrice.isObject()) {
            ASSERT(FALSE);
            continue;
        }
        const Json::Value& cInstrument = cPrice["instrument"];
        if (!cInstrument.isString()) {
            ASSERT(FALSE);
            continue;
        }
        const SOandaPair* psPair = GetTradePairByName(cInstrument.asCString());
        if (NULL == psPair) {
            ASSERT(FALSE);
            continue;
        }
        const Json::Value& cAskArray = cPrice["asks"];
        const Json::Value& cBidArray = cPrice["bids"];
        if (!cAskArray.isArray() || !cBidArray.isArray()) {
            ASSERT(FALSE);
            continue;
        }
        const Json::Value& cAskPrice = cAskArray[0]["price"];
        if (!cAskPrice.isString()) {
            ASSERT(FALSE);
            continue;
        }
        const Json::Value& cBidPrice = cBidArray[0]["price"];
        if (!cBidPrice.isString()) {
            ASSERT(FALSE);
            continue;
        }
        const FLOAT fAskPrice = std::stod(cAskPrice.asCString());
        const FLOAT fBidPrice = std::stod(cBidPrice.asCString());
        SOandaPair* psPairEdit = const_cast<SOandaPair*>(psPair);
        psPairEdit->fAskPrice = fAskPrice;
        psPairEdit->fBidPrice = fBidPrice;
    }
    CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_FETCH_SPREADS_REPLY));
}
