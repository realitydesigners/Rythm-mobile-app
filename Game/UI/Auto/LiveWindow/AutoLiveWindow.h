#if !defined(AUTO_LIVE_WINDOW_H)
#define AUTO_LIVE_WINDOW_H

#include "AutoStatisticsList.h"
#include "CryptoUtil.h"
#include "DetailedStatisticsList.h"
#include "GameWindow.h"

class CAutoLiveWindow : public CGameWindow
{
public:
    CAutoLiveWindow(ESOURCE_MARKET eSource, UINT uTradePairNum);
    virtual ~CAutoLiveWindow();
    
    VOID Init(VOID);
    virtual VOID Release(VOID) OVERRIDE;

    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
    virtual BOOLEAN OnTouchBegin(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchMove(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchEnd(FLOAT fX, FLOAT fY) OVERRIDE;
protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID OnRegainFocus(VOID) OVERRIDE;
private:
    enum EBTN_ID {
        EBTN_START=0,
        EBTN_STOP,
        EBTN_CHANGE_FRACTAL_SET,
        EEVENT_SELECTED_LOGIC,
        EEVENT_SELECTED_TRADE_PAIR,
        EEVENT_SELECTED_FRACTAL_INDEX,
        EBTN_REATTACH,
    };
    const ESOURCE_MARKET    m_eSource;
    const UINT              m_uTradePairNum;
    CUIButton*              m_pcStartBtn;
    CUIButton*              m_pcStopBtn;
    CUIButton*              m_pcFractalBtn;
    CUITextLabel*           m_pcTitle;
    CUITextLabel*           m_pcEntryTime; // start for stop
    CUITextLabel*           m_pcCloseTime; // time for close
    CAutoStatisticsList     m_cList;
    CDetailedStatisticsList m_cDetails;
    UINT                    m_uSelectedLogicIndex;
    
    BOOLEAN         m_bUITouched;
    BOOLEAN         m_bScrollMode; // whether we are scrolling
    FLOAT           m_afPrevTouch[2];

    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID UpdateStatus(VOID);
    VOID OnTapStartBtn(VOID);
    VOID OnTapStopBtn(VOID);
    
    VOID OnSelectedLogic(UINT uLogicIndex);
    
    VOID ReattachToParent(VOID);
    
    VOID UpdateFractalSetLabel(VOID);
    
    VOID OnTapFractalBtn(VOID);
    VOID OnSelectedFractalIndex(UINT uIndex);
};
#endif // #if !defined(AUTO_LIVE_WINDOW_H)
