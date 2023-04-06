#if !defined(CONDITION_CELL_H)
#define CONDITION_CELL_H

#include "PatternDataDefs.h"
#include "UIButton.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UITextLabel.h"

#define CONDITION_SRC_DEPTH_WIDTH       (25.0f)
#define CONDITION_CONDITION_WIDTH       (40.0f)
#define CONDITION_WAS_VALUE_01_WIDTH    (40.0f)
#define CONDITION_WAS_VALUE_02_WIDTH    CONDITION_WAS_VALUE_01_WIDTH
#define CONDITION_IS_VALUE_01_WIDTH     CONDITION_WAS_VALUE_01_WIDTH
#define CONDITION_IS_VALUE_02_WIDTH     CONDITION_WAS_VALUE_01_WIDTH

#define CONDITION_CELL_WIDTH            (CONDITION_SRC_DEPTH_WIDTH +    \
                                        CONDITION_CONDITION_WIDTH + \
                                        CONDITION_WAS_VALUE_01_WIDTH + CONDITION_WAS_VALUE_02_WIDTH + \
                                        CONDITION_IS_VALUE_01_WIDTH + CONDITION_IS_VALUE_02_WIDTH)
#define CONDITION_CELL_HEIGHT           (25.0f)

class CConditionCell : public CUIContainer
{
public:
    CConditionCell();
    virtual ~CConditionCell();
    
    VOID Initialize(VOID);
    VOID Refresh(const SPatternRow& sRow);
    VOID Clear(VOID);
    virtual VOID Release(VOID) OVERRIDE;

private:
    CUIImage        m_cNumberBG;
    CUIImage        m_cConditionBG;

    CUITextLabel    m_cSrcDepthTxt;
    CUITextLabel    m_cConditionTxt;
    CUITextLabel    m_cWasValue01Txt;
    CUITextLabel    m_cWasValue02Txt;
    CUITextLabel    m_cIsValue01Txt;
    CUITextLabel    m_cIsValue02Txt;

};

#endif // #if !defined(CONDITION_CELL_H)
