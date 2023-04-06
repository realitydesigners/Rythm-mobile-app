#if !defined(MAIN_DEBUG_H)
#define MAIN_DEBUG_H

#if defined(DEBUG)
#include "GameWindow.h"
class CMainDebug : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_TOGGLE_FPS=0,
        EBTN_PING,
        EBTN_EXIT,
    };

    CMainDebug();
    virtual ~CMainDebug();
    
    virtual VOID OnBackBtnPressed(VOID) OVERRIDE;
protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
};

#endif // #if defined(DEBUG)
#endif // #if !defined(MAIN_DEBUG_H)
