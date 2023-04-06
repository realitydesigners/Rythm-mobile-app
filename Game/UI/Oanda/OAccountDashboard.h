#if !defined(OANDA_ACCOUNT_DASHBOARD_H)
#define OANDA_ACCOUNT_DASHBOARD_H

#include "MegaZZDef.h"
#include "OTradeList.h"
#include "OUIDefs.h"
#include "TradeLog.h"
#include "UIButton.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UITextLabel.h"

#if defined(DEBUG)
#if !defined(USE_DEMO_ACCOUNT)
#define USE_DEMO_ACCOUNT
#endif // #if !defined(USE_DEMO_ACCOUNT)
#endif // #if defined(DEBUG)

class COAccountDashboard : public CUIContainer
{
public:
    COAccountDashboard();
    virtual ~COAccountDashboard();
    
    VOID OnChangeTradePair(UINT uTradePairIndex);
    VOID Initialize(VOID);
    virtual VOID Release(VOID) OVERRIDE;
    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
    
    virtual BOOLEAN OnTouchBegin(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchMove(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchEnd(FLOAT fX, FLOAT fY) OVERRIDE;
private:
    enum EBTN_ID {
        EBTN_REFRESH=0,
        EBTN_SELETED_TRADE,
        EBTN_REATTACH
    };
    CUIImage        m_cTradePairBG;
    CUITextLabel    m_cTradePair;

    CUIContainer    m_cBody;
    
    CUIImage        m_acBG[2];
    CUIImage        m_acOrdersBG[2];
#if defined(USE_DEMO_ACCOUNT)
    CUITextLabel    m_cDemoAcct;
#endif // #if defined(USE_DEMO_ACCOUNT)
    CUITextLabel*   m_apcAccountID[2];
    CUITextLabel*   m_apcBalance[2];
    CUITextLabel*   m_apcMarginAvail[2];
    CUITextLabel*   m_apcMarginUsed[2];
    
    BOOLEAN         m_bRefreshAll;
    BOOLEAN         m_bUITouched;
    BOOLEAN         m_bScrollMode;
    FLOAT           m_afPrevTouch[2];
    
    // trade list ui
    COTradeList     m_acTradeList[2];
    
    UINT            m_uTradePairIndex;
    
    // pending variables
    UINT            m_uPendingCreateOrderMsgID;
    STradeLog       m_sPendingLog;
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID UpdateLabels(UINT uAccountIndex);
    
    VOID OnTapRefresh(VOID);
    VOID OnTapSelectTrade(UINT uTradePairIndex);
    
    VOID RefreshOpenTrades(UINT uAccountIndex);
    
    VOID OnOandaAccountDetailsUpdated(UINT uAccountIndex, BOOLEAN bSuccess);
    VOID OnQueryOpenTrades(UINT uAccountIndex, BOOLEAN bSuccess);
    
    VOID OnToggleDisplay(VOID);
    VOID ReattachToParent(VOID);
};

#endif // #if !defined(OANDA_ACCOUNT_DASHBOARD_H)
