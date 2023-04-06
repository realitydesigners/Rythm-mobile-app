#include "stdafx.h"
#include "CMain.h"
#include "GameGL.h"
#include "GameRenderer.h"
#include "Sema3D.h"
#include "ShaderManager.h"

#define FRONT_GRADIENT  (255)
#define SIDE_GRADIENT   (128)
#define BOTTOM_GRADIENT (196)

static UINT GetColorFromGradient(UINT uColor, BYTE byGradient)
{
    const BYTE byR = (GET_RED(uColor) * byGradient / 255);
    const BYTE byG = (GET_GREEN(uColor) * byGradient / 255);
    const BYTE byB = (GET_BLUE(uColor) * byGradient / 255);
    return RGBA(byR, byG, byB, 0xFF);
}

VOID CSema3D::Transform(SHLVector2D& sResult, FLOAT fX, FLOAT fY, FLOAT fZ) const
{
    const SHLVector2D& sSize = GetLocalSize();
    sResult.x = sSize.x * 0.5f - m_sScrollOffset.x + fX;
    sResult.y = sSize.y * 0.5f + m_sScrollOffset.y - fY;
    sResult.x += fZ * 0.5f;
    sResult.y -= fZ * 0.5f;
}
VOID CSema3D::UnTransform(SHLVector2D& sResult, FLOAT fUIX, FLOAT fUIY) const
{
    const SHLVector2D& sSize = GetLocalSize();
    sResult.x = fUIX + m_sScrollOffset.x - sSize.x * 0.5f;
    sResult.y = m_sScrollOffset.y + sSize.y * 0.5f - fUIY;
}

VOID CSema3D::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    SHLVector2D sPos = GetWorldPosition();
    const SHLVector2D& sSize = GetWorldSize();
    sPos.x += fOffsetX;
    sPos.y += fOffsetY;
    CGameRenderer::DrawRectOutline(sPos.x-1, sPos.y-1, sSize.x+2, sSize.y+2, 0xFFFFFFFF);
    if (m_cBG.IsVisible()) {
        m_cBG.OnRender(sPos.x, sPos.y);
    }
    CMain::SetScissorTest(sPos.x, sPos.y, sSize.x, sSize.y);
    RenderPolygons(sPos.x, sPos.y);
    RenderLines(sPos.x, sPos.y);
    RenderSemaphores(sPos.x, sPos.y);
    RenderChannelLabels(sPos.x, sPos.y);
    RenderBoxText(sPos.x, sPos.y);
    CMain::ClearScissorTest();

    CSRBaseGraph::OnRender(fOffsetX, fOffsetY);
}

VOID CSema3D::AddPolygon2D(const SHLVector2D& sTopLeft, const SHLVector2D& sTopRight, const SHLVector2D& sBottomLeft, const SHLVector2D& sBottomRight, UINT uColor, UINT uDepth)
{
    if (POLYGON_NUM <= m_uUsedPoly2D) {
        ASSERT(FALSE);
        return;
    }
    SPolygon2D& sPoly = m_asPoly2D[m_uUsedPoly2D];
    sPoly.sTopLeft = sTopLeft;
    sPoly.sTopRight = sTopRight;
    sPoly.sBottomLeft = sBottomLeft;
    sPoly.sBottomRight = sBottomRight;
    sPoly.uColor = uColor;
    sPoly.uDepth = uDepth;
    ++m_uUsedPoly2D;
}

