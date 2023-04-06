#if !defined(TRADE_CHANGE_WINDOW_H)
#define TRADE_CHANGE_WINDOW_H

#include "GameWindow.h"
#include "UI9PartImage.h"
#include "CryptoUtil.h"
#include "UIScroller.h"

class CTradeChangeWindow : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_EXIT=0,
        EBTN_CHANGE_BTN,
    };
    
    CTradeChangeWindow(ESOURCE_MARKET eSource, UINT uSelectedTradeIndex);
    virtual ~CTradeChangeWindow();
    
protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID OnTradeBtnPressed(UINT uIndex);
    
    const ESOURCE_MARKET    m_eMarket;
    const UINT              m_uSelectedTradeIndex;
    CUI9PartImage           m_cBG;
    CUIScroller             m_cScroller;
};
#endif // #if !defined(TRADE_CHANGE_WINDOW_H)
