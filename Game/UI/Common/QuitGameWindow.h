#if !defined(QUIT_GAME_WINDOW_H)
#define QUIT_GAME_WINDOW_H

#include "GameWindow.h"
class CQuitGameWindow : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_EXIT = 0,
        EBTN_QUIT,
    };

    CQuitGameWindow();
    virtual ~CQuitGameWindow();
protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
};
#endif // #if !defined(QUIT_GAME_WINDOW_H)