VOID CSema3D::AddPolygonAsBox(FLOAT fTopLeftX, FLOAT fTopLeftY, FLOAT fBottomRightX, FLOAT fBottomRightY, FLOAT fZ, UINT uColor, UINT uDepth)
{
    // simple culling
    if (m_fCullTopLeftX > fTopLeftX && m_fCullTopLeftX > fBottomRightX) {
        return;
    }
    if (m_fCullBottomRightX < fTopLeftX && m_fCullBottomRightX < fBottomRightX) {
        return;
    }
    if (m_fCullTopLeftY < fTopLeftY && m_fCullTopLeftY < fBottomRightY) {
        return;
    }
    if (m_fCullBottomRightY > fTopLeftY && m_fCullBottomRightY > fBottomRightY) {
        return;
    }
    SHLVector2D sTopLeft;
    Transform(sTopLeft, fTopLeftX, fTopLeftY, fZ);
    SHLVector2D sBottomRight;
    Transform(sBottomRight, fBottomRightX, fBottomRightY, fZ);
    const SHLVector2D sTopRight = { sBottomRight.x, sTopLeft.y };
    const SHLVector2D sBottomLeft= { sTopLeft.x, sBottomRight.y };
    const UINT uFrontColor = GetColorFromGradient(uColor, FRONT_GRADIENT);
    AddPolygon2D(sTopLeft, sTopRight, sBottomLeft, sBottomRight, uFrontColor, uDepth);
    if (0.0f == fZ) {
        return;
    }
    BOOLEAN bAddLeftSide = TRUE;
    BOOLEAN bAddBottomSide = uDepth == m_uBiggestChannelIndex;
    SHLVector2D sZeroBottomLeft;
    if (bAddLeftSide) {
        SHLVector2D sZeroTopLeft;
        Transform(sZeroTopLeft, fTopLeftX, fTopLeftY, 0.0f);
        Transform(sZeroBottomLeft, fTopLeftX, fBottomRightY, 0.0f);
        const UINT uSideColor = GetColorFromGradient(uColor, SIDE_GRADIENT);
        AddPolygon2D(sTopLeft, sBottomLeft, sZeroTopLeft, sZeroBottomLeft, uSideColor, uDepth);
    }
    if (bAddBottomSide) {
        if (!bAddLeftSide) {
            Transform(sZeroBottomLeft, fTopLeftX, fBottomRightY, 0.0f);
        }
        SHLVector2D sZeroBottomRight;
        Transform(sZeroBottomRight, fBottomRightX, fBottomRightY, 0.0f);
        const UINT uBottomColor = GetColorFromGradient(uColor, BOTTOM_GRADIENT);
        AddPolygon2D(sBottomLeft, sBottomRight, sZeroBottomLeft, sZeroBottomRight, uBottomColor, uDepth);
    }
}
VOID CSema3D::AddPolygon(FLOAT fTopLeftX, FLOAT fTopLeftY, FLOAT fTopRightX, FLOAT fTopRightY, FLOAT fBottomLeftX, FLOAT fBottomLeftY, FLOAT fBottomRightX, FLOAT fBottomRightY, FLOAT fZ, UINT uColor, UINT uDepth)
{
    // simple culling
    if (m_fCullTopLeftX > fTopLeftX && m_fCullTopLeftX > fTopRightX && m_fCullTopLeftX > fBottomLeftX && m_fCullTopLeftX > fBottomRightX) {
        return;
    }
    if (m_fCullBottomRightX < fTopLeftX && m_fCullBottomRightX < fTopRightX && m_fCullBottomRightX < fBottomLeftX && m_fCullBottomRightX < fBottomRightX) {
        return;
    }
    if (m_fCullTopLeftY < fTopLeftY && m_fCullTopLeftY < fTopRightY && m_fCullTopLeftY < fBottomLeftY && m_fCullTopLeftY < fBottomRightY) {
        return;
    }
    if (m_fCullBottomRightY > fTopLeftY && m_fCullBottomRightY > fTopRightY && m_fCullBottomRightY > fBottomLeftY && m_fCullBottomRightY > fBottomRightY) {
        return;
    }
    SHLVector2D sTopLeft;
    Transform(sTopLeft, fTopLeftX, fTopLeftY, fZ);
    SHLVector2D sTopRight;
    Transform(sTopRight, fTopRightX, fTopRightY, fZ);
    SHLVector2D sBottomLeft;
    Transform(sBottomLeft, fBottomLeftX, fBottomLeftY, fZ);
    SHLVector2D sBottomRight;
    Transform(sBottomRight, fBottomRightX, fBottomRightY, fZ);
    uColor = GetColorFromGradient(uColor, FRONT_GRADIENT);
    AddPolygon2D(sTopLeft, sTopRight, sBottomLeft, sBottomRight, uColor, uDepth);
}

