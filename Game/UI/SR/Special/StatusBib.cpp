#include "stdafx.h"
#include "AppMain.h"
#include "MegaZZ.h"
#include "PlayerData.h"
#include "TradeLogDefs.h"
#include "SaveData.h"
#include "StatusBib.h"
#include <math.h>

#define BLACK_COLOR         RGBA(0,0,0,0xFF)
#define RETRACE_LINE_COLOR  RGBA(0xFF,0xFF,0xFF,0xFF)
CStatusBib::CStatusBib() :
CUIContainer(EBaseWidget_Container),
m_uDepthIndex(0),
m_uDepthNum(4),
m_uPoly2DNum(0),
m_uLine2DNum(0)
{
    
}

CStatusBib::~CStatusBib()
{

}
    
VOID CStatusBib::Initialize(VOID)
{
}
    
VOID CStatusBib::Release(VOID)
{

    CUIContainer::Release();
}

VOID CStatusBib::ClearChart(VOID)
{
    m_uPoly2DNum = 0;
    m_uLine2DNum = 0;
}

VOID CStatusBib::AddPoly2D(FLOAT fTopLeftX, FLOAT fTopLeftY,
                      FLOAT fTopRightX, FLOAT fTopRightY,
                      FLOAT fBottomLeftX, FLOAT fBottomLeftY,
                      FLOAT fBottomRightX, FLOAT fBottomRightY,
                      UINT uColor, UINT uDepthIndex)
{
    if (POLY2D_MAX_NUM <= m_uPoly2DNum) {
        return;
    }
    SPoly2D& sPoly = m_asPoly2D[m_uPoly2DNum];
    sPoly.afPos[0] = fTopLeftX;
    sPoly.afPos[1] = fTopLeftY;
    
    sPoly.afPos[2] = fTopRightX;
    sPoly.afPos[3] = fTopRightY;
    
    sPoly.afPos[4] = fBottomLeftX;
    sPoly.afPos[5] = fBottomLeftY;
    
    sPoly.afPos[6] = fBottomRightX;
    sPoly.afPos[7] = fBottomRightY;

    sPoly.uColor = uColor;
    sPoly.uDepthIndex = uDepthIndex;
    ++m_uPoly2DNum;
}

VOID CStatusBib::AddLine2D(FLOAT fPt1X, FLOAT fPt1Y, FLOAT fPt2X, FLOAT fPt2Y, FLOAT fThickness, UINT uColor, UINT uDepthIndex)
{
    if (LINE2D_MAX_NUM <= m_uLine2DNum) {
        return;
    }
    SLine2D& sLine = m_asLine2D[m_uLine2DNum];
    sLine.sPt1.x = fPt1X;
    sLine.sPt1.y = fPt1Y;
    
    sLine.sPt2.x = fPt2X;
    sLine.sPt2.y = fPt2Y;
    
    if (1.0f >= fThickness) {
        sLine.sTangents.x = sLine.sTangents.y = 0.0f;
    }
    else {
        const FLOAT fDiffX = fPt2X - fPt1X;
        const FLOAT fDiffY = fPt2Y - fPt1Y;
        const FLOAT fDist = (FLOAT)sqrt(fDiffX * fDiffX + fDiffY * fDiffY);
        // rotate 90 degrees (0,1) => (1,0) => (0,-1) => (-1,0)
        sLine.sTangents.x = fDiffY * fThickness * 0.5f / fDist;
        sLine.sTangents.y = -fDiffX * fThickness * 0.5f / fDist;
    }

    sLine.uColor = uColor;
    sLine.uDepthIndex = uDepthIndex;
    ++m_uLine2DNum;
}

#define BOTTOM_GRADIENT (142)
#define RETRACE_COLOR_UP   RGBA(110,110,110,0xFF)
#define RETRACE_COLOR_DN   RGBA(63,63,63,0xFF)

#define HEIGHT_FOR_3D (8.0f)
static VOID TransformToUI(SHLVector2D& sResult, const SHLVector2D& sOrigin, const SHLVector2D& sRight, const SHLVector2D& sUp, FLOAT fX, FLOAT fY)
{
    sResult.x = sOrigin.x + fX * sRight.x + fY * sUp.x;
    sResult.y = sOrigin.y + fX * sRight.y + fY * sUp.y;
}


#define RETRACE_UP_COLOR        RGBA(110,210,230,0xFF)
#define RETRACE_DOWN_COLOR      RGBA(255,110,110,0xFF)

