#include "stdafx.h"
#include "AutomationMgr.h"
#include "AutoWidget.h"
#include "CMain.h"
#include "ConfirmationWindow.h"
#include "CryptoUtil.h"
#include "Event.h"
#include "EventID.h"
#include "EventManager.h"
#include "ForexWindow.h"
#include "MegaZZ.h"
#include "NowLoadingWindow.h"
#include "OandaMgr.h"
#include "QuitGameWindow.h"


#define SOURCE_NUM  (1)

VOID CForexWindow::OnBackBtnPressed(VOID)
{
    CQuitGameWindow* pcWin = new CQuitGameWindow();
    if (NULL != pcWin) {
        pcWin->DoModal();
    }
}

CForexWindow::CForexWindow() :
CGameWindow(),
m_cDashboard(EEVENT_SELECTED_TRADE_PAIR),
m_cLive(ESOURCE_MARKET_OANDA, SOURCE_NUM),
INITIALIZE_TEXT_LABEL(m_cName),
INITIALIZE_TEXT_LABEL(m_cPrice),
INITIALIZE_TEXT_LABEL(m_cSpread),
m_pcChangeSizeBtn(NULL),
m_pcChangeBoxNumBtn(NULL),
m_uCurrentTradePairIndex(0),
m_uInnerZZDepth(7)
{
    m_bDestroyOnExitModal = TRUE;
}

CForexWindow::~CForexWindow()
{
}

VOID CForexWindow::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();
    const FLOAT fX = (FLOAT)CMain::GetDisplaySideMargin();
    const FLOAT fScreenW = (FLOAT)CMain::GetScreenWidth();
    m_cLogo.SetTexture(ETEX::Rythm_Logo, TRUE);
    m_cLogo.SetScale(0.8f);
    m_cLogo.SetLocalPosition(fX, -5.0f);
    AddChild(m_cLogo);
    m_cRefreshOptions.Initialize();
    const FLOAT fRefreshOptionX = fScreenW - fX - m_cRefreshOptions.GetLocalSize().x - 10.0f;
    m_cRefreshOptions.SetLocalPosition(fRefreshOptionX, 5.0f);
    AddChild(m_cRefreshOptions);
    
    FLOAT fOptionsX = fRefreshOptionX - 105.0f;
    CreateColorButton(fOptionsX, 5.0f, 100.0f, 20.0f, EBTN_OPEN_CHECK_UI, RYTHM_UI_NOT_SELECTED_FONT_COLOR, "Checker", TRUE);
    
    m_cLogs.SetLocalPosition(fOptionsX, 30.0f);
    m_cLogs.Initialize();
    AddChild(m_cLogs);
    
//    m_cAlerts.SetLocalPosition(fOptionsX - 105.0f, 5.0f);
//    m_cAlerts.Initialize();
//    AddChild(m_cAlerts);    

    const FLOAT fDashboardY = 55.0f;
    m_cDashboard.SetLocalPosition(0.0f, fDashboardY);
    m_cDashboard.UpdateFractalIndex();
        

    const FLOAT fBoxX = fX + 370.0f;
    FLOAT fBoxDimensions = (fScreenW - fBoxX - 10.0f) * 0.5f;
    const FLOAT fScreenH = CMain::GetScreenHeight() - CMain::GetDisplayBottomMargin();
    const FLOAT fMaxHeight = fScreenH - fDashboardY - 10.0f;
    if (fBoxDimensions > fMaxHeight) {
        fBoxDimensions = fMaxHeight;
    }
    
    m_cBoxMain.SetLocalPosition(fBoxX, fDashboardY);
    m_cBoxMain.SetLocalSize(fBoxDimensions, fBoxDimensions + NEW_BIB_BOX_HEIGHT_OFFSET);
//    m_cBoxMain.SetDefaultDepthNum(4);
//    m_cBoxMain.SetDefaultDepthIndex(2);
    m_cBoxMain.Initialize();
    AddChild(m_cBoxMain);
    
    const FLOAT fSubBoxX = fBoxX + fBoxDimensions;
    m_cBoxSub.SetLocalPosition(fSubBoxX, fDashboardY);
    m_cBoxSub.SetLocalSize(fBoxDimensions, fBoxDimensions);
    m_cBoxSub.SetDefaultDepthIndex(5);
    m_cBoxSub.SetDefaultDepthNum(4);
    m_cBoxSub.Initialize();
    AddChild(m_cBoxSub);
