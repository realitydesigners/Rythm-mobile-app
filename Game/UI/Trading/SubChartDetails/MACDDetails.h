#if !defined(MACD_DETAILS_H)
#define MACD_DETAILS_H

#include "GameWindow.h"
#include "PlayerDataDef.h"
#include "TradeChart.h"
#include "UI9PartImage.h"

class CMACDDetails : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_EXIT = 0,
        EBTN_LINE_1_SMA,
        EBTN_LINE_1_EMA,
        EBTN_LINE_1_TOGGLE,
        EBTN_LINE_1_INTERVAL,
        
        EBTN_LINE_2_SMA,
        EBTN_LINE_2_EMA,
        EBTN_LINE_2_TOGGLE,
        EBTN_LINE_2_INTERVAL,

        EBTN_LINE_3_SMA,
        EBTN_LINE_3_EMA,
        EBTN_LINE_3_TOGGLE,
        EBTN_LINE_3_INTERVAL,

        EEVENT_ON_TEXT_INPUT_COMPLETE,
    };
    
    CMACDDetails();
    virtual ~CMACDDetails();
    
protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    CUI9PartImage   m_cBG;
    CUITextLabel*   m_pcLbls[3];
    CUIButton*      m_apcBtns[3][3];    // sma, ema, none
    CUIButton*      m_apcToggles[3];    // opening, closing, average
    CHAR            m_szBuffer[64];

    VOID RefreshBtns(UINT uIndex, EMA_TYPE eType);
    VOID RefreshToggles(UINT uIndex, EMA_VALUE_SOURCE eSource);
    
    VOID ChangeCurveType(UINT uIndex, EMA_TYPE eType);
    VOID ToggleSourceType(UINT uIndex);
    
    VOID ChangeInterval(UINT uIndex);
    VOID OnTextInputComplete(UINT uIndex);
    
};
#endif // #if !defined(MA_DETAILS_H)
