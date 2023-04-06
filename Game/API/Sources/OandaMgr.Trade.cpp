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


#define OANDA_OPEN_TRADES_URL OANDA_MAIN_URL "v3/accounts/%s/openTrades"
#define OANDA_CLOSE_TRADE_URL OANDA_MAIN_URL "v3/accounts/%s/trades/%d/close"

const COandaMgr::CTradeList* COandaMgr::GetOpenTrades(UINT uAccountIndex) const
{
    const SOandaAccount* psAcct = const_cast<COandaMgr*>(this)->GetAccount(uAccountIndex);
    if (NULL == psAcct) {
        return NULL;
    }
    return &psAcct->cOpenTradeList;
}
const COandaMgr::SOandaTrade* COandaMgr::GetTrade(UINT uAccountIndex, UINT uTradeID) const
{
    const CTradeList* pcList = GetOpenTrades(uAccountIndex);
    if (NULL == pcList) {
        return NULL;
    }
    for (const auto& sTrade : *pcList) {
        if (sTrade.uTradeID == uTradeID) {
            return &sTrade;
        }
    }
    return NULL;
}

VOID COandaMgr::QueryOpenTrades(UINT uAccountIndex)
{
    const CHAR* szAccountID = GetAccountID(uAccountIndex);
    if (NULL == szAccountID) { // must be already signed in
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_OPEN_TRADES_REPLY, uAccountIndex, FALSE));
        return;
    }
    CHAR szCredentials[128];
    if (!CPlayerData::GetOandaCredential(szCredentials, 128)) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_OPEN_TRADES_REPLY, uAccountIndex, FALSE));
        return;
    }

    CHAR szBuffer[1024];
    snprintf(szBuffer, 1024, "Bearer %s", szCredentials);
    Json::Value cHeaderFields;
    cHeaderFields["Authorization"] = szBuffer;
    cHeaderFields["Accept-Datetime-Format"] = "UNIX"; // unix time
    CHAR szURL[2048];
    snprintf(szURL, 2048, OANDA_OPEN_TRADES_URL, szAccountID);
    Json::Value* pcParams = new Json::Value();
    (*pcParams)["nounce"] = uAccountIndex;
    CSimpleHttpClient::DoGet(szURL, *pcParams, EGLOBAL_OANDA_MGR, EGLOBAL_INTERNAL_OANDA_GET_OPEN_TRADES, &cHeaderFields);
}

VOID COandaMgr::OnGetOpenTradesReply(const CEvent& cEvent)
{
    const Json::Value* pcJson   = (const Json::Value*)cEvent.GetPtrParam(5);
    const UINT uAccountIndex    = (*pcJson)["nounce"].asUInt();
    const INT nErrCode          = cEvent.GetIntParam(6);
    const CHAR* szResult        = (const CHAR*) cEvent.GetPtrParam(7);
    CAutoPtr<const Json::Value> cJson(pcJson);
    CAutoPtrArray<const CHAR> cResult(szResult);

    if (CSimpleHttpClient::ESuccess != nErrCode) {
        CMessageLog::AddLog("Query Trades Error");
        if (NULL != szResult) {
            CMessageLog::AddLog(szResult);
        }
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_OPEN_TRADES_REPLY, uAccountIndex, FALSE));
        return;
    }
    Json::Reader cReader;
    Json::Value cData;
    if (!cReader.parse(szResult, cData) || !cData.isObject()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_OPEN_TRADES_REPLY, uAccountIndex, FALSE));
        return;
    }
    const Json::Value& cTrades = cData["trades"];
    if (!cTrades.isArray()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_OPEN_TRADES_REPLY, uAccountIndex, FALSE));
        return;
    }
    SOandaAccount* psAcct = GetAccount(uAccountIndex);
    if (NULL == psAcct) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_OPEN_TRADES_REPLY, uAccountIndex, FALSE));
        return;
    }
    CTradeList& cTradeList = psAcct->cOpenTradeList;
    cTradeList.clear();
    const UINT uTradeNum = (UINT)cTrades.size();
    for (UINT uIndex = 0; uTradeNum > uIndex; ++uIndex) {
        const Json::Value& cTrade = cTrades[uIndex];
        const Json::Value& cID = cTrade["id"];
        const Json::Value& cTradePair = cTrade["instrument"];
        const Json::Value& cTime = cTrade["openTime"];
        const Json::Value& cState = cTrade["state"];
        const Json::Value& cPrice = cTrade["price"];
        const Json::Value& cInitialUnits = cTrade["initialUnits"];
        const Json::Value& cCurrentUnits = cTrade["currentUnits"];
        const Json::Value& cRealizedPL = cTrade["realizedPL"];
        const Json::Value& cUnRealizedPL = cTrade["unrealizedPL"];
        if (!cID.isString()) {
            ASSERT(FALSE);
            continue;
        }
        const UINT uID = atoi(cID.asCString());
        if (!cTime.isString()) {
            ASSERT(FALSE);
            continue;
        }
        if (!cTradePair.isString() || !cPrice.isString()) {
            ASSERT(FALSE);
            continue;
        }
        const SOandaPair* psPair = GetTradePairByName(cTradePair.asCString());
        if (NULL == psPair) {
            ASSERT(FALSE);
            continue;
        }
        const UINT uTimeStamp = atoi(cTime.asCString());
        if (!cState.isString()) {
            ASSERT(FALSE);
            continue;
        }
        const CHAR* szState = cState.asCString();
        ETRADE_STATE eState;
        if (0 == strcmp("OPEN", szState)) {
            eState = ETRADE_OPEN;
        }
        else if (0 == strcmp("CLOSED", szState)) {
            eState = ETRADE_CLOSED;
        }
        else if (0 == strcmp("CLOSE_WHEN_TRADEABLE", szState)) {
            eState = ETRADE_CLOSED_WHEN_TRADABLE;
        }
        else {
            ASSERT(FALSE);
            continue;
        }
        if (!cInitialUnits.isString() && !cCurrentUnits.isString() && !cRealizedPL.isString() && !cUnRealizedPL.isString()) {
            ASSERT(FALSE);
            continue;
        }
        const DOUBLE dPrice = std::stod(cPrice.asCString());
        const DOUBLE dInitialUnits = std::stod(cInitialUnits.asCString());
        const DOUBLE dCurrentUnits = std::stod(cCurrentUnits.asCString());
        const DOUBLE dRealizedPL = std::stod(cRealizedPL.asCString());
        const DOUBLE dUnRealizedPL = std::stod(cUnRealizedPL.asCString());
        const SOandaTrade sTmp = {
            uID,            //            UINT            uTradeID;
            psPair->uIndex, //            UINT            uTradePairIndex;
            dPrice,         //            DOUBLE          dPrice;
            uTimeStamp,     //            UINT            uOpenTime;
            eState,         //            ETRADE_STATE    eState;
            dInitialUnits,  //            DOUBLE          dInitialUnits;
            dCurrentUnits,  //            DOUBLE          dCurrentUnits;
            dRealizedPL,    //            DOUBLE          dRealizedPL;
            dUnRealizedPL   //            DOUBLE          dUnRealizedPL;
        };
        cTradeList.push_back(sTmp);
    }
    CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_OPEN_TRADES_REPLY, uAccountIndex, TRUE));
}

