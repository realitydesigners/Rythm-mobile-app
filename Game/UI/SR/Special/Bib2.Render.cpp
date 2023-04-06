#include "stdafx.h"
#include "Bib2.h"
#include "GameRenderer.h"
#include "MegaZZDef.h"

VOID CBib2::RenderLines(FLOAT fOffsetX, FLOAT fOffsetY, UINT uDepthIndex) const
{
    FLOAT afPos[8];
    for (UINT uIndex = 0; m_uLine2DNum > uIndex; ++uIndex) {
        const SLine2D& sLine = m_asLine2D[uIndex];
        if (sLine.uDepthIndex != uDepthIndex) {
            continue;
        }
        if (sLine.sTangents.x == 0.0f && sLine.sTangents.y == 0.0f) {
            CGameRenderer::DrawLine(sLine.sPt1.x + fOffsetX,
                                    sLine.sPt1.y + fOffsetY,
                                    sLine.sPt2.x + fOffsetX,
                                    sLine.sPt2.y + fOffsetY,
                                    sLine.uColor);
            continue;
        }
        const FLOAT fX1 = sLine.sPt1.x - sLine.sTangents.x + fOffsetX;
        const FLOAT fY1 = sLine.sPt1.y - sLine.sTangents.y + fOffsetY;
        
        const FLOAT fX2 = sLine.sPt2.x - sLine.sTangents.x + fOffsetX;
        const FLOAT fY2 = sLine.sPt2.y - sLine.sTangents.y + fOffsetY;
        
        const FLOAT fX3 = fX1 + sLine.sTangents.x * 2.0f;
        const FLOAT fY3 = fY1 + sLine.sTangents.y * 2.0f;
        const FLOAT fX4 = fX2 + sLine.sTangents.x * 2.0f;
        const FLOAT fY4 = fY2 + sLine.sTangents.y * 2.0f;
        afPos[0] = fX1; afPos[1] = fY1;
        afPos[2] = fX2; afPos[3] = fY2;
        afPos[4] = fX3; afPos[5] = fY3;
        afPos[6] = fX4; afPos[7] = fY4;
        CGameRenderer::DrawFilledPolygon(afPos, 4, sLine.uColor);
    }
}

VOID CBib2::RenderPoly(FLOAT fOffsetX, FLOAT fOffsetY, UINT uDepthIndex) const
{
    FLOAT afPos[8];
    for (UINT uIndex = 0; m_uPoly2DNum > uIndex; ++uIndex) {
        const SPoly2D& sPoly = m_asPoly2D[uIndex];
        if (uDepthIndex != sPoly.uDepthIndex) {
            continue;
        }
        for (UINT uPtIndex = 0; 4 > uPtIndex; ++uPtIndex) {
            afPos[uPtIndex * 2] = sPoly.afPos[uPtIndex * 2] + fOffsetX;
            afPos[uPtIndex * 2 + 1] = sPoly.afPos[uPtIndex * 2 + 1] + fOffsetY;
        }
        CGameRenderer::DrawFilledPolygon(afPos, 4, sPoly.uColor);
    }

}
VOID CBib2::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    SHLVector2D sPos = GetWorldPosition();
    sPos.x += fOffsetX;
    sPos.y += fOffsetY;
    
    const SHLVector2D& sSize = GetWorldSize();
    CGameRenderer::DrawRectOutline(sPos.x-1, sPos.y-1, sSize.x+2, sSize.y+2, RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH >= uIndex; ++uIndex) {
        RenderPoly(sPos.x, sPos.y, uIndex);
        RenderLines(sPos.x, sPos.y, uIndex);
    }
    CUIContainer::OnRender(fOffsetX, fOffsetY);
}
