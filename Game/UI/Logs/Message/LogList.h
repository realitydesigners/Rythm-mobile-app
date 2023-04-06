#if !defined(LOG_LIST_H)
#define LOG_LIST_H

#include "UIContainer.h"
#include "UIScroller.h"

class CLogList : public CUIContainer
{
public:
    CLogList();
    virtual ~CLogList();
    
    VOID Initialize(VOID);
    virtual VOID Release(VOID) OVERRIDE;

private:
    BOOLEAN     m_bRegistered;
    CUIScroller m_cScroller;

    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
};

#endif // #if !defined(LOG_LIST_H)
