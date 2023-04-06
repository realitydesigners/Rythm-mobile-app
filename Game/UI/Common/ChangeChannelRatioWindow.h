#if !defined(CHANGE_CHANNEL_RATIO_WINDOW_H)
#define CHANGE_CHANNEL_RATIO_WINDOW_H

#include "GameWindow.h"
class CChangeChannelRatioWindow : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_EXIT = 0,
        EBTN_CHANGE_00,
        EBTN_CHANGE_01,
        EBTN_CHANGE_02,
        EBTN_CHANGE_03,
        EBTN_CHANGE_04,
        EBTN_CHANGE_05,
        EBTN_CHANGE_06,
        EBTN_CHANGE_07,
        EBTN_CHANGE_08,
        EEVENT_TEXT_INPUT_REPLY,
    };

    CChangeChannelRatioWindow();
    virtual ~CChangeChannelRatioWindow();
    
    virtual VOID OnBackBtnPressed(VOID) OVERRIDE;
protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    CUITextLabel*   m_apcTitles[9];
    CUIButton*      m_apcBtns[9];
    USHORT          m_aushRatios[9];
    CHAR            m_szBuffer[64];
    BOOLEAN         m_bContentsChanged;
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID OnPressButton(UINT uIndex);
    VOID OnTextInputReply(UINT uIndex);
    VOID DoExit(VOID);
};

#endif // #if !defined(CHANGE_CHANNEL_RATIO_WINDOW_H)
