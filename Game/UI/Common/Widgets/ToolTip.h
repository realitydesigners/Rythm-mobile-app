#if !defined(TOOLTIP_H)
#define TOOLTIP_H

#include "UIContainer.h"
#include "UITextLabel.h"
#include "UI9PartImage.h"
class CToolTip : public CUIContainer
{
public:
    CToolTip();
    virtual ~CToolTip();

    VOID Initialize(const CHAR* szString);
    virtual VOID Release(VOID) OVERRIDE;
private:
    CUI9PartImage   m_cBG;
    CUITextLabel    m_cTxt;
};
#endif // #if !defined(TOOLTIP_H)