//    const FLOAT fZZY = fDashboardY + fBoxDimensions + 5.0f;
//    m_cLeft.SetLocalPosition(fBoxX, fZZY);
//    m_cLeft.SetLocalSize(fScreenW - 10.0f - 40.0f - fBoxX, 190.0f);
//    m_cLeft.Initialize(100);
//    AddChild(m_cLeft);

//    const FLOAT fDepthBtnX = fScreenW - 5.0f - 40.0f;
//    CreateColorButton(fDepthBtnX, fZZY, 30.0f, 20.0f, EBTN_INC_DEPTH, RGBA(0x4F, 0x4F, 0x4F, 0xFF), "D+", TRUE);
//    CreateColorButton(fDepthBtnX, fZZY + 25.0f, 30.0f, 20.0f, EBTN_DEC_DEPTH, RGBA(0x4F, 0x4F, 0x4F, 0xFF), "D-", TRUE);

    
    m_cName.SetFont(EGAMEFONT_SIZE_22);
    m_cName.SetAlignment(CUITextLabel::EALIGNMENT_LEFT);
    m_cName.SetColor(RYTHM_UI_SELECTED_FONT_COLOR);
    m_cName.SetLocalPosition(fBoxX + 5.0f, fDashboardY - 25.0f);
    m_cName.SetAnchor(0.0f, 1.0f);
    m_cName.SetString("-------");
    AddChild(m_cName);
    
    m_cPrice.SetFont(EGAMEFONT_SIZE_18);
    m_cPrice.SetAlignment(CUITextLabel::EALIGNMENT_LEFT);
    m_cPrice.SetColor(RYTHM_UI_SELECTED_FONT_COLOR);
    m_cPrice.SetLocalPosition(fBoxX + 5.0f, fDashboardY - 5.0f);
    m_cPrice.SetAnchor(0.0f, 1.0f);
    m_cPrice.SetString("-------");
    AddChild(m_cPrice);
    
    m_cSpread.SetFont(EGAMEFONT_SIZE_12);
    m_cSpread.SetAlignment(CUITextLabel::EALIGNMENT_LEFT);
    m_cSpread.SetColor(RYTHM_UI_SELECTED_FONT_COLOR);
    m_cSpread.SetLocalPosition(fBoxX + 120.0f, 2.0f);
    m_cSpread.SetAnchor(0.0f, 0.0f);
    m_cSpread.SetString("Ask: ----\nBid: ----\nSpread: ----");
    AddChild(m_cSpread);

    COandaMgr& cMgr = COandaMgr::GetInstance();
    if (cMgr.IsSignedIn()) {
        OnOandaSignInReply(TRUE);
        return;
    }
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_SIGN_IN, TRUE);
    CNowLoadingWindow::DisplayWindow(0xFFFFFFFF, "Signing in...");
    cMgr.TrySignIn();
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_AUTOMATION_MGR, EGLOBAL_EVT_AUTOMATION_MEGAZZ_UPDATED);
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_FETCH_SPREADS_REPLY);
}

VOID CForexWindow::Release(VOID)
{
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_FETCH_SPREADS_REPLY);
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_AUTOMATION_MGR, EGLOBAL_EVT_AUTOMATION_MEGAZZ_UPDATED);
    m_cDashboard.Release();
    m_cDashboard.RemoveFromParent();
    
    m_cLive.Release();
    m_cLive.RemoveFromParent();
    
    m_cOAccount.Release();
    m_cOAccount.RemoveFromParent();

    m_cLogs.Release();
    m_cLogs.RemoveFromParent();
    
    m_cTrades.Release();
    m_cTrades.RemoveFromParent();
    
    m_cAlerts.Release();
    m_cAlerts.RemoveFromParent();

    m_cLogo.RemoveFromParent();
    
    m_cRefreshOptions.Release();
    m_cRefreshOptions.RemoveFromParent();
    
    m_cBoxMain.Release();
    m_cBoxMain.RemoveFromParent();
    
    m_cBoxSub.Release();
    m_cBoxSub.RemoveFromParent();

