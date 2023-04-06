#include "stdafx.h"
#include "CryptoUtil.h"
#include "HLTime.h"
#include "OandaMgr.h"

typedef std::pair<UINT, STradeCandle>               CTradeCandlePair;
typedef std::pair<UINT, SCandleIntervalData>        CCandleIntervalPair;
typedef std::pair<std::string, CCandleIntervalMap>  CCandlePairDataPair;


VOID CCryptoUtil::GeneratePriceDisplayString(ESOURCE_MARKET eMarket, UINT uTradePairIndex, DOUBLE dPrice, CHAR* szBuffer, UINT uBufferLen)
{
    if (ESOURCE_MARKET_OANDA == eMarket) {
        COandaMgr::GetInstance().GetPriceDisplayString(szBuffer, uBufferLen, uTradePairIndex, dPrice);
    }
    else {
        GenerateString(dPrice, szBuffer, uBufferLen);
    }
}
VOID CCryptoUtil::GenerateString(DOUBLE dValue, CHAR* szBuffer, UINT uBufferLen)
{
    CHAR szFormat[32] = { 0 };
    if (100000 <= dValue) {
        strcpy(szFormat, "%.0f");
    }
    else if (10000 <= dValue) {
        strcpy(szFormat, "%.0f");
    }
    else if (1000 <= dValue) {
        strcpy(szFormat, "%.1f");
    }
    else if (100 <= dValue) {
        strcpy(szFormat, "%.2f");
    }
    else if (10 <= dValue) {
        strcpy(szFormat, "%.3f");
    }
    else if (1 <= dValue) {
        strcpy(szFormat, "%.4f");
    }
    else if (0.1 <= dValue) {
        strcpy(szFormat, "%.5f");
    }
    else {
        strcpy(szFormat, "%.6f");
    }
    snprintf(szBuffer, uBufferLen, szFormat, dValue);
}
VOID CCryptoUtil::InsertTrade(CTradePairDataList& cList, const std::string& cKey, const STrade& sTrade)
{
    CTradeList* pcList = NULL;
    const CHAR* szKey = cKey.c_str();
    for (auto& sTradeList : cList) {
        if (0 == strcmp(sTradeList.cKey.c_str(), szKey)) {
            pcList = &sTradeList.cList;
        }
    }
    if (NULL == pcList) { // if not found
        const STradePairData sData = {
            cKey,
        };
        cList.push_back(sData);
        auto& sInserted = cList.back();
        ASSERT(0 == strcmp(sInserted.cKey.c_str(), cKey.c_str()));
        pcList = &sInserted.cList;
    }
    if (NULL == pcList) {
        ASSERT(FALSE);
        return;
    }
    pcList->push_back(sTrade);
}


SCandleIntervalData* CCryptoUtil::FindCandleInterval(CCandlePairDataMap& cMap, const std::string& cKey, UINT uInterval, BOOLEAN bSkipInsert)
{
    CCandlePairDataMap::iterator itFound = cMap.find(cKey);
    if (cMap.end() == itFound) { // if not found
        if (bSkipInsert) {
            return NULL;
        }
        CCandleIntervalMap cTmp;
        cMap.insert(CCandlePairDataPair(cKey, cTmp));
        itFound = cMap.find(cKey);
    }
    if (cMap.end() == itFound) { // if not found
        ASSERT(FALSE);
        return NULL;
    }
    CCandleIntervalMap& cCandleMap = itFound->second;
    CCandleIntervalMap::iterator itFound2 = cCandleMap.find(uInterval);
    if (cCandleMap.end() == itFound2) {
        if (bSkipInsert) {
            return NULL;
        }
        const SCandleIntervalData sTmp = {
            uInterval,  //                    UINT                uInterval;
            0,          //                    UINT                uLastQueryLocalTimeSecs;
            "0",        //                    std::string         m_cLastID;  // ID to continue querying new data
                        //                    CTradeCandleMap     cCandleMap; // map of the candles
        };
        cCandleMap.insert(CCandleIntervalPair(uInterval, sTmp));
        itFound2 = cCandleMap.find(uInterval);
    }
    if (cCandleMap.end() == itFound2) {
        ASSERT(FALSE);
        return NULL;
    }
    return &(itFound2->second);
}

