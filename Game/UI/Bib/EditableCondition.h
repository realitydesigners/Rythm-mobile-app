#if !defined(EDITABLE_CONDITION_H)
#define EDITABLE_CONDITION_H

#include "PatternDataDefs.h"
#include "UIButton.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UITextLabel.h"

#define E_CONDITION_SRC_DEPTH_WIDTH         (25.0f)
#define E_CONDITION_STATUS_WIDTH            (40.0f)
#define E_CONDITION_WAS_VALUE_01_WIDTH      (40.0f)
#define E_CONDITION_WAS_VALUE_02_WIDTH      E_CONDITION_WAS_VALUE_01_WIDTH
#define E_CONDITION_IS_VALUE_01_WIDTH       E_CONDITION_WAS_VALUE_01_WIDTH
#define E_CONDITION_IS_VALUE_02_WIDTH       E_CONDITION_WAS_VALUE_01_WIDTH
#define E_CONDITION_MATCH_WIDTH             (60.0f)
#define E_CONDITION_MATCH_T_WIDTH           (25.0f)

#define E_CONDITION_CELL_WIDTH          (E_CONDITION_SRC_DEPTH_WIDTH +    \
                                        E_CONDITION_STATUS_WIDTH + \
                                        E_CONDITION_WAS_VALUE_01_WIDTH + E_CONDITION_WAS_VALUE_02_WIDTH + \
                                        E_CONDITION_IS_VALUE_01_WIDTH + E_CONDITION_IS_VALUE_02_WIDTH + \
                                        E_CONDITION_MATCH_WIDTH + E_CONDITION_MATCH_T_WIDTH)
#define E_CONDITION_CELL_HEIGHT         (25.0f)

class CEditableCondition : public CUIContainer
{
public:
    CEditableCondition();
    virtual ~CEditableCondition();

    UINT GetIndeX(VOID) const                       { return m_uIndex; }
    const SPatternRow* GetPatternRow(VOID) const    { return m_psRow;   }
    VOID Initialize(UINT uIndex, SPatternRow& sRow, UINT uRefreshEvent);
    virtual VOID Release(VOID) OVERRIDE;

    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
private:
    enum EBTN {
        EBTN_DEPTH=0,
        EBTN_STATUS,
        EBTN_WAS_L,
        EBTN_WAS_H,
        EBTN_IS_L,
        EBTN_IS_H,
        EBTN_MATCH,
        EBTN_MATCH_T,
        EBTN_NUM
    };
    enum EEVENT {
        EEVENT_START = EBTN_NUM,
        EEVENT_REPLY_DEPTH,
        EEVENT_REPLY_VALUE,
        EEVENT_REPLY_MATCH_DEPTH,
    };
    UINT            m_uIndex;
    SPatternRow*    m_psRow;
    UINT            m_uRefreshEvent;
    CUIImage        m_acBtnImg[EBTN_NUM];
    CUIButton       m_acBtn[EBTN_NUM];
    CUITextLabel*   m_apcTxt[EBTN_NUM];

    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID OnTapDepthBtn(VOID);
    VOID OnUserReplyDepth(UINT uDepth);
    
    VOID OnTapStatusBtn(VOID);
    
    VOID OnTapValueBtn(UINT uEventID);
    VOID OnUserReplyValue(UINT uEventID, UINT uValue);
    
    VOID OnTapMatchBtn(VOID);
    VOID OnTapMatchDepthBtn(VOID);
    VOID OnUserReplyMatchDepth(UINT uDepth);
    
    VOID SendRefreshParentEvent(VOID);
    VOID UpdateLabels(VOID);
    
};

#endif // #if !defined(EDITABLE_CONDITION_H)
