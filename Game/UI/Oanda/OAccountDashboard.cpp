#include "stdafx.h"
#include "CMain.h"
#include "Event.h"
#include "EventManager.h"
#include "NowLoadingWindow.h"
#include "OAccountDashboard.h"
#include "OandaMgr.h"

#define ORDERS_W    (OTRADE_WIDGET_W)
#define ORDERS_H    (200.0f)
#define DASHBOARD_W (ORDERS_W)
#define DASHBOARD_H (66.0f)


VOID COAccountDashboard::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    // do not post to parent.
    CEventManager::PostEvent(*this, cEvent, fDelay);
}


COAccountDashboard::COAccountDashboard() :
CUIContainer(),
#if defined(USE_DEMO_ACCOUNT)
INITIALIZE_TEXT_LABEL(m_cDemoAcct),
#endif // #if defined(USE_DEMO_ACCOUNT)
m_bRefreshAll(TRUE),
m_bUITouched(FALSE),
m_bScrollMode(FALSE),
INITIALIZE_TEXT_LABEL(m_cTradePair),
m_uTradePairIndex(0),
m_uPendingCreateOrderMsgID(0)
{
    m_afPrevTouch[0] = m_afPrevTouch[1] = 0.0f;
    memset(m_apcAccountID, 0, sizeof(m_apcAccountID));
    memset(m_apcBalance, 0, sizeof(m_apcBalance));
    memset(m_apcMarginAvail, 0, sizeof(m_apcMarginAvail));
    memset(m_apcMarginUsed, 0, sizeof(m_apcMarginUsed));
}
    
COAccountDashboard::~COAccountDashboard()
{
}
 
