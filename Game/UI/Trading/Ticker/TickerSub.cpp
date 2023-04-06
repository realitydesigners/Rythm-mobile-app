#include "stdafx.h"
#include "BinanceMgr.h"
#include "CMain.h"
#include "Event.h"
#include "EventID.h"
#include "EventManager.h"
#include "GameRenderer.h"
#include "TickerSub.h"
#include "TickerUIDefs.h"

CTickerSub::CTickerSub(UINT uTradeIndex) :
CUIContainer(),
m_uTradeIndex(uTradeIndex),
INITIALIZE_TEXT_LABEL(m_cTitle),
m_cPriceMins(FALSE),
m_cPriceSeconds(TRUE),
m_cOBVMins(FALSE),
m_cOBVSeconds(TRUE),
m_uMACDSignalColor(0),
m_bRenderVerticalLine(FALSE),
m_fVerticalX(0.0f),
m_bListening(FALSE)
{
    m_cPriceSeconds.SetMaster(&m_cPriceMins);
    m_cOBVSeconds.SetMaster(&m_cOBVMins);
    memset(m_auSignalColor, 0, sizeof(m_auSignalColor));
}

CTickerSub::~CTickerSub()
{
}

VOID CTickerSub::SetupTicker(VOID)
{
    const SHLVector2D& sLocalSize = GetLocalSize();
    const STradePair* psPair = CBinanceMgr::GetInstance().GetTradePair(m_uTradeIndex);
    if (NULL != psPair) {
        CHAR szBuffer[256];
        snprintf(szBuffer, 256, "%s-%s", psPair->szBase, psPair->szQuote);
        m_cTitle.SetFont(EGAMEFONT_SIZE_18);
        m_cTitle.SetString(szBuffer);
        m_cTitle.SetAnchor(0.5f, 0.0f);
        m_cTitle.SetLocalPosition(sLocalSize.x * 0.5f, 0.0f);
        m_cTitle.OnUpdate(0.0f);
        AddChild(m_cTitle);
    }
    const FLOAT fTitleHeight = m_cTitle.GetLocalSize().y;
    const FLOAT fSubTickerWidth = sLocalSize.x * 0.8;
    const FLOAT fSubTickerHeight = (sLocalSize.y - fTitleHeight) / 3.0f;
    const FLOAT fSubSecondsTickerWidth = sLocalSize.x * 0.2f;
    {   // init ticker price
        m_cPriceMins.SetLocalPosition(0.0f, fTitleHeight);
        m_cPriceMins.SetLocalSize(fSubTickerWidth, fSubTickerHeight);
        AddChild(m_cPriceMins);
        
        m_cPriceSeconds.SetLocalPosition(fSubTickerWidth, fTitleHeight);
        m_cPriceSeconds.SetLocalSize(fSubSecondsTickerWidth, fSubTickerHeight);
        AddChild(m_cPriceSeconds);
    }
    {   // init ticker obv
        m_cOBVMins.SetLocalPosition(0.0f, fTitleHeight + fSubTickerHeight);
        m_cOBVMins.SetLocalSize(fSubTickerWidth, fSubTickerHeight);
        AddChild(m_cOBVMins);
        m_cOBVSeconds.SetLocalPosition(fSubTickerWidth, fTitleHeight + fSubTickerHeight);
        m_cOBVSeconds.SetLocalSize(fSubSecondsTickerWidth, fSubTickerHeight);
        AddChild(m_cOBVSeconds);
    }
    {   // init ticker macd
        m_cMACD.SetLocalPosition(0.0f, fTitleHeight + fSubTickerHeight + fSubTickerHeight);
        m_cMACD.SetLocalSize(fSubTickerWidth, fSubTickerHeight);
        AddChild(m_cMACD);
    }
    CEventManager::PostEvent(*this, CEvent(EEVENT_FETCH_TICKER), m_uTradeIndex * 1.0f);
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_BINANCE_MGR, EGLOBAL_EVT_BINANCE_TICK_UPDATE);
}

