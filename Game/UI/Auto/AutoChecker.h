#if !defined(AUTO_CHECKER_H)
#define AUTO_CHECKER_H

#include "GameWindow.h"
class CAutoChecker : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_QUIT=0,
        EBTN_PATTERN_CHECK,
        EBTN_TP_CHECK,
        EBTN_LOGIC_CHECK,
        EBTN_BIB_EDITOR,
    };
    CAutoChecker();
    virtual ~CAutoChecker();
    
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID OnTapPatternCheck(VOID);
};

#endif // #if !defined(AUTO_CHECKER_H)
