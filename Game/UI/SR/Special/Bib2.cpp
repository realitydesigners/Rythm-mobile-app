#include "stdafx.h"
#include "AppMain.h"
#include "MegaZZ.h"
#include "PlayerData.h"
#include "TradeLogDefs.h"
#include "SaveData.h"
#include "Bib2.h"
#include <math.h>

#define BLACK_COLOR         RGBA(0,0,0,0xFF)
#define RETRACE_LINE_COLOR  RGBA(0xFF,0xFF,0xFF,0xFF)
CBib2::CBib2() :
CUIContainer(EBaseWidget_Container),
m_uDepthIndex(0),
m_uDepthNum(4),
m_bSquareMode(TRUE),
m_bFlatMode(TRUE),
m_bHideX(TRUE),
m_bHideBars(TRUE),
m_bSmallestChannelWhite(FALSE),
m_uPoly2DNum(0),
m_uLine2DNum(0)
{
    
}

CBib2::~CBib2()
{

}
    
VOID CBib2::Initialize(VOID)
{
}
VOID CBib2::ChangeMode(BOOLEAN bSquareMode, BOOLEAN bFlatMode)
{
    m_bSquareMode = bSquareMode;
    m_bFlatMode = bFlatMode;
}
    
VOID CBib2::Release(VOID)
{

    CUIContainer::Release();
}

VOID CBib2::ClearChart(VOID)
{
    m_uPoly2DNum = 0;
    m_uLine2DNum = 0;
}

