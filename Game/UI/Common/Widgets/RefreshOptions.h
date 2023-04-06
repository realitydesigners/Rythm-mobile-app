#if !defined(REFRESH_OPTIONS_H)
#define REFRESH_OPTIONS_H

#include "UIButton.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UITextLabel.h"

class CRefreshOptions : public CUIContainer
{
public:
    CRefreshOptions();
    virtual ~CRefreshOptions();

    BOOLEAN Initialize(VOID);
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
private:
    CUIImage        m_cBG;
    CUITextLabel    m_cTitle;
    CUIImage        m_cSelectedImg;
    CUIImage        m_cNotSelectedImg;
    CUIButton       m_acBtns[5];
    CUITextLabel*   m_apcLbls[5];
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
};
#endif // #if !defined(REFRESH_OPTIONS_H)
