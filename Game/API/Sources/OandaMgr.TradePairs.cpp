#include "stdafx.h"
#include "AutoPtr.h"
#include "ConfirmationWindow.h"
#include "EventID.h"
#include "EventManager.h"
#include "json.h"
#include "OandaMgr.h"
#include "PlayerData.h"
#include "SimpleHttpClient.h"
#include <string.h>
#include <math.h>

#define OANDA_INSTRUMENT_URL    OANDA_MAIN_URL "v3/accounts/%s/instruments"

static const CHAR* PRIORITY_PAIRS[] = {
    "AUD_USD",
    "AUD_JPY",
    "CAD_JPY",
    "EUR_JPY",
//    "GBP_JPY",
    "GBP_USD",
    "NZD_JPY",
    "NZD_USD",
    "USD_CAD",
//    "USD_CHF",
    "USD_JPY",
    "XAU_USD",
};
static const UINT PRIORITY_PAIR_NUM = sizeof(PRIORITY_PAIRS)/sizeof(const CHAR*);


BOOLEAN COandaMgr::AreTradePairsQueried(VOID) const
{
    return 0 < m_uTradePairNum;
}

UINT COandaMgr::GetTradePairNum(VOID)
{
    return m_uTradePairNum;
}

const COandaMgr::SOandaPair* COandaMgr::GetTradePair(UINT uIndex)
{
    if (NULL == m_psTradePairs) {
        ASSERT(FALSE);
        return NULL;
    }
    return &m_psTradePairs[uIndex];
}
const COandaMgr::SOandaPair* COandaMgr::GetTradePairByName(const CHAR* szName)
{
    if (NULL == m_psTradePairs) {
        ASSERT(FALSE);
        return NULL;
    }
    for (UINT uIndex = 0; m_uTradePairNum > uIndex; ++uIndex) {
        if (0 == strcmp(m_psTradePairs[uIndex].szTradePair, szName)) {
            return &m_psTradePairs[uIndex];
        }
    }
    return NULL;
}

VOID COandaMgr::UpdateTradePairPrice(UINT uTradePairIndex, DOUBLE dPrice)
{
    if (NULL == m_psTradePairs) {
        ASSERT(FALSE);
        return;
    }
    if (m_uTradePairNum <= uTradePairIndex) {
        ASSERT(FALSE);
        return;
    }
    m_psTradePairs[uTradePairIndex].dPrice = dPrice;
}
VOID COandaMgr::QueryTradePairs(VOID)
{
    const CHAR* szAccountID = GetAccountID(0);
    if (NULL == szAccountID) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_TRADE_PAIR_QUERY, FALSE));
        return;
    }
    if (AreTradePairsQueried()) {
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_TRADE_PAIR_QUERY, TRUE));
        return;
    }
    CHAR szCredentials[128];
    if (!CPlayerData::GetOandaCredential(szCredentials, 128) || 0 == szCredentials[0]) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_TRADE_PAIR_QUERY, FALSE));
        return;
    }

    CHAR szBuffer[1024];
    snprintf(szBuffer, 1024, "Bearer %s", szCredentials);
    Json::Value cHeaderFields;
    cHeaderFields["Authorization"] = szBuffer;    
    CHAR szURL[1024];
    snprintf(szURL, 1024, OANDA_INSTRUMENT_URL, szAccountID);
    const Json::Value* pcParams = new Json::Value();
    CSimpleHttpClient::DoGet(szURL, *pcParams, EGLOBAL_OANDA_MGR, EGLOBAL_INTERNAL_OANDA_GET_TRADE_PAIRS, &cHeaderFields);
}