VOID CBib2::AddPoly2D(FLOAT fTopLeftX, FLOAT fTopLeftY,
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

VOID CBib2::AddLine2D(FLOAT fPt1X, FLOAT fPt1Y, FLOAT fPt2X, FLOAT fPt2Y, FLOAT fThickness, UINT uColor, UINT uDepthIndex)
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

VOID CBib2::PrepareChart(const CMegaZZ& cMegaZZ)
{
    const MegaZZ::SChannel* psChannels[MEGAZZ_MAX_DEPTH];
    FLOAT afSizes[MEGAZZ_MAX_DEPTH];
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        psChannels[uIndex] = &cMegaZZ.GetCurrentChannel(uIndex);
        afSizes[uIndex] = cMegaZZ.GetChannelSize(uIndex);
    }
    Prepare(psChannels, afSizes, cMegaZZ.GetCurrentPrice());
}
VOID CBib2::PrepareChart(const STradeLog& sLog, BOOLEAN bOpen)
{
    const MegaZZ::SChannel* psChannels[MEGAZZ_MAX_DEPTH];
    FLOAT afSizes[MEGAZZ_MAX_DEPTH];
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        psChannels[uIndex] = bOpen ? &sLog.asStart[uIndex] : &sLog.asEnd[uIndex];
        afSizes[uIndex] = psChannels[uIndex]->fTop - psChannels[uIndex]->fBottom;
    }
    Prepare(psChannels, afSizes, sLog.fOpenPrice);
}
VOID CBib2::Prepare(const MegaZZ::SChannel** ppsChannels, const FLOAT* pfSizes, FLOAT fCurrentPrice)
{
    ClearChart();
    UINT uActualEndIndex = m_uDepthIndex + m_uDepthNum;
    if (MEGAZZ_MAX_DEPTH < uActualEndIndex) {
        uActualEndIndex = MEGAZZ_MAX_DEPTH;
    }
    const SHLVector2D& sSize = GetLocalSize();
    ASSERT(MEGAZZ_MAX_DEPTH > m_uDepthIndex);
    const MegaZZ::SChannel& sBiggestChannel = *ppsChannels[m_uDepthIndex];
    
    const FLOAT fCenterPrice = (sBiggestChannel.fTop + sBiggestChannel.fBottom) * 0.5f;
    FLOAT fMaxPriceMovement = sSize.y;
    SHLVector2D sUp, sRight;
    if (!m_bSquareMode) { // diamond, use height of widget to compute price movement
        sUp.x = 0.0f; sUp.y = -1.0f;
        sRight.x = 1.0f; sRight.y = 0.0f;
    }
    else { // square, use diagonal to compute price movement
        const FLOAT fSqrtf1 = sqrtf(1.0f);
        sUp.x = fSqrtf1; sUp.y = -fSqrtf1;
        sRight.x = fSqrtf1; sRight.y = fSqrtf1;
    }
    const FLOAT fPixelPerPrice = fMaxPriceMovement / pfSizes[m_uDepthIndex];
    FLOAT afTop[MEGAZZ_MAX_DEPTH + 1];
    FLOAT afBottom[MEGAZZ_MAX_DEPTH + 1];
    UINT auDepthIndex[MEGAZZ_MAX_DEPTH + 1];
    UINT uUsedChannelNum = 0;
    {
        for (UINT uDepthIndex = m_uDepthIndex; uActualEndIndex > uDepthIndex; ++uDepthIndex) {
            const MegaZZ::SChannel& sChannel = *ppsChannels[uDepthIndex];
            if (0.0f == sChannel.fTop) {
                continue;
            }
            auDepthIndex[uUsedChannelNum] = uDepthIndex;
            afTop[uUsedChannelNum] = (sChannel.fTop - fCenterPrice) * fPixelPerPrice;
            afBottom[uUsedChannelNum] = (sChannel.fBottom - fCenterPrice) * fPixelPerPrice;
            ++uUsedChannelNum;
        }
        if (0 == uUsedChannelNum) { // no data!
            return;
        }
        ASSERT(uUsedChannelNum < (MEGAZZ_MAX_DEPTH + 1));
        const UINT uLastDepthIndex = auDepthIndex[uUsedChannelNum-1];
        const MegaZZ::SChannel& sChannel = *ppsChannels[uLastDepthIndex];
        if (0.0f < sChannel.fTop) {
            const FLOAT fDiff = (afTop[uUsedChannelNum-1] - afBottom[uUsedChannelNum-1]) * 0.1f;
            if (MegaZZ::EType_Up == sChannel.byType) {
                const FLOAT fTopY = afTop[uUsedChannelNum-1];
                afTop[uUsedChannelNum] = fTopY - fDiff * sChannel.byDisplayNumber;
                afBottom[uUsedChannelNum] = afTop[uUsedChannelNum] - fDiff;
            }
            else {
                const FLOAT fBottomY = afBottom[uUsedChannelNum-1];
                afBottom[uUsedChannelNum] = fBottomY + fDiff * sChannel.byDisplayNumber;
                afTop[uUsedChannelNum] = afBottom[uUsedChannelNum] + fDiff;
            }
        }
        else {
            afTop[uUsedChannelNum] = (fCurrentPrice - fCenterPrice) * fPixelPerPrice;
            afBottom[uUsedChannelNum] = afTop[uUsedChannelNum];
        }
    }
    
    const SHLVector2D& sLocalSize = GetLocalSize();
    // compute actual UI coordinates
    const SHLVector2D sOrigin = { sLocalSize.x * 0.5f - (m_bHideBars ? 0.0f : (sLocalSize.x * 0.5f - fMaxPriceMovement * 0.5f) + 3.0f), sLocalSize.y * 0.5f };
    SHLVector2D sUI_Left, sUI_Right, sUI_Top, sUI_Bottom;

    FLOAT afUITop[MEGAZZ_MAX_DEPTH + 1];
    FLOAT afUIBottom[MEGAZZ_MAX_DEPTH + 1];
    const UINT uUpColor = CPlayerData::GetBiBColor(0);
    const UINT uDownColor = CPlayerData::GetBiBColor(1);
    for (UINT uIndex = 0; uUsedChannelNum > uIndex; ++uIndex) {
        const UINT uDepthIndex = auDepthIndex[uIndex];
        const MegaZZ::SChannel& sChannel = *ppsChannels[uDepthIndex];
        const UINT uColor = (sChannel.byType == MegaZZ::EType_Down) ? uDownColor : uUpColor;
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
        
        afUITop[uIndex] = sUI_Top.y;
        afUIBottom[uIndex] = sUI_Bottom.y;
        {
            UINT uLineColor = BLACK_COLOR;
            if (m_bSmallestChannelWhite && (uUsedChannelNum == (uIndex + 1))) {
                uLineColor = 0xFFFFFFFF;
            }
            AddLine2D(sUI_Left.x, sUI_Left.y, sUI_Top.x, sUI_Top.y, 1.0f, uLineColor, uDepthIndex);
            AddLine2D(sUI_Right.x, sUI_Right.y, sUI_Top.x, sUI_Top.y, 1.0f, uLineColor, uDepthIndex);
            AddLine2D(sUI_Right.x, sUI_Right.y, sUI_Bottom.x, sUI_Bottom.y, 1.0f, uLineColor, uDepthIndex);
            AddLine2D(sUI_Left.x, sUI_Left.y, sUI_Bottom.x, sUI_Bottom.y, 1.0f, uLineColor, uDepthIndex);
        }
        
        const FLOAT fPerRetraceSize = (fTopY - fBottomY) * 0.01f;
        const FLOAT fRetracedAmount = sChannel.byRetracementPercent * fPerRetraceSize;
        const BOOLEAN bUp = MegaZZ::EType_Up == sChannel.byType;
        if (((uUsedChannelNum-1) > uIndex)) { // if not yet last to draw
            // now we try to draw the retrace channel
            const FLOAT fInnerTopY = afTop[uIndex+1];
            const FLOAT fInnerBottomY = afBottom[uIndex+1];
            const FLOAT fInnerHeight = fInnerTopY - fInnerBottomY;
            const FLOAT fInnerRightX = (fInnerTopY - fInnerBottomY) * 0.5f;
            const FLOAT fInnerLeftX = -fInnerRightX;
            FLOAT fRetraceTopY = fTopY;
            FLOAT fRetraceBottomY = fBottomY;
            if (fRetracedAmount > fInnerHeight) {
                const FLOAT fRetraceDiff = fRetracedAmount - fInnerHeight;
                if (bUp) { // if channel is UP
                    fRetraceBottomY = fTopY - fRetraceDiff - fInnerHeight;
                }
                else { // if channel is DN
                    fRetraceTopY = fBottomY + fRetraceDiff + fInnerHeight;
                }
            }
            else {
                if (bUp) { // if channel is UP
                    fRetraceBottomY = fTopY - fInnerHeight;
                }
                else {
                    fRetraceTopY = fBottomY + fInnerHeight;
                }
            }
            if (bUp) { // if channel is UP
                if (fInnerBottomY < fRetraceBottomY) {
                    fRetraceBottomY = fInnerBottomY;
                }
            }
            else { // if channel is DN
                if (fInnerTopY > fRetraceTopY) {
                    fRetraceTopY = fInnerTopY;
                }
            }
            const FLOAT fRetraceLeftX = fInnerLeftX;
            const FLOAT fRetraceY1 = fRetraceTopY + fRetraceLeftX;
            const FLOAT fRetraceY2 = fRetraceBottomY - fRetraceLeftX;
            const FLOAT fInnerCenterY = (fInnerTopY + fInnerBottomY) * 0.5f;
            
            TransformToUI(sUI_Left,     sOrigin, sRight, sUp, fRetraceLeftX,   fRetraceY1);
            TransformToUI(sUI_Top,      sOrigin, sRight, sUp, 0.0f,    fRetraceTopY);
            TransformToUI(sUI_Right,    sOrigin, sRight, sUp, 0.0f,  fInnerCenterY);
            TransformToUI(sUI_Bottom,   sOrigin, sRight, sUp, fRetraceLeftX, fInnerCenterY);
            
            AddPoly2D(sUI_Left.x, sUI_Left.y, sUI_Top.x, sUI_Top.y, sUI_Bottom.x, sUI_Bottom.y, sUI_Right.x, sUI_Right.y, uDownColor, uDepthIndex);
            AddLine2D(sUI_Left.x, sUI_Left.y, sUI_Top.x, sUI_Top.y, 1.0f, BLACK_COLOR, uDepthIndex);
            AddLine2D(sUI_Left.x, sUI_Left.y, sUI_Bottom.x, sUI_Bottom.y, 1.0f, BLACK_COLOR, uDepthIndex);

            TransformToUI(sUI_Left,     sOrigin, sRight, sUp, 0.0f,   fRetraceTopY);
            TransformToUI(sUI_Top,      sOrigin, sRight, sUp, fInnerRightX,    fRetraceY1);
            TransformToUI(sUI_Right,    sOrigin, sRight, sUp, fInnerRightX,  fInnerCenterY);
            TransformToUI(sUI_Bottom,   sOrigin, sRight, sUp, 0.0f, fInnerCenterY);
            AddPoly2D(sUI_Left.x, sUI_Left.y, sUI_Top.x, sUI_Top.y, sUI_Bottom.x, sUI_Bottom.y, sUI_Right.x, sUI_Right.y, uDownColor, uDepthIndex);
            AddLine2D(sUI_Left.x, sUI_Left.y, sUI_Top.x, sUI_Top.y, 1.0f, BLACK_COLOR, uDepthIndex);
            AddLine2D(sUI_Top.x, sUI_Top.y, sUI_Right.x, sUI_Right.y, 1.0f, BLACK_COLOR, uDepthIndex);

            
            TransformToUI(sUI_Left,     sOrigin, sRight, sUp, fRetraceLeftX,   fInnerCenterY);
            TransformToUI(sUI_Top,      sOrigin, sRight, sUp, 0.0f,    fInnerCenterY);
            TransformToUI(sUI_Right,    sOrigin, sRight, sUp, 0.0f,  fRetraceBottomY);
            TransformToUI(sUI_Bottom,   sOrigin, sRight, sUp, fRetraceLeftX, fRetraceY2);
            
            AddPoly2D(sUI_Left.x, sUI_Left.y, sUI_Top.x, sUI_Top.y, sUI_Bottom.x, sUI_Bottom.y, sUI_Right.x, sUI_Right.y, uUpColor, uDepthIndex);
            AddLine2D(sUI_Left.x, sUI_Left.y, sUI_Bottom.x, sUI_Bottom.y, 1.0f, BLACK_COLOR, uDepthIndex);
            AddLine2D(sUI_Right.x, sUI_Right.y, sUI_Bottom.x, sUI_Bottom.y, 1.0f, BLACK_COLOR, uDepthIndex);

            TransformToUI(sUI_Left,     sOrigin, sRight, sUp, 0.0f,   fInnerCenterY);
            TransformToUI(sUI_Top,      sOrigin, sRight, sUp, fInnerRightX,    fInnerCenterY);
            TransformToUI(sUI_Right,    sOrigin, sRight, sUp, fInnerRightX,  fRetraceY2);
            TransformToUI(sUI_Bottom,   sOrigin, sRight, sUp, 0.0f, fRetraceBottomY);
            
            AddPoly2D(sUI_Left.x, sUI_Left.y, sUI_Top.x, sUI_Top.y, sUI_Bottom.x, sUI_Bottom.y, sUI_Right.x, sUI_Right.y, uUpColor, uDepthIndex);
            AddLine2D(sUI_Top.x, sUI_Top.y, sUI_Right.x, sUI_Right.y, 1.0f, BLACK_COLOR, uDepthIndex);
            AddLine2D(sUI_Bottom.x, sUI_Bottom.y, sUI_Right.x, sUI_Right.y, 1.0f, BLACK_COLOR, uDepthIndex);

        } //end if
        else { // last entry
            if (0.0f < fRetracedAmount) {
                FLOAT fRetraceTopY = fTopY;
                FLOAT fRetraceBottomY = fBottomY;
                if (bUp) { // if channel is UP
                    fRetraceBottomY = fTopY - fRetracedAmount;
                }
                else {
                    fRetraceTopY = fBottomY + fRetracedAmount;
                }
                const FLOAT fRetraceLeftX = fPerRetraceSize * -5.0f;
                const FLOAT fRetraceRightX = -fRetraceLeftX;
                const FLOAT fRetraceY1 = fRetraceTopY - fRetraceRightX;
                const FLOAT fRetraceY2 = fRetraceBottomY + fRetraceRightX;
                const FLOAT fCenterY = (afTop[uUsedChannelNum] + afBottom[uUsedChannelNum]) * 0.5f;
                
                if (bUp || (sChannel.byDisplayNumber < sChannel.byRetracementNumber)) {
                    TransformToUI(sUI_Left,     sOrigin, sRight, sUp, fRetraceLeftX,   fRetraceY1);
                    TransformToUI(sUI_Top,      sOrigin, sRight, sUp, 0.0f,    fRetraceTopY);
                    
                    TransformToUI(sUI_Right,    sOrigin, sRight, sUp, 0.0f,  fCenterY);
                    TransformToUI(sUI_Bottom,   sOrigin, sRight, sUp, fRetraceLeftX, fCenterY);
                    
                    AddPoly2D(sUI_Left.x, sUI_Left.y, sUI_Top.x, sUI_Top.y, sUI_Bottom.x, sUI_Bottom.y, sUI_Right.x, sUI_Right.y, uDownColor, uDepthIndex);
                    AddLine2D(sUI_Left.x, sUI_Left.y, sUI_Top.x, sUI_Top.y, 1.0f, BLACK_COLOR, uDepthIndex);
                    AddLine2D(sUI_Left.x, sUI_Left.y, sUI_Bottom.x, sUI_Bottom.y, 1.0f, BLACK_COLOR, uDepthIndex);
                    
                    TransformToUI(sUI_Left,     sOrigin, sRight, sUp, fRetraceRightX,   fRetraceY1);
                    TransformToUI(sUI_Bottom,   sOrigin, sRight, sUp, fRetraceRightX, fCenterY);
                    AddPoly2D(sUI_Top.x, sUI_Top.y, sUI_Left.x, sUI_Left.y, sUI_Right.x, sUI_Right.y, sUI_Bottom.x, sUI_Bottom.y, uDownColor, uDepthIndex);
                    AddLine2D(sUI_Left.x, sUI_Left.y, sUI_Top.x, sUI_Top.y, 1.0f, BLACK_COLOR, uDepthIndex);
                    AddLine2D(sUI_Left.x, sUI_Left.y, sUI_Bottom.x, sUI_Bottom.y, 1.0f, BLACK_COLOR, uDepthIndex);
                }
                if (!bUp || (sChannel.byDisplayNumber < sChannel.byRetracementNumber)) {
                    TransformToUI(sUI_Left,   sOrigin, sRight, sUp, fRetraceLeftX, fCenterY);
                    TransformToUI(sUI_Top,    sOrigin, sRight, sUp, 0.0f,  fCenterY);
                    TransformToUI(sUI_Right,    sOrigin, sRight, sUp, 0.0f,  fRetraceBottomY);
                    TransformToUI(sUI_Bottom,   sOrigin, sRight, sUp, fRetraceLeftX, fRetraceY2);
                    AddPoly2D(sUI_Left.x, sUI_Left.y, sUI_Top.x, sUI_Top.y, sUI_Bottom.x, sUI_Bottom.y, sUI_Right.x, sUI_Right.y, uUpColor, uDepthIndex);
                    AddLine2D(sUI_Left.x, sUI_Left.y, sUI_Bottom.x, sUI_Bottom.y, 1.0f, BLACK_COLOR, uDepthIndex);
                    AddLine2D(sUI_Right.x, sUI_Right.y, sUI_Bottom.x, sUI_Bottom.y, 1.0f, BLACK_COLOR, uDepthIndex);
                    
                    TransformToUI(sUI_Left,   sOrigin, sRight, sUp, fRetraceRightX, fCenterY);
                    TransformToUI(sUI_Bottom,   sOrigin, sRight, sUp, fRetraceRightX, fRetraceY2);
                    AddPoly2D(sUI_Top.x, sUI_Top.y, sUI_Left.x, sUI_Left.y, sUI_Right.x, sUI_Right.y, sUI_Bottom.x, sUI_Bottom.y, uUpColor, uDepthIndex);
                    AddLine2D(sUI_Left.x, sUI_Left.y, sUI_Bottom.x, sUI_Bottom.y, 1.0f, BLACK_COLOR, uDepthIndex);
                    AddLine2D(sUI_Right.x, sUI_Right.y, sUI_Bottom.x, sUI_Bottom.y, 1.0f, BLACK_COLOR, uDepthIndex);
                }
            }
            // draw currwnt black box
            const FLOAT fCurrentPriceOffset = sChannel.byDisplayNumber * fPerRetraceSize * 10.0f;
            const FLOAT fCurrentPriceLeftX = -fPerRetraceSize * 5.0f;
            const FLOAT fCurrentPriceRightX = -fCurrentPriceLeftX;
            FLOAT fCurrentTopY, fCurrentBottomY;
            if (bUp) {
                fCurrentTopY = fTopY - fCurrentPriceOffset;
                fCurrentBottomY = fCurrentTopY - fPerRetraceSize * 10.0f;
            }
            else {
                fCurrentBottomY = fBottomY + fCurrentPriceOffset;
                fCurrentTopY = fCurrentBottomY + fPerRetraceSize * 10.0f;
            }
            const FLOAT fCenterPriceY = (fCurrentBottomY + fCurrentTopY) * 0.5f;
            TransformToUI(sUI_Left,     sOrigin, sRight, sUp, fCurrentPriceLeftX,   fCenterPriceY);
            TransformToUI(sUI_Top,      sOrigin, sRight, sUp, 0.0f,    fCurrentTopY);
            TransformToUI(sUI_Right,    sOrigin, sRight, sUp, fCurrentPriceRightX,  fCenterPriceY);
            TransformToUI(sUI_Bottom,   sOrigin, sRight, sUp, 0.0f, fCurrentBottomY);
            
            AddPoly2D(sUI_Left.x, sUI_Left.y, sUI_Top.x, sUI_Top.y, sUI_Bottom.x, sUI_Bottom.y, sUI_Right.x, sUI_Right.y, BLACK_COLOR, uDepthIndex);
        }
    } // end for
    if (m_bHideBars) {
        return;
    }
    // now we draw the right side chart
    const FLOAT fStartX = sOrigin.x + fMaxPriceMovement * 0.5f + 5.0f;
    const FLOAT fEndX = sSize.x  - 5.0f;
    const FLOAT fXSize = (fEndX - fStartX - uUsedChannelNum) / uUsedChannelNum;
    for (UINT uIndex = 0; uUsedChannelNum > uIndex; ++uIndex) {
        const FLOAT fX = fStartX + uIndex * (fXSize + 1);
        const FLOAT fX2 = fX + fXSize;
        const FLOAT fTopY = afUITop[uIndex];
        const FLOAT fBottomY = afUIBottom[uIndex];
        const UINT uDepthIndex = auDepthIndex[uIndex];
        const MegaZZ::SChannel& sChannel = *ppsChannels[uDepthIndex];
        
        const UINT uColor = (sChannel.byType == MegaZZ::EType_Down) ? uDownColor : uUpColor;
        const UINT uRetraceColor = (sChannel.byType == MegaZZ::EType_Up) ? uDownColor : uUpColor;
        AddPoly2D(fX, fTopY, fX2, fTopY, fX, fBottomY, fX2, fBottomY, uColor, uIndex);
        const FLOAT fUnitY = (fBottomY - fTopY) * 0.1f;
        const BOOLEAN bUp = MegaZZ::EType_Up == sChannel.byType;
        if (0 < sChannel.byRetracementNumber) {
            FLOAT fRYTop;
            FLOAT fRYBottom;
            if (bUp) {
                fRYBottom = fTopY + (sChannel.byRetracementNumber + 0.5f) * fUnitY;
                fRYTop = fTopY;
            }
            else {
                fRYTop = fBottomY - (sChannel.byRetracementNumber + 0.5f) * fUnitY;
                fRYBottom = fBottomY;
            }
            AddPoly2D(fX+2, fRYTop, fX2-2, fRYTop, fX+2, fRYBottom, fX2-2, fRYBottom, uRetraceColor, uIndex);
        }
        FLOAT fYTop;
        FLOAT fYBottom;
        if (bUp) {
            fYBottom = fTopY + (sChannel.byDisplayNumber + 1) * fUnitY;
            fYTop = fYBottom - fUnitY;
        }
        else {
            fYTop = fBottomY - (sChannel.byDisplayNumber + 1) * fUnitY;
            fYBottom = fYTop + fUnitY;
        }
        AddPoly2D(fX+2, fYTop, fX2-2, fYTop, fX+2, fYBottom, fX2-2, fYBottom, BLACK_COLOR, uIndex);
    }
}
