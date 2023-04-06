#if !defined(BIB_BOX_H)
#define BIB_BOX_H
#include "Bib2.h"
#include "DepthDisplay.h"
#include "RenkoDef.h"
#include "TradeLogDefs.h"
#include "UIButton.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UITextLabel.h"
#include "RythmDashboardWidget.h"

class CMegaZZ;
class CBibBox : public CUIContainer
{
public:
    CBibBox();
    virtual ~CBibBox();
    
    BOOLEAN Initialize(VOID);
    VOID HideSideBars(BOOLEAN bHide);
    VOID DisableTaps(VOID) { m_bDisableTap = TRUE; }
    VOID SetDefaultDepthIndex(UINT uDepthIndex);
    VOID SetDefaultDepthNum(UINT uDepthNum);
    VOID OnTapToggleX(VOID);
    VOID SetSmallestChannelWhite(VOID);

    VOID PrepareChart(const STradeLog& sLog, BOOLEAN bOpen);
    VOID PrepareChart(const CMegaZZ& cMegaZZ);
    VOID ClearChart(VOID);
    
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
    
    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
private:
    enum EBTN_ID {
        EBTN_CHANGE_UP_COLOR=0,
        EBTN_CHANGE_DOWN_COLOR,
        EBTN_TOGGLE_HIDE_X,
        EBTN_CHANGE_DEPTH,
        EBTN_UNNUSED,
        
        EBTN_CHANGE_DEPTH_NUM,
        
        EEVENT_ON_CHANGE_UP_COLOR,
        EEVENT_ON_CHANGE_DOWN_COLOR,
        
        EEVENT_ON_CHANGE_DEPTH_INDEX,
        EEVENT_ON_CHANGE_DEPTH_NUM,
    };
    const CMegaZZ*  m_pcMegaZZ;
    STradeLog       m_sTmp;
    const STradeLog* m_psTradelog;
    BOOLEAN         m_bTradeLogOpen;
    CBib2           m_cBibWidget;
    CDepthDisplay           m_cDepthDisplay;
    CRythmDashboardWidget   m_cDepthValues;
    CUIImage        m_cBtnBG;
    CUIImage        m_cSelectedBtnBG;
    CUIButton       m_cChangeDepthBtn;
    CUITextLabel    m_cChangeDepthTxt;
    CUIButton       m_cXToggleBtn;
    CUITextLabel    m_cXToggleTxt;
    CUIImage        m_cUpBG;
    CUIImage        m_cDownBG;
    CUIButton       m_cUpBtn;
    CUIButton       m_cDownBtn;
    CUIButton       m_cChangeDepthNumBtn;
    CUITextLabel    m_cChangeDepthNumTxt;
    BOOLEAN         m_bDisableTap;
    
    VOID UpdateDepthLabel(VOID);
    VOID UpdateDepthNumLabel(VOID);
    VOID UpdateBtnColors(VOID);
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID OnTapChangeColor(BOOLEAN bUp);
    VOID OnChangeColor(BOOLEAN bUp, UINT uColor);
    
    VOID OnTapChangeDepth(VOID);
    VOID OnTapDepthBtn(UINT uDepthIndex);
    
    VOID OnTapChangeDepthNum(VOID);
    VOID OnTapDepthNumBtn(UINT uIndex);
    
    VOID OnBibColorUpdated(VOID);
    
    VOID ReloadChart(VOID);
};

#endif // #if !defined(BIB_BOX_H)