VOID COandaMgr::OnGetTradePairsReply(const CEvent& cEvent)
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
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_TRADE_PAIR_QUERY, FALSE));
        return;
    }
    Json::Reader cReader;
    Json::Value cData;
    if (!cReader.parse(szResult, cData) || !cData.isObject()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_TRADE_PAIR_QUERY, FALSE));
        return;
    }
    const Json::Value& cInstruments = cData["instruments"];
    if (!cInstruments.isArray()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_TRADE_PAIR_QUERY, FALSE));
        return;
    }
    const UINT uTotalInstrumentNum = (UINT)cInstruments.size();
    UINT uCurrencyNum = 0;
    for (UINT uIndex = 0; uTotalInstrumentNum > uIndex; ++uIndex) {
        const Json::Value& cType = cInstruments[uIndex]["type"];
        if (!cType.isString()) {
            ASSERT(FALSE);
            continue;
        }
        const CHAR* szType = cType.asCString();
        if (0 == strcmp("CURRENCY", szType) || 0 == strcmp("METAL", szType)) {
            ++uCurrencyNum;
        }
    }
    SAFE_DELETE_ARRAY(m_psTradePairs); // safety
    m_psTradePairs = new SOandaPair[uCurrencyNum];
    if (NULL == m_psTradePairs) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_TRADE_PAIR_QUERY, FALSE));
        return;
    }
    memset(m_psTradePairs, 0, sizeof(SOandaPair) * uCurrencyNum);
    UINT uAddedNum = 0;
    for (UINT uIndex = 0; uTotalInstrumentNum > uIndex; ++uIndex) {
        const Json::Value& cPair = cInstruments[uIndex];
        const Json::Value& cType = cPair["type"];
        if (!cType.isString()) {
            ASSERT(FALSE);
            continue;
        }
        const CHAR* szType = cType.asCString();
        if (0 == strcmp("CURRENCY", szType) || 0 == strcmp("METAL", szType)) {
            const Json::Value& cName = cPair["name"];
            const CHAR* szName = cName.asCString();
            CHAR* szNewName = new CHAR[strlen(szName) + 1];
            if (NULL == szNewName) {
                ASSERT(FALSE);
                continue;
            }
            strcpy(szNewName, szName);
            SOandaPair& sPair = m_psTradePairs[uAddedNum];
            sPair.szTradePair = szNewName;
            sPair.uIndex = uAddedNum;
            const Json::Value& cPipLocation = cPair["pipLocation"];
            if (!cPipLocation.isInt()) {
                ASSERT(FALSE);
                continue;
            }
            const INT nPipLocation = cPipLocation.asInt();
            const DOUBLE dPipMultiplier = pow(10.0, (DOUBLE)nPipLocation);
            sPair.dPipMultiplier = 1.0f / dPipMultiplier;
            const Json::Value& cDisplayPrecision = cPair["displayPrecision"];
            if (!cDisplayPrecision.isUInt()) {
                ASSERT(FALSE);
                continue;
            }
            sPair.uDisplayPrecision = cDisplayPrecision.asUInt();
            const Json::Value& cMinTradeSize = cPair["minimumTradeSize"];
            if (!cMinTradeSize.isString()) {
                ASSERT(FALSE);
                continue;
            }
            sPair.uMinTradeSize = atoi(cMinTradeSize.asCString());
            const Json::Value& cMaxOrderUnits = cPair["maximumOrderUnits"];
            if (!cMaxOrderUnits.isString()) {
                ASSERT(FALSE);
                continue;
            }
            sPair.uMaxOrderUnits = atoi(cMaxOrderUnits.asCString());
            const Json::Value& cMinTrailStop = cPair["minimumTrailingStopDistance"];
            if (!cMinTrailStop.isString()) {
                ASSERT(FALSE);
                continue;
            }
            sPair.dMinTrailingStop = std::stod(cMinTrailStop.asCString());
            sPair.dPrice = 0.0;
            sPair.fAskPrice = 0.0f;
            sPair.fBidPrice = 0.0f;
            ++uAddedNum;
        }
    }
    ASSERT(uAddedNum == uCurrencyNum);
    m_uTradePairNum = uAddedNum;
    
    // move default 6 to the 1s 6 index
    for (UINT uIndex = 0; PRIORITY_PAIR_NUM > uIndex; ++uIndex) {
        const CHAR* szPriorityPair = PRIORITY_PAIRS[uIndex];
        for (UINT uI = uIndex + 1; m_uTradePairNum > uI; ++uI) {
            if (0 == strcmp(szPriorityPair, m_psTradePairs[uI].szTradePair)) { // if found a match
                ASSERT(m_psTradePairs[uI].uIndex == uI);
                ASSERT(m_psTradePairs[uIndex].uIndex == uIndex);
                const SOandaPair sTmp = m_psTradePairs[uI];
                m_psTradePairs[uI] = m_psTradePairs[uIndex];
                m_psTradePairs[uIndex] = sTmp;
                m_psTradePairs[uI].uIndex = uI;
                m_psTradePairs[uIndex].uIndex = uIndex;
                break;
            }
        }
    }
    
    CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_TRADE_PAIR_QUERY, TRUE));
}
