#include "stdafx.h"
#include "AutoPtr.h"
#include "MAUtil.h"
#include "PlayerData.h"
#include "SignalsUtil.h"

#define PIP_MULTIPLIER (0.0001)

#define PIP(x)  (x*PIP_MULTIPLIER)

// set commision 1 pip
#define COMMISION           PIP(1)
// set buy markup at 1 pip
#define BUY_SELL_MARKUP     PIP(1)
// set stop loss at 20 pip
#define STOP_LOSS           PIP(20)
// 50% profit taking pip
#define HALF_PROFIT_TAKE    PIP(10)

// full profit take when price dips below ema price
#define FULL_PROFIT_TAKE    PIP(10)

#define BUY_PRICE_MULTIPLIER        (1.0 + COMMISION + BUY_SELL_MARKUP)
#define SELL_PRICE_MULTIPLIER       (1.0 - COMMISION - BUY_SELL_MARKUP)
#define STOP_LOSS_MULTIPLIER        (1.0 - STOP_LOSS)
#define HALF_PROFIT_MULTIPLIER      (1.0 + HALF_PROFIT_TAKE)
#define FULL_PROFIT_MULTIPLIER      (1.0 - FULL_PROFIT_TAKE)

VOID CSignalsUtil::GenerateFOMO(CSignalList& cList,
                                DOUBLE dAmountAllocated,
                                const DOUBLE* pdPrices, const DOUBLE* pdVolumes, UINT uPriceNum)
{
//    // generate EMA20
//    SMovingAverage sEMA20(EMA_EMA, EMA_VALUE_CLOSE, 20);
//    if (!CMAUtil::GenerateMA(sEMA20, pdPrices, uPriceNum, 0, uPriceNum)) {
//        return;
//    }
//    CAutoPtrArray<DOUBLE> cGuardEMA20(sEMA20.pdPrice);
//
//    // Generate SMA50
//    SMovingAverage sSMA50(EMA_EMA, EMA_VALUE_CLOSE, 50);
//    if (!CMAUtil::GenerateMA(sSMA50, pdPrices, uPriceNum, 0, uPriceNum)) {
//        return;
//    }
//    CAutoPtrArray<DOUBLE> cGuardSMA50(sSMA50.pdPrice);
//
//    // generate MACD 12,26,9
//    SMACD sMACD = {
//        {
//            { EMA_EMA, EMA_VALUE_CLOSE, 12, NULL },
//            { EMA_EMA, EMA_VALUE_CLOSE, 26, NULL },
//            { EMA_EMA, EMA_VALUE_CLOSE, 9, NULL }
//        },
//        NULL,
//        NULL
//    };
//    if (!CMAUtil::GenerateMACD(sMACD, pdPrices, uPriceNum, 0, uPriceNum)) {
//        return;
//    }
//    CAutoPtrArray<DOUBLE> cGuardMACD(sMACD.pdMACD);
//    CAutoPtrArray<DOUBLE> cGuardSignal(sMACD.pdSignal);
//    UINT uIndex = 0;
//    BOOLEAN bPrevPriceCrossEMA20 = FALSE;
//    UINT uMACDPositiveCounter = 0;
//    
//    BOOLEAN bSoldHalf = FALSE;
//    DOUBLE dStopLossPrice = 0.0;
//    DOUBLE dHalfSellPrice = 0.0;
//    DOUBLE dStockAmt = 0.0;
//    DOUBLE dPrevPrice = 0.0;
//    DOUBLE dPrevSMA50 = 0.0;
//    for ( ; uPriceNum > uIndex; ++uIndex) {
//        //get current price
//        const DOUBLE dCurrent_Price = pdPrices[uIndex];
//        const DOUBLE dEMA20_Price = sEMA20.pdPrice[uIndex];
//        const BOOLEAN bPriceCrossed = (dCurrent_Price > dEMA20_Price);
//        
//        const DOUBLE dMACD_Price = sMACD.pdMACD[uIndex];
//        const DOUBLE dSignal_Price = sMACD.pdSignal[uIndex];
//        
//        const DOUBLE dNowSMA50 = sSMA50.pdPrice[uIndex];
////        const BOOLEAN bIsSMA50Rising = dNowSMA50 > dPrevSMA50;
//        dPrevSMA50 = dNowSMA50;
//        if (dSignal_Price > dMACD_Price) {
//            uMACDPositiveCounter = 0;
//        }
//        else {
//            ++uMACDPositiveCounter;
//        }
//        const BOOLEAN bCurrentPriceCrossesEMA20 = !bPrevPriceCrossEMA20 && bPriceCrossed;
//        const DOUBLE dPriceDiff = dCurrent_Price - dPrevPrice;
//        bPrevPriceCrossEMA20 = bPriceCrossed;
//        dPrevPrice = dCurrent_Price;
//        if (0.0 == dStockAmt) { // if not yet bought stocks
//            if (0 == uIndex) { // skip 1st index
//                continue;
//            }
//            if (!bCurrentPriceCrossesEMA20) {
//                continue;
//            }
//            // make sure macd crossver also within past 5 interval
//            if (1 <= uMACDPositiveCounter && 5 >= uMACDPositiveCounter) {
//                const DOUBLE dBuyingPrice = BUY_PRICE_MULTIPLIER * dCurrent_Price;
//                const DOUBLE dVolumeBought = dAmountAllocated / dBuyingPrice;
//                // implement buy signal
//                const SSignal sBuy = {
//                    uIndex,         //                UINT        uCandleIndex;   // exact candle index
//                    ESIG_BUY,       //                ESIG_TYPE   eType;
//                    dBuyingPrice,   //                DOUBLE      dPrice;         // the price to perform action
//                    dVolumeBought,  //                DOUBLE      dVolume;        // volume bought/sold
//                    dAmountAllocated//                DOUBLE      dAmount;        // amount used/earned
//                };
//                cList.push_back(sBuy);
//                dStockAmt = dVolumeBought; // set stock amount
//                bSoldHalf = FALSE;
//                dStopLossPrice = STOP_LOSS_MULTIPLIER * dCurrent_Price;
//                dHalfSellPrice = HALF_PROFIT_MULTIPLIER * dCurrent_Price;
//            }
//            continue;
//        }
//        // bought stocks already
//        if (0.0 < dPriceDiff) {
//            dStopLossPrice += dPriceDiff; // update stop loss
//        }
//        // check for stop loss
//        if (dCurrent_Price < dStopLossPrice) {
//            // implement stop loss signal
//            const DOUBLE dSellingPrice = SELL_PRICE_MULTIPLIER * dCurrent_Price;
//            const DOUBLE dAmtSold = dSellingPrice * dStockAmt;
//            const SSignal sStopLoss = {
//                uIndex,         //                UINT        uCandleIndex;   // exact candle index
//                ESIG_STOP_LOSS, //                ESIG_TYPE   eType;
//                dSellingPrice,  //                DOUBLE      dPrice;         // the price to perform action
//                dStockAmt,      //                DOUBLE      dVolume;        // volume bought/sold
//                dAmtSold        //                DOUBLE      dAmount;        // amount used/earned
//            };
//            cList.push_back(sStopLoss);
//            dStockAmt = 0.0;
//            continue;
//        }
//        // if not yet sold 1/2 the stocks
//        if (!bSoldHalf) {
//            // check if price is above 1/2 sold price
//            if (dCurrent_Price > dHalfSellPrice) {
//                const DOUBLE dSellingPrice = SELL_PRICE_MULTIPLIER * dCurrent_Price;
//                const DOUBLE dAmtToSell = dStockAmt;
//                const DOUBLE dAmtSold = dSellingPrice * dAmtToSell;
//                const SSignal sHalfProfit = {
//                    uIndex,             //                UINT        uCandleIndex;   // exact candle index
//                    ESIG_TAKE_PROFIT,   //                ESIG_TYPE   eType;
//                    dSellingPrice,      //                DOUBLE      dPrice;         // the price to perform action
//                    dAmtToSell,         //                DOUBLE      dVolume;        // volume bought/sold
//                    dAmtSold            //                DOUBLE      dAmount;        // amount used/earned
//                };
//                cList.push_back(sHalfProfit);
//                dStockAmt -= dAmtToSell;
//                //bSoldHalf = TRUE;
//                // try to sell half again
//                dHalfSellPrice = HALF_PROFIT_MULTIPLIER * dCurrent_Price;
//                // shift stop loss
//                dStopLossPrice = STOP_LOSS_MULTIPLIER * dCurrent_Price;
//                continue;
//            }
//        }
//        // check final sell price
//        const DOUBLE dEndGamePrice = FULL_PROFIT_MULTIPLIER * dEMA20_Price;
//        if (dCurrent_Price < dEndGamePrice) {
//            // sell all the remaining stocks
//            const DOUBLE dSellingPrice = SELL_PRICE_MULTIPLIER * dCurrent_Price;
//            const DOUBLE dAmtSold = dSellingPrice * dStockAmt;
//            const SSignal sHalfProfit = {
//                uIndex,             //                UINT        uCandleIndex;   // exact candle index
//                ESIG_END_PROFIT,    //                ESIG_TYPE   eType;
//                dSellingPrice,      //                DOUBLE      dPrice;         // the price to perform action
//                dStockAmt,          //                DOUBLE      dVolume;        // volume bought/sold
//                dAmtSold            //                DOUBLE      dAmount;        // amount used/earned
//            };
//            dStockAmt = 0.0;
//            cList.push_back(sHalfProfit);
//            bSoldHalf = TRUE;
//        }
//    }
}