//    m_cLeft.Release();
//    m_cLeft.RemoveFromParent();
    
    m_cName.Release();
    m_cName.RemoveFromParent();
    
    m_cPrice.Release();
    m_cPrice.RemoveFromParent();
    
    m_cSpread.Release();
    m_cSpread.RemoveFromParent();

    CGameWindow::Release();
}


VOID CForexWindow::OnReceiveEvent(CEvent& cEvent)
{
    switch (cEvent.GetIntParam(0)) {
        case EBTN_OPEN_CHECK_UI:
            OnTapCheckUI();
            break;
        case EBTN_INC_DEPTH:
            ChangeInnerZZDepth(-1);
            break;
        case EBTN_DEC_DEPTH:
            ChangeInnerZZDepth(+1);
            break;
        case EGLOBAL_EVT_OANDA_SIGN_IN:
            OnOandaSignInReply(cEvent.GetIntParam(1));
            break;
        case EGLOBAL_EVT_OANDA_TRADE_PAIR_QUERY:
            OnOandaTradePairQueryReply(cEvent.GetIntParam(1));
            break;
        case EGLOBAL_EVT_AUTOMATION_MEGAZZ_UPDATED:
            OnUpdateMegaZZ(cEvent.GetIntParam(1));
            break;
        case EEVENT_SELECTED_TRADE_PAIR:
            OnTradePairSelected(cEvent.GetIntParam(1));
            break;
        case EEVENT_LISTEN_TO_RYTHM:
            COandaMgr::GetInstance().ListenToRythm(cEvent.GetIntParam(1));
            break;
        case EEVENT_FETCH_SPREAD:
            OnFetchSpreadCallback();
            break;
        case EGLOBAL_EVT_OANDA_FETCH_SPREADS_REPLY:
            UpdateSpreadLabel();
            break;
    }
}
VOID CForexWindow::OnOandaSignInReply(BOOLEAN bSuccess)
{
    if (CNowLoadingWindow::IsDisplayed()) {
        CNowLoadingWindow::HideWindow();
    }
    if (!bSuccess) {
        CConfirmationWindow::DisplayErrorMsg("Failed Oanda Sign In", NULL, 0);
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

VOID CForexWindow::OnOandaTradePairQueryReply(BOOLEAN bSuccess)
{
    if (CNowLoadingWindow::IsDisplayed()) {
        CNowLoadingWindow::HideWindow();
    }
    if (!bSuccess) {
        CConfirmationWindow::DisplayErrorMsg("Failed Fetch Oanda Trade Pairs!", NULL, 0);
        return;
    }
    COandaMgr::GetInstance().ListenToRythm(0);
    
    for (UINT uIndex = 1; SOURCE_NUM > uIndex; ++uIndex) {
        CEvent cListenEvent(EEVENT_LISTEN_TO_RYTHM, uIndex);
        PostEvent(cListenEvent, uIndex * 1.0f);
    }
    m_cDashboard.Initialize(ESOURCE_MARKET_OANDA, SOURCE_NUM);
    AddChild(m_cDashboard);
    
    m_cLive.Init();
    m_cLive.SetLocalPosition(10.0f, m_cDashboard.GetLocalPosition().y + m_cDashboard.GetLocalSize().y + 5.0f);
    AddChild(m_cLive);
    const FLOAT fOandaX = 5.0f;
    const FLOAT fOandaY = 55.0f + 300.0f + m_cLive.GetLocalSize().y + 5.0f;
    m_cOAccount.SetLocalPosition(fOandaX, fOandaY);
    m_cOAccount.Initialize();
    AddChild(m_cOAccount);

    m_cTrades.SetLocalPosition(fOandaX + 105.0f, fOandaY);
    m_cTrades.Initialize();
    AddChild(m_cTrades);

    
    OnTradePairSelected(m_uCurrentTradePairIndex);
    
    CEvent cSpreadEvent(EEVENT_FETCH_SPREAD);
    PostEvent(cSpreadEvent, 1.0f);
}

VOID CForexWindow::OnTradePairSelected(UINT uTradePairIndex)
{
    m_uCurrentTradePairIndex = uTradePairIndex;
    CHAR szBuffer[512];
    strncpy(szBuffer, CCryptoUtil::GetTradePairName(ESOURCE_MARKET_OANDA, uTradePairIndex), 512);
    m_cName.SetString(szBuffer);

    OnUpdateMegaZZ(uTradePairIndex);
    UpdateSpreadLabel();
}
VOID CForexWindow::OnUpdateMegaZZ(UINT uTradePairIndex)
{
    if (m_uCurrentTradePairIndex != uTradePairIndex) { 
        return;
    }
    const CMegaZZ* pcMegaZZ =  CAutomationMgr::GetInstance().FindMegaZZ(uTradePairIndex, CAutomationMgr::GetInstance().GetFractalIndex());
    if (NULL == pcMegaZZ) {
        return;
    }
    CHAR szBuffer[512];
    COandaMgr::GetInstance().GetPriceDisplayString(szBuffer, 512, uTradePairIndex, pcMegaZZ->GetCurrentPrice());
    m_cPrice.SetString(szBuffer);
    m_cBoxMain.PrepareChart(*pcMegaZZ);
    m_cBoxSub.PrepareChart(*pcMegaZZ);
//    m_cLeft.PrepareChart(*pcMegaZZ, m_uInnerZZDepth);
}

#include "AutoChecker.h"
VOID CForexWindow::OnTapCheckUI(VOID)
{
    CAutoChecker* pcWin = new CAutoChecker();
    if (NULL != pcWin) {
        pcWin->DoModal();
    }
}

VOID CForexWindow::OnFetchSpreadCallback(VOID)
{
    CEvent cSpreadEvent(EEVENT_FETCH_SPREAD);
    PostEvent(cSpreadEvent, 1.5f);
    UINT auTradePairIndex[SOURCE_NUM];
    for (UINT uIndex = 0; SOURCE_NUM > uIndex; ++uIndex) {
        auTradePairIndex[uIndex] = uIndex;
    }
    COandaMgr::GetInstance().QueryLatestSpreads(auTradePairIndex, SOURCE_NUM);
}

VOID CForexWindow::UpdateSpreadLabel(VOID)
{
    const COandaMgr::SOandaPair* psPair = COandaMgr::GetInstance().GetTradePair(m_uCurrentTradePairIndex);
    if (NULL == psPair) {
        return;
    }
    const FLOAT fAskPip = (psPair->fAskPrice - psPair->dPrice) * psPair->dPipMultiplier;
    const FLOAT fBidPip = (psPair->dPrice - psPair->fBidPrice) * psPair->dPipMultiplier;
    const FLOAT fSpread = (psPair->fAskPrice - psPair->fBidPrice) * psPair->dPipMultiplier;
    CHAR szBuffer[1024];
    snprintf(szBuffer, 1024, "Ask: %.1fpip\nBid: %.1fpip\nSpread: %.1fpip", fAskPip, fBidPip, fSpread);
    m_cSpread.SetString(szBuffer);
    if (3.0f > fSpread) {
        m_cSpread.SetColor(0xFFFFFFFF);
    }
    else {
        m_cSpread.SetColor(RGBA(0xFF, 0x4F, 0x4F, 0xFF));
    }
}

VOID CForexWindow::ChangeInnerZZDepth(INT nChange)
{
    const UINT uFractalIndex = CAutomationMgr::GetInstance().GetFractalIndex();
    const CMegaZZ* pcMegaZZ = NULL;
    if (0 < nChange) {
        if (8 > m_uInnerZZDepth) {
            ++m_uInnerZZDepth;
            pcMegaZZ =  CAutomationMgr::GetInstance().FindMegaZZ(m_uCurrentTradePairIndex, uFractalIndex);
        }
    }
    else {
        if (0 < m_uInnerZZDepth) {
            --m_uInnerZZDepth;
            pcMegaZZ =  CAutomationMgr::GetInstance().FindMegaZZ(m_uCurrentTradePairIndex, uFractalIndex);
        }
    }
    if (NULL != pcMegaZZ) {
//        m_cLeft.PrepareChart(*pcMegaZZ, m_uInnerZZDepth);
    }
}
