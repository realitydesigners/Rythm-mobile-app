#if !defined(AUTO_WIDGET_H)
#define AUTO_WIDGET_H

#include "AutoLogicList.h"
#include "AutoGraphPanel.h"
#include "AutoLogicEditor.h"
#include "GameWindow.h"

#define AUTO_LOGIC_LIST_W  (160.0f)
#define AUTO_WIDGET_WIDTH  (AUTO_LOGIC_LIST_W + AUTO_GRAPH_PANEL_WIDTH + AUTO_LOGIC_EDITOR_WIDTH)
#define AUTO_WIDGET_HEIGHT AUTO_GRAPH_PANEL_HEIGHT

class CAutoWidget : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_QUIT=0,
        EEVENT_SELECTED_LOGIC,
    };
    CAutoWidget();
    virtual ~CAutoWidget();
    
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    BOOLEAN                     m_bRegistered;

    CAutoLogicList              m_cLogicList;
    CAutoGraphPanel             m_cGraph;
    CAutoLogicEditor            m_cEditor;
    UINT                        m_uPatternIndex;
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID OnSelected(UINT uPatternIndex);
    VOID AdjPattern(INT nAdj);
};

#endif // #if !defined(AUTO_WIDGET_H)
