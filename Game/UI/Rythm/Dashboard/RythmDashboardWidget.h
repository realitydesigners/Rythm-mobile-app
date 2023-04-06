#if !defined(RYTHM_DASHBOARD_WIDGET_H)
#define RYTHM_DASHBOARD_WIDGET_H

#include "RythmUIDefs.h"
#include "UIButton.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UITextLabel.h"

#define RYTHM_DASHBOARD_WIDGET_NAME_X               (10.0f)
#define RYTHM_DASHBOARD_WIDGET_PRICE_X              (80.0f)
#define RYTHM_DASHBOARD_WIDGET_BTN_START_X          (145.0f)
#define RYTHM_DASHBOARD_WIDGET_HEIGHT               (24.0f)
#define RYTHM_DASHBOARD_WIDGET_WIDTH                (RYTHM_DASHBOARD_WIDGET_BTN_START_X + 9 * RYTHM_DASHBOARD_WIDGET_HEIGHT + 5.0f)
struct STradeLog;
class CMegaZZ;
class CRythmDashboardWidget : public CUIContainer
{
public:
    CRythmDashboardWidget(UINT uTradeIndex, UINT uParentHandlerID, UINT uEventID);
    virtual ~CRythmDashboardWidget();
    
    VOID SetAdditionalSpacing(FLOAT fSpacing) { m_fAdditionalSpacing = fSpacing; }
    VOID SetDisplay(UINT uNum, UINT uBiggestDepthIndex) { m_uDisplayNum = uNum; m_uBiggestDepthIndex = uBiggestDepthIndex; };
    VOID SetSelected(VOID);
    VOID ClearSelection(VOID); // clears any selection if any
    VOID Initialize(VOID); // 1st selected
    VOID HideNameAndPrice(VOID);
    VOID ToggleVisibility(const BOOLEAN* pbVisible);
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
    
    VOID UpdateTradePairName(const CHAR* szName);
    VOID DisplayMessage(const CHAR* szMsg);
    VOID Update(const CMegaZZ& cMegaZZ);
    VOID Update(const STradeLog& sLog, BOOLEAN bOpen);
    VOID Clear(VOID);
private:
    const UINT              m_uTradeIndex;
    const UINT              m_uParentHandlerID;
    const UINT              m_uEventID;
    BOOLEAN                 m_bSelected;
    BOOLEAN                 m_bSpreadAbove3;
    FLOAT                   m_fAdditionalSpacing;
    UINT                    m_uDisplayNum;
    UINT                    m_uBiggestDepthIndex;
    CUITextLabel            m_cName1;
    CUITextLabel            m_cName2;
    CUITextLabel            m_cPrice;
    CUIImage                m_cInvImg;      // invisible img for name/price btn
    CUIButton               m_cBtn;         // btn for tapping name/price
    CUIImage                m_acBoxImg[9];   // invisible img
    CUITextLabel*           m_apcBoxLbls[9];
    CUITextLabel*           m_apcRLbls[9];
    BOOLEAN                 m_abVisible[9];
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    VOID OnTapped(VOID);
    VOID RecalculateNamePosition(VOID);
};

#endif // #if !defined(RYTHM_DASHBOARD_WIDGET_H)
