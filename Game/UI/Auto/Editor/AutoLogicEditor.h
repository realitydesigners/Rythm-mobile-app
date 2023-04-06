#if !defined(AUTO_LOGIC_EDITOR_H)
#define AUTO_LOGIC_EDITOR_H

#include "AutoGraphPanel.h"
//#include "MainConditonsTable.h"
#include "MultiCellTable.h"
#include "UIButton.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UITextLabel.h"

#define AUTO_LOGIC_EDITOR_WIDTH     (430.0f)
#define AUTO_LOGIC_EDITOR_HEIGHT    AUTO_GRAPH_PANEL_HEIGHT
class CAutoLogicEditor : public CUIContainer
{
public:
    CAutoLogicEditor();
    virtual ~CAutoLogicEditor();
    
    VOID Initialize(VOID);
    VOID Refresh(UINT uPatternIndex);
    VOID Clear(VOID);
    
    virtual VOID Release(VOID) OVERRIDE;
    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
private:
    UINT            m_uPatternIndex;
    
    CUIImage        m_cBG;
    CUITextLabel    m_cNameTxt;
    
    CMultiCellTable m_cTable;
//    CMainConditionsTable    m_cMainConditions;
    
    CHAR            m_szBuffer[MEGAZZ_LOGIC_NAME_LENGTH];
    
};

#endif // #if !defined(AUTO_LOGIC_EDITOR_H)
