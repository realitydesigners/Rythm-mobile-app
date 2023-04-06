#if !defined(TRADE_LOG_GRAPH_H)
#define TRADE_LOG_GRAPH_H

//#include "AutoGraph.h"
#include "BibBox.h"
#include "DepthDisplay.h"
#include "RythmDashboardWidget.h"
#include "TradeLogDefs.h"
#include "UIContainer.h"
#include "UIImage.h"

class STradeLog;

class CTradeLogGraph : public CUIContainer
{
public:
    CTradeLogGraph();
    virtual ~CTradeLogGraph();
    
    VOID Initialize(VOID);
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
    
    VOID Update(const STradeLog& sLog);
    VOID Clear(VOID);
private:
    enum EBTN_ID {
        EBTN_OPEN=0,
        EBTN_CLOSE,
    };
    CUIImage                m_cBG;
    CUITextLabel            m_cPriceTxt;
    CUITextLabel            m_cSlippageTxt;
    CUITextLabel            m_cEntryList;
    CUITextLabel            m_cStopLoss;
    CUITextLabel            m_cTPEntry;
    CUITextLabel*           m_apcBarLbls[9];
    
    CUIImage                m_acBtnIMG[2];
    CUIButton               m_acBtn[2];
    CUITextLabel            m_cOpenTxt;
    CUITextLabel            m_cCloseTxt;
    CRythmDashboardWidget   m_cDashboard;
    CBibBox                 m_cTop;
    CBibBox                 m_cBottom;
    STradeLog               m_sLog; // a copy
    BOOLEAN                 m_bOpen;
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    VOID ToggleDisplay(BOOLEAN bOpen);
    VOID UpdateBtnDisplay(VOID);
};

#endif // #if !defined(TRADE_LOG_GRAPH_H)
