#if !defined(STATUS_BOX_H)
#define STATUS_BOX_H
#include "StatusBib.h"
#include "DepthDisplay.h"
#include "RenkoDef.h"
#include "TradeLogDefs.h"
#include "UIButton.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UITextLabel.h"
#include "RythmDashboardWidget.h"

class CMegaZZ;
class CStatusBox : public CUIContainer
{
public:
    CStatusBox();
    virtual ~CStatusBox();
    
    BOOLEAN Initialize(VOID);
    VOID DisableTaps(VOID) { m_bDisableTap = TRUE; }
    VOID SetDefaultDepthIndex(UINT uDepthIndex);
    VOID SetDefaultDepthNum(UINT uDepthNum);

    VOID PrepareChart(const CMegaZZ& cMegaZZ);
    VOID ClearChart(VOID);
    
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
    
    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
private:
    enum EBTN_ID {
        EBTN_CHANGE_DEPTH=0,
        EBTN_CHANGE_DEPTH_NUM,
        EBTN_UNNUSED,
        EEVENT_ON_CHANGE_DEPTH_INDEX,
        EEVENT_ON_CHANGE_DEPTH_NUM,
    };
    const CMegaZZ*  m_pcMegaZZ;
    BOOLEAN         m_bTradeLogOpen;
    CStatusBib      m_cBibWidget;
    CDepthDisplay           m_cDepthDisplay;
    CRythmDashboardWidget   m_cDepthValues;
    CUIImage        m_cBtnBG;
    CUIImage        m_cSelectedBtnBG;
    CUIButton       m_cChangeDepthBtn;
    CUITextLabel    m_cChangeDepthTxt;
    CUIButton       m_cChangeDepthNumBtn;
    CUITextLabel    m_cChangeDepthNumTxt;
    BOOLEAN         m_bDisableTap;
    
    VOID UpdateDepthLabel(VOID);
    VOID UpdateDepthNumLabel(VOID);
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID OnTapChangeDepth(VOID);
    VOID OnTapDepthBtn(UINT uDepthIndex);
    
    VOID OnTapChangeDepthNum(VOID);
    VOID OnTapDepthNumBtn(UINT uIndex);
    
    VOID OnBibColorUpdated(VOID);
    
    VOID ReloadChart(VOID);
};

#endif // #if !defined(STATUS_BOX_H)
