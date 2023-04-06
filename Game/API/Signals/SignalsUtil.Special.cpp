#include "stdafx.h"
#include "AutoPtr.h"
#include "MAUtil.h"
#include "PlayerData.h"
#include "SignalsUtil.h"
#include "VolumeUtil.h"

#define PIP_MULTIPLIER (0.0001)

#define PIP(x)  (x*PIP_MULTIPLIER)
// set commision 10 pip
#define COMMISION           PIP(1)
// set buy markup at 1 pip
#define BUY_SELL_MARKUP     PIP(1)
// set stop loss at 20 pip
#define STOP_LOSS           PIP(20)
// take profit taking pip
#define HALF_PROFIT_TAKE    PIP(20)


#define BUY_PRICE_MULTIPLIER        (1.0 + COMMISION + BUY_SELL_MARKUP)
#define SELL_PRICE_MULTIPLIER       (1.0 - COMMISION - BUY_SELL_MARKUP)
#define STOP_LOSS_MULTIPLIER        (1.0 - STOP_LOSS)
#define HALF_PROFIT_MULTIPLIER      (1.0 + HALF_PROFIT_TAKE)

#define BB_INTERVAL (15)
#define BB_DEVIATION (2.0)

#define HALF_PROFIT_SELL_RATIO (0.5)

static VOID GenerateSignal(CSignalList& cList, DOUBLE dAmountAllocated,
                           UINT uValueNum, const BOOLEAN* pbGreenCandle, const DOUBLE* pdPrices,
                           const DOUBLE* pdOBV, const DOUBLE* pdMA, const DOUBLE* pdDeviation)
{
    
    BOOLEAN bSoldHalf = FALSE;
    BOOLEAN bPrepareToBuy = FALSE;
    BOOLEAN bPrepareToSell = FALSE;
    DOUBLE dStopLossPrice = 0.0;
    DOUBLE dHalfSellPrice = 0.0;
    DOUBLE dStockAmt = 0.0;
    UINT uIndex = BB_INTERVAL; // skip first intervals
    
    DOUBLE dPrevPrice = pdPrices[uIndex-1];
    DOUBLE dPrevOBV = pdOBV[uIndex-1];
    DOUBLE dPrevOBVDiff;
    {
        const DOUBLE dPrevPrevOBV = pdOBV[uIndex-2];
        dPrevOBVDiff = dPrevOBV - dPrevPrevOBV;
    }
    for ( ; uValueNum > uIndex; ++uIndex) {
        const DOUBLE dCurrentPrice = pdPrices[uIndex];
        const DOUBLE dDiffPrice = dCurrentPrice - dPrevPrice;
        dPrevPrice = dCurrentPrice;
        const DOUBLE dCurrentOBV = pdOBV[uIndex];
        const DOUBLE dCurrentDiffOBV = dCurrentOBV - dPrevOBV;
        const DOUBLE dDiffOBVDifference = dCurrentDiffOBV - dPrevOBVDiff;
        dPrevOBVDiff = dCurrentDiffOBV;
        dPrevOBV = dCurrentOBV;
        
        if (0.0 == dStockAmt) {
            if (!bPrepareToBuy) {
                // check if price is below bottom 1/2 of BB
                const DOUBLE dBBLowerBound = pdMA[uIndex] - BB_DEVIATION * pdDeviation[uIndex];
                bPrepareToBuy = (dCurrentPrice < dBBLowerBound);
            }
            if (!bPrepareToBuy) {
                continue;
            }
            if (!pbGreenCandle[uIndex]) {
                continue;
            }
            // also check difference OBV increasing
            if (0.0 > dDiffOBVDifference) {
                continue;
            }
            // double make sure the difference in obv is not too great
            
            // generate BUY signal
            const DOUBLE dBuyingPrice = BUY_PRICE_MULTIPLIER * dCurrentPrice;
            const DOUBLE dVolumeBought = dAmountAllocated / dBuyingPrice;
            // implement buy signal
            const SSignal sBuy = {
                uIndex,         //                UINT        uCandleIndex;   // exact candle index
                ESIG_BUY,       //                ESIG_TYPE   eType;
                dBuyingPrice,   //                DOUBLE      dPrice;         // the price to perform action
                dVolumeBought,  //                DOUBLE      dVolume;        // volume bought/sold
                dAmountAllocated//                DOUBLE      dAmount;        // amount used/earned
            };
            cList.push_back(sBuy);
            dStockAmt = dVolumeBought; // set stock amount
            bSoldHalf = FALSE;
            bPrepareToBuy = FALSE;
            bPrepareToSell = FALSE;
            dStopLossPrice = STOP_LOSS_MULTIPLIER * dCurrentPrice;
            dHalfSellPrice = HALF_PROFIT_MULTIPLIER * dCurrentPrice;
            continue;
        }
        // bought stocks already
        if (0.0 < dDiffPrice) {
            dStopLossPrice += dDiffPrice; // update stop loss
        }
        // check for stop loss
        if (dCurrentPrice <= dStopLossPrice) {
            // implement stop loss signal
            const DOUBLE dSellingPrice = SELL_PRICE_MULTIPLIER * dCurrentPrice;
            const DOUBLE dAmtSold = dSellingPrice * dStockAmt;
            const SSignal sStopLoss = {
                uIndex,         //                UINT        uCandleIndex;   // exact candle index
                ESIG_STOP_LOSS, //                ESIG_TYPE   eType;
                dSellingPrice,  //                DOUBLE      dPrice;         // the price to perform action
                dStockAmt,      //                DOUBLE      dVolume;        // volume bought/sold
                dAmtSold        //                DOUBLE      dAmount;        // amount used/earned
            };
            cList.push_back(sStopLoss);
            dStockAmt = 0.0;
            continue;
        }
        // if not yet sold 1/2 the stocks
        if (!bSoldHalf) {
            // check if price is above 1/2 sold price
            if (dCurrentPrice > dHalfSellPrice) {
                const DOUBLE dSellingPrice = SELL_PRICE_MULTIPLIER * dCurrentPrice;
                const DOUBLE dAmtToSell = dStockAmt * HALF_PROFIT_SELL_RATIO;
                const DOUBLE dAmtSold = dSellingPrice * dAmtToSell;
                const SSignal sHalfProfit = {
                    uIndex,             //                UINT        uCandleIndex;   // exact candle index
                    ESIG_TAKE_PROFIT,   //                ESIG_TYPE   eType;
                    dSellingPrice,      //                DOUBLE      dPrice;         // the price to perform action
                    dAmtToSell,         //                DOUBLE      dVolume;        // volume bought/sold
                    dAmtSold            //                DOUBLE      dAmount;        // amount used/earned
                };
                cList.push_back(sHalfProfit);
                dStockAmt -= dAmtToSell;
                //bSoldHalf = TRUE;
                // try to sell half again
                dHalfSellPrice = HALF_PROFIT_MULTIPLIER * dCurrentPrice;
                // shift stop loss
                dStopLossPrice = STOP_LOSS_MULTIPLIER * dCurrentPrice;
                continue;
            }
        }
        // final exit check
        if (!bPrepareToSell) {
            // check if price is above top 1/2 of BB
            const DOUBLE dBBUpperBound = pdMA[uIndex] + BB_DEVIATION * pdDeviation[uIndex];
            bPrepareToSell = (dCurrentPrice > dBBUpperBound);
        }
        if (!bPrepareToSell) {
            continue;
        }
        if (0.0 < dCurrentDiffOBV) { // if OBV still rising, skip
            continue;
        }
        // time to sell
        // sell all the remaining stocks
        const DOUBLE dSellingPrice = SELL_PRICE_MULTIPLIER * dCurrentPrice;
        const DOUBLE dAmtSold = dSellingPrice * dStockAmt;
        const SSignal sHalfProfit = {
            uIndex,             //                UINT        uCandleIndex;   // exact candle index
            ESIG_END_PROFIT,    //                ESIG_TYPE   eType;
            dSellingPrice,      //                DOUBLE      dPrice;         // the price to perform action
            dStockAmt,          //                DOUBLE      dVolume;        // volume bought/sold
            dAmtSold            //                DOUBLE      dAmount;        // amount used/earned
        };
        dStockAmt = 0.0;
        cList.push_back(sHalfProfit);
        bSoldHalf = TRUE;
    }
}

