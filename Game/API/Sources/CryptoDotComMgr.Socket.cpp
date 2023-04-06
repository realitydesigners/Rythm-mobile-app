#include "stdafx.h"
#include "CryptoDotComMgr.h"
#include "EventID.h"
#include "EventManager.h"
#include "HLTime.h"
#include "json.h"

#define USER_SOCKET_URL   "wss://stream.crypto.com/v2/user"
#define MARKET_SOCKET_URL "wss://stream.crypto.com/v2/market"

#define GET_ASSETS      "public/get-instruments"
#define ORDER_BOOK      "public/get-book"
#define GET_TRADES      "public/get-trades"

// candlestick.{interval}.{instrument_name}
#define CANDLESTICK_FORMAT "candlestick.%s.%s"

VOID CCryptoDotComMgr::OnMarketSocketConnected(VOID)
{
    CEventManager::BroadcastEvent(EGLOBAL_CRYPTO_DOT_COM_MGR, CEvent(EGLOBAL_EVT_CRYPTO_MARKET_SOCKET_CONNECT_REPLY), 1.0f);
}

VOID CCryptoDotComMgr::OnMarketSocketConnectFail(const CHAR* szReason)
{
    TRACE("CCryptoDotComMgr::OnMarketSocketConnectFail:%s\n", szReason);
    CEventManager::BroadcastEvent(EGLOBAL_CRYPTO_DOT_COM_MGR, CEvent(EGLOBAL_EVT_CRYPTO_MARKET_SOCKET_ERROR));
}
VOID CCryptoDotComMgr::OnMarketSocketClosed(const CHAR* szReason)
{
    TRACE("CCryptoDotComMgr::OnMarketSocketClosed:%s\n", szReason);
}

BOOLEAN CCryptoDotComMgr::IsMarketSocketConnected(VOID)
{
    return IsMarketSocketConnectedInternal();
}

VOID CCryptoDotComMgr::ConnectMarketSocket(VOID)
{
    TRACE("Opening Market Socket\n");
    ConnectMarketSocketInternal(MARKET_SOCKET_URL);
}

BOOLEAN CCryptoDotComMgr::SendMessageToMarket(const Json::Value& cParams)
{
    std::string cStyledString = cParams.toStyledString();
    return SendMessageToMarketInternal(cStyledString.c_str());
}

VOID CCryptoDotComMgr::OnReceiveMarketMessage(const CHAR* szMessage)
{
    Json::Value cValue;
    {
        Json::Reader cReader;
        if (!cReader.parse(szMessage, cValue)) {
            ASSERT(FALSE);
            return;
        }
    }
    if (!cValue.isObject()) {
        ASSERT(FALSE);
        return;
    }
    UINT64 ullID = 0;
    const Json::Value& cID = cValue["id"];
    if (cID.isUInt64()) {
        ullID = cID.asUInt64();
    }
    const Json::Value& cMethod = cValue["method"];
    if (!cMethod.isString()) {
        ASSERT(FALSE);
        return;
    }
    const CHAR* szMethod = cMethod.asCString();
    if (0 == strcmp("public/heartbeat", szMethod)) { // if system heartbeat.
        // reply heartbeat and finish processing
        Json::Value cParam;
        cParam["id"] = ullID;
        cParam["method"] = "public/respond-heartbeat";
        SendMessageToMarket(cParam);
        return;
    }
    const Json::Value& cResult = cValue["result"];
    if (!cResult.isObject()) {
        return;
    }

    if (0 == strcmp("subscribe", szMethod)) {
        ParseMarketSubscribeResult(ullID, cResult);
        return;
    }
    TRACE("Unknown reply: %s\n", szMethod);
    ASSERT(FALSE);
}

VOID CCryptoDotComMgr::ParseMarketSubscribeResult(UINT64 ullID, const Json::Value& cResult)
{
    const Json::Value& cSubscription = cResult["subscription"];
    if (!cSubscription.isString()) {
        TRACE("Result is missing subscription!\n");
        ASSERT(FALSE);
        return;
    }
    // set channel as subscribed (so we do not spam server)
    CCryptoUtil::SetSubscribedStatus(m_cSubscribedChannels, cSubscription.asString(), TRUE);
    
    const Json::Value& cKey = cResult["instrument_name"];
    if (!cKey.isString()) {
        TRACE("Result is missing instrument_name!\n");
        ASSERT(FALSE);
        return;
    }
    const CHAR* szKey = cKey.asCString();
    const Json::Value& cChannel = cResult["channel"];
    if (!cChannel.isString()) {
        TRACE("Result is missing channel!\n");
        ASSERT(FALSE);
        return;
    }
    const CHAR* szChannel = cChannel.asCString();
    if (0 == strcmp("candlestick", szChannel)) { // if candle stick channel
        const Json::Value& cInterval = cResult["interval"];
        if (!cInterval.isString()) {
            TRACE("Result is missing channel!\n");
            return;
        }
        const UINT uCandleIndex = GetCandleIndexFromDisplayName(cInterval.asCString());
//        TRACE("candlestick %s [%s]\n", szKey, GetCandleTypeDisplayName(uCandleIndex));
        OnQueryOHLC(cResult["data"], uCandleIndex, szKey);
        return;
    }
    TRACE("Unhandled channel: %s\n", szChannel);
    ASSERT(FALSE);
}

