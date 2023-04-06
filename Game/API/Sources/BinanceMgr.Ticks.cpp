#include "stdafx.h"
#include "AutoPtr.h"
#include "BinanceMgr.h"
#include "CryptoUtil.h"
#include "Event.h"
#include "EventID.h"
#include "EventManager.h"
#include "HLTime.h"
#include "json.h"
#include "SimpleHttpClient.h"

#include <math.h>
#include <memory.h>

//#include "SoundManager.h"

// FORMULA for average price = Quote Asset Volume / Volume

static VOID RefreshMinuteTicker(STickListener& sTick, const STick& sNewTick)
{
    CTickList& cList = sTick.cMinuteList;
    if (TICK_HISTORY_MINS != cList.size()) {
        ASSERT(FALSE);
        return;
    }
    CTickList::reverse_iterator itrTick = cList.rbegin();
    STick& sLastTick = *(itrTick);
    if (sLastTick.uOpenSec > sNewTick.uOpenSec) {
        ASSERT((sLastTick.uOpenSec - sNewTick.uOpenSec) == 60);
        return;
    }
    if (sLastTick.uOpenSec < sNewTick.uOpenSec) { // insert a new tick into the list
        if (60 != (sNewTick.uOpenSec - sLastTick.uOpenSec)) {
            ASSERT(FALSE);
            return;
        }
        const DOUBLE dPriceDiff = sNewTick.dAvgPrice - sLastTick.dAvgPrice;
        const DOUBLE dOBVDiff = dPriceDiff * sNewTick.dTotalVolume;
        const DOUBLE dOBV = sLastTick.dOBV + dOBVDiff;

        const STick sTmp = {
            sNewTick.dCLosePrice,        //            DOUBLE  dCLosePrice;        // closing price
            sNewTick.dAvgPrice,          //            DOUBLE  dAvgPrice;          // average price in this tick
            sNewTick.dActualVolume,      //            DOUBLE  dActualVolume;      // actual volume of this tick
            sNewTick.dTotalVolume,       //            DOUBLE  dTotalVolume;       // total volume for candle
            sNewTick.dTotalQuoteVolume,  //            DOUBLE  dTotalQuoteVolume;  // total quote volume for candle
            dOBV,                        //            DOUBLE  dOBV;               // OBV
            dOBVDiff,                    //            DOUBLE  dOBVDiff;           // time related
            sNewTick.uOpenSec,           //            UINT    uOpenSec;           // time when tick opened
            sNewTick.uCloseSec,          //            UINT    uCloseSec;          // time of latest update
        };
        cList.push_back(sTmp);
        cList.pop_front();
        ASSERT(TICK_HISTORY_MINS == cList.size());
        return;
    }
    ASSERT(sLastTick.uOpenSec == sNewTick.uOpenSec);
    ++itrTick;
    const STick& sPrevLastTick = *(itrTick);
    
    const DOUBLE dPriceDiff = sNewTick.dAvgPrice - sPrevLastTick.dAvgPrice;
    const DOUBLE dOBVDiff = dPriceDiff * sNewTick.dTotalVolume;
    const DOUBLE dOBV = sPrevLastTick.dOBV + dOBVDiff;

    const STick sTmp = {
        sNewTick.dCLosePrice,        //            DOUBLE  dCLosePrice;        // closing price
        sNewTick.dAvgPrice,          //            DOUBLE  dAvgPrice;          // average price in this tick
        sNewTick.dActualVolume,      //            DOUBLE  dActualVolume;      // actual volume of this tick
        sNewTick.dTotalVolume,       //            DOUBLE  dTotalVolume;       // total volume for candle
        sNewTick.dTotalQuoteVolume,  //            DOUBLE  dTotalQuoteVolume;  // total quote volume for candle
        dOBV,                        //            DOUBLE  dOBV;               // OBV
        dOBVDiff,                    //            DOUBLE  dOBVDiff;           // time related
        sNewTick.uOpenSec,           //            UINT    uOpenSec;           // time when tick opened
        sNewTick.uCloseSec,          //            UINT    uCloseSec;          // time of latest update

    };

    cList.pop_back();
    cList.push_back(sTmp);
    ASSERT(TICK_HISTORY_MINS == cList.size());
}
static BOOLEAN GetTickFromJson(STick& sTmp, const Json::Value& cCandle, DOUBLE dPrevAvgPrice, DOUBLE dPrevOBV)
{
    if (!cCandle.isArray()) {
        ASSERT(FALSE);
        return FALSE;
    }
    const Json::Value& cTime    = cCandle[0];
    const Json::Value& cOpen    = cCandle[1];
    const Json::Value& cHigh    = cCandle[2];
    const Json::Value& cLow     = cCandle[3];
    const Json::Value& cClose   = cCandle[4];
    const Json::Value& cVol     = cCandle[5];
    const Json::Value& cCloseTime           = cCandle[6];
    const Json::Value& cQuoteAssetVolume    = cCandle[7];
    const Json::Value& cCnt                 = cCandle[8];
    if (!cTime.isUInt64() || !cOpen.isString() || !cHigh.isString() || !cLow.isString() || !cClose.isString()
        || !cCloseTime.isUInt64() || !cQuoteAssetVolume.isString() || !cVol.isString() || !cCnt.isUInt()) {
        ASSERT(FALSE);
        return FALSE;
    }
    const DOUBLE dVolume = std::stod(cVol.asString());
    const DOUBLE dQuoteAssetVolume = std::stod(cQuoteAssetVolume.asString());
    DOUBLE dAvgPrice;
    if (0.0 == dVolume) {
        dAvgPrice = dPrevAvgPrice;
    }
    else {
        dAvgPrice = dQuoteAssetVolume / dVolume;
    }
    if (0.0 == dPrevAvgPrice) {
        dPrevAvgPrice = dAvgPrice;
    }
    const DOUBLE dPriceDiff = dAvgPrice - dPrevAvgPrice;
    const DOUBLE dOBVDiff = dPriceDiff * dVolume;
    const DOUBLE dOBV = dPrevOBV + dOBVDiff;

    sTmp.dCLosePrice = std::stod(cClose.asString());
    sTmp.dAvgPrice = dAvgPrice;
    sTmp.dActualVolume = dVolume;
    sTmp.dTotalVolume = dVolume;
    sTmp.dTotalQuoteVolume = dQuoteAssetVolume;
    sTmp.dOBV = dOBV;
    sTmp.dOBVDiff = dOBVDiff;
    
    const UINT uOpenSec = (UINT)(cTime.asUInt64() / 1000);
    const UINT uCloseSec = (UINT)(cCloseTime.asUInt64() / 1000);

    sTmp.uOpenSec = uOpenSec;
    sTmp.uCloseSec = uCloseSec;
    return TRUE;
}
static const STick* FindTick(const CTickList& cMinuteList, UINT uTimeSec)
{
    for (auto& sTick : cMinuteList) {
        if (sTick.uOpenSec == uTimeSec) {
            return &sTick;
        }
    }
    return NULL;
}
static VOID ProcessTick(STickListener& sTick, const STick& sSecond)
{
    CTickList& cList = sTick.cSecondList;
    // if there was no data in the list
    if (0 == cList.size()) {
        // only process the 2nd candle. (We start from there)
        // find the minute candle and use it instead
        const STick* psUpdated = FindTick(sTick.cMinuteList, sSecond.uOpenSec);
        if (NULL == psUpdated) {
            ASSERT(FALSE);
            return;
        }
        cList.push_back(*psUpdated);
        return;
    }
    // already exists data, so we check if we need to insert
    CTickList::reverse_iterator itCandle = cList.rbegin();
    const STick& sLastTick = (*itCandle);
    if (sLastTick.uOpenSec < sSecond.uOpenSec) {    // if candle has moved, clear second list
        cList.clear();
        const STick* psUpdated = FindTick(sTick.cMinuteList, sSecond.uOpenSec);
        if (NULL == psUpdated) {
            ASSERT(FALSE);
            return;
        }
        cList.push_back(*psUpdated);
        return;
    }
    ASSERT(sLastTick.uOpenSec == sSecond.uOpenSec); // FACT!
    STick sTmp = sLastTick;            // copy over all data 1st
    if (sSecond.dTotalVolume > sLastTick.dTotalVolume) { // have increase in trade
        sTmp.dCLosePrice = sSecond.dCLosePrice;
        sTmp.dTotalVolume = sSecond.dTotalVolume;
        sTmp.dTotalQuoteVolume = sSecond.dTotalQuoteVolume;
        const DOUBLE dVolDiff = sSecond.dTotalVolume - sLastTick.dTotalVolume;
        ASSERT(0.0f < dVolDiff);
        const DOUBLE dQuoteVolDiff = sSecond.dTotalQuoteVolume - sLastTick.dTotalQuoteVolume;
        ASSERT(0.0 < dQuoteVolDiff);
        const DOUBLE dTickAvgPrice = dQuoteVolDiff / dVolDiff;
        ASSERT(!isnan(dTickAvgPrice));
        const DOUBLE dPriceDiff = dTickAvgPrice - sLastTick.dAvgPrice;
        const DOUBLE dOBVDiff = dPriceDiff * dVolDiff;
        const DOUBLE dOBV = sLastTick.dOBV + dOBVDiff;
        sTmp.dAvgPrice = dTickAvgPrice;
        sTmp.dActualVolume = dVolDiff;
        sTmp.dOBV = dOBV;
        sTmp.dOBVDiff = dOBVDiff;
    }
    else {  // no trades
        sTmp.dActualVolume = 0.0;
        sTmp.dOBVDiff = 0.0;
    }
    cList.push_back(sTmp);
}

