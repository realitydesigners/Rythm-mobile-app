#if !defined(AUTO_LOGIC_LIST_H)
#define AUTO_LOGIC_LIST_H

#include "UIButton.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UIScroller.h"
#include "UITextLabel.h"

class CAutoLogicList : public CUIContainer
{
public:
    CAutoLogicList();
    virtual ~CAutoLogicList();
    
    VOID Initialize(UINT uSelectedEventID);
    VOID RefreshPatternList(VOID);
    VOID FilterPatternByCategory(UINT uEntryCategoryIndex);
    VOID OnSelectedWidget(UINT uPatternIndex);

    virtual VOID Release(VOID) OVERRIDE;
    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
    
private:
    enum EBTN_ID {
        EEVENT_SELECTED_LOGIC=0,
    };

    UINT                      m_uSelectedEventID;
    UINT                      m_uSelectedPatternIndex;
    CUIImage                  m_cBG;
    CUIScroller               m_cScroller;

    VOID ClearList(VOID);
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
};

#endif // #if !defined(AUTO_LOGIC_LIST_H)
