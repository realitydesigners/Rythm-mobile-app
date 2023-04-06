#if !defined(BIB_EDITOR_H)
#define BIB_EDITOR_H

#include "GameWindow.h"
#include "EditConditionsTable.h"
#include "AutoGraphPanel.h"

class CBibEditor : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_EXIT = 0,
        EBTN_CHANGE_ROW_NUM,
        EEVENT_REFRESH,
        EEVENT_USER_REPLY_ROW_NUM,
    };

    CBibEditor();
    virtual ~CBibEditor();
protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    CAutoGraphPanel         m_cBox;
    CEditConditionsTable    m_cTbl;
    CUITextLabel*           m_pcRowNumTxt;
    UINT                    m_uUsedRowNum;
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    VOID DoExit(VOID);
    VOID UpdateLabel(VOID);
    
    VOID OnTapChangeRowNum(VOID);
    VOID OnUserReplyRowNum(UINT uRowIndex);
    VOID RefreshChart(VOID);
};

#endif // #if !defined(BIB_EDITOR_H)
