#include "stdafx.h"
#include "EventManager.h"
#include "TradeLog.h"
#include "TradeLogButton.h"
#include "TradeLogWidget.h"
#include "PatternData.h"
#define CLOSED_W (100.0f)
#define CLOSED_H (30.0f)

#define LOG_BG_W (300.0f)
#define LOG_BG_H (400.0f)

#define LOG_WIDGET_BG_COLOR RGBA(0x1F, 0x1F, 0x1F, 0xFF)

CTradeLogButton::CTradeLogButton() :
CUIContainer(),
INITIALIZE_TEXT_LABEL(m_cLogTitle),
INITIALIZE_TEXT_LABEL(m_cClearTxt)
#if defined(MAC_PORT)
,INITIALIZE_TEXT_LABEL(m_cExportTxt)
#endif // #if defined(MAC_PORT)

{
    
}
CTradeLogButton::~CTradeLogButton()
{
    
}

BOOLEAN CTradeLogButton::Initialize(VOID)
{
    Release(); // safety
    m_cHeaderBG.SetLocalSize(CLOSED_W, CLOSED_H);
    m_cHeaderBG.SetColor(RGBA(0x4F, 0x4F, 0x4F, 0xFF));
    AddChild(m_cHeaderBG);

    m_cLogTitle.SetFont(EGAMEFONT_SIZE_14);
    m_cLogTitle.SetEffect(CUITextLabel::EFFECT_SHADOW, 1.0f, 1.0f);
    m_cLogTitle.SetAnchor(0.5f, 0.5f);
    m_cLogTitle.SetLocalPosition(CLOSED_W * 0.5f, CLOSED_H * 0.5f);
    AddChild(m_cLogTitle);
    
    m_cGraph.Initialize();
    m_cGraph.SetVisibility(FALSE);
    const SHLVector2D& sGraphSize = m_cGraph.GetLocalSize();
    const FLOAT fX = LOG_BG_W;
    const FLOAT fY = -sGraphSize.y - 5.0f;
    m_cGraph.SetLocalPosition(fX, fY);
    AddChild(m_cGraph);
    
    const FLOAT fListX = 0.0f;
    m_cLogBG.SetColor(LOG_WIDGET_BG_COLOR);
    m_cLogBG.SetBorderColor(RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    m_cLogBG.SetLocalPosition(fListX, fY + 25.0f);
    m_cLogBG.SetLocalSize(LOG_BG_W, LOG_BG_H);
    m_cLogBG.SetVisibility(FALSE);
    AddChild(m_cLogBG);
    m_cList.SetLocalPosition(fListX, fY + 25.0f);
    m_cList.SetLocalSize(LOG_BG_W, LOG_BG_H);
    m_cList.SetVisibility(FALSE);
    AddChild(m_cList);
    
    UpdateTitle();
    
    m_cClearBtnBG.SetLocalSize(60.0f, 20.0f);
    m_cClearBtnBG.SetBorderColor(0xFFFFFFFF);
    m_cClearBtnBG.SetColor(RGBA(0x4F, 0x4F, 0x4F, 0xFF));
    m_cClearBTn.SetDisplayWidgets(m_cClearBtnBG, m_cClearBtnBG);
    m_cClearBTn.SetID(EBTN_CLEAR);
    m_cClearBTn.SetLocalPosition(fListX + LOG_BG_W - 65.0f, fY);
    m_cClearBTn.SetVisibility(FALSE);
    AddChild(m_cClearBTn);
    m_cClearTxt.SetFont(EGAMEFONT_SIZE_12);
    m_cClearTxt.SetEffect(CUITextLabel::EFFECT_SHADOW, 1.0f, 1.0f);
    m_cClearTxt.SetAnchor(0.5f, 0.5f);
    m_cClearTxt.SetLocalPosition(30.0f, 10.0f);
    m_cClearTxt.SetString("Clear");
    m_cClearBTn.AddChild(m_cClearTxt);

#if defined(MAC_PORT)
    m_cExportBtn.SetDisplayWidgets(m_cClearBtnBG, m_cClearBtnBG);
    m_cExportBtn.SetID(EBTN_EXPORT);
    m_cExportBtn.SetLocalPosition(fListX + LOG_BG_W - 65.0f - 65.0f, fY);
    m_cExportBtn.SetVisibility(FALSE);
    AddChild(m_cExportBtn);
    m_cExportTxt.SetFont(EGAMEFONT_SIZE_12);
    m_cExportTxt.SetEffect(CUITextLabel::EFFECT_SHADOW, 1.0f, 1.0f);
    m_cExportTxt.SetAnchor(0.5f, 0.5f);
    m_cExportTxt.SetLocalPosition(30.0f, 10.0f);
    m_cExportTxt.SetString("Export");
    m_cExportBtn.AddChild(m_cExportTxt);
#endif // #if defined(MAC_PORT)

    CEventManager::RegisterForBroadcast(*this, EGLOBAL_TRADE_LOG, EGLOBAL_EVT_TRADE_LOG_REFRESHED);
    m_bRegistered = TRUE;
    return TRUE;
}

VOID CTradeLogButton::Release(VOID)
{
    if (m_bRegistered) {
        CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_TRADE_LOG, EGLOBAL_EVT_TRADE_LOG_REFRESHED);
        m_bRegistered = FALSE;
    }
    m_cHeaderBG.RemoveFromParent();
    m_cLogTitle.Release();
    m_cLogTitle.RemoveFromParent();
    
    m_cGraph.Release();
    m_cGraph.RemoveFromParent();
    
    m_cLogBG.RemoveFromParent();
    
    m_cList.Release();
    m_cList.RemoveFromParent();
    
    m_cClearTxt.Release();
    m_cClearTxt.RemoveFromParent();
    m_cClearBTn.RemoveFromParent();
    
#if defined(MAC_PORT)
    m_cExportTxt.Release();
    m_cExportTxt.RemoveFromParent();
    m_cExportBtn.RemoveFromParent();
#endif // #if defined(MAC_PORT)
}