VOID CTickerSub::Release(VOID)
{
    if (m_bListening) {
        CBinanceMgr::GetInstance().StopListenToTicks(m_uTradeIndex);
    }
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_BINANCE_MGR, EGLOBAL_EVT_BINANCE_TICK_UPDATE);
    
    m_cPriceMins.Release();
    m_cPriceMins.RemoveFromParent();
    
    m_cPriceSeconds.Release();
    m_cPriceSeconds.RemoveFromParent();
    
    m_cOBVMins.Release();
    m_cOBVMins.RemoveFromParent();
    
    m_cOBVSeconds.Release();
    m_cOBVSeconds.RemoveFromParent();
    
    m_cMACD.Release();
    m_cMACD.RemoveFromParent();
    
    m_cTitle.Release();
    m_cTitle.RemoveFromParent();
    
    CUIContainer::Release();
}

VOID CTickerSub::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    CUIContainer::OnRender(fOffsetX, fOffsetY);
    RenderSignals(fOffsetX, fOffsetY);
    
    const SHLVector2D& sWorldPos = GetWorldPosition();
    const SHLVector2D& sWorldSize = GetWorldSize();
    
    const FLOAT fX = sWorldPos.x + fOffsetX;
    const FLOAT fY = sWorldPos.y + fOffsetY;
    CGameRenderer::DrawRectOutline(fX, fY, sWorldSize.x, sWorldSize.y, 0xFFFFFFFF);

    if (m_bRenderVerticalLine) {
        const FLOAT fXLine = fX + m_fVerticalX;
        CGameRenderer::DrawLine(fXLine, fY + 22.0f, fXLine, fY + sWorldSize.y, 0xFFFFFFFF);
    }
}

VOID CTickerSub::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_TOGGLE_OBV:
            OnPressedToggleOBV();
            break;
        case EBTN_TOGGLE_MACD:
            OnPressedToggleMACD();
            break;
        case EEVENT_FETCH_TICKER:
            if (m_bListening) {
                ASSERT(FALSE);
                return;
            }
            m_bListening = TRUE;
            CBinanceMgr::GetInstance().ListenToTicks(m_uTradeIndex);
            break;
        case EGLOBAL_EVT_BINANCE_TICK_UPDATE:
            OnTickerUpdate(cEvent.GetIntParam(1));
            break;
        default:
            break;
    }
}

VOID CTickerSub::OnPressedToggleOBV(VOID)
{
    
}

VOID CTickerSub::OnPressedToggleMACD(VOID)
{
    
}

VOID CTickerSub::OnTickerUpdate(UINT uTradeIndex)
{
    if (m_uTradeIndex != uTradeIndex) {
        return;
    }
    const STickListener* psTicker = CBinanceMgr::GetInstance().GetTick(m_uTradeIndex);
    if (NULL == psTicker) {
        ASSERT(FALSE);
        return;
    }
    m_cPriceMins.UpdateTick(*psTicker);
    m_cPriceSeconds.UpdateTick(*psTicker);
    
    m_cOBVMins.UpdateTick(*psTicker);
    m_cOBVSeconds.UpdateTick(*psTicker);
    
    m_cMACD.UpdateTick(*psTicker);
    
    const STradePair* psType = CBinanceMgr::GetInstance().GetTradePair(m_uTradeIndex);
    if (NULL != psType) {
        const FLOAT fRatio = m_cPriceMins.GetPriceChangeRatio();
        CHAR szBuffer[256];
        snprintf(szBuffer, 256, "%s-%s (%.1f)%%", psType->szBase, psType->szQuote, fRatio);
        m_cTitle.SetString(szBuffer);
    }
    
    UpdateSignal(psTicker->sProcessed);

}

