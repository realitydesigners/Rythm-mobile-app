#if !defined(LOG_WIDGET_H)
#define LOG_WIDGET_H

#include "LogList.h"
#include "UIButton.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UITextLabel.h"

class CLogWidget : public CUIContainer
{
public:
    CLogWidget();
    virtual ~CLogWidget();
    
    VOID Initialize(VOID);
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
    
    virtual BOOLEAN OnTouchBegin(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchMove(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchEnd(FLOAT fX, FLOAT fY) OVERRIDE;

private:
    enum EBTN_ID {
        EBTN_CLEAR=0,
        EBTN_REATTACH,
    };
    BOOLEAN         m_bRegistered;
    CUIImage        m_cHeaderBG;
    CUITextLabel    m_cLogTitle;
    
    CUIImage        m_cLogBG;
    CLogList        m_cList;
    CUITextLabel    m_cClear;
    CUIImage        m_cClearBtnBG;
    CUIButton       m_cClearBtn;
    
    BOOLEAN         m_bUITouched;

    VOID UpdateTitle(VOID);
    VOID ToggleLog(VOID);
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID ReattachToParent(VOID);
};

#endif // #if !defined(LOG_WIDGET_H)
