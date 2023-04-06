#if !defined(LOGIC_CHECK_H)
#define LOGIC_CHECK_H

#include "GameWindow.h"
#include "MultiCellTable.h"

class CLogicCheck : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_QUIT=0,
        EBTN_UNUSED,
    };
    CLogicCheck();
    virtual ~CLogicCheck();
    
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    CMultiCellTable         m_cList;
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
};

#endif // #if !defined(LOGIC_CHECK_H)
