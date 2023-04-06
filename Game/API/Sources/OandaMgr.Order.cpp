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


#define OANDA_PENDING_ORDERS_URL OANDA_MAIN_URL "v3/accounts/%s/pendingOrders"
#define OANDA_CREATE_ORDER_URL OANDA_MAIN_URL "v3/accounts/%s/orders"
#define OANDA_CANCEL_ORDER_URL OANDA_MAIN_URL "v3/accounts/%s/orders/%d/cancel"

VOID COandaMgr::QueryPendingOrders(UINT uAccountIndex)
{
    const CHAR* szAccountID = GetAccountID(uAccountIndex);
    if (NULL == szAccountID) { // must be already signed in
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_PENDING_ORDERS_UPDATED, uAccountIndex, FALSE));
        return;
    }
    CHAR szCredentials[128];
    if (!CPlayerData::GetOandaCredential(szCredentials, 128)) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_PENDING_ORDERS_UPDATED, uAccountIndex, FALSE));
        return;
    }

    CHAR szBuffer[1024];
    snprintf(szBuffer, 1024, "Bearer %s", szCredentials);
    Json::Value cHeaderFields;
    cHeaderFields["Authorization"] = szBuffer;
    cHeaderFields["Accept-Datetime-Format"] = "UNIX"; // unix time
    CHAR szURL[2048];
    snprintf(szURL, 2048, OANDA_PENDING_ORDERS_URL, szAccountID);
    Json::Value* pcParams = new Json::Value();
    (*pcParams)["nounce"] = uAccountIndex;
    CSimpleHttpClient::DoGet(szURL, *pcParams, EGLOBAL_OANDA_MGR, EGLOBAL_INTERNAL_OANDA_GET_PENDING_ORDERS, &cHeaderFields);
}

VOID COandaMgr::OnGetPendingOrdersReply(const CEvent& cEvent)
{
    const Json::Value* pcJson   = (const Json::Value*)cEvent.GetPtrParam(5);
    const UINT uAccountIndex    = (*pcJson)["nounce"].asUInt();
    const INT nErrCode          = cEvent.GetIntParam(6);
    const CHAR* szResult        = (const CHAR*) cEvent.GetPtrParam(7);
    CAutoPtr<const Json::Value> cJson(pcJson);
    CAutoPtrArray<const CHAR> cResult(szResult);

    if (CSimpleHttpClient::ESuccess != nErrCode) {
        CMessageLog::AddLog("GetPendingOrders Error");
        if (NULL != szResult) {
            CMessageLog::AddLog(szResult);
        }
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_PENDING_ORDERS_UPDATED, uAccountIndex, FALSE));
        return;
    }
    //    if (NULL != szResult) {
    //        TRACE("Result:\n%s\n", szResult);
    //    }
    SOandaAccount* psAccount = GetAccount(uAccountIndex);
    if (NULL == psAccount) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_PENDING_ORDERS_UPDATED, uAccountIndex, FALSE));
        return;
    }
    Json::Reader cReader;
    Json::Value cData;
    if (!cReader.parse(szResult, cData) || !cData.isObject()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_PENDING_ORDERS_UPDATED, uAccountIndex, FALSE));
        return;
    }
    const Json::Value& cOrders = cData["orders"];
    if (!cOrders.isArray()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_PENDING_ORDERS_UPDATED, uAccountIndex, FALSE));
        return;
    }
    COrderList& cPendingList = psAccount->cPendingOrderList;
    cPendingList.clear();
    
    const UINT uOrderNum = (UINT)cOrders.size();
    for (UINT uIndex = 0; uOrderNum > uIndex; ++uIndex) {
        const Json::Value& cOrder = cOrders[uIndex];
        const Json::Value& cOrderID = cOrder["id"];
        const Json::Value& cTime = cOrder["createTime"];
        const Json::Value& cState = cOrder["state"];
        const Json::Value& cType = cOrder["type"];
        if (!cOrderID.isUInt()) {
            ASSERT(FALSE);
            continue;
        }
        const UINT uOrderID = cOrderID.asUInt();
        if (!cTime.isString()) {
            ASSERT(FALSE);
            continue;
        }
        const UINT uTimeStamp = atoi(cTime.asCString());
        if (!cState.isString()) {
            ASSERT(FALSE);
            continue;
        }
        const CHAR* szOrderState = cState.asCString();
        EORDER_STATE eState;
        if (0 == strcmp("PENDING", szOrderState)) {
            eState = EORDER_PENDING;
        }
        else if (0 == strcmp("FILLED", szOrderState)) {
            eState = EORDER_FILLED;
        }
        else if (0 == strcmp("TRIGGERED", szOrderState)) {
            eState = EORDER_TRIGGERED;
        }
        else if (0 == strcmp("CANCELLED", szOrderState)) {
            eState = EORDER_CANCELLED;
        }
        else {
            ASSERT(FALSE);
            continue;
        }
        if (!cType.isString()) {
            ASSERT(FALSE);
            continue;
        }
        EORDER_TYPE eType;
        const CHAR* szType = cType.asCString();
        if (0 == strcmp("MARKET", szType)) {
            eType = EORDER_OANDA_MARKET;
        }
        else if (0 == strcmp("STOP_LOSS", szType)) {
            eType = EORDER_OANDA_STOP_LOSS;
        }
        else if (0 == strcmp("TRAILING_STOP_LOSS", szType)) {
            eType = EORDER_OANDA_TRAILING_STOP_LOSS;
        }
        else {
            ASSERT(FALSE);
            continue;
        }
        SOandaOrder sTmp = {
            uOrderID,   //            UINT            uOrderID;           // order ID
            uTimeStamp, //            UINT            uCreateTimestamp;   // timestamp of creation (seconds)
            eState,     //            EORDER_STATE    eState;             // current state of the order
            eType,      //            EORDER_TYPE     eType;
            0,          //            UINT            uTradePairIndex;    // which trading pair (only used by market order)
            0.0,        //            DOUBLE          dUnits;             // market order only
            0.0,        //            DOUBLE          dPrice;             // market order only
            0           //            UINT            uTradeID;           // trade id of the trade (for stop loss, trailing stop loss)
        };
        if (EORDER_OANDA_MARKET == eType) { // if market
            const Json::Value& cInstrument = cOrder["instrument"];
            const Json::Value& cUnits = cOrder["units"];
            const Json::Value& cPriceBound = cOrder["priceBound"];
            if (!cInstrument.isString() || !cUnits.isString() || !cPriceBound.isString()) {
                ASSERT(FALSE);
                continue;
            }
            const SOandaPair* psPair = GetTradePairByName(cInstrument.asCString());
            if (NULL == psPair) {
                TRACE("Error cannot find trade pair: %s\n", cInstrument.asCString());
                ASSERT(FALSE);
                continue;
            }
            sTmp.uTradePairIndex = psPair->uIndex;
            sTmp.dUnits = std::stod(cUnits.asCString());
            sTmp.dPrice = std::stod(cPriceBound.asCString());
        }
        else {
            ASSERT(FALSE);
            continue;
        }
        cPendingList.push_back(sTmp);
    }
    CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_PENDING_ORDERS_UPDATED, uAccountIndex, TRUE));
}