VOID CTradeLogButton::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    CEventManager::PostEvent(*this, cEvent, fDelay);
}

VOID CTradeLogButton::UpdateTitle(VOID)
{
    UINT uNum = (UINT)CTradeLog::GetLogs().size();
    uNum += (UINT)CTradeLog::GetActiveLogs().size();
    CHAR szBuffer[128];
    snprintf(szBuffer, 128, "Trade(%d)", uNum);
    m_cLogTitle.SetString(szBuffer);
}


BOOLEAN CTradeLogButton::OnTouchBegin(FLOAT fX, FLOAT fY)
{
    m_bUITouched = CUIContainer::OnTouchBegin(fX, fY);
    if (m_bUITouched) {
        ReattachToParent();
        return TRUE;
    }
    const SHLVector2D sPos = { fX, fY };
    BOOLEAN bHit = m_cHeaderBG.HitTest(sPos);
    if (!bHit && m_cGraph.IsVisible()) {
        bHit = m_cGraph.HitTest(sPos);
    }
    if (bHit) {
        m_bScrollMode = FALSE;
        m_afPrevTouch[0] = fX;
        m_afPrevTouch[1] = fY;
        ReattachToParent();
        return TRUE;
    }
    return FALSE;
}
BOOLEAN CTradeLogButton::OnTouchMove(FLOAT fX, FLOAT fY)
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

BOOLEAN CTradeLogButton::OnTouchEnd(FLOAT fX, FLOAT fY)
{
    if (m_bUITouched) {
        return CUIContainer::OnTouchEnd(fX, fY);
    }
    if (m_bScrollMode) {
        return TRUE;
    }
    // Toggle open/close log
    const SHLVector2D sPos = { fX, fY };
    const BOOLEAN bHit = m_cHeaderBG.HitTest(sPos);
    if (bHit) {
        ToggleLog();
    }
    return TRUE;
}


VOID CTradeLogButton::ToggleLog(VOID)
{
    const BOOLEAN bVisibility = !m_cLogBG.IsVisible();
    m_cLogBG.SetVisibility(bVisibility);
    m_cList.SetVisibility(bVisibility);
    m_cGraph.SetVisibility(bVisibility);
    m_cClearBTn.SetVisibility(bVisibility);
#if defined(MAC_PORT)
    m_cExportBtn.SetVisibility(bVisibility);
#endif // #if defined(MAC_PORT)
    if (bVisibility) {
        m_cList.Initialize(EBTN_TAP_WIDGET);
    }
}

VOID CTradeLogButton::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EGLOBAL_EVT_TRADE_LOG_REFRESHED:
            UpdateTitle();
            break;
        case EBTN_TAP_WIDGET:
            OnTapWidget((CTradeLogWidget*)cEvent.GetPtrParam(1));
            break;
        case EBTN_REATTACH:
            {
                CUIContainer* pcParent = GetParent();
                if (NULL != pcParent) {
                    pcParent->ReAttachChild(*this);
                }
            }
            break;
        case EBTN_CLEAR:
            CTradeLog::Clear();
            m_cGraph.Clear();
            if (m_cLogBG.IsVisible()) {
                m_cList.Initialize(EBTN_TAP_WIDGET);
            }
            break;
