#if !defined(MAIN_CONDITONS_TABLE_H)
#define MAIN_CONDITONS_TABLE_H

#include "ConditionCell.h"
#include "UIButton.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UITextLabel.h"

#define MAIN_CONDITION_TABLE_WIDTH (CONDITION_CELL_WIDTH)
#define MAIN_CONDITION_CELL_HEIGHT (CONDITION_CELL_HEIGHT)

class CMainConditionsTable : public CUIContainer
{
public:
    CMainConditionsTable();
    virtual ~CMainConditionsTable();
    
    VOID Initialize(VOID);
    VOID Refresh(UINT uPatternIndex);
    VOID Clear(VOID);
    
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
private:
    enum {
        MAX_CONDITION_NUM=10,
    };
    UINT            m_uCellsVisible;
    CUITextLabel    m_cDepthTxt;
    CUITextLabel    m_cConditionTxt;
    CUITextLabel    m_cWasLv01Txt;
    CUITextLabel    m_cWasLv02Txt;
    CUITextLabel    m_cIsLv01Txt;
    CUITextLabel    m_cIsLv02Txt;
    CConditionCell  m_acCells[MAX_CONDITION_NUM];
    VOID Resize(UINT uNumCells);
};

#endif // #if !defined(AUTO_LOGIC_EDITOR_H)
