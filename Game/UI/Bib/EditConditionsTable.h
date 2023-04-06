#if !defined(EDIT_CONDITIONS_TABLE_H)
#define EDIT_CONDITIONS_TABLE_H

#include "EditableCondition.h"
#include "UIButton.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UITextLabel.h"

class CEditConditionsTable : public CUIContainer
{
public:
    enum {
        MAX_CONDITION_NUM=8,
    };
    CEditConditionsTable();
    virtual ~CEditConditionsTable();
    
    VOID Initialize(UINT uRefreshEvent);
    VOID SetMaxRow(UINT uRow);
    const SPatternRow* GetPatternRows(VOID) { return m_asPatternRow; }
    
    
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
private:
    CUITextLabel*       m_apcTxt[8];
    CEditableCondition  m_acCells[MAX_CONDITION_NUM];
    SPatternRow         m_asPatternRow[MAX_CONDITION_NUM];
    
    VOID Refresh(VOID);
};

#endif // #if !defined(EDIT_CONDITIONS_TABLE_H)