const STickListener* CBinanceMgr::GetTick(UINT uTradeIndex)
{
    for (auto& sTick : m_cTickListenerList) {
        if (sTick.uTradeIndex == uTradeIndex) {
            return &sTick;
        }
    }
    return NULL;
}
VOID CBinanceMgr::Update(FLOAT fLapsed)
{
    UpdateRythms(fLapsed);
    
    for (auto& sTick : m_cTickListenerList) {
        if (sTick.bFetching) {
            continue;
        }
        sTick.fTimeToNextFetch -= fLapsed;
        if (0.0f < sTick.fTimeToNextFetch) {
            continue;
        }
        sTick.fTimeToNextFetch = TICK_LISTENER_DURATION;
        sTick.bFetching = TRUE;
        FetchTick(sTick.uTradeIndex, 0 == sTick.cMinuteList.size());
    }
}

BOOLEAN CBinanceMgr::IsTickListening(UINT uTradeIndex)
{
    for (auto& sTick : m_cTickListenerList) {
        if (sTick.uTradeIndex == uTradeIndex) { // already listening
            return TRUE;
        }
    }
    return FALSE;
}
VOID CBinanceMgr::ListenToTicks(UINT uTradeIndex)
{
    for (auto& sTick : m_cTickListenerList) {
        if (sTick.uTradeIndex == uTradeIndex) { // already listening
            ++sTick.uRefCnt;
            return;
        }
    }
    // not found, insert new entry
    STickListener sTmp = {
        uTradeIndex,
        1,
        0.0f,
        FALSE,
    };
    memset(&sTmp.sProcessed, 0, sizeof(SProcessedTick));
    
    ASSERT(0 == sTmp.cMinuteList.size());
    ASSERT(0 == sTmp.cSecondList.size());
    m_cTickListenerList.push_back(sTmp);
}

