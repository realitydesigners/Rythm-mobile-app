#if !defined(TIME_INDICATOR_H)
#define TIME_INDICATOR_H

#include "UIWidget.h"
#include "UITextLabel.h"
#include "UIImage.h"

class CTimeIndicator : public CUIWidget
{
public:
    CTimeIndicator();
    virtual ~CTimeIndicator();

    VOID SetTimeSec(UINT uTimeSec);
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
private:
    CUIImage        m_cBG;
    CUITextLabel    m_cTxt;
    
    virtual VOID OnWorldCoordinatesUpdated(VOID) OVERRIDE { }
};
#endif // #if !defined(TIME_INDICATOR_H)
