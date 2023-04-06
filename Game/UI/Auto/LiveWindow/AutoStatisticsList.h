#if !defined(AUTO_STATISTICS_LIST_H)
#define AUTO_STATISTICS_LIST_H

#include "UIButton.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UIScroller.h"
#include "UITextLabel.h"
#include "AutoStatsDef.h"

// this list filters by logic id
class CAutoStatisticsList : public CUIContainer
{
public:
    enum EBTN_ID {
        EEVENT_SELECTED_ENTRY_INDEX=0,
    };

    CAutoStatisticsList(UINT uSelectedEventID);
    virtual ~CAutoStatisticsList();
    
    VOID Initialize(VOID);
    VOID RefreshList(VOID);
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
private:
    const UINT              m_uSelectedEventID;
    UINT                    m_uSelectedIndex;
    SFilteredData*          m_psFilteredData;
    UINT                    m_uFilteredNum;
    SFilteredData**         m_ppsFilteredActiveOnly;
    UINT                    m_uFilteredActiveOnlyNum;
    
    CUITextLabel            m_cName;
    CUITextLabel            m_cLong;
    CUITextLabel            m_cTargetOpenedNum;
    CUITextLabel            m_cTakeProfitNum;
    CUITextLabel            m_cStopLossNum;
    CUITextLabel            m_cCumulativePip;
    CUIScroller             m_cScroller;

    VOID ClearList(VOID);
    VOID OnSelectedEntryIndex(UINT uIndex, BOOLEAN bFireEvent);
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
};

#endif // #if !defined(AUTO_STATISTICS_LIST_H)