VOID CSema3D::AddLine(FLOAT fX1, FLOAT fY1, FLOAT fZ1, FLOAT fX2, FLOAT fY2, FLOAT fZ2, UINT uColor)
{
    if (LINE_NUM <= m_uUsedLineNum) {
        ASSERT(FALSE);
        return;
    }
    if (m_fCullTopLeftX > fX1 || m_fCullBottomRightX < fX1) {
        if (m_fCullTopLeftX > fX2 || m_fCullBottomRightX < fX2) {
            return;
        }
    }
    if (m_fCullTopLeftY < fY1 && m_fCullTopLeftY < fY2) {
        return;
    }
    if (m_fCullBottomRightY > fY1 && m_fCullBottomRightY > fY2) {
        return;
    }
    SHLVector2D sPt1;
    Transform(sPt1, fX1, fY1, fZ1);
    SHLVector2D sPt2;
    Transform(sPt2, fX2, fY2, fZ2);


    SLine& sLine = m_asLines[m_uUsedLineNum];
    sLine.sPt1.x = sPt1.x;
    sLine.sPt1.y = sPt1.y;
    sLine.sPt2.x = sPt2.x;
    sLine.sPt2.y = sPt2.y;
    sLine.uColor = uColor;
    ++m_uUsedLineNum;
}
VOID CSema3D::AddSemaphore(FLOAT fX, FLOAT fY, FLOAT fZ, UINT uLevel, BOOLEAN bUp, UINT uColor)
{
    if (SEMAPHORE_NUM <= m_uUsedSemaNum) {
        ASSERT(FALSE);
        return;
    }
    if (m_fCullTopLeftX > fX || m_fCullBottomRightX < fX) {
        return;
    }
    if (m_fCullTopLeftY < fY || m_fCullBottomRightY > fY) {
        return;
    }

    CSemaPhore& cSema = m_acSemaphores[m_uUsedSemaNum];
    cSema.SetColor(uColor);
    CHAR szBuffer[64];
    BOOLEAN bSmallerCircle = FALSE;
    if (3 <= uLevel) {
        snprintf(szBuffer, 64, "D%d", uLevel-3 + 1);
        cSema.SetScale(1.0f);
    }
    else {
        bSmallerCircle = TRUE;
        snprintf(szBuffer, 64, "%d", uLevel);
        cSema.SetScale(0.85f + uLevel * 0.05f);
    }
    fY += (bUp ? 10.0f : -10.0f);
    SHLVector2D sPt;
    Transform(sPt, fX, fY, fZ);

    cSema.SetLocalPosition(sPt.x, sPt.y);
    cSema.Initialize(szBuffer, bSmallerCircle);
    cSema.OnUpdate(0.0f);
    ++m_uUsedSemaNum;
}

VOID CSema3D::RenderPolygons(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    FLOAT afPos[8];
    for (UINT uDepthIndex = 0; MEGAZZ_MAX_DEPTH > uDepthIndex; ++uDepthIndex) {
        for (UINT uIndex = 0; m_uUsedPoly2D > uIndex; ++uIndex) {
            const SPolygon2D& sPolygon = m_asPoly2D[uIndex];
            if (sPolygon.uDepth != uDepthIndex) {
                continue;
            }
            afPos[0] = sPolygon.sTopLeft.x + fOffsetX;
            afPos[1] = sPolygon.sTopLeft.y + fOffsetY;
            afPos[2] = sPolygon.sTopRight.x + fOffsetX;
            afPos[3] = sPolygon.sTopRight.y + fOffsetY;
            afPos[4] = sPolygon.sBottomLeft.x + fOffsetX;
            afPos[5] = sPolygon.sBottomLeft.y + fOffsetY;
            afPos[6] = sPolygon.sBottomRight.x + fOffsetX;
            afPos[7] = sPolygon.sBottomRight.y + fOffsetY;
            CGameRenderer::DrawFilledPolygon(afPos, 4, sPolygon.uColor);
        }
    }
}

VOID CSema3D::RenderLines(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    for (UINT uIndex = 0; m_uUsedLineNum > uIndex; ++uIndex) {
        const SLine& sLine = m_asLines[uIndex];
        CGameRenderer::DrawLine(sLine.sPt1.x + fOffsetX, sLine.sPt1.y + fOffsetY, sLine.sPt2.x + fOffsetX, sLine.sPt2.y + fOffsetY, sLine.uColor);
    }
}

VOID CSema3D::RenderSemaphores(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    for (UINT uIndex = 0; m_uUsedSemaNum > uIndex; ++uIndex) {
        const CSemaPhore& cSema = m_acSemaphores[uIndex];
        cSema.OnRender(fOffsetX, fOffsetY);
    }
}

VOID CSema3D::RenderChannelLabels(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    SHLVector2D sPt;
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        if (!m_abDepthEnabled[uIndex]) {
            continue;
        }
        CUITextLabel* pcLbl = m_apcChannelTxt[uIndex];
        if (NULL == pcLbl || !pcLbl->IsVisible()) {
            continue;
        }
        const SHLVector2D& sPos = pcLbl->GetLocalPosition();
        const FLOAT fZ = m_bNewMethod ? 0.0f : (FLOAT)(Z_THICKNESS + uIndex);
        Transform(sPt, sPos.x, sPos.y, fZ);
        sPt.x += fOffsetX - sPos.x;
        sPt.y += fOffsetY - sPos.y;
        pcLbl->OnRender(sPt.x, sPt.y);
    }
}
