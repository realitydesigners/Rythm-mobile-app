#include "stdafx.h"
#include "ConfirmationWindow.h"
#include "EventManager.h"
#include "NowLoadingWindow.h"
#include "OandaMgr.h"
#include "TradeMainWindow.h"


VOID CTradingMainWindow::OnBinanceTradePairsQuery(BOOLEAN bSuccess)
{
    CNowLoadingWindow::HideWindow();
    if (!bSuccess) {
        CConfirmationWindow::DisplayErrorMsg("Unable To Load Trading Pairs from Binance!", NULL, 0);
        return;
    }
    OnToggleSource(ESOURCE_MARKET_BINANCE);
    OnToggleDuration(m_uDurationIndex);
    RefreshAllChartSizes();
}

VOID CTradingMainWindow::OnCryptoDotComSocketError(VOID)
{
    if (ESOURCE_MARKET_CRYPTO_COM != m_eCurrentSource) {
        return;
    }
    if (CNowLoadingWindow::IsDisplayed()) {
        CNowLoadingWindow::HideWindow();
        CConfirmationWindow::DisplayErrorMsg("Socket Error.", NULL, 0);
    }
}


VOID CTradingMainWindow::OnCryptoDotComTradePairUpdate(BOOLEAN bSuccess)
{
    if (CNowLoadingWindow::IsDisplayed()) {
        CNowLoadingWindow::HideWindow();
    }
    if (!bSuccess) {
        CConfirmationWindow::DisplayErrorMsg("Unable To Load Trading Pairs from Crypto.com!", NULL, 0);
        return;
    }
    OnToggleSource(ESOURCE_MARKET_CRYPTO_COM);
    OnToggleDuration(m_uDurationIndex);
    RefreshAllChartSizes();
}

VOID CTradingMainWindow::OnCryptoDotComCandleUpdate(UINT uTradePairIndex, UINT uCandleIndex, BOOLEAN bSuccess)
{
    if (ESOURCE_MARKET_CRYPTO_COM != m_eCurrentSource) {    // not the same source
        return;
    }
    if (uTradePairIndex != m_uTradeIndex) {                 // not the matching trading pair
        return;
    }
    if (uCandleIndex != m_uCandleDurationIndex) {           // not viewing the same index now
        return;
    }
    if (CNowLoadingWindow::IsDisplayed()) { // hide loading window if shown
        CNowLoadingWindow::HideWindow();
    }
    OnQueryCandlesEnd(bSuccess);
}
VOID CTradingMainWindow::OnOandaSignInReply(BOOLEAN bSuccess)
{
    if (CNowLoadingWindow::IsDisplayed()) {
        CNowLoadingWindow::HideWindow();
    }
    if (!bSuccess) {
        return;
    }
    COandaMgr& cMgr = COandaMgr::GetInstance();
    if (cMgr.AreTradePairsQueried()) {
        OnOandaTradePairQueryReply(TRUE);
        return;
    }
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_TRADE_PAIR_QUERY, TRUE);
    CNowLoadingWindow::DisplayWindow(0xFFFFFFFF, "Loading Trade Pairs");
    cMgr.QueryTradePairs();
}

VOID CTradingMainWindow::OnOandaTradePairQueryReply(BOOLEAN bSuccess)
{
    if (CNowLoadingWindow::IsDisplayed()) {
        CNowLoadingWindow::HideWindow();
    }
    if (!bSuccess) {
        CConfirmationWindow::DisplayErrorMsg("Fail Load Trading Pairs from Oanda!", NULL, 0);
        return;
    }
    OnToggleSource(ESOURCE_MARKET_OANDA);
    OnToggleDuration(m_uDurationIndex);
    RefreshAllChartSizes();
}