VOID CSignalsUtil::GenerateSpecial(CSignalList& cList,
                                   DOUBLE dAmountAllocated,
                                   const DOUBLE* pdPrices, const DOUBLE* pdVolumes, UINT uPriceNum,
                                   UINT uCandleStartTimeSec, UINT uCandleDurationSec,
                                   const CTradeCandleMap& cCandles)
{
    const DOUBLE* pdOBV = CVolumeUtil::GenerateOBV(pdPrices, pdVolumes, uPriceNum, 0, uPriceNum, TRUE);
    if (NULL == pdOBV) {
        return;
    }
    CAutoPtrArray<const DOUBLE> cOBV(pdOBV);
    // generate BB (15,2)
    SBollingerBands sBB = {
        { EMA_SMA, EMA_VALUE_CLOSE, BB_INTERVAL },
        NULL
    };
    if (!CMAUtil::GenerateBollingerBands(sBB, pdPrices, uPriceNum, 0, uPriceNum)) {
        return;
    }
    if (NULL == sBB.sMA.pdPrice) {
        ASSERT(FALSE);
        return;
    }
    CAutoPtrArray<const DOUBLE> cBBPrice(sBB.sMA.pdPrice);
    if (NULL == sBB.pdDeviation) {
        ASSERT(FALSE);
        return;
    }
    CAutoPtrArray<const DOUBLE> cBBDeviation(sBB.pdDeviation);
    
    // need to determine candle type
    BOOLEAN* pbGreenCandle = new BOOLEAN[uPriceNum];
    if (NULL == pbGreenCandle) {
        ASSERT(FALSE);
        return;
    }
    CAutoPtrArray<BOOLEAN> cGreenCandles(pbGreenCandle);
    UINT uIndex = 0;
    for ( ; uPriceNum > uIndex; ++uIndex, uCandleStartTimeSec += uCandleDurationSec) {
        CTradeCandleMap::const_iterator itFound = cCandles.find(uCandleStartTimeSec);
        if (cCandles.end() == itFound) {
            continue;
        }
        const STradeCandle& sCandle = itFound->second;
        pbGreenCandle[uIndex] = sCandle.dOpen < sCandle.dClose;
    }
    GenerateSignal(cList, dAmountAllocated, uPriceNum, pbGreenCandle, pdPrices, pdOBV, sBB.sMA.pdPrice, sBB.pdDeviation);
}