VOID CStatusBib::PrepareChart(const CMegaZZ& cMegaZZ)
{
    const MegaZZ::SChannel* psChannels[MEGAZZ_MAX_DEPTH];
    FLOAT afSizes[MEGAZZ_MAX_DEPTH];
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        psChannels[uIndex] = &cMegaZZ.GetCurrentChannel(uIndex);
        afSizes[uIndex] = cMegaZZ.GetChannelSize(uIndex);
    }
    ClearChart();
    UINT uActualEndIndex = m_uDepthIndex + m_uDepthNum;
    if (MEGAZZ_MAX_DEPTH < uActualEndIndex) {
        uActualEndIndex = MEGAZZ_MAX_DEPTH;
    }
    const SHLVector2D& sLocalSize = GetLocalSize();
    ASSERT(MEGAZZ_MAX_DEPTH > m_uDepthIndex);
    SHLVector2D sUp, sRight;
    const FLOAT fSqrtf1 = sqrtf(1.0f);
    sUp.x = fSqrtf1; sUp.y = -fSqrtf1;
    sRight.x = fSqrtf1; sRight.y = fSqrtf1;

    const FLOAT fPixelRatio = 0.5f * (sLocalSize.y / afSizes[m_uDepthIndex]);
    FLOAT afTop[MEGAZZ_MAX_DEPTH];
    FLOAT afBottom[MEGAZZ_MAX_DEPTH];
    UINT auDepthIndex[MEGAZZ_MAX_DEPTH];
    UINT uUsedChannelNum = 0;
    {
        for (UINT uDepthIndex = m_uDepthIndex; uActualEndIndex > uDepthIndex; ++uDepthIndex) {
            auDepthIndex[uUsedChannelNum] = uDepthIndex;
            afTop[uUsedChannelNum] = afSizes[uDepthIndex] * fPixelRatio;
            afBottom[uUsedChannelNum] = -afTop[uUsedChannelNum];
            ++uUsedChannelNum;
        }
        if (0 == uUsedChannelNum) { // no data!
            return;
        }
        ASSERT(uUsedChannelNum < (MEGAZZ_MAX_DEPTH + 1));
    }
    
    // compute actual UI coordinates
    const SHLVector2D sOrigin = { sLocalSize.x * 0.5f, sLocalSize.y * 0.5f };
    SHLVector2D sUI_Left, sUI_Right, sUI_Top, sUI_Bottom;
    const UINT uUpColor = CPlayerData::GetBiBColor(0);
    const UINT uDownColor = CPlayerData::GetBiBColor(1);
    for (UINT uIndex = 0; uUsedChannelNum > uIndex; ++uIndex) {
        const UINT uDepthIndex = auDepthIndex[uIndex];
        const MegaZZ::SChannel& sChannel = *psChannels[uDepthIndex];
        UINT uColor;
        if (sChannel.byType == MegaZZ::EType_Down) {
            if (6 < sChannel.byDisplayNumber) {
                uColor = RETRACE_DOWN_COLOR;
            }
            else {
                uColor = uDownColor;
            }
        }
        else {
            if (6 < sChannel.byDisplayNumber) {
                uColor = RETRACE_UP_COLOR;
            }
            else {
                uColor = uUpColor;
            }
        }
        FLOAT fTopY = afTop[uIndex];
        FLOAT fBottomY = afBottom[uIndex];
        FLOAT fCenterY = (fTopY + fBottomY) * 0.5f;
        const FLOAT fRightX = (fTopY - fBottomY) * 0.5f;
        FLOAT fRightY = fCenterY;
        const FLOAT fLeftX = -fRightX;
        FLOAT fLeftY = fCenterY;

        // add main channel image
        TransformToUI(sUI_Left,     sOrigin, sRight, sUp, fLeftX,   fLeftY);
        TransformToUI(sUI_Top,      sOrigin, sRight, sUp, 0.0f,    fTopY);
        TransformToUI(sUI_Right,    sOrigin, sRight, sUp, fRightX,  fRightY);
        TransformToUI(sUI_Bottom,   sOrigin, sRight, sUp, 0.0f, fBottomY);
        AddPoly2D(sUI_Left.x, sUI_Left.y, sUI_Top.x, sUI_Top.y, sUI_Bottom.x, sUI_Bottom.y, sUI_Right.x, sUI_Right.y, uColor, uDepthIndex);
        
        {
            UINT uLineColor = BLACK_COLOR;
            AddLine2D(sUI_Left.x, sUI_Left.y, sUI_Top.x, sUI_Top.y, 1.0f, uLineColor, uDepthIndex);
            AddLine2D(sUI_Right.x, sUI_Right.y, sUI_Top.x, sUI_Top.y, 1.0f, uLineColor, uDepthIndex);
            AddLine2D(sUI_Right.x, sUI_Right.y, sUI_Bottom.x, sUI_Bottom.y, 1.0f, uLineColor, uDepthIndex);
            AddLine2D(sUI_Left.x, sUI_Left.y, sUI_Bottom.x, sUI_Bottom.y, 1.0f, uLineColor, uDepthIndex);
        }
    } // end for
}
