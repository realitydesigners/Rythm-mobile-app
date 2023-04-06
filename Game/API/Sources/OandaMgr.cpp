#include "stdafx.h"
#include "OandaMgr.h"
#include "Event.h"
#include "EventID.h"

static COandaMgr* s_pcInst = NULL;

VOID COandaMgr::InitInstance(VOID)
{
    ASSERT(NULL == s_pcInst);
    s_pcInst = new COandaMgr();
}
VOID COandaMgr::ReleaseInstance(VOID)
{
    SAFE_RELEASE_DELETE(s_pcInst);
}

COandaMgr& COandaMgr::GetInstance(VOID)
{
    ASSERT(NULL != s_pcInst);
    return *s_pcInst;
}


static const SCandleIntervalDisplayData s_asCandles[] = {
    { 1,        "M1"    },
    { 5,        "M5"    },
    { 15,       "M15"   },
    { 30,       "M30"   },
    { 60,       "H1"    },
    { 120,      "H2"    },
    { 240,      "H4"    },
    { 360,      "H6"    },
    { 480,      "H8"    },
    { 720,      "H12"   },
    { 1440,     "D"     }
};
static const UINT s_uCandleIntervalNum = sizeof(s_asCandles)/sizeof(SCandleIntervalDisplayData);

UINT COandaMgr::GetCandleTypeNum(VOID)
{
    return s_uCandleIntervalNum;
}
const SCandleIntervalDisplayData* COandaMgr::GetAllCandleDisplayData(VOID)
{
    return s_asCandles;
}
UINT COandaMgr::GetCandleDurationMins(UINT uIndex)
{
    return s_asCandles[uIndex % s_uCandleIntervalNum].uTimeMins;
}
const CHAR* COandaMgr::GetCandleTypeDisplayName(UINT uIndex)
{
    return s_asCandles[uIndex % s_uCandleIntervalNum].szDisplayName;
}
UINT COandaMgr::GetCandleIndexFromDisplayName(const CHAR* szName)
{
    UINT uIndex = 0;
    for ( ; s_uCandleIntervalNum > uIndex; ++uIndex) {
        if (0 == strcmp(s_asCandles[uIndex].szDisplayName, szName)) {
            return uIndex;
        }
    }
    ASSERT(FALSE);
    return 0; // safety return
}

COandaMgr::COandaMgr() :
CEventHandler(EGLOBAL_OANDA_MGR),
m_psAccounts(NULL),
m_uAccountNum(0),
m_psTradePairs(NULL),
m_uTradePairNum(0)
{
    
}

COandaMgr::~COandaMgr()
{

}

VOID COandaMgr::Release(VOID)
{
    ReleaseAccounts();
    
    // clear tradable pairs
    if (NULL != m_psTradePairs) {
        for (UINT uIndex = 0; m_uTradePairNum > uIndex; ++uIndex) {
            delete [] m_psTradePairs[uIndex].szTradePair;
        }
        SAFE_DELETE_ARRAY(m_psTradePairs);
        m_uTradePairNum = 0;
    }
    // clear candle map
    ClearCandleMap();
}

VOID COandaMgr::ReleaseAccounts(VOID)
{
    if (NULL != m_psAccounts) {
        for (UINT uIndex = 0; m_uAccountNum > uIndex; ++uIndex) {
            SOandaAccount& sAcct = m_psAccounts[uIndex];
            SAFE_DELETE_ARRAY(sAcct.szAccountID);
            SAFE_DELETE_ARRAY(sAcct.szCurrency);
        }
        SAFE_DELETE_ARRAY(m_psAccounts);
        m_uAccountNum = 0;
    }
}

VOID COandaMgr::Update(FLOAT fLapsed)
{
    UpdateRythms(fLapsed);
}

VOID COandaMgr::GetPriceDisplayString(CHAR* szBuffer, UINT uBufferLen, UINT uTradePairIndex, DOUBLE dPrice)
{
    const SOandaPair* psPair = GetTradePair(uTradePairIndex);
    if (NULL == psPair) {
        ASSERT(FALSE);
        snprintf(szBuffer, uBufferLen, "%.5f", dPrice);
        return;
    }
    switch (psPair->uDisplayPrecision) {
        case 5:
            snprintf(szBuffer, uBufferLen, "%.5f", dPrice);
            break;
        case 4:
            snprintf(szBuffer, uBufferLen, "%.4f", dPrice);
            break;
        case 3:
            snprintf(szBuffer, uBufferLen, "%.3f", dPrice);
            break;
        case 2:
            snprintf(szBuffer, uBufferLen, "%.2f", dPrice);
            break;
        case 1:
            snprintf(szBuffer, uBufferLen, "%.1f", dPrice);
            break;
        case 0:
            snprintf(szBuffer, uBufferLen, "%.0f", dPrice);
            break;
        default:
            snprintf(szBuffer, uBufferLen, "%.5f", dPrice);
            break;
    }
}
VOID COandaMgr::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EGLOBAL_INTERNAL_OANDA_ENTER_CREDENTIAL:
            OnCredentialInput();
            break;
        case EGLOBAL_INTERNAL_OANDA_GET_ACCOUNT:
            OnGetAccountReply(cEvent);
            break;
        case EGLOBAL_INTERNAL_OANDA_GET_ACCOUNT_DETAILS:
            OnGetAccountDetailsReply(cEvent);
            break;
        case EGLOBAL_INTERNAL_OANDA_GET_PENDING_ORDERS:
            OnGetPendingOrdersReply(cEvent);
            break;
        case EGLOBAL_INTERNAL_OANDA_CREATE_MARKET_ORDER:
            OnCreateMarketOrderReply(cEvent);
            break;
//        case EGLOBAL_INTERNAL_OANDA_CANCEL_ORDER:
//            OnCancelOrderReply(cEvent);
//            break;
        case EGLOBAL_INTERNAL_OANDA_GET_OPEN_TRADES:
            OnGetOpenTradesReply(cEvent);
            break;
        case EGLOBAL_INTERNAL_OANDA_CLOSE_TRADE:
            OnCloseTradeReply(cEvent);
            break;
        case EGLOBAL_INTERNAL_OANDA_GET_TRADE_PAIRS:
            OnGetTradePairsReply(cEvent);
            break;
        case EGLOBAL_INTERNAL_OANDA_GET_OHLC:
            OnGetOHLCReply(cEvent);
            break;
        case EGLOBAL_INTERNAL_OANDA_GET_RYTHM:
            OnRythmFetched(cEvent);
            break;
        case EGLOBAL_INTERNAL_OANDA_GET_SPREADS:
            OnGetSpreadsReply(cEvent);
            break;
            
        default:
            break;
    }
}