static VOID SetupTexLbl(CUITextLabel& cLbl, FLOAT fX, FLOAT fY, FLOAT fW, FLOAT fH)
{
    cLbl.SetFont(EGAMEFONT_SIZE_12);
    cLbl.SetAlignment(CUITextLabel::EALIGNMENT_LEFT);
    cLbl.SetAnchor(0.0f, 0.0f);
    cLbl.SetLocalPosition(fX, fY);
    cLbl.SetLocalSize(fW, fH);
}
VOID COAccountDashboard::Initialize(VOID)
{
    m_cTradePairBG.SetLocalSize(100.0f, 30.0f);
    m_cTradePairBG.SetColor(RGBA(0x4F, 0x4F, 0x4F, 0xFF));
    AddChild(m_cTradePairBG);
    
    m_cTradePair.SetFont(EGAMEFONT_SIZE_14);
    m_cTradePair.SetEffect(CUITextLabel::EFFECT_SHADOW, 1.0f, 1.0f);
    m_cTradePair.SetAnchor(0.5f, 0.5f);
    m_cTradePair.SetLocalPosition(50.0f, 15.0f);
    m_cTradePair.SetString("Oanda Acct");
    AddChild(m_cTradePair);
    
    m_cBody.SetLocalPosition(0.0f, -DASHBOARD_H - ORDERS_H - 5.0f);
    m_cBody.SetLocalSize(DASHBOARD_W * 2.0f, DASHBOARD_H + ORDERS_H);

    for (UINT uIndex = 0; 2 > uIndex; ++uIndex) {
        CUIImage& cImg = m_acBG[uIndex];
        cImg.SetLocalSize(DASHBOARD_W, DASHBOARD_H);
        const FLOAT fX = uIndex * (DASHBOARD_W);
        cImg.SetLocalPosition(fX, 0.0f);
        cImg.SetColor(RGBA(0x1F, 0x1F, 0x1F, 0xFF));
        cImg.SetBorderColor(RGBA(0x7F, 0x7F, 0x7F, 0xFF));
        m_cBody.AddChild(cImg);

        CUIImage& cOrdersBG = m_acOrdersBG[uIndex];
        cOrdersBG.SetLocalPosition(fX, DASHBOARD_H);
        cOrdersBG.SetLocalSize(ORDERS_W, ORDERS_H);
        cOrdersBG.SetColor(RGBA(0x1F, 0x1F, 0x1F, 0xFF));
        cOrdersBG.SetBorderColor(RGBA(0x7F, 0x7F, 0x7F, 0xFF));
        m_cBody.AddChild(cOrdersBG);

        COTradeList& cList = m_acTradeList[uIndex];
        cList.SetLocalPosition(fX, DASHBOARD_H);
        cList.SetLocalSize(ORDERS_W, ORDERS_H);
        cList.Initialize(EBTN_SELETED_TRADE, uIndex);
        m_cBody.AddChild(cList);

        CUITextLabel* pcLbl = NEW_TEXT_LABEL;
        if (NULL == pcLbl) {
            ASSERT(FALSE);
            return;
        }
        SetupTexLbl(*pcLbl, fX + 3.0f, 0.0f, DASHBOARD_W, 16.0f);
        m_cBody.AddChild(*pcLbl);
        m_apcAccountID[uIndex] = pcLbl;

        pcLbl = NEW_TEXT_LABEL;
        if (NULL == pcLbl) {
            ASSERT(FALSE);
            return;
        }
        SetupTexLbl(*pcLbl, fX + 3.0f, 16.0f, 180.0f, 16.0f);
        m_cBody.AddChild(*pcLbl);
        m_apcBalance[uIndex] = pcLbl;

        pcLbl = NEW_TEXT_LABEL;
        if (NULL == pcLbl) {
            ASSERT(FALSE);
            return;
        }
        SetupTexLbl(*pcLbl, fX + 3.0f, 32.0f, 180.0f, 16.0f);
        m_cBody.AddChild(*pcLbl);
        m_apcMarginAvail[uIndex] = pcLbl;

        pcLbl = NEW_TEXT_LABEL;
        if (NULL == pcLbl) {
            ASSERT(FALSE);
            return;
        }
        SetupTexLbl(*pcLbl, fX + 3.0f, 48.0f, 180.0f, 16.0f);
        m_cBody.AddChild(*pcLbl);
        m_apcMarginUsed[uIndex] = pcLbl;
    }
#if defined(USE_DEMO_ACCOUNT)
    SetupTexLbl(m_cDemoAcct, DASHBOARD_W * 2.0f, 0.0f, 100.0f, 16.0f);
    m_cDemoAcct.SetEffect(CUITextLabel::EFFECT_4SIDE_DIAG, 1.0f, 1.0f);
    m_cDemoAcct.SetAnchor(1.0f, 1.0f);
    m_cDemoAcct.SetString("Demo Acct");
    m_cBody.AddChild(m_cDemoAcct);
#endif // #if defined(USE_DEMO_ACCOUNT)

    CEventManager::RegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_CLOSE_TRADE_REPLY);
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_CREATE_MARKET_ORDER_REPLY);
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_OPEN_TRADES_REPLY);
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_ACCOUNT_DETAILS_UPDATED);
    if (COandaMgr::GetInstance().IsSignedIn()) {
        OnTapRefresh();
    }
    else {
        CEventManager::RegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_SIGN_IN, TRUE);
    }
}

VOID COAccountDashboard::Release(VOID)
{
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_CLOSE_TRADE_REPLY);
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_CREATE_MARKET_ORDER_REPLY);
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_OPEN_TRADES_REPLY);
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_OANDA_MGR, EGLOBAL_EVT_OANDA_ACCOUNT_DETAILS_UPDATED);
    
    m_cTradePair.Release();
    m_cTradePair.RemoveFromParent();
    m_cTradePairBG.RemoveFromParent();

    for (UINT uIndex = 0; 2 > uIndex; ++uIndex) {
        m_acBG[uIndex].RemoveFromParent();
        m_acOrdersBG[uIndex].RemoveFromParent();
        m_acTradeList[uIndex].Release();
        m_acTradeList[uIndex].RemoveFromParent();
    }
    
#if defined(USE_DEMO_ACCOUNT)
    m_cDemoAcct.Release();
    m_cDemoAcct.RemoveFromParent();
