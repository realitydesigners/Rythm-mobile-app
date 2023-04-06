#if !defined(CHANGE_SEMA_WINDOW_H)
#define CHANGE_SEMA_WINDOW_H

#include "ColorPicker.h"
#include "GameWindow.h"
#include "SliderBar.h"
class CChangeSemaWindow : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_EXIT = 0,
        EBTN_CHANGE_00,
        EBTN_CHANGE_01,
        EBTN_CHANGE_02,
        EBTN_CHANGE_03,
        EEVENT_TEXT_INPUT_REPLY,
    };

    CChangeSemaWindow(UINT uHandlerID, UINT uEventID);
    virtual ~CChangeSemaWindow();
    
    virtual VOID OnBackBtnPressed(VOID) OVERRIDE;
protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    const UINT      m_uHandlerID;
    const UINT      m_uEventID;
    CColorPicker    m_acPicker[4];
    CUIButton*      m_apcBtns[4];
    FLOAT           m_afSema[4];
    CHAR            m_szBuffer[64];
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID OnPressButton(UINT uIndex);
    VOID OnTextInputReply(UINT uIndex);
    VOID DoExit(VOID);
};

#endif // #if !defined(CHANGE_SEMA_WINDOW_H)