VOID CBinanceMgr::StopListenToTicks(UINT uTradeIndex)
{
    CTickListener::iterator itTick = m_cTickListenerList.begin();
    const CTickListener::iterator itEnd = m_cTickListenerList.end();
    for ( ; itEnd != itTick; ++itTick) {
        STickListener& sTickListener = *(itTick);
        if (sTickListener.uTradeIndex == uTradeIndex) {
            ASSERT(0 < sTickListener.uRefCnt);
            --sTickListener.uRefCnt;
            if (0 == sTickListener.uRefCnt) {
                sTickListener.cMinuteList.clear();
                sTickListener.cSecondList.clear();
                SProcessedTick& sProcessed = sTickListener.sProcessed;
                SAFE_DELETE_ARRAY(sProcessed.pdMACD);
                SAFE_DELETE_ARRAY(sProcessed.pdSignal);
                m_cTickListenerList.erase(itTick);
                return;
            }
        }
    }
    TRACE("Warn: unable to stop listening to ticker [%d]\n", uTradeIndex);
}


VOID CBinanceMgr::FetchTick(UINT uTradeIndex, BOOLEAN bFullQuery)
{
    const STradePair* psPair = GetTradePair(uTradeIndex);
    if (NULL == psPair) {
        ASSERT(FALSE);
        return;
    }
    Json::Value* pcParams = new Json::Value();
    (*pcParams)["interval"] = "1m";
    (*pcParams)["symbol"] = psPair->szTradeKey;
    (*pcParams)["limit"] = bFullQuery ? TICK_HISTORY_MINS : 2;
    CSimpleHttpClient::DoGet("https://api.binance.com/api/v3/klines", *pcParams, EGLOBAL_BINANCE_MGR, EGLOBAL_EVT_BINANCE_QUERY_TICK);
}