VOID CTickerSub::UpdateMaxDisplayableNum(UINT uNum)
{
    m_cPriceMins.UpdateMaxDisplayableNum(uNum);
    m_cOBVMins.UpdateMaxDisplayableNum(uNum);
    m_cMACD.UpdateMaxDisplayableNum(uNum);
    const STickListener* psTicker = CBinanceMgr::GetInstance().GetTick(m_uTradeIndex);
    if (NULL == psTicker) {
        return;
    }
    m_cPriceMins.UpdateTick(*psTicker);
    m_cOBVMins.UpdateTick(*psTicker);
    m_cMACD.UpdateTick(*psTicker);
}

VOID CTickerSub::UpdateSignal(const SProcessedTick& sProcessed)
{
    const UINT auOBVSignalColor[EOBV_SIG_NUM] = {
        TICKER_GRAY,    //        EOBV_SIG_NEUTRAL=0,
        TICKER_GREEN,   //        EOBV_SIG_UP=0,
        TICKER_RED     //        EOBV_SIG_DOWN,
    };
    m_auSignalColor[ESIGNAL_UI_PRICE_LOW_4HR]     = auOBVSignalColor[sProcessed.aeLowerSignalPrice[EOBV_TYPE_4HR]];
    m_auSignalColor[ESIGNAL_UI_PRICE_LOW_2HR]     = auOBVSignalColor[sProcessed.aeLowerSignalPrice[EOBV_TYPE_2HR]];
    m_auSignalColor[ESIGNAL_UI_PRICE_LOW_1HR]     = auOBVSignalColor[sProcessed.aeLowerSignalPrice[EOBV_TYPE_1HR]];
    m_auSignalColor[ESIGNAL_UI_PRICE_LOW_30MIN]   = auOBVSignalColor[sProcessed.aeLowerSignalPrice[EOBV_TYPE_30_MINS]];
    m_auSignalColor[ESIGNAL_UI_PRICE_HIGH_4HR]    = auOBVSignalColor[sProcessed.aeUpperSignalPrice[EOBV_TYPE_4HR]];
    m_auSignalColor[ESIGNAL_UI_PRICE_HIGH_2HR]    = auOBVSignalColor[sProcessed.aeUpperSignalPrice[EOBV_TYPE_2HR]];
    m_auSignalColor[ESIGNAL_UI_PRICE_HIGH_1HR]    = auOBVSignalColor[sProcessed.aeUpperSignalPrice[EOBV_TYPE_1HR]];
    m_auSignalColor[ESIGNAL_UI_PRICE_HIGH_30MIN]  = auOBVSignalColor[sProcessed.aeUpperSignalPrice[EOBV_TYPE_30_MINS]];

    m_auSignalColor[ESIGNAL_UI_OBV_LOW_4HR]     = auOBVSignalColor[sProcessed.aeLowerSignalOBV[EOBV_TYPE_4HR]];
    m_auSignalColor[ESIGNAL_UI_OBV_LOW_2HR]     = auOBVSignalColor[sProcessed.aeLowerSignalOBV[EOBV_TYPE_2HR]];
    m_auSignalColor[ESIGNAL_UI_OBV_LOW_1HR]     = auOBVSignalColor[sProcessed.aeLowerSignalOBV[EOBV_TYPE_1HR]];
    m_auSignalColor[ESIGNAL_UI_OBV_LOW_30MIN]   = auOBVSignalColor[sProcessed.aeLowerSignalOBV[EOBV_TYPE_30_MINS]];
    m_auSignalColor[ESIGNAL_UI_OBV_HIGH_4HR]    = auOBVSignalColor[sProcessed.aeUpperSignalOBV[EOBV_TYPE_4HR]];
    m_auSignalColor[ESIGNAL_UI_OBV_HIGH_2HR]    = auOBVSignalColor[sProcessed.aeUpperSignalOBV[EOBV_TYPE_2HR]];
    m_auSignalColor[ESIGNAL_UI_OBV_HIGH_1HR]    = auOBVSignalColor[sProcessed.aeUpperSignalOBV[EOBV_TYPE_1HR]];
    m_auSignalColor[ESIGNAL_UI_OBV_HIGH_30MIN]  = auOBVSignalColor[sProcessed.aeUpperSignalOBV[EOBV_TYPE_30_MINS]];
    
    m_uMACDSignalColor = auOBVSignalColor[sProcessed.eMACD];
}

