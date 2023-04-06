#include "stdafx.h"
#include "Event.h"
#include "EventID.h"
#include "NowLoadingWindow.h"
#include "TradeMainWindow.h"

VOID CTradingMainWindow::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EGLOBAL_EVT_OANDA_SIGN_IN:
            OnOandaSignInReply(cEvent.GetIntParam(1));
            break;
        case EGLOBAL_EVT_OANDA_TRADE_PAIR_QUERY:
            OnOandaTradePairQueryReply(cEvent.GetIntParam(1));
            break;
        case EEVENT_ON_SLIDER_SLIDED:
            OnSliderSlided(cEvent.GetIntParam(1));
            break;
        case EBTN_PREDICT:
            PerformPrediction();
            break;
        case EBTN_CANDLE_NEXT:
            AdjustSelectedCandle(1);
            break;
        case EBTN_CANDLE_PREV:
            AdjustSelectedCandle(-1);
            break;
        case EBTN_CHANGE_TRADE:
            OnPressTradeChangeBtn();
            break;
        case EGLOBAL_EVT_TRADE_MAIN_WINDOW_CHANGE_TRADE:
            OnTradeChange(cEvent.GetIntParam(1));
            break;
        case EBTN_CRYPTO_DOT_COM_CHART:
            TryChangeSource(ESOURCE_MARKET_CRYPTO_COM);
            break;
        case EBTN_BINANCE_CHART:
            TryChangeSource(ESOURCE_MARKET_BINANCE);
            break;
        case EBTN_OANDA_CHART:
            TryChangeSource(ESOURCE_MARKET_OANDA);
            break;
        
        case EGLOBAL_EVT_BINANCE_TRADE_PAIR_UPDATE:
            OnBinanceTradePairsQuery(cEvent.GetIntParam(1));
            break;
        case EGLOBAL_EVT_BINANCE_ON_OHLC_QUERY_END:
            CNowLoadingWindow::HideWindow();
            OnQueryCandlesEnd(cEvent.GetIntParam(1));
            break;
        case EGLOBAL_EVT_OANDA_OHLC_QUERY:
            CNowLoadingWindow::HideWindow();
            OnQueryCandlesEnd(cEvent.GetIntParam(1));
            break;
            
        case EGLOBAL_EVT_CRYPTO_MARKET_SOCKET_CONNECT_REPLY:
            CNowLoadingWindow::HideWindow();
            TryChangeSource(ESOURCE_MARKET_CRYPTO_COM);
            break;
        case EGLOBAL_EVT_CRYPTO_MARKET_SOCKET_ERROR:
            OnCryptoDotComSocketError();
            break;
        case EGLOBAL_EVT_CRYPTO_TRADE_PAIR_UPDATE:
            OnCryptoDotComTradePairUpdate(cEvent.GetIntParam(1));
            break;
        case EGLOBAL_EVT_CRYPTO_OHLC_UPDATE:
            OnCryptoDotComCandleUpdate(cEvent.GetIntParam(1), cEvent.GetIntParam(2), cEvent.GetIntParam(3));
            break;

        case EBTN_OVERLAY_BTN:
            OnOverlayBtnPressed();
            break;
        case EGLOBAL_EVT_TRADE_MAIN_WINDOW_REFRESH_ATTACH_CHART:
            RefreshAttachedCharts(cEvent.GetIntParam(1));
            break;
        case EGLOBAL_EVT_TRADE_MAIN_WINDOW_TOGGLE_ATTACH_CHART:
            ToogleAttachChart(cEvent.GetIntParam(1));
            break;
        case EGLOBAL_EVT_TRADE_MAIN_WINDOW_DISPLAY_TOOLTIP:
            OnToolTip(cEvent.GetIntParam(1), cEvent.GetIntParam(2));
            break;
        case EBTN_RENKO:
            OnPressRenkoBtn();
            break;
        default:
            if ((EBTN_CANDLE_DURATION_CHANGE_START <= uEventID) && (EBTN_CANDLE_DURATION_CHANGE_END > uEventID)) {
                OnToggleCandleDuration(uEventID - EBTN_CANDLE_DURATION_CHANGE_START, TRUE);
                break;
            }
            if ((EBTN_DISPLAY_DURATION_CHANGE_START <= uEventID) && (EBTN_DISPLAY_DURATION_CHANGE_END > uEventID)) {
                OnToggleDuration(uEventID - EBTN_DISPLAY_DURATION_CHANGE_START);
                break;
            }
            break;
    }
}

