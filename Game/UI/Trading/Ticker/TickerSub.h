#if !defined(TICKER_SUB_H)
#define TICKER_SUB_H

#include "TickerMACD.h"
#include "TickerPrice.h"
#include "TickerOBV.h"
#include "UIContainer.h"
#include "UITextLabel.h"

struct SProcessedTick;
class CTickerSub : public CUIContainer
{
public:
    enum EBTN_ID {
        EBTN_TOGGLE_OBV=0,
        EBTN_TOGGLE_MACD,
        
        EEVENT_FETCH_TICKER,    // to start the ticking!
    };
    
    CTickerSub(UINT uTradeIndex);
    virtual ~CTickerSub();

    VOID UpdateMaxDisplayableNum(UINT uNum);
    VOID SetupTicker(VOID);
    
    VOID EnableDrawVerticalLine(BOOLEAN bEnable, FLOAT fX);
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    enum ESIGNAL_UI {
        ESIGNAL_UI_PRICE_HIGH_4HR=0,
        ESIGNAL_UI_PRICE_LOW_4HR,
        ESIGNAL_UI_PRICE_HIGH_2HR,
        ESIGNAL_UI_PRICE_LOW_2HR,
        ESIGNAL_UI_PRICE_HIGH_1HR,
        ESIGNAL_UI_PRICE_LOW_1HR,
        ESIGNAL_UI_PRICE_HIGH_30MIN,
        ESIGNAL_UI_PRICE_LOW_30MIN,
        ESIGNAL_UI_OBV_HIGH_4HR,
        ESIGNAL_UI_OBV_LOW_4HR,
        ESIGNAL_UI_OBV_HIGH_2HR,
        ESIGNAL_UI_OBV_LOW_2HR,
        ESIGNAL_UI_OBV_HIGH_1HR,
        ESIGNAL_UI_OBV_LOW_1HR,
        ESIGNAL_UI_OBV_HIGH_30MIN,
        ESIGNAL_UI_OBV_LOW_30MIN,
        ESIGNAL_UI_NUM
    };
    const UINT      m_uTradeIndex;
    CUITextLabel    m_cTitle;
    CTickerPrice    m_cPriceMins;
    CTickerPrice    m_cPriceSeconds;
    CTickerOBV      m_cOBVMins;
    CTickerOBV      m_cOBVSeconds;
    CTickerMACD     m_cMACD;
    UINT            m_auSignalColor[ESIGNAL_UI_NUM];
    UINT            m_uMACDSignalColor;
    BOOLEAN         m_bRenderVerticalLine;
    FLOAT           m_fVerticalX;
    BOOLEAN         m_bListening;
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID OnPressedToggleOBV(VOID);
    VOID OnPressedToggleMACD(VOID);
    VOID OnTickerUpdate(UINT uTradeIndex);
    
    VOID UpdateSignal(const SProcessedTick& sProcessed);
    VOID RenderSignals(FLOAT fOffsetX, FLOAT fOffsetY) const;
};
#endif // #if !defined(TICKER_SUB_H)
