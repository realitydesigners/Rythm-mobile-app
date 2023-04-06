#include "stdafx.h"
#include "AutoPtr.h"
#include "ConfirmationWindow.h"
#include "CryptoDotComMgr.h"
#include "CryptoUtil.h"
#include "Event.h"
#include "EventID.h"
#include "EventManager.h"
#include "HLTime.h"
#include "json.h"
#include "SimpleHttpClient.h"
#include "System.h"

UINT CCryptoDotComMgr::GetTradePairNum(VOID)
{
    return (UINT)m_cTradePairMap.size();
}

const STradePair* CCryptoDotComMgr::GetTradePair(UINT uIndex)
{
    const UINT uNum = GetTradePairNum();
    if (0 == uNum) {
        ASSERT(FALSE);
        return NULL;
    }
    uIndex %= uNum;
    const auto itFound = m_cTradePairMap.find(uIndex);
    if (m_cTradePairMap.end() == itFound) {
        ASSERT(FALSE);
        return NULL;
    }
    return &(itFound->second);
}

UINT CCryptoDotComMgr::GetTradePairIndex(const CHAR* szTradeKey)
{
    for (const auto& it : m_cTradePairMap) {
        if (0 == strcmp(it.second.szTradeKey, szTradeKey)) {
            return it.first;
        }
    }
    ASSERT(FALSE);
    return 0;
}


VOID CCryptoDotComMgr::ClearTradePairs(VOID)
{
    for (auto itPair : m_cTradePairMap) {
        STradePair& sPair = itPair.second;
        SAFE_DELETE_ARRAY(sPair.szTradeKey);
        SAFE_DELETE_ARRAY(sPair.szBase);
        SAFE_DELETE_ARRAY(sPair.szQuote);
    }
    m_cTradePairMap.clear();
}
BOOLEAN CCryptoDotComMgr::AreTradePairsQueried(VOID) const
{
    return 0 < m_cTradePairMap.size();
}

BOOLEAN CCryptoDotComMgr::QueryTradePairs(VOID)
{
    // WS version not working!
//    Json::Value cParam;
//    cParam["id"] = 1;
//    cParam["method"] = "public/get-instruments";
//    cParam["nonce"] = CHLTime::GetTimeMilliSecs();
//    if (!SendMessageToUser(cParam)) {
//        return FALSE;
//    }
    const Json::Value* pcParams = new Json::Value();
    CSimpleHttpClient::DoGet("https://api.crypto.com/v2/public/get-instruments", *pcParams, EGLOBAL_CRYPTO_DOT_COM_MGR, EGLOBAL_EVT_CRYPTO_ON_TRADE_PAIR_QUERIED);
    return TRUE;
}

VOID CCryptoDotComMgr::OnQueryTradePairs(const CEvent& cEvent)
{
    const Json::Value* pcJson   = (const Json::Value*)cEvent.GetPtrParam(5);
    const INT nErrCode          = cEvent.GetIntParam(6);
    const CHAR* szResult        = (const CHAR*) cEvent.GetPtrParam(7);
    CAutoPtr<const Json::Value> cJson(pcJson);
    CAutoPtrArray<const CHAR> cResult(szResult);

    if (CSimpleHttpClient::ESuccess != nErrCode) {
        CHAR szBuffer[1024];
        snprintf(szBuffer, 1024, "Crypto.com Err Code:%d", nErrCode);
        CConfirmationWindow::DisplayErrorMsg(szBuffer, NULL, 0);
        CEventManager::BroadcastEvent(EGLOBAL_CRYPTO_DOT_COM_MGR, CEvent(EGLOBAL_EVT_CRYPTO_TRADE_PAIR_UPDATE, FALSE));
        return;
    }
    
    Json::Reader cReader;
    Json::Value cData;
    if (!cReader.parse(szResult, cData) || !cData.isObject()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_CRYPTO_DOT_COM_MGR, CEvent(EGLOBAL_EVT_CRYPTO_TRADE_PAIR_UPDATE, FALSE));
        return;
    }
    const Json::Value& cInstruments = cData["result"]["instruments"];
    if (!cInstruments.isArray()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_CRYPTO_DOT_COM_MGR, CEvent(EGLOBAL_EVT_CRYPTO_TRADE_PAIR_UPDATE, FALSE));
        return;
    }
    const UINT uNum = (UINT)cInstruments.size();
    UINT uInsertedNum = 0;
    for (UINT uIndex = 0; uNum > uIndex; ++uIndex) {
        const Json::Value& cInstrument = cInstruments[uIndex];
        if (!cInstrument.isObject()) {
            ASSERT(FALSE);
            return;
        }
        const Json::Value& cTradeKey = cInstrument["instrument_name"];
        const Json::Value& cQuote = cInstrument["quote_currency"];
        const Json::Value& cBase = cInstrument["base_currency"];
        const Json::Value& cPriceDecimals = cInstrument["price_decimals"];
        const Json::Value& cQuantityDecimals = cInstrument["quantity_decimals"];
        if (!cTradeKey.isString() || !cQuote.isString() || !cBase.isString() || !cPriceDecimals.isUInt() || !cQuantityDecimals.isUInt()) {
            ASSERT(FALSE);
            return;
        }
        const CHAR* szQuote = cQuote.asCString();        
        if (0 != strcmp("USDT", szQuote)) { // skip non usdt pairs
            continue;
        }
        CHAR* szTradeKey = CSystem::CloneString(cTradeKey.asCString());
        CHAR* szNewQuote = CSystem::CloneString(szQuote);
        CHAR* szBase = CSystem::CloneString(cBase.asCString());
        const STradePair sTmp = {
            szTradeKey,     //            const CHAR* szTradeKey;
            szBase,         //            const CHAR* szBase;
            szNewQuote,     //            const CHAR* szQuote;
            cPriceDecimals.asUInt(),    //            UINT uPriceDecimals;
            cQuantityDecimals.asUInt()  //            UINT uQuantityDecimals;
        };
        m_cTradePairMap.insert(CTradePairPair(uInsertedNum, sTmp));
        ++uInsertedNum;
    }
    CEventManager::BroadcastEvent(EGLOBAL_CRYPTO_DOT_COM_MGR, CEvent(EGLOBAL_EVT_CRYPTO_TRADE_PAIR_UPDATE, TRUE));
}

VOID CCryptoDotComMgr::OnReceiveEvent(CEvent& cEvent)
{
    switch (cEvent.GetIntParam(0)) {
        case EGLOBAL_EVT_CRYPTO_ON_TRADE_PAIR_QUERIED:
            OnQueryTradePairs(cEvent);
            break;
        default:
            break;
    }
}
