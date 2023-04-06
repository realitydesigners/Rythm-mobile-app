#include "stdafx.h"
#include "TimeIndicator.h"
#include "GameWidgetDefs.h"

CTimeIndicator::CTimeIndicator() :
CUIWidget(EGameWidget_TimeIndicator),
INITIALIZE_TEXT_LABEL(m_cTxt)
{
    m_cTxt.SetAlignment(CUITextLabel::EALIGNMENT_CENTER);
    m_cTxt.SetFont(EGAMEFONT_SIZE_12);
    m_cTxt.SetAnchor(0.5f, 0.5f);
    m_cTxt.AutoSize();
    
    m_cBG.SetColor(RGBA(0x2F, 0x2F, 0x2F, 0xFF));
    m_cBG.SetAnchor(0.5f, 0.5f);
}
    
CTimeIndicator::~CTimeIndicator()
{
    
}

VOID CTimeIndicator::SetTimeSec(UINT uTimeSec)
{
    CHAR szBuffer[64];
    const UINT uSec = uTimeSec % 60;
    const UINT uMin = (uTimeSec / 60) % 60;
    const UINT uHr = uTimeSec / 3600;
    if (0 == uHr) {
        if (0 < uMin) {
            if (5 <= uMin || 0 == uSec) {
                snprintf(szBuffer, 64, "%dm", uMin);
            }
            else {
                snprintf(szBuffer, 64, "%dm\n%ds", uMin, uSec);
            }
        }
        else {
            snprintf(szBuffer, 64, "%ds", uSec);
        }
    }
    else {
        if (0 == uMin) {
            snprintf(szBuffer, 64, "%dh", uHr);
        }
        else {
            snprintf(szBuffer, 64, "%dh\n%dm", uHr, uMin);
        }
    }
    m_cTxt.SetString(szBuffer);
    m_cTxt.OnUpdate(0.0f);
    const SHLVector2D& sSize = m_cTxt.GetWorldSize();
    m_cBG.SetLocalSize(sSize.x + 6.0f, sSize.y + 6.0f);
    m_cBG.OnUpdate(0.0f);
}
    
VOID CTimeIndicator::Release(VOID)
{
    m_cBG.Release();
    m_cTxt.Release();
    CUIWidget::Release();
}

VOID CTimeIndicator::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    const SHLVector2D& sPos = GetWorldPosition();
    fOffsetX += sPos.x;
    fOffsetY += sPos.y;
    m_cBG.OnRender(fOffsetX, fOffsetY);
    m_cTxt.OnRender(fOffsetX, fOffsetY);
}
