#if !defined(OBV_DETAILS_H)
#define OBV_DETAILS_H

#include "GameWindow.h"
#include "PlayerDataDef.h"
#include "UI9PartImage.h"

class COBVDetails : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_EXIT = 0,
        EBTN_LINE_1_SMA,
        EBTN_LINE_1_EMA,
        EBTN_LINE_1_NONE,
        EBTN_LINE_1_INTERVAL,
        
        EBTN_LINE_2_SMA,
        EBTN_LINE_2_EMA,
        EBTN_LINE_2_NONE,
        EBTN_LINE_2_INTERVAL,
        
        EBTN_TOGGLE_OBV_TYPE,
        
        EEVENT_ON_TEXT_INPUT_COMPLETE,
    };
    
    COBVDetails();
    virtual ~COBVDetails();
    
protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    CUI9PartImage   m_cBG;
    CUITextLabel*   m_pcLbls[2];
    CUIButton*      m_apcBtns[2][3];    // sma, ema, none
    CUIButton*      m_apcToggles[2];    // opening, closing, average
    CHAR            m_szBuffer[64];
    VOID RefreshBtns(UINT uIndex);
    
    VOID OnOBVTypeChange(VOID);
    VOID ChangeCurveType(UINT uIndex, EMA_TYPE eType);
    VOID ChangeInterval(UINT uIndex);
    VOID OnTextInputComplete(UINT uIndex);
    VOID UpdateCurveName(UINT uIndex);
};
#endif // #if !defined(OBV_DETAILS_H)
