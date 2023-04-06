#if !defined(AUTO_GRAPH_PANEL_H)
#define AUTO_GRAPH_PANEL_H

#include "AutoGraph.h"
#include "PatternDataDefs.h"
#include "UIButton.h"
#include "UIContainer.h"
#include "UIImage.h"

#define AUTO_GRAPH_PANEL_WIDTH  (600.0f)
#define AUTO_GRAPH_PANEL_HEIGHT (660.0f)

class CAutoGraphPanel : public CUIContainer {
public:
    CAutoGraphPanel();
    virtual ~CAutoGraphPanel();
    
    VOID Initialize(FLOAT fW = AUTO_GRAPH_PANEL_WIDTH, FLOAT fH = AUTO_GRAPH_PANEL_HEIGHT);
    
    VOID Refresh(UINT uPatternIndex);
    VOID Refresh(const SPatternRow* psRows, UINT uRowNum);
    
    virtual VOID Release(VOID) OVERRIDE;
    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
private:
    enum EBTN {
        EBTN_TOGGLE_TRANSPARENCY=0,
        EBTN_TOGGLE_GRID,
    };
    CUIImage        m_cBG;
    CUIImage        m_cBtnBG;
    CUITextLabel    m_cT;
    CUITextLabel    m_cG;
    CUIButton       m_cTBtn;
    CUIButton       m_cGBtn;
    
    CAutoGraph      m_cTop;
    UINT            m_uPatternIndex;
    const SPatternRow* m_psRows;
    UINT                m_uRowNum;
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID Refresh(VOID);
};
#endif // #if !defined((AUTO_GRAPH_PANEL_H))
