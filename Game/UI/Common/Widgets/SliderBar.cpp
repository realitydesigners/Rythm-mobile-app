#include "stdafx.h"
#include "SliderBar.h"
#include "EventManager.h"
#include <math.h>

#define MIN_BUTTON_WIDTH (50.0f)
CSliderBar::CSliderBar(UINT uEventHandler, UINT uEventID) :
CUIContainer(),
m_uEventHandler(uEventHandler),
m_uEventID(uEventID),
m_fEndX(0.0f),
m_bTouchHit(FALSE),
m_bSliding(FALSE),
m_fPrevTouchX(0.0f),
m_uStartTimeSec(0),
m_uTotalTimeSec(0)
{
    AddChild(m_cBar);
    m_cBar.SetColor(RGBA(0x8F,0x8F,0x8F,0xFF));
    SetVisibility(FALSE);
}
    
CSliderBar::~CSliderBar()
{
    
}

VOID CSliderBar::SetupBar(UINT uStartSecs, UINT uEndSecs, UINT uNowSecs, UINT uVisibleSecs)
{
    ASSERT(uEndSecs > uStartSecs);
    m_uStartTimeSec = uStartSecs;
    const UINT uDiff = uEndSecs - uStartSecs;
    m_uTotalTimeSec = uDiff;
    const SHLVector2D& sLocalSize = GetLocalSize();
    if (uVisibleSecs >= uDiff) {        // if the timeline is shorter than whatever is visble now
        m_fEndX = 0.0f;     // do not allow bar to move (position stay at 0
        m_cBar.SetLocalSize(sLocalSize.x, sLocalSize.y);
        m_cBar.SetLocalPosition(0.0f, 0.0f);
    }
    else {
        const FLOAT fBarWidth = (sLocalSize.x * uVisibleSecs) / uDiff;
        m_cBar.SetLocalSize(fBarWidth, sLocalSize.y);
        m_fEndX = sLocalSize.x - fBarWidth;
        if (uNowSecs > uEndSecs) {
            uNowSecs = uEndSecs; // safety
        }
        FLOAT fNowRatio = (FLOAT)(uEndSecs - uNowSecs) / (FLOAT)uDiff;
        if (1.0f < fNowRatio) {
            fNowRatio = 1.0f;
        }
        if (0.0f > fNowRatio) {
            fNowRatio = 0.0f;
        }
        fNowRatio = 1.0f - fNowRatio;
        const FLOAT fNowX = fNowRatio * m_fEndX;
        m_cBar.SetLocalPosition(fNowX, 0.0f);
    }
    SetVisibility(TRUE);
}
VOID CSliderBar::SetupBar(UINT uStartSec, UINT uEndSec, UINT uNowSec)
{
    ASSERT(uEndSec > uStartSec);
    ASSERT(uEndSec >= uNowSec);
    ASSERT(uStartSec <= uNowSec);
    m_uStartTimeSec = uStartSec;
    const UINT uDiff = uEndSec - uStartSec;
    m_uTotalTimeSec = uDiff;
    const SHLVector2D& sLocalSize = GetLocalSize();
    ASSERT(sLocalSize.x > 60.0f);
    m_fEndX = sLocalSize.x - 60.0f;
    m_cBar.SetLocalSize(60.0f, 60.0f);
    
    // determine current position
    Update(uNowSec);
    SetVisibility(TRUE);
}
VOID CSliderBar::SetupBar(UINT uStart, UINT uEnd, UINT uNow, FLOAT fDimension, UINT uColor)
{
    ASSERT(uEnd > uStart);
    ASSERT(uEnd >= uNow);
    ASSERT(uStart <= uNow);
    m_uStartTimeSec = uStart;
    const UINT uDiff = uEnd - uStart;
    m_uTotalTimeSec = uDiff;
    const SHLVector2D& sLocalSize = GetLocalSize();
    ASSERT(sLocalSize.x > fDimension);
    m_fEndX = sLocalSize.x - fDimension;
    m_cBar.SetLocalSize(fDimension, fDimension);
    m_cBar.SetColor(uColor);
    // determine current position
    Update(uNow);
    SetVisibility(TRUE);
}
VOID CSliderBar::Update(UINT uNowSec)
{
    const UINT uDiffNow = uNowSec - m_uStartTimeSec;
    const FLOAT fX = (uDiffNow * m_fEndX) / m_uTotalTimeSec;
    m_cBar.SetLocalPosition(fX, 0.0f);
}

VOID CSliderBar::Release(VOID)
{
    m_cBar.Release();
    m_cBar.RemoveFromParent();
    CUIContainer::Release();
}
BOOLEAN CSliderBar::OnTouchBegin(FLOAT fX, FLOAT fY)
{
    const SHLVector2D sPos = { fX, fY };
    m_bTouchHit = m_cBar.HitTest(sPos);
    m_bSliding = FALSE;
    m_fPrevTouchX = fX;
    return m_bTouchHit;
}
#define SLIDE_BUFFER (5.0f)
BOOLEAN CSliderBar::OnTouchMove(FLOAT fX, FLOAT fY)
{
    if (0.0f == m_fEndX) {
        return TRUE;
    }
    if (m_bTouchHit) {
        const FLOAT fDiffX = fX - m_fPrevTouchX;
        if (!m_bSliding) {
            if (SLIDE_BUFFER <= fabsf(fDiffX)) {
                m_bSliding = TRUE;
                m_fPrevTouchX = fX;
                return TRUE;
            }
        }
        else {
            const SHLVector2D& sPos = m_cBar.GetLocalPosition();
            FLOAT fMovedX = sPos.x + fDiffX;
            if (0.0f > fMovedX) {
                fMovedX = 0.0f;
            }
            if (m_fEndX < fMovedX) {
                fMovedX = m_fEndX;
            }
            m_fPrevTouchX = fX;
            m_cBar.SetLocalPosition(fMovedX, sPos.y);
            const FLOAT fRatio = 1.0f - (fMovedX / m_fEndX); // 1 to 0
            const UINT uNewTimeSec = (m_uStartTimeSec + m_uTotalTimeSec) - (UINT)(m_uTotalTimeSec * fRatio);
            if (0 != m_uEventHandler) {
                CEventManager::PostGlobalEvent(m_uEventHandler, CEvent(m_uEventID, uNewTimeSec));
            }
            else {
                CUIContainer* pcParent = GetParent();
                if (NULL != pcParent) {
                    CEvent cEvent(m_uEventID, uNewTimeSec);
                    pcParent->PostEvent(cEvent, 0.0f);
                }
            }
        }
        return TRUE;
    }
    return FALSE;
}
BOOLEAN CSliderBar::OnTouchEnd(FLOAT fX, FLOAT fY)
{
    return OnTouchMove(fX, fY);
}
