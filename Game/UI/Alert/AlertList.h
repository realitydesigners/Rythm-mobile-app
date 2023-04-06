#if !defined(ALERT_LIST_H)
#define ALERT_LIST_H

#include "MultiCellTable.h"
#include "UIButton.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UITextLabel.h"
#include <map>

class CAlertList : public CUIContainer
{
public:
    CAlertList();
    virtual ~CAlertList();
    
    VOID Initialize(VOID);
    virtual VOID Release(VOID) OVERRIDE;

    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
private:
    typedef std::map<UINT,INT> CTradePairPatternMap;
    typedef std::pair<UINT,INT> CTradePairPatternPair;
    
    enum EBTN_ID {
        EBTN_TOGGLE_ENABLE=0,
        EBTN_MUTE,
        EBTN_DO_NOTHING,
    };
    CUIImage                m_cBG;
    CUIImage                m_cBtnBG;
    CUITextLabel            m_cEnable;
    CUIButton               m_cEnableBtn;
    CUITextLabel            m_cMute;
    CUIButton               m_cMuteBtn;
    CMultiCellTable         m_cTbl;
    BOOLEAN                 m_bEnabled;
    BOOLEAN                 m_bMuted;
    CTradePairPatternMap    m_cMap;

    VOID ToggleEnable(VOID);
    VOID Mute(VOID);
    VOID UpdateEnableLabel(VOID);
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID OnUpdateMegaZZ(UINT uTradePairIndex);
    
    VOID SetPair(UINT uTradePairIndex, INT nPattern);
    VOID UpdateList(VOID);
};

#endif // #if !defined(ALERT_LIST_H)
