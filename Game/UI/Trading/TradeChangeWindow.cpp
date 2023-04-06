#include "stdafx.h"
#include "TradeChangeWindow.h"
#include "CMain.h"
#include "Event.h"
#include "EventID.h"
#include "EventManager.h"

#include "BinanceMgr.h"
#include "CryptoDotComMgr.h"
#include "OandaMgr.h"

#define SELECTED_COLOR      RGBA(0x1F,0x1F,0xFF,0xFF)
#define UNSELECTED_COLOR    RGBA(0x7F, 0x7F, 0x7F, 0xFF)

#define CREATE_BTN(x, y, w, h, id, text) CreateColorButton(x, y, w, h, id, UNSELECTED_COLOR, text, TRUE)

static FLOAT s_fLastScrollerOffset = 0.0f;

CTradeChangeWindow::CTradeChangeWindow(ESOURCE_MARKET eSource, UINT uSelectedTradeIndex) :
CGameWindow(),
m_eMarket(eSource),
m_uSelectedTradeIndex(uSelectedTradeIndex),
m_cScroller(FALSE)
{
    m_bDestroyOnExitModal = TRUE;
}

CTradeChangeWindow::~CTradeChangeWindow()
{
}
VOID CTradeChangeWindow::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();
    
    UINT uTradeNum = 0;
    switch (m_eMarket) {
        case ESOURCE_MARKET_CRYPTO_COM:
            uTradeNum = CCryptoDotComMgr::GetInstance().GetTradePairNum();
            break;
        case ESOURCE_MARKET_BINANCE:
            uTradeNum = CBinanceMgr::GetInstance().GetTradePairNum();
            break;
        case ESOURCE_MARKET_OANDA:
            uTradeNum = COandaMgr::GetInstance().GetTradePairNum();
            break;
        default:
            ASSERT(FALSE);
            break;
    }
    const FLOAT fScreenW = CMain::GetScreenWidth();
    const FLOAT fMaxW = CMain::GetScreenWidth() - 2.0f * CMain::GetDisplaySideMargin();
    const FLOAT fScreenHeight = CMain::GetScreenHeight() - CMain::GetDisplayBottomMargin();
    SetAnchor(0.5f, 0.5f);
    SetLocalPosition(fScreenW * 0.5f, fScreenHeight * 0.5f);

    const FLOAT fMaxH = fScreenHeight - 20.0f;
    
    UINT uMaxRowNum = 0;
    FLOAT fY = 80.0f;
    while (fMaxH > (fY + 70.0f)) { // while we can insert another row
        ++uMaxRowNum;
        fY += 70.0f;
    }
    if (0 == uMaxRowNum) {
        ASSERT(FALSE);
        return;
    }
    const UINT uColNum = (uTradeNum + (uMaxRowNum-1)) / uMaxRowNum;
    ASSERT(0 < uColNum);
    FLOAT fScrollerW = uColNum * 90.0f + (uColNum - 1) * 10.0f;
    if ((fScrollerW + 20.0f) > fMaxW) {
        fScrollerW = fMaxW - 20.0f;
    }
    const FLOAT fScrollerH = uMaxRowNum * 60.0f + (uMaxRowNum - 1) * 10.0f;
    m_cScroller.SetLocalPosition(10.0f, 80.0f);
    m_cScroller.SetLocalSize(fScrollerW, fScrollerH);
    const FLOAT fWinW = fScrollerW + 20.0f;
    const FLOAT fWinH = 80.0f + fScrollerH + 10.0f;
    SetLocalSize(fWinW, fWinH);
    
    m_cBG.SetLocalSize(fWinW, fWinH);
    m_cBG.Set9PartTexture(ETEX::ID::ui_9part_filter_off, 3.0f, 3.0f, 3.0f, 3.0f);
    m_cBG.SetColor(RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    AddChild(m_cBG);
    
    CREATE_BTN((fWinW - 70.0f), 10.0f, 60.0f, 60.0f, EBTN_EXIT, "Quit");

    AddChild(m_cScroller);
    
    CHAR szBuffer[64];
    UINT uIndex = 0;
    fY = 0.0f;
    for ( ; uTradeNum > uIndex; ++uIndex) {
        const CHAR* szName = NULL;
        switch (m_eMarket) {
            case ESOURCE_MARKET_CRYPTO_COM:
                {
                    const STradePair* psTrade = CCryptoDotComMgr::GetInstance().GetTradePair(uIndex);
                    if (NULL != psTrade) {
                        szName = psTrade->szBase;
                    }
                }
                break;
            case ESOURCE_MARKET_BINANCE:
                {
                    const STradePair* psTrade = CBinanceMgr::GetInstance().GetTradePair(uIndex);
                    if (NULL != psTrade) {
                        szName = psTrade->szBase;
                    }
                }
                break;
            case ESOURCE_MARKET_OANDA:
                {
                    const COandaMgr::SOandaPair* psPair = COandaMgr::GetInstance().GetTradePair(uIndex);
                    if (NULL != psPair) {
                        szName = psPair->szTradePair;
                    }
                }
                break;
            default:
                ASSERT(FALSE);
                return;
        }
        if (NULL == szName) {
            ASSERT(FALSE);
            return;
        }
        const UINT uRowIndex = uIndex % uMaxRowNum;
        const UINT uColIndex = uIndex / uMaxRowNum;
        const FLOAT fBtnX = uColIndex * 100.0f;
        const FLOAT fBtnY = uRowIndex * 70.0f;
        snprintf(szBuffer, 64, "%s", szName);
        CUIButton* pcBtn = CREATE_BTN(fBtnX, fBtnY, 90.0f, 60.0f, EBTN_CHANGE_BTN + uIndex, szBuffer);
        if (NULL != pcBtn) {
            if (uIndex == m_uSelectedTradeIndex) {
                CUIImage* pcImg = CUIImage::CastToMe(pcBtn->GetIdleWidget());
                if (NULL != pcImg) {
                    pcImg->SetColor(SELECTED_COLOR);
                }
            }
            pcBtn->RemoveFromParent();
            m_cScroller.AddChild(*pcBtn);
        }
    }
    m_cScroller.TrySetOffSet(s_fLastScrollerOffset);
}

VOID CTradeChangeWindow::Release(VOID)
{
    s_fLastScrollerOffset = m_cScroller.GetDisplayOffset();
    CUIWidget* pcWidget = m_cScroller.GetChild();
    while (NULL != pcWidget) {
        CUIWidget* pcTmp = pcWidget;
        pcWidget = pcWidget->GetNextSibling();
        pcTmp->RemoveFromParent();
    };
    m_cBG.RemoveFromParent();
    CGameWindow::Release();
}

VOID CTradeChangeWindow::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_EXIT:
            ExitModal();
            return;
        
        default:
            OnTradeBtnPressed(uEventID - EBTN_CHANGE_BTN);
            break;
    }
}

VOID CTradeChangeWindow::OnTradeBtnPressed(UINT uIndex)
{
    if (m_uSelectedTradeIndex != uIndex) {
        CEventManager::PostGlobalEvent(EGLOBAL_TRADE_MAIN_WINDOW, CEvent(EGLOBAL_EVT_TRADE_MAIN_WINDOW_CHANGE_TRADE, uIndex));
    }
    ExitModal();
}
