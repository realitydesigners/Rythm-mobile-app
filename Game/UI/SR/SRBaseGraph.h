#if !defined(SR_BASE_GRAPH_H)
#define SR_BASE_GRAPH_H

#include "UIButton.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UITextLabel.h"
#include "TimeIndicator.h"

class CMegaZZ;
class CSRBaseGraph : public CUIContainer
{
public:
    enum EType {
        EType_Channel=0,
        EType_Straight,
        EType_Inner,
        EType_PNF,
        EType_Multi,
        EType_Sema,
        EType_BoxSema,
        Etype_MegaSema,
        Etype_NewSema2,
        Etype_PNFStraight,
        EType_Sema3D,
    };
    CSRBaseGraph(EType eType, UINT uGlobalHandlerID = 0);
    virtual ~CSRBaseGraph();

    virtual BOOLEAN Initialize(UINT uBoxNum)=0;
    virtual VOID PrepareChart(const CMegaZZ& cMegaZZ, UINT uDepth) { };
    virtual VOID ClearChart(VOID)=0;
    
    virtual VOID Release(VOID) OVERRIDE=0;
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
    
    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
    
    EType GetType(VOID) const { return m_eType; }
protected:
    VOID ClearTimeLines(VOID);
    BOOLEAN AddTimeLine(FLOAT fX, FLOAT fY, UINT uTimeSec);
    
    VOID SetIndicatorString(const CHAR* szString);
    VOID AddSnapLatestBtn(UINT uEventID);
    VOID AddShowHideBtn(UINT uEventID);
    
    VOID UpdateShowHideStatus(BOOLEAN bHide);
    VOID SetupLabel(CUITextLabel& cLabel);
private:
    enum {
        MAX_TIMELINE=50,
    };
    const EType     m_eType;
    CUITextLabel    m_cIndicator;
    
    CUIImage        m_cBtnBG;
    CUIButton       m_cSnapBtn;
    CUITextLabel    m_cShowHideTxt;
    CUIButton       m_cShowHideBtn;
    
    CTimeIndicator  m_acTime[MAX_TIMELINE];
    UINT            m_uUsedTL;
};

#endif // #if !defined(SR_BASE_GRAPH_H)
