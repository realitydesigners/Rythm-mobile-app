#include "stdafx.h"
#include "CryptoUtil.h"

#include "BinanceMgr.h"
#include "CryptoDotComMgr.h"
#include "OandaMgr.h"

static const CHAR* s_szDefaultPairName = "Pending";


const CHAR* CCryptoUtil::GetTradePairName(ESOURCE_MARKET eSource, UINT uTradeIndex, BOOLEAN bShortForm)
{
    const CHAR* szTradePair = NULL;
    switch (eSource) {
        case ESOURCE_MARKET_CRYPTO_COM:
            if (CCryptoDotComMgr::GetInstance().AreTradePairsQueried()) {
                const STradePair* psPair = CCryptoDotComMgr::GetInstance().GetTradePair(uTradeIndex);
                if (NULL != psPair) {
                    if (bShortForm) {
                        szTradePair = psPair->szBase;
                    }
                    else {
                        szTradePair = psPair->szTradeKey;
                    }
                }
            }
            break;
        case ESOURCE_MARKET_BINANCE:
            if (CBinanceMgr::GetInstance().AreTradePairsQueried()) {
                const STradePair* psPair = CBinanceMgr::GetInstance().GetTradePair(uTradeIndex);
                if (NULL != psPair) {
                    if (bShortForm) {
                        szTradePair = psPair->szBase;
                    }
                    else {
                        szTradePair = psPair->szTradeKey;
                    }
                }
            }
            break;
        case ESOURCE_MARKET_OANDA:
            if (COandaMgr::GetInstance().AreTradePairsQueried()) {
                const COandaMgr::SOandaPair* psPair = COandaMgr::GetInstance().GetTradePair(uTradeIndex);
                if (NULL != psPair) {
                    szTradePair = psPair->szTradePair;
                }
            }
            break;
        default:
            ASSERT(FALSE);
            break;
    }
    if (NULL == szTradePair) {
        szTradePair = s_szDefaultPairName;
    }
    return szTradePair;
}
