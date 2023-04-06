#if !defined(ALERT_WIDGET_H)
#define ALERT_WIDGET_H

#include "AlertList.h"
#include "UIButton.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UITextLabel.h"

class CAlertWidget : public CUIContainer
{
public:
    CAlertWidget();
    virtual ~CAlertWidget();
    
    VOID Initialize(VOID);
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
    
    virtual BOOLEAN OnTouchBegin(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchMove(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchEnd(FLOAT fX, FLOAT fY) OVERRIDE;

private:
    enum EBTN_ID {
        EBTN_REATTACH,
    };
    CUIImage        m_cTitleBG;
    CUITextLabel    m_cTitle;
    CAlertList      m_cAlerts;
    
    BOOLEAN         m_bUITouched;

    VOID ReattachToParent(VOID);
    VOID Toggle(VOID);
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
};

#endif // #if !defined(ALERT_WIDGET_H)
