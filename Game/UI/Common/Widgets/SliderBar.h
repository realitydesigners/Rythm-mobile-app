#if !defined(SLIDER_BAR_H)
#define SLIDER_BAR_H

#include "UIContainer.h"
#include "UIImage.h"
class CSliderBar : public CUIContainer
{
public:
    CSliderBar(UINT uEventHandler, UINT uEventID);
    virtual ~CSliderBar();

    virtual VOID Release(VOID) OVERRIDE;
    
    virtual BOOLEAN OnTouchBegin(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchMove(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchEnd(FLOAT fX, FLOAT fY) OVERRIDE;

    VOID SetupBar(UINT uStartSecs, UINT uEndSecs, UINT uNowSecs, UINT uVisibleSecs);
    VOID SetupBar(UINT uStartSec, UINT uEndSec, UINT uNowSec);
    VOID SetupBar(UINT uStart, UINT uEnd, UINT uNow, FLOAT fDimension, UINT uColor);
    VOID Update(UINT uNowSec);
private:
    const UINT  m_uEventHandler;
    const UINT  m_uEventID;
    FLOAT       m_fEndX;
    BOOLEAN     m_bTouchHit;
    BOOLEAN     m_bSliding;
    FLOAT       m_fPrevTouchX;
    UINT        m_uStartTimeSec;
    UINT        m_uTotalTimeSec;
    CUIImage    m_cBar;
};
#endif // #if !defined(SLIDER_BAR_H)
