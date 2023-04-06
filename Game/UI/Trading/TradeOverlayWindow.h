#if !defined(TRADE_OVERLAY_WINDOW_H)
#define TRADE_OVERLAY_WINDOW_H

#include "GameWindow.h"
#include "UI9PartImage.h"

class CTradeChart;
class CTradeOverlayWindow : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_EXIT = 0,
        
        EBTN_TOGGLE_CANDLE,
        EBTN_SUB_CHART_UI,
        
        EBTN_TOGGLE_MA,
        EBTN_MA_DETAILS,
        
        EBTN_TOGGLE_BB,
        EBTN_BB_CHANGE_INTERVAL,
        EBTN_BB_CHANGE_MULTIPLIER,
        
        EEVENT_ON_TEXT_INPUT_COMPLETE,
    };
    
    CTradeOverlayWindow(CTradeChart& cChart);
    virtual ~CTradeOverlayWindow();
    
protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;

    VOID OnToggleCandle(VOID);
    VOID OnPressSubChartUI(VOID);
    
    VOID OnToggleMA(VOID);
    VOID OnPressMADetails(VOID);
    
    VOID OnToggleBB(VOID);
    VOID OnPressBBInterval(VOID);
    VOID OnPressBBMultiplier(VOID);
    
    
    VOID OnTextInputComplete(UINT uPrevEventID, UINT uParam1);
    
    CUI9PartImage   m_cBG;
    CTradeChart&    m_cChart;
    CHAR            m_szBuffer[64];
};
#endif // #if !defined(TRADE_OVERLAY_WINDOW_H)
