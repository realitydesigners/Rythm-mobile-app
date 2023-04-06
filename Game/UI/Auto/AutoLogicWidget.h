#if !defined(AUTO_LOGIC_WIDGET_H)
#define AUTO_LOGIC_WIDGET_H

#include "UIButton.h"
#include "UIImage.h"
#include "UITextLabel.h"

class CAutoLogicWidget : public CUIButton
{
public:
    CAutoLogicWidget(UINT uPatternIndex);
    virtual ~CAutoLogicWidget();
    
    VOID SetSelected(BOOLEAN bSelected);
    VOID Initialize(UINT uEventID);
    UINT GetPatternIndex(VOID) const  { return m_uPatternIndex; }
    
    virtual VOID Release(VOID) OVERRIDE;
private:
    const UINT              m_uPatternIndex;
    CUIImage                m_cBG;
    CUITextLabel            m_cName;

    virtual BOOLEAN CreateButtonEvent(CEvent& cEvent) OVERRIDE;
};

#endif // #if !definedAUTO_LOGIC_WIDGET_H
