#include "stdafx.h"
#include "Semaphore.h"
#include "GameRenderer.h"
#include "GameWidgetDefs.h"

CSemaPhore::CSemaPhore() :
CUIWidget(EGameWidget_Semaphore),
INITIALIZE_TEXT_LABEL(m_cTxt),
m_fZ(0.0f)
{
    m_cTxt.SetFont(EGAMEFONT_SIZE_10);
    m_cTxt.SetAnchor(0.5f, 0.5f);
    m_cTxt.AutoSize();
    m_cTxt.SetLocalPosition(1.0f, 0.0f);
    m_cBG.SetAnchor(0.5f, 0.5f);
}
    
CSemaPhore::~CSemaPhore()
{
    
}

VOID CSemaPhore::Initialize(const CHAR* szString, BOOLEAN bSmallerCircle)
{
    m_cBG.SetTexture(ETEX::white_ring, TRUE);
    const FLOAT fSize = bSmallerCircle ? 20.0f : 25.0f;
    m_cBG.SetLocalSize(fSize, fSize);
    m_cTxt.SetString(szString);
    m_cTxt.OnUpdate(0.0f);
    m_cBG.OnUpdate(0.0f);
}
    
VOID CSemaPhore::SetColor(UINT uColor)
{
    m_cTxt.SetColor(uColor);
    m_cBG.SetColor(uColor);
}

VOID CSemaPhore::SetScale(FLOAT fScale)
{
    m_cTxt.SetScale(fScale);
    m_cBG.SetScale(fScale);
}

VOID CSemaPhore::Release(VOID)
{
    m_cBG.Release();
    m_cTxt.Release();
    CUIWidget::Release();
}

VOID CSemaPhore::OnWorldCoordinatesUpdated(VOID)
{
    
}

VOID CSemaPhore::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    const SHLVector2D& sPos = GetWorldPosition();
    fOffsetX += sPos.x;
    fOffsetY += sPos.y;
    const SHLVector2D& sWorldPos = m_cBG.GetWorldPosition();
    const SHLVector2D& sWorldSize = m_cBG.GetWorldSize();
    const FLOAT fX1 = fOffsetX + sWorldPos.x;
    const FLOAT fX2 = fX1 + sWorldSize.x * 0.5f;
    const FLOAT fX3 = fX1 + sWorldSize.x;
    const FLOAT fY1 = fOffsetY + sWorldPos.y;
    const FLOAT fY2 = fY1 + sWorldSize.y * 0.5f;
    const FLOAT fY3 = fY1 + sWorldSize.y;
    const UINT uColor = m_cTxt.GetTextColor();
    
    FLOAT afPos[8];
    afPos[0] = fX1;
    afPos[1] = fY2;
    afPos[2] = fX2;
    afPos[3] = fY1;
    afPos[4] = fX2;
    afPos[5] = fY3;
    afPos[6] = fX3;
    afPos[7] = fY2;
    CGameRenderer::DrawFilledPolygon(afPos, 4, RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    
    CGameRenderer::DrawLine(fX1, fY2, fX2, fY1, uColor);
    CGameRenderer::DrawLine(fX1, fY2, fX2, fY3, uColor);
    CGameRenderer::DrawLine(fX3, fY2, fX2, fY1, uColor);
    CGameRenderer::DrawLine(fX3, fY2, fX2, fY3, uColor);
    m_cTxt.OnRender(fOffsetX, fOffsetY);
}