#define SIGNAL_PAD      (2.0f)
#define SIGNAL_WIDTH    (24.0f)
#define SIGNAL_HEIGHT   (8.0f)

#define SIGNAL_X_OFFSET (SIGNAL_PAD + SIGNAL_WIDTH)
#define SIGNAL_POS_X(x) (SIGNAL_PAD + x * SIGNAL_X_OFFSET)
#define X_OFFSET (283.0f)
VOID CTickerSub::RenderSignals(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    const FLOAT afOffsets[ESIGNAL_UI_NUM][2] = {
        { SIGNAL_POS_X(0), SIGNAL_PAD, },
        { SIGNAL_POS_X(0), SIGNAL_PAD + SIGNAL_HEIGHT + SIGNAL_PAD, },
        
        { SIGNAL_POS_X(1), SIGNAL_PAD, },
        { SIGNAL_POS_X(1), SIGNAL_PAD + SIGNAL_HEIGHT + SIGNAL_PAD, },
        
        { SIGNAL_POS_X(2), SIGNAL_PAD, },
        { SIGNAL_POS_X(2), SIGNAL_PAD + SIGNAL_HEIGHT + SIGNAL_PAD, },
        
        { SIGNAL_POS_X(3), SIGNAL_PAD, },
        { SIGNAL_POS_X(3), SIGNAL_PAD + SIGNAL_HEIGHT + SIGNAL_PAD, },

        { SIGNAL_POS_X(0) + X_OFFSET, SIGNAL_PAD, },
        { SIGNAL_POS_X(0) + X_OFFSET, SIGNAL_PAD + SIGNAL_HEIGHT + SIGNAL_PAD, },
        
        { SIGNAL_POS_X(1) + X_OFFSET, SIGNAL_PAD, },
        { SIGNAL_POS_X(1) + X_OFFSET, SIGNAL_PAD + SIGNAL_HEIGHT + SIGNAL_PAD, },
        
        { SIGNAL_POS_X(2) + X_OFFSET, SIGNAL_PAD, },
        { SIGNAL_POS_X(2) + X_OFFSET, SIGNAL_PAD + SIGNAL_HEIGHT + SIGNAL_PAD, },
        
        { SIGNAL_POS_X(3) + X_OFFSET, SIGNAL_PAD, },
        { SIGNAL_POS_X(3) + X_OFFSET, SIGNAL_PAD + SIGNAL_HEIGHT + SIGNAL_PAD, },
    };
    const SHLVector2D& sPos = GetWorldPosition();
    fOffsetX += sPos.x;
    fOffsetY += sPos.y;
    UINT uIndex = 0;
    for ( ; ESIGNAL_UI_NUM > uIndex; ++uIndex) {
        const FLOAT fX = fOffsetX + afOffsets[uIndex][0];
        const FLOAT fY = fOffsetY + afOffsets[uIndex][1];
        CGameRenderer::DrawRectNoTex(fX, fY, SIGNAL_WIDTH, SIGNAL_HEIGHT, m_auSignalColor[uIndex]);
    }
    const FLOAT fX = fOffsetX + 5.0f;
    const FLOAT fY = fOffsetY + GetWorldSize().y - 3.0f;
    const FLOAT fH = m_cOBVMins.GetLocalSize().y - 6.0f;
    CGameRenderer::DrawRectNoTex(fX, fY, 50.0f, -fH, m_uMACDSignalColor);
}

VOID CTickerSub::EnableDrawVerticalLine(BOOLEAN bEnable, FLOAT fX)
{
    m_bRenderVerticalLine = bEnable;
    m_fVerticalX = fX;
}