static VOID CreateMarketOrderReply(UINT uMsgID, UINT uAccountIndex, UINT uTradeID, FLOAT fPrice)
{
    CEvent cEvent(EGLOBAL_EVT_OANDA_CREATE_MARKET_ORDER_REPLY);
    cEvent.SetIntParam(1, uMsgID);
    cEvent.SetIntParam(2, uAccountIndex);
    cEvent.SetIntParam(3, uTradeID);
    cEvent.SetFloatParam(4, fPrice);
    CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, cEvent);

}
UINT COandaMgr::CreateMarketOrder(UINT uAccountIndex, UINT uTradePairIndex, INT nUnits)
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
    const SOandaPair* psPair = GetTradePair(uTradePairIndex);
    if (NULL == psPair) {
        ASSERT(FALSE);
        return 0;
    }
    
    static UINT s_CreateOrderRunningID = 1;
    const UINT uMsgID = s_CreateOrderRunningID;
    ++s_CreateOrderRunningID;
    CHAR szBuffer[1024];
    snprintf(szBuffer, 1024, "Bearer %s", szCredentials);
    Json::Value cHeaderFields;
    cHeaderFields["Authorization"] = szBuffer;
    cHeaderFields["Accept-Datetime-Format"] = "UNIX"; // unix time
    CHAR szURL[2048];
    snprintf(szURL, 2048, OANDA_CREATE_ORDER_URL, szAccountID);
    Json::Value* pcParams = new Json::Value();
    snprintf(szBuffer, 1024, "%d", nUnits);
    (*pcParams)["order"]["units"] = szBuffer;
    (*pcParams)["order"]["instrument"] = psPair->szTradePair;
    (*pcParams)["order"]["timeInForce"] = "FOK";
    (*pcParams)["order"]["type"] = "MARKET";
    (*pcParams)["order"]["positionFill"] = "DEFAULT";
    (*pcParams)["nounce"] = uAccountIndex;
    (*pcParams)["nounce2"] = uMsgID;
    CSimpleHttpClient::DoPost(szURL, *pcParams, EGLOBAL_OANDA_MGR, EGLOBAL_INTERNAL_OANDA_CREATE_MARKET_ORDER, &cHeaderFields);
    return uMsgID;
}

