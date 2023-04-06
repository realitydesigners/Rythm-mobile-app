#if !defined(AUTO_DASHBOARD_LIST_H)
#define AUTO_DASHBOARD_LIST_H

#include "CryptoUtil.h"
#include "UIButton.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UIScroller.h"
#include "UITextLabel.h"

class CRythmDashboardWidget;
class CAutoDashboardList : public CUIContainer
{
public:
    enum EBTN_ID {
        EEVENT_SELECTED_PAIR=0,
        EBTN_TAP_BOX_00,
        EBTN_TAP_BOX_01,
        EBTN_TAP_BOX_02,
        EBTN_TAP_BOX_03,
        EBTN_TAP_BOX_04,
        EBTN_TAP_BOX_05,
        EBTN_TAP_BOX_06,
        EBTN_TAP_BOX_07,
        EBTN_TAP_BOX_08
    };

    CAutoDashboardList(UINT uSelectedEventID);
    virtual ~CAutoDashboardList();
    
    VOID Initialize(ESOURCE_MARKET eSource, UINT uTradePairNum);
    VOID UpdateFractalIndex(VOID);
    VOID RefreshList(BOOLEAN bInit);
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
private:
    const UINT              m_uSelectedEventID;
    CUITextLabel            m_cNameLbl;
    CUITextLabel            m_cPriceLbl;
    CUITextLabel*           m_apcBarLbls[9];
    CUIImage                m_cBtnImg;
    CUIButton               m_acBtns[9];
    CRythmDashboardWidget** m_ppcWidgets;
    ESOURCE_MARKET          m_eSource;
    UINT                    m_uTradePairNum;
    BOOLEAN                 m_abToggleOn[9];

    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID OnMegaZZUpdated(UINT uTradePairIndex);
    VOID OnTradePairSelected(UINT uTradePairIndex);
    VOID OnToggleDepthIndex(UINT uDepthIndex);
};

#endif // #if !defined(AUTO_DASHBOARD_LIST_H)