#if defined(MAC_PORT)
        case EBTN_EXPORT:
            OnTapExportBtn();
            break;
#endif // #if defined(MAC_PORT)
    }
}

VOID CTradeLogButton::ReattachToParent(VOID)
{
    CEvent cEvent(EBTN_REATTACH);
    PostEvent(cEvent, 0.0f);
}

VOID CTradeLogButton::OnTapWidget(CTradeLogWidget* pcWidget)
{
    if (NULL == pcWidget) {
        return;
    }
    if (!m_cList.SelectWidget(*pcWidget)) { // if cannot select widget, skip
        return;
    }
    const STradeLog& sLog = pcWidget->GetLog();
    m_cGraph.Update(sLog);
}

#if defined(MAC_PORT)
#include "AppResourceManager.h"
#include "TradeLogWidget.h"
#include "OandaMgr.h"
#include "AutomationMgr.h"
VOID CTradeLogButton::OnTapExportBtn(VOID)
{
    CHAR szFilePath[1024];
    CAppResourceManager::GetLocalStoragePath("Trade.csv", szFilePath, 1024);
    FILE* phFile = fopen(szFilePath, "w");
    if (NULL == phFile) {
        ASSERT(FALSE);
        return;
    }
    fprintf(phFile, "StartTime,EndTime,Open,Close,Pair,Name,Entry,Was,F9,F8,F7,F6,F5,F4,F3,F2,F1,Is,F9,F8,F7,F6,F5,F4,F3,F2,F1,Pip,Highest,Lowest\n");
    CHAR szTimeString[1024];
    CHAR szEndTimeString[1024];
    CHAR szOpenPrice[32];
    CHAR szClosePrice[32];
    const CTradeLogs& cLogs = CTradeLog::GetLogs();
    for (auto& sLog : cLogs) {
        CTradeLogWidget::GetTimeString(sLog.uOpenTimeSec, szTimeString, 1024);
        if (0 < sLog.uCloseTimeSec) {
            CTradeLogWidget::GetTimeString(sLog.uCloseTimeSec, szEndTimeString, 1024);
        }
        else {
            snprintf(szEndTimeString, 1024, "not closed");
        }
        const COandaMgr::SOandaPair* psPair = COandaMgr::GetInstance().GetTradePair(sLog.byTradePairIndex);
        const CHAR* szPairName = "unknown";
        if (NULL != psPair) {
            szPairName = psPair->szTradePair;
        }
        const CHAR* szEntry = sLog.byLong ? "LONG" : "SHORT";
        const CHAR* szName = CPatternDataMgr::GetPattern(sLog.uCategoryPatternIndex).szName;
        COandaMgr::GetInstance().GetPriceDisplayString(szOpenPrice, 32, sLog.byTradePairIndex, sLog.fOpenPrice);
        COandaMgr::GetInstance().GetPriceDisplayString(szClosePrice, 32, sLog.byTradePairIndex, sLog.fCLosePrice);
        fprintf(phFile, "%s,%s,%s,%s,%s,%s,%s,,", szTimeString, szEndTimeString, szOpenPrice, szClosePrice, szPairName, szName, szEntry);
        for (UINT uI = 0; 9 > uI; ++uI) {
            const MegaZZ::SChannel& sChannel = sLog.asStart[uI];
            const BOOLEAN bUp = sChannel.byType == MegaZZ::EType_Up;
            if (bUp) {
                fprintf(phFile, "U%d,", sChannel.byRetracementNumber);
            }
            else {
                fprintf(phFile, "D%d,", sChannel.byRetracementNumber);
            }
        }
        fprintf(phFile, ",");
        for (UINT uI = 0; 9 > uI; ++uI) {
            const MegaZZ::SChannel& sChannel = sLog.asStart[uI];
            const BOOLEAN bUp = sChannel.byType == MegaZZ::EType_Up;
            if (bUp) {
                fprintf(phFile, "U%d,", sChannel.byDisplayNumber);
            }
            else {
                fprintf(phFile, "D%d,", sChannel.byDisplayNumber);
            }
        }
        FLOAT fGain;
        if (sLog.byLong) {
            fGain = (sLog.fCLosePrice - sLog.fOpenPrice) * psPair->dPipMultiplier;
        }
        else {
            fGain = (sLog.fOpenPrice - sLog.fCLosePrice) * psPair->dPipMultiplier;
        }
        fprintf(phFile, "%.1f,%.1f,%.1f\n", fGain, sLog.fHighestPipReached, sLog.fLowestPipReached);
    }
    
    fclose(phFile);
    
    CAutomationMgr::GetInstance().ExportBibLog();
}
#endif // #if defined(MAC_PORT)