VOID COandaMgr::OnCreateMarketOrderReply(const CEvent& cEvent)
{
    const Json::Value* pcJson   = (const Json::Value*)cEvent.GetPtrParam(5);
    const UINT uAccountIndex    = (*pcJson)["nounce"].asUInt();
    const UINT uMsgID           = (*pcJson)["nounce2"].asUInt();
    const INT nErrCode          = cEvent.GetIntParam(6);
    const CHAR* szResult        = (const CHAR*) cEvent.GetPtrParam(7);
    CAutoPtr<const Json::Value> cJson(pcJson);
    CAutoPtrArray<const CHAR> cResult(szResult);

    if (CSimpleHttpClient::ESuccess != nErrCode) {
        CMessageLog::AddLog("Create Order Error");
        if (NULL != szResult) {
            CMessageLog::AddLog(szResult);
        }
        CreateMarketOrderReply(uMsgID, uAccountIndex, 0, 0.0f);
        return;
    }
    CHAR szBuffer[2048];
    
    UINT uTradeID = 0;
    Json::Reader cReader;
    Json::Value cData;
    FLOAT fResultPrice = 0.0f;
    if (!cReader.parse(szResult, cData) || !cData.isObject()) {
        ASSERT(FALSE);
        CMessageLog::AddLog("CO Err: fail make json");
    }
    else {
        const Json::Value& cOrderFillTransaction = cData["orderFillTransaction"];
        if (cOrderFillTransaction.isObject()) { // cannot find order fill transaction
            const Json::Value& cResultTradeID = cOrderFillTransaction["id"];
            if (cResultTradeID.isString()) {
                uTradeID = atoi(cResultTradeID.asCString());
            }
            else {
                CMessageLog::AddLog("CO Err: Missing TradeID");
            }
            const Json::Value& cResultPrice = cOrderFillTransaction["price"];
            if (cResultPrice.isString()) {
                fResultPrice = atof(cResultPrice.asCString());
            }
            else {
                CMessageLog::AddLog("CO Err: Missing Price");
            }
        }
        else {
            const Json::Value& cCancelTransaction = cData["orderCancelTransaction"];
            if (cCancelTransaction.isObject()) {
                const Json::Value& cReason = cCancelTransaction["reason"];
                if (cReason.isString()) {
                    snprintf(szBuffer, 2048, "CO Err Reason:%s", cReason.asCString());
                    CMessageLog::AddLog(szBuffer);
                }
            }
        }
    }
    const CHAR* szTradePair = "unknown";
    const CHAR* szUnits = "unknown";
    const Json::Value& cTradePair = (*pcJson)["order"]["instrument"];
    if (cTradePair.isString()) {
        szTradePair = cTradePair.asCString();
    }
    const Json::Value& cUnits = (*pcJson)["order"]["units"];
    if (cUnits.isString()) {
        szUnits = cUnits.asCString();
    }
    CreateMarketOrderReply(uMsgID, uAccountIndex, uTradeID, fResultPrice);
}
//
//VOID COandaMgr::CancelOrder(UINT uAccountIndex, UINT uOrderID)
//{
//    const CHAR* szAccountID = GetAccountID(uAccountIndex);
//    if (NULL == szAccountID) { // must be already signed in
//        ASSERT(FALSE);
//        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_CANCEL_ORDER_REPLY, uAccountIndex, FALSE));
//        return;
//    }
//    CHAR szCredentials[128];
//    if (!CPlayerData::GetOandaCredential(szCredentials, 128)) {
//        ASSERT(FALSE);
//        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_CANCEL_ORDER_REPLY, uAccountIndex, FALSE));
//        return;
//    }
//    CHAR szBuffer[1024];
//    snprintf(szBuffer, 1024, "[%s] Cancel Order:%d", szAccountID, uOrderID);
//    CMessageLog::AddLog(szBuffer);
//    
//    snprintf(szBuffer, 1024, "Bearer %s", szCredentials);
//    Json::Value cHeaderFields;
//    cHeaderFields["Authorization"] = szBuffer;
//    cHeaderFields["Accept-Datetime-Format"] = "UNIX"; // unix time
//    CHAR szURL[2048];
//    snprintf(szURL, 2048, OANDA_CANCEL_ORDER_URL, szAccountID, uOrderID);
//    Json::Value* pcParams = new Json::Value();
//    (*pcParams)["nounce"] = uAccountIndex;
//    CSimpleHttpClient::DoPut(szURL, *pcParams, EGLOBAL_OANDA_MGR, EGLOBAL_INTERNAL_OANDA_CANCEL_ORDER, &cHeaderFields);
//}
//
//VOID COandaMgr::OnCancelOrderReply(const CEvent& cEvent)
//{
//    const Json::Value* pcJson   = (const Json::Value*)cEvent.GetPtrParam(5);
//    const UINT uAccountIndex    = (*pcJson)["nounce"].asUInt();
//    const INT nErrCode          = cEvent.GetIntParam(6);
//    const CHAR* szResult        = (const CHAR*) cEvent.GetPtrParam(7);
//    CAutoPtr<const Json::Value> cJson(pcJson);
//    CAutoPtrArray<const CHAR> cResult(szResult);
//
//    if (CSimpleHttpClient::ESuccess != nErrCode) {
//        CMessageLog::AddLog("Cancel Order Error");
//        if (NULL != szResult) {
//            CMessageLog::AddLog(szResult);
//        }
//        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_CANCEL_ORDER_REPLY, uAccountIndex, FALSE));
//        return;
//    }
//    if (NULL != szResult) {
//        TRACE("Result:\n%s\n", szResult);
//    }
//    CMessageLog::AddLog("Cancel Order Success");
//    CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_CANCEL_ORDER_REPLY, uAccountIndex, TRUE));
//}