VOID CBinanceMgr::OnTickQueried(CEvent& cEvent)
{
    const Json::Value* pcJson   = (const Json::Value*)cEvent.GetPtrParam(5);
    const INT nErrCode          = cEvent.GetIntParam(6);
    const CHAR* szResult        = (const CHAR*) cEvent.GetPtrParam(7);
    OnTickQueried(*pcJson, nErrCode, szResult);
    if (NULL != pcJson) {
        delete pcJson;
    }
    if (NULL != szResult) {
        delete [] szResult;
    }
}


VOID CBinanceMgr::OnTickQueried(const Json::Value& cParams, INT nErrCode, const CHAR* szResult)
{
    const std::string cKey = cParams["symbol"].asString();
    UINT uTradeIndex = 0;
    {
        UINT uIndex = 0;
        const UINT uTradeNum = GetTradePairNum();
        const CHAR* szKey = cKey.c_str();
        for ( ; uTradeNum > uIndex; ++uIndex) {
            const STradePair* psPair = GetTradePair(uIndex);
            if (NULL != psPair) {
                if (0 == strcmp(psPair->szTradeKey, szKey)) {
                    uTradeIndex = uIndex;
                    break;
                }
            }
        }
    }
    STickListener* psTick = NULL;
    for (auto& sTick : m_cTickListenerList) {
        if (uTradeIndex == sTick.uTradeIndex) { // found
            psTick = &sTick;
            break;
        }
    }
    if (NULL == psTick) {
        TRACE("Warn Tick: %s NOT Found!\n", cKey.c_str());
        return;
    }
    psTick->bFetching = FALSE;

    if (CSimpleHttpClient::ESuccess != nErrCode) {
        TRACE("Binance Tick Err Code: %d\n", nErrCode);
        return;
    }
    
    Json::Reader cReader;
    Json::Value cValue;
    if (!cReader.parse(szResult, cValue) || !cValue.isArray()) {
        ASSERT(FALSE);
        return;
    }
    const UINT uCandleNum = cValue.size();
    const BOOLEAN bFullQuery = (2 != uCandleNum);
    CTickList& cMinuteList = psTick->cMinuteList;
    if (bFullQuery) {
        cMinuteList.clear();
        DOUBLE dPrevOBV = 0.0;
        DOUBLE dPrevAvgPrice = 0.0;
        UINT uIndex = 0;
        STick sTmp = { 0 };
        for ( ; uCandleNum > uIndex; ++uIndex) {
            const Json::Value& cCandle = cValue[uIndex];
            if (!GetTickFromJson(sTmp, cCandle, dPrevAvgPrice, dPrevOBV)) {
                ASSERT(FALSE);
                return;
            }
            cMinuteList.push_back(sTmp);
            dPrevAvgPrice = sTmp.dAvgPrice;
            dPrevOBV = sTmp.dOBV;
        }
        // insert last tick into seconds data
        ASSERT(0 < cMinuteList.size());
        STick& sLastTick = *(cMinuteList.rbegin());
        sLastTick.uCloseSec = CHLTime::GetTimeSecs();   // override last tick end time
        psTick->cSecondList.push_back(sLastTick);
    }
    else {
        STick sFirst;
        if (!GetTickFromJson(sFirst, cValue[0], 0.0f, 0.0f)) {
            ASSERT(FALSE);
            return;
        }
        RefreshMinuteTicker(*psTick, sFirst);
        STick sSecond;
        if (!GetTickFromJson(sSecond, cValue[1], 0.0f, 0.0f)) {
            ASSERT(FALSE);
            return;
        }
        const UINT uNowSec = CHLTime::GetTimeSecs();;
        sSecond.uCloseSec = uNowSec; // override last tick end time
        RefreshMinuteTicker(*psTick, sSecond);
        ProcessTick(*psTick, sSecond); // bug here, new inserts do not have obv
    }
    GenerateSignals(*psTick);
    CEventManager::BroadcastEvent(EGLOBAL_BINANCE_MGR, CEvent(EGLOBAL_EVT_BINANCE_TICK_UPDATE, uTradeIndex));
}

