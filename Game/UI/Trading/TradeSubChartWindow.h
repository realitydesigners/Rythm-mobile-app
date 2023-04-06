#if !defined(TRADE_SUB_CHART_WINDOW_H)
#define TRADE_SUB_CHART_WINDOW_H

#include "GameWindow.h"
#include "UI9PartImage.h"

class CTradeSubChartWindow : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_EXIT=0,

        EBTN_TOGGLE_MACD,
        EBTN_MACD_DETAILS,
        
        EBTN_TOGGLE_CMO,
        EBTN_CHANGE_CMO_VALUE,
        
        EBTN_TOGGLE_RSI,
        EBTN_CHANGE_RSI_VALUE,
        
        EBTN_TOGGLE_OBV,
        EBTN_OBV_DETAILS,

        EEVENT_ON_TEXT_INPUT_COMPLETE,
    };
    
    CTradeSubChartWindow();
    virtual ~CTradeSubChartWindow();
    
protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID OnCMOValueChange(VOID);
    VOID OnPressMACDDetails(VOID);
    VOID OnRSIValueChange(VOID);
    VOID OnPressOBVDetails(VOID);
    
    VOID OnTextInputComplete(UINT uPrevEventID, UINT uParam1);
    
    CUI9PartImage   m_cBG;
    CHAR            m_szBuffer[64];
};
#endif // #if !defined(TRADE_SUB_CHART_WINDOW_H)