#endif // #if defined(USE_DEMO_ACCOUNT)

    for (UINT uIndex = 0; 2 > uIndex; ++uIndex) {
        SAFE_REMOVE_RELEASE_DELETE(m_apcAccountID[uIndex]);
        SAFE_REMOVE_RELEASE_DELETE(m_apcBalance[uIndex]);
        SAFE_REMOVE_RELEASE_DELETE(m_apcMarginAvail[uIndex]);
        SAFE_REMOVE_RELEASE_DELETE(m_apcMarginUsed[uIndex]);
    }
}

VOID COAccountDashboard::UpdateLabels(UINT uAccountIndex)
{
    ASSERT(2 > uAccountIndex);
    const COandaMgr::SOandaAccount* psAcct = COandaMgr::GetInstance().GetAccount(uAccountIndex);
    if (NULL == psAcct) {
        return;
    }
    CHAR szBuffer[256];
    if (NULL != m_apcAccountID[uAccountIndex]) {
        snprintf(szBuffer, 256, "Acct: %s", psAcct->szAccountID);
        m_apcAccountID[uAccountIndex]->SetString(szBuffer);
    }
    if (NULL != m_apcBalance[uAccountIndex]) {
        snprintf(szBuffer, 256, "Bal: %.2f %s", psAcct->dBalance, psAcct->szCurrency);
        m_apcBalance[uAccountIndex]->SetString(szBuffer);
    }
    if (NULL != m_apcMarginAvail[uAccountIndex]) {
        snprintf(szBuffer, 256, "Margin Avail: %.2f", psAcct->dMarginAvailable);
        m_apcMarginAvail[uAccountIndex]->SetString(szBuffer);
    }
    if (NULL != m_apcMarginUsed[uAccountIndex]) {
        snprintf(szBuffer, 256, "Margin Used: %.2f", psAcct->dMarginUsed);
        m_apcMarginUsed[uAccountIndex]->SetString(szBuffer);
    }
}

BOOLEAN COAccountDashboard::OnTouchBegin(FLOAT fX, FLOAT fY)
{
    m_bUITouched = CUIContainer::OnTouchBegin(fX, fY);
    if (m_bUITouched) {
        ReattachToParent();
        return TRUE;
    }
    const SHLVector2D sPos = { fX, fY };
    BOOLEAN bHitTest = m_cTradePairBG.HitTest(sPos);
    if (!bHitTest && NULL != m_cBody.GetParent()) {
        bHitTest = m_cBody.HitTest(sPos);
    }
    if (bHitTest) {
        ReattachToParent();
        m_bScrollMode = FALSE;
        m_afPrevTouch[0] = fX;
        m_afPrevTouch[1] = fY;
        return TRUE;
    }
    return FALSE;
}
BOOLEAN COAccountDashboard::OnTouchMove(FLOAT fX, FLOAT fY)
{
    if (m_bUITouched) {
        return CUIContainer::OnTouchMove(fX, fY);
    }
    const FLOAT fDiffX = fX - m_afPrevTouch[0];
    const FLOAT fDiffY = fY - m_afPrevTouch[1];
    if (!m_bScrollMode) {
        if ((fDiffX * fDiffX + fDiffY * fDiffY) > 25.0f) {
            m_bScrollMode = TRUE;
            m_afPrevTouch[0] = fX;
            m_afPrevTouch[1] = fY;
        }
        return TRUE;
    }
    m_afPrevTouch[0] = fX;
    m_afPrevTouch[1] = fY;
    SHLVector2D sLocalPos = GetLocalPosition();
    sLocalPos.x += fDiffX;
    sLocalPos.y += fDiffY;
    SetLocalPosition(sLocalPos.x, sLocalPos.y);
    return TRUE;
}

BOOLEAN COAccountDashboard::OnTouchEnd(FLOAT fX, FLOAT fY)
{
    if (m_bUITouched) {
        return CUIContainer::OnTouchEnd(fX, fY);
    }
    if (m_bScrollMode) {
        return TRUE;
    }

    const SHLVector2D sPos = { fX, fY };
    const BOOLEAN bHitTest = m_cTradePairBG.HitTest(sPos);
    if (bHitTest) {
        OnToggleDisplay();
    }
    return TRUE;
}

