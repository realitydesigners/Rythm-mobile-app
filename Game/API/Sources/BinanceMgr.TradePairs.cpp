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
#include "System.h"


static const CHAR* PRIORITY_PAIRS[] = {
    "BTC",
    "ETH",
    "LINK",
    "VET",
    "XRP",
    "BNB",
    "BAND",
    "NEO",
};
static const UINT PRIORITY_PAIR_NUM = sizeof(PRIORITY_PAIRS)/sizeof(const CHAR*);

UINT CBinanceMgr::GetTradePairNum(VOID)
{
    return m_uTradePairNum;
}

const STradePair* CBinanceMgr::GetTradePair(UINT uIndex)
{
    if (NULL == m_psTradePairs) {
        ASSERT(FALSE);
        return NULL;
    }
    if (m_uTradePairNum <= uIndex) {
        ASSERT(FALSE);
        return NULL;
    }
    return &m_psTradePairs[uIndex];
}


VOID CBinanceMgr::ClearTradePairs(VOID)
{
    if (NULL != m_psTradePairs) {
        for (UINT uIndex = 0; m_uTradePairNum > uIndex; ++uIndex) {
            STradePair& sPair = m_psTradePairs[uIndex];
            SAFE_DELETE_ARRAY(sPair.szTradeKey);
            SAFE_DELETE_ARRAY(sPair.szBase);
            SAFE_DELETE_ARRAY(sPair.szQuote);
        }
        SAFE_DELETE_ARRAY(m_psTradePairs);
    }
    m_uTradePairNum = 0;
}
BOOLEAN CBinanceMgr::AreTradePairsQueried(VOID) const
{
    return 0 < m_uTradePairNum;
}

BOOLEAN CBinanceMgr::QueryTradePairs(VOID)
{
    if (AreTradePairsQueried()) {
        CEventManager::BroadcastEvent(EGLOBAL_BINANCE_MGR, CEvent(EGLOBAL_EVT_BINANCE_TRADE_PAIR_UPDATE, TRUE));
        return TRUE;
    }
    ASSERT(NULL == m_psTradePairs);
    const Json::Value* pcParams = new Json::Value();
    CSimpleHttpClient::DoGet("https://api.binance.com/api/v3/exchangeInfo", *pcParams, EGLOBAL_BINANCE_MGR, EGLOBAL_EVT_BINANCE_ON_TRADE_PAIR_QUERIED);
    return TRUE;
}

VOID CBinanceMgr::OnQueryTradePairs(const CEvent& cEvent)
{
    const Json::Value* pcJson   = (const Json::Value*)cEvent.GetPtrParam(5);
    const INT nErrCode          = cEvent.GetIntParam(6);
    const CHAR* szResult        = (const CHAR*) cEvent.GetPtrParam(7);
    CAutoPtr<const Json::Value> cJson(pcJson);
    CAutoPtrArray<const CHAR> cResult(szResult);

    if (CSimpleHttpClient::ESuccess != nErrCode) {
        CHAR szBuffer[1024];
        snprintf(szBuffer, 1024, "Binance Err Code:%d", nErrCode);
        CConfirmationWindow::DisplayErrorMsg(szBuffer, NULL, 0);
        CEventManager::BroadcastEvent(EGLOBAL_BINANCE_MGR, CEvent(EGLOBAL_EVT_BINANCE_TRADE_PAIR_UPDATE, FALSE));
        return;
    }
    
    Json::Reader cReader;
    Json::Value cData;
    if (!cReader.parse(szResult, cData) || !cData.isObject()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_BINANCE_MGR, CEvent(EGLOBAL_EVT_BINANCE_TRADE_PAIR_UPDATE, FALSE));
        return;
    }
    const Json::Value& cInstruments = cData["symbols"];
    if (!cInstruments.isArray()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_BINANCE_MGR, CEvent(EGLOBAL_EVT_BINANCE_TRADE_PAIR_UPDATE, FALSE));
        return;
    }
    CTradePairMap cPairMap;
    const UINT uNum = (UINT)cInstruments.size();
    UINT uInsertedNum = 0;
    for (UINT uIndex = 0; uNum > uIndex; ++uIndex) {
        const Json::Value& cInstrument = cInstruments[uIndex];
        if (!cInstrument.isObject()) {
            ASSERT(FALSE);
            continue;
        }
        const Json::Value& cStatus = cInstrument["status"];
        if (!cStatus.isString()) {
            ASSERT(FALSE);
            continue;
        }
        if (0 == strcmp("BREAK", cStatus.asCString())) {
            continue;
        }
        const Json::Value& cTradeKey = cInstrument["symbol"];
        const Json::Value& cQuote = cInstrument["quoteAsset"];
        const Json::Value& cBase = cInstrument["baseAsset"];
        const Json::Value& cPriceDecimals = cInstrument["quotePrecision"];
        const Json::Value& cQuantityDecimals = cInstrument["baseAssetPrecision"];
        if (!cTradeKey.isString() || !cQuote.isString() || !cBase.isString() || !cPriceDecimals.isUInt() || !cQuantityDecimals.isUInt()) {
            ASSERT(FALSE);
            continue;
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
        cPairMap.insert(CTradePairPair(uInsertedNum, sTmp));
        ++uInsertedNum;
    }
    const UINT uPairNum = (UINT)cPairMap.size();
    ASSERT(uPairNum == uInsertedNum);
    m_psTradePairs = new STradePair[uPairNum];
    if (NULL == m_psTradePairs) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_BINANCE_MGR, CEvent(EGLOBAL_EVT_BINANCE_TRADE_PAIR_UPDATE, FALSE));
        return;
    }
    // now populate the array
    for (UINT uIndex = 0; uPairNum > uIndex; ++uIndex) {
        m_psTradePairs[uIndex] = cPairMap.find(uIndex)->second;
    }
    m_uTradePairNum = uPairNum;
    
    
    for (UINT uIndex = 0; PRIORITY_PAIR_NUM > uIndex; ++uIndex) {
        const CHAR* szPriorityPair = PRIORITY_PAIRS[uIndex];
        for (UINT uI = uIndex + 1; m_uTradePairNum > uI; ++uI) {
            if (0 == strcmp(szPriorityPair, m_psTradePairs[uI].szBase)) { // if found a match
                const STradePair sTmp = m_psTradePairs[uIndex];
                m_psTradePairs[uIndex] = m_psTradePairs[uI];
                m_psTradePairs[uI] = sTmp;
                break;
            }
        }
    }
    CEventManager::BroadcastEvent(EGLOBAL_BINANCE_MGR, CEvent(EGLOBAL_EVT_BINANCE_TRADE_PAIR_UPDATE, TRUE));
}
