#if !defined(COLOR_PICKER_H)
#define COLOR_PICKER_H

#include "UIContainer.h"
#include "UIImage.h"
#include "UITextLabel.h"
#include "SliderBar.h"

class CColorPicker : public CUIContainer
{
public:
    CColorPicker();
    virtual ~CColorPicker();

    VOID Initialize(UINT uColor);
    VOID SetGrayscaleMode(VOID);

    virtual VOID Release(VOID) OVERRIDE;
    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
    
    UINT GetColor(VOID) const { return m_uColor; }
private:
    enum EEVENT_ID {
        EEVENT_RED_SLIDE=0,
        EEVENT_GREEN_SLIDE,
        EEVENT_BLUE_SLIDE
    };
    UINT            m_uColor;
    BOOLEAN         m_bGrayscaleMode;
    CUIImage        m_cBG;
    CUIImage        m_cColor;
    CUIImage        m_acBar[3];
    CSliderBar*     m_apcSlider[3];
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
};
#endif // #if !defined(TOOLTIP_H)