static VOID CreateCloseTradeReply(UINT uMsgID, UINT uAccountIndex, UINT uTradeID, BOOLEAN bResult, FLOAT fPrice)
{
    CEvent cEvent(EGLOBAL_EVT_OANDA_CLOSE_TRADE_REPLY);
    cEvent.SetIntParam(1, uMsgID);
    cEvent.SetIntParam(2, uAccountIndex);
    cEvent.SetIntParam(3, uTradeID);
    cEvent.SetIntParam(4, bResult);
    cEvent.SetFloatParam(5, fPrice);
    CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, cEvent);
    
}
UINT COandaMgr::CloseTrade(UINT uAccountIndex, UINT uTradeID, UINT uUnits)
{
    const CHAR* szAccountID = GetAccountID(uAccountIndex);
    if (NULL == szAccountID) { // must be already signed in
        ASSERT(FALSE);
        return 0;
    }
    CHAR szCredentials[128];
    if (!CPlayerData::GetOandaCredential(szCredentials, 128)) {
        ASSERT(FALSE);
        return 0;
    }
    static UINT s_uCloseTradeMsgID = 1;
    const UINT uMsgID = s_uCloseTradeMsgID;
    ++s_uCloseTradeMsgID;
    CHAR szBuffer[1024];
    snprintf(szBuffer, 1024, "Bearer %s", szCredentials);
    Json::Value cHeaderFields;
    cHeaderFields["Authorization"] = szBuffer;
    cHeaderFields["Accept-Datetime-Format"] = "UNIX"; // unix time
    CHAR szURL[2048];
    snprintf(szURL, 2048, OANDA_CLOSE_TRADE_URL, szAccountID, uTradeID);
    Json::Value* pcParams = new Json::Value();
    (*pcParams)["nounce"] = uAccountIndex;
    (*pcParams)["nounce2"] = uTradeID;
    (*pcParams)["nounce3"] = uMsgID;
    if (0 == uUnits) {
        (*pcParams)["units"] = "ALL";
    }
    else {
        CHAR szBuffer[64];
        snprintf(szBuffer, 64, "%d", uUnits);
        (*pcParams)["units"] = szBuffer;
    }
    CSimpleHttpClient::DoPut(szURL, *pcParams, EGLOBAL_OANDA_MGR, EGLOBAL_INTERNAL_OANDA_CLOSE_TRADE, &cHeaderFields);
    return uMsgID;
}

VOID COandaMgr::OnCloseTradeReply(const CEvent& cEvent)
{
    const Json::Value* pcJson   = (const Json::Value*)cEvent.GetPtrParam(5);
    const UINT uAccountIndex    = (*pcJson)["nounce"].asUInt();
    const UINT uTradeID         = (*pcJson)["nounce2"].asUInt();
    const UINT uMsgID           = (*pcJson)["nounce3"].asUInt();
    const INT nErrCode          = cEvent.GetIntParam(6);
    const CHAR* szResult        = (const CHAR*) cEvent.GetPtrParam(7);
    CAutoPtr<const Json::Value> cJson(pcJson);
    CAutoPtrArray<const CHAR> cResult(szResult);
    if (CSimpleHttpClient::ESuccess != nErrCode) {
        CMessageLog::AddLog("Close Trade Error");
        if (NULL != szResult) {
            CMessageLog::AddLog(szResult);
        }
        CreateCloseTradeReply(uMsgID, uAccountIndex, uTradeID, FALSE, 0.0f);
        return;
    }
    Json::Reader cReader;
    Json::Value cData;
    FLOAT fResultPrice = 0.0f;
    if (!cReader.parse(szResult, cData) || !cData.isObject()) {
        ASSERT(FALSE);
        CMessageLog::AddLog("CT Err: fail make json");
    }
    else {
        const Json::Value& cOrderFillTransaction = cData["orderFillTransaction"];
        const Json::Value& cResultPrice = cOrderFillTransaction["price"];        
        if (cResultPrice.isString()) {
            fResultPrice = atof(cResultPrice.asCString());
        }
        else {
            CMessageLog::AddLog("CO Err: Missing Price");
        }
    }
    CreateCloseTradeReply(uMsgID, uAccountIndex, uTradeID, TRUE, fResultPrice);
}
