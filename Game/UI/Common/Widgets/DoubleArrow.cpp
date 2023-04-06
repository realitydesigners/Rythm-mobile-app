#include "stdafx.h"
#include "DoubleArrow.h"
#include "GameRenderer.h"
#include "GameWidgetDefs.h"



#define ARROW_GREY  RGBA(0x7F,  0x7F,   0x7F,   0xFF)
#define ARROW_UP    RGBA(0,     0xFF,   0,      0xFF)
#define ARROW_DOWN  RGBA(0xFF,  0,      0,      0xFF)

#define BG_GREY     RGBA(0x7F,  0x7F,   0x7F,   0x7F)
#define BG_GREEN    RGBA(0,     0x7F,   0x0,    0x7F)
#define BG_RED      RGBA(0x7F,  0,      0x0,    0x7F)

CDoubleArrow::CDoubleArrow() :
CUIWidget(EGameWidget_DoubleArrow),
m_uBGColor(BG_GREY),
m_uOriginalColor(BG_GREY)
{
    SetRotation(0.0f, 0.0f);
}
CDoubleArrow::~CDoubleArrow()
{
    
}

VOID CDoubleArrow::Initialize(UINT uBGColor)
{
    m_uBGColor = uBGColor;
    m_uOriginalColor = uBGColor;
    SetLocalSize(DOUBLE_ARROW_DIMENSION, DOUBLE_ARROW_DIMENSION);
}
VOID CDoubleArrow::SetEnabled(BOOLEAN bEnabled)
{
    if (bEnabled) {
        m_uBGColor = m_uOriginalColor;
    }
    else {
        m_uBGColor = BG_GREY;
    }
    SetRotation(0.0f, 0.0f);
}
VOID CDoubleArrow::SetRotation(FLOAT fTop, FLOAT fBottom)
{
    m_afRotation[0] = fTop;
    m_afRotation[1] = fBottom;
    const BOOLEAN bTopUp = 0.0f > fTop;
    const BOOLEAN bTopDown = 0.0f < fTop;
    const BOOLEAN bBottomUp = 0.0f > fBottom;
    const BOOLEAN bBottomDown = 0.0f < fBottom;
    if (bTopUp) {
        m_auArrowColor[0] = ARROW_UP;
    }
    else if (bTopDown) {
        m_auArrowColor[0] = ARROW_DOWN;
    }
    else {
        m_auArrowColor[0] = ARROW_GREY;
    }
    if (bBottomUp) {
        m_auArrowColor[1] = ARROW_UP;
    }
    else if (bBottomDown) {
        m_auArrowColor[1] = ARROW_DOWN;
    }
    else {
        m_auArrowColor[1] = ARROW_GREY;
    }
}

VOID CDoubleArrow::OnWorldCoordinatesUpdated(VOID)
{
    
}
VOID CDoubleArrow::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    const SHLVector2D& sPos = GetWorldPosition();
    const SHLVector2D& sSize= GetWorldSize();
    fOffsetX += sPos.x;
    fOffsetY += sPos.y;
    CGameRenderer::DrawRectNoTex(fOffsetX - 2.0f, fOffsetY - 2.0f, sSize.x + 4.0f, sSize.y + 4.0f, m_uBGColor);
    CGameRenderer::DrawRectNoTex(fOffsetX + 2.0f, fOffsetY + 2.0f, sSize.x - 4.0f, sSize.y - 4.0f, BG_GREY);

    fOffsetX += sSize.x * 0.5f;
    fOffsetY += sSize.y * 0.25f;
    for (UINT uIndex = 0; 2 > uIndex; ++uIndex) {
        CGameRenderer::DrawRectWithRotation(fOffsetX, fOffsetY, m_afRotation[uIndex], 1.0f, ETEX::icon_arrow, m_auArrowColor[uIndex]);
        fOffsetY += sSize.y * 0.5f;
    }
}
