#if !defined(COLOR_PICKER_WINDOW_H)
#define COLOR_PICKER_WINDOW_H

#include "ColorPicker.h"
#include "GameWindow.h"
class CColorPickerWindow : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_EXIT = 0,
    };

    CColorPickerWindow(UINT uHandlerID, UINT uEventID, UINT uColor);
    virtual ~CColorPickerWindow();
    
    VOID SetGrayscaleMode(VOID);
    
    virtual VOID OnBackBtnPressed(VOID) OVERRIDE;
protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    const UINT      m_uHandlerID;
    const UINT      m_uEventID;
    UINT            m_uColor;
    CColorPicker    m_cPicker;
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID OnPressButton(UINT uIndex);
    VOID DoExit(VOID);
};

#endif // #if !defined(COLOR_PICKER_WINDOW_H)
