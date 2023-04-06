#if !defined(LOG_UI_H)
#define LOG_UI_H

#include "UIContainer.h"
#include "UIImage.h"
#include "UITextLabel.h"
#include "MessageLog.h"

class CLogUI : public CUIContainer
{
public:
    CLogUI();
    virtual ~CLogUI();
    
    BOOLEAN Initialize(const SMessageLog& sLog);
    virtual VOID Release(VOID) OVERRIDE;
private:
    CUIImage     m_cBG;
    CUITextLabel m_cDate;
    CUITextLabel m_cMsg;
};

#endif // #if !defined(LOG_UI_H)