VOID COAccountDashboard::OnChangeTradePair(UINT uTradePairIndex)
{
    m_uTradePairIndex = uTradePairIndex;
    const COandaMgr::SOandaPair* psPair = COandaMgr::GetInstance().GetTradePair(m_uTradePairIndex);
    if (NULL == psPair) {
        ASSERT(FALSE);
        return;
    }
    CHAR szBuffer[128];
    snprintf(szBuffer, 128, "Trade: %s", psPair->szTradePair);
    m_cTradePair.SetString(szBuffer);
}

VOID COAccountDashboard::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EGLOBAL_EVT_OANDA_ACCOUNT_DETAILS_UPDATED:
            OnOandaAccountDetailsUpdated(cEvent.GetIntParam(1), cEvent.GetIntParam(2));
            break;
        case EGLOBAL_EVT_OANDA_OPEN_TRADES_REPLY:
            OnQueryOpenTrades(cEvent.GetIntParam(1), cEvent.GetIntParam(2));
            break;
        case EGLOBAL_EVT_OANDA_SIGN_IN: // oanda signed in successfully
            OnTapRefresh();
            break;
        case EBTN_REFRESH:
            OnTapRefresh();
            break;
        case EBTN_SELETED_TRADE:
            OnTapSelectTrade(cEvent.GetIntParam(1));
            break;
        case EBTN_REATTACH:
            {
                CUIContainer* pcParent = GetParent();
                if (NULL != pcParent) {
                    pcParent->ReAttachChild(*this);
                }
            }
            break;
        default:
            break;
    }
}

VOID COAccountDashboard::OnOandaAccountDetailsUpdated(UINT uAccountIndex, BOOLEAN bSuccess)
{
    if (2 <= uAccountIndex) {
        return;
    }
    if (!bSuccess) {
        return;
    }
    UpdateLabels(uAccountIndex);
    if (!m_bRefreshAll) {
        return;
    }
    if (0 == uAccountIndex) {
        COandaMgr::GetInstance().QueryAccountDetails(1);
        return;
    }
    // all accounts queried. try query trade
    COandaMgr::GetInstance().QueryOpenTrades(0);
}
VOID COAccountDashboard::RefreshOpenTrades(UINT uAccountIndex)
{
    COandaMgr::GetInstance().QueryOpenTrades(uAccountIndex);
}

VOID COAccountDashboard::OnQueryOpenTrades(UINT uAccountIndex, BOOLEAN bSuccess)
{
    if (2 <= uAccountIndex) {
        return;
    }
    if (!bSuccess) {
        return;
    }
    m_acTradeList[uAccountIndex].Initialize(EBTN_SELETED_TRADE, uAccountIndex);
    if (!m_bRefreshAll) {
        return;
    }
    if (0 == uAccountIndex) {
        COandaMgr::GetInstance().QueryOpenTrades(1);
        return;
    }
    // all refresh completed
    m_bRefreshAll = FALSE;
}

VOID COAccountDashboard::OnTapRefresh(VOID)
{
    m_bRefreshAll = TRUE;
    COandaMgr::GetInstance().QueryAccountDetails(0);
}

VOID COAccountDashboard::OnToggleDisplay(VOID)
{
    if (NULL != m_cBody.GetParent()) {
        m_cBody.RemoveFromParent();
    }
    else {
        AddChild(m_cBody);
    }
}


VOID COAccountDashboard::ReattachToParent(VOID)
{
    CEvent cEvent(EBTN_REATTACH);
    PostEvent(cEvent, 0.0f);
}

VOID COAccountDashboard::OnTapSelectTrade(UINT uTradePairIndex)
{
    const CEvent cEvent(EGLOBAL_EVT_RYTHM_DASHBOARD_CHANGE_TRADE_PAIR, uTradePairIndex);
    CEventManager::PostGlobalEvent(EGLOBAL_RYTHM_DASHBOARD, cEvent);
}

