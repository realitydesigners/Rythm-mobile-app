#if !defined(FOREX_WINDOW_H)
#define FOREX_WINDOW_H

#include "AlertWidget.h"
#include "AutoDashboardList.h"
#include "AutoLiveWindow.h"
#include "StatusBox.h"
#include "NewBibBox.h"
#include "GameWindow.h"
#include "LogWidget.h"
#include "OAccountDashboard.h"
#include "PlayerDataDef.h"
#include "RefreshOptions.h"
#include "SRInnerZZ.h"
#include "TradeLogButton.h"
#include "UITextLabel.h"

class CForexWindow : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_OPEN_CHECK_UI=0,
        EBTN_INC_DEPTH,
        EBTN_DEC_DEPTH,
        EEVENT_SELECTED_TRADE_PAIR,
        EEVENT_LISTEN_TO_RYTHM,
        EEVENT_FETCH_SPREAD,
    };

    CForexWindow();
    virtual ~CForexWindow();
    virtual VOID OnBackBtnPressed(VOID) OVERRIDE;
    
protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    enum {
        TOTAL_GRAPH_NUM = 10,
    };
    CUIImage            m_cLogo;
    CRefreshOptions     m_cRefreshOptions;
    CAutoDashboardList  m_cDashboard;
    CAutoLiveWindow     m_cLive;
    CStatusBox          m_cBoxMain;
    CBibBox             m_cBoxSub;
//    CSRInnerZZ          m_cLeft;
    
    CUITextLabel        m_cName;
    CUITextLabel        m_cPrice;
    CUITextLabel        m_cSpread;
    
    CUIButton*          m_pcChangeSizeBtn;
    CUIButton*          m_pcChangeBoxNumBtn;
    
    COAccountDashboard  m_cOAccount;
    CLogWidget          m_cLogs;
    CTradeLogButton     m_cTrades;
    CAlertWidget        m_cAlerts;
    UINT                m_uCurrentTradePairIndex;
    UINT                m_uInnerZZDepth;
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    // trading pairs related
    VOID OnOandaSignInReply(BOOLEAN bSuccess);
    VOID OnOandaTradePairQueryReply(BOOLEAN bSuccess);
    VOID OnTradePairsQueried(VOID);
    
    
    VOID OnTradePairSelected(UINT uTradePairIndex);
    VOID OnUpdateMegaZZ(UINT uTradePairIndex);
    
    VOID OnTapCheckUI(VOID);
    
    VOID OnFetchSpreadCallback(VOID);
    
    VOID UpdateSpreadLabel(VOID);
    VOID ChangeInnerZZDepth(INT nChange);
};
#endif // #if !defined(FOREX_WINDOW_H)