VOID CCryptoUtil::InsertCandle(CTradeCandleMap& cMap, const STradeCandle& sCandle)
{
    cMap.insert(CTradeCandlePair(sCandle.uTimeSec, sCandle));
}

VOID CCryptoUtil::OnInsertCandleEnd(CTradeCandleMap& cMap, UINT uCandleDurationSec, UINT uLastCandleTimeSec)
{
    UINT uStartSec = 0xFFFFFFFF;
    for (auto itCandle : cMap) {
        const UINT uTimeSec = itCandle.first;
        if (uStartSec > uTimeSec) {
            uStartSec = uTimeSec;
        }
    }
    const UINT uNow = uLastCandleTimeSec;
    DOUBLE dPrevPrice = 0.0;
    for ( ; uNow > uStartSec; uStartSec += uCandleDurationSec) {
        CTradeCandleMap::iterator itFound = cMap.find(uStartSec);
        if (cMap.end() == itFound) {
            if (0.0 == dPrevPrice) {
                continue;
            }
            const STradeCandle sTmp = {
                uStartSec,  //                UINT    uTimeSec;
                0,          //                UINT    uTradeNum;  // number of trades
                dPrevPrice, //                DOUBLE  dOpen;      // opening price
                0,          //                DOUBLE  dOpenTime;  // opening time
                dPrevPrice, //                DOUBLE  dClose;     // closing price
                0,          //                DOUBLE  dCloseTime; // closing time
                dPrevPrice, //                DOUBLE  dHigh;      // highest price
                dPrevPrice, //                DOUBLE  dLow;       // lowest price
                dPrevPrice, //                DOUBLE  dVWAP;      // volume weighted average price
                0.0,        //                DOUBLE  dVolume;    // total volume traded
            };
            cMap.insert(CTradeCandlePair(uStartSec, sTmp));
            continue;
        }
        dPrevPrice = itFound->second.dClose;
    }
}

typedef std::pair<std::string,UINT>                 CSubscribedChannelPair;

BOOLEAN CCryptoUtil::IsChannelSubscribed(const CSubscribedChannelMap& cMap, const std::string& cChannel)
{
    return (cMap.find(cChannel) != cMap.end());
}

VOID CCryptoUtil::SetSubscribedStatus(CSubscribedChannelMap& cMap, const std::string& cChannel, BOOLEAN bSubscribed)
{
    if (bSubscribed) {
        cMap.insert(CSubscribedChannelPair(cChannel, 1));
    }
    else {
        cMap.erase(cChannel);
    }
}

UINT CCryptoUtil::FindFirstCandleTimeSec(const CTradeCandleMap& cCandleMap)
{
    UINT uTimeSec = 0xFFFFFFFF;
    for (auto& it : cCandleMap) {
        const UINT uCandleTime = it.first;
        if (uCandleTime < uTimeSec) {
            uTimeSec = uCandleTime;
        }
    }
    return uTimeSec;
}

BOOLEAN CCryptoUtil::FindHighLowClosing(DOUBLE& dHigh, DOUBLE dLow, const CTradeCandleMap& cCandleMap)
{
    BOOLEAN bInitialized = FALSE;
    for(auto itCandle : cCandleMap) {
        const STradeCandle& sCandle = itCandle.second;
        if (!bInitialized) {
            bInitialized = TRUE;
            dHigh = dLow = sCandle.dClose;
            continue;
        }
        if (sCandle.dClose > dHigh) {
            dHigh = sCandle.dClose;
        }
        else if (sCandle.dClose < dLow) {
            dLow = sCandle.dClose;
        }
    }
    return bInitialized;
}

