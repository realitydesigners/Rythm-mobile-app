#include "stdafx.h"
#include "CMain.h"
#include "EventManager.h"
#include "GameRenderer.h"
#include "ShiftedRenko.h"
#include "SRGraphDef.h"
#include "SRChannel.h"
#include <math.h>

#define CHANNEL_PADDING (4.0f)

CSRChannel::CSRChannel() :
CSRBaseGraph(CSRBaseGraph::EType_Channel),
m_pcSR(NULL),
m_uDepth(0),
m_psSquares(NULL),
m_uMax(0),
m_uUsed(0),
m_psAreas(NULL),
m_uAreaMax(0),
m_uAreaUsed(0),
m_uBlockNum(0),
m_uMaxdisplayNum(0),
m_uHistoryNum(0),
m_uXOffset(0),
m_uMaxXOffset(0),
m_nMaxYOffset(0),
m_nMinYOffset(0),
m_nYOffset(0),
m_fBoxPixel(0.0f),
m_fPrevTouchX(0.0f),
m_fPrevTouchY(0.0f),
m_bTouched(FALSE),
m_bScrollMode(FALSE)
{

}

CSRChannel::~CSRChannel()
{
    ASSERT(NULL == m_psSquares);
}


BOOLEAN CSRChannel::Initialize(UINT uBoxNum)
{
    if (!CSRBaseGraph::Initialize(uBoxNum)) {
        return FALSE;
    }
    uBoxNum *= 2;
    ASSERT(0 < uBoxNum);
    const SHLVector2D& sSize = GetLocalSize();
    m_fBoxPixel = (sSize.y - 2.0f * CHANNEL_PADDING) / uBoxNum;
    m_uBlockNum = uBoxNum;
    m_uMaxdisplayNum = (UINT)((sSize.x - 20.0f) / m_fBoxPixel);
    {
        const UINT uMaxSq = m_uMaxdisplayNum + 1;
        if (uMaxSq != m_uMax) {
            SAFE_DELETE_ARRAY(m_psSquares);
            m_psSquares = new SR_Square[uMaxSq];
        }
        if (NULL == m_psSquares) {
            ASSERT(FALSE);
            return FALSE;
        }
        m_uMax = uMaxSq;
        m_uUsed = 0;
    }
    {
        const UINT uMaxArea = m_uMaxdisplayNum;
        if (uMaxArea != m_uAreaMax) {
            SAFE_DELETE_ARRAY(m_psAreas);
            m_psAreas = new SR_Area[uMaxArea];
        }
        if (NULL == m_psAreas) {
            ASSERT(FALSE);
            return FALSE;
        }
        m_uAreaMax = uMaxArea;
        m_uAreaUsed = 0;
    }
    AddSnapLatestBtn(0);
    return TRUE;
}
    
VOID CSRChannel::Release(VOID)
{
    m_pcSR = NULL;
    m_uDepth = 0;
    SAFE_DELETE_ARRAY(m_psSquares);
    m_uMax = 0;
    CSRBaseGraph::Release();
}

VOID CSRChannel::ClearChart(VOID)
{
    m_uUsed = 0;
    m_uAreaUsed = 0;
}

VOID CSRChannel::AddBox(FLOAT fX, FLOAT fY, FLOAT fW, FLOAT fH, UINT uColor)
{
    if (m_uMax <= m_uUsed) {
        ASSERT(FALSE);
        return;
    }
    SR_Square& sSq = m_psSquares[m_uUsed];
    sSq.fX = fX;
    sSq.fY = fY;
    sSq.fW = fW;
    sSq.fH = fH;
    sSq.uColor = uColor;
    ++m_uUsed;
}
VOID CSRChannel::AddArea(FLOAT fX1, FLOAT fY1, FLOAT fX2, FLOAT fY2, FLOAT fY3, FLOAT fY4, UINT uColor)
{
    if (m_uAreaMax <= m_uAreaUsed) {
        ASSERT(FALSE);
        return;
    }
    SR_Area& sArea = m_psAreas[m_uAreaUsed];
    sArea.fX1 = fX1;
    sArea.fY1 = fY1;
    sArea.fX2 = fX2;
    sArea.fY2 = fY2;
    sArea.fY3 = fY3;
    sArea.fY4 = fY4;
    sArea.uColor = uColor;
    ++m_uAreaUsed;
}

VOID CSRChannel::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    const SHLVector2D& sPos = GetWorldPosition();
    const SHLVector2D& sSize = GetWorldSize();
    CGameRenderer::DrawRectOutline(sPos.x-1 + fOffsetX, sPos.y-1 + fOffsetY, sSize.x+2, sSize.y+2, RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    if (NULL == m_psSquares) {
        return;
    }
    CMain::SetScissorTest(sPos.x-1 + fOffsetX, sPos.y-1 + fOffsetY, sSize.x+1, sSize.y+1);
    const FLOAT fNewOffsetX = fOffsetX + sPos.x;
    const FLOAT fNewOffsetY = fOffsetY + sPos.y;
    FLOAT afPos[8] = { 0.0f };
    for (UINT uIndex = 0; m_uAreaUsed > uIndex; ++uIndex) {
        const SR_Area& sArea = m_psAreas[uIndex];
        afPos[0] = fNewOffsetX + sArea.fX1;
        afPos[1] = fNewOffsetY + sArea.fY1;
        
        afPos[2] = fNewOffsetX + sArea.fX2;
        afPos[3] = fNewOffsetY + sArea.fY2;
        
        afPos[4] = afPos[0];
        afPos[5] = fNewOffsetY + sArea.fY3;
        
        afPos[6] = afPos[2];
        afPos[7] = fNewOffsetY + sArea.fY4;
        CGameRenderer::DrawFilledPolygon(afPos, 4, sArea.uColor);
    }
    FLOAT fCrossX = -1.0f;
    FLOAT fCrossY = -1.0f;
    for (UINT uIndex = 0; m_uUsed > uIndex; ++uIndex) {
        const SR_Square& sSquare = m_psSquares[uIndex];
        CGameRenderer::DrawRectNoTex(fNewOffsetX + sSquare.fX,
                                     fNewOffsetY + sSquare.fY,
                                     sSquare.fW,
                                     sSquare.fH,
                                     sSquare.uColor);
        if (SR_UP_COLOR_3 == sSquare.uColor) {
            fCrossX = fNewOffsetX + sSquare.fX + sSquare.fW * 0.5f;
            fCrossY = fNewOffsetY + sSquare.fY + sSquare.fH * 0.5f;
        }
    }
    if (0.0f < fCrossX) {
        const FLOAT fX1 = fCrossX - 10.0f;
        const FLOAT fX2 = fCrossX + 10.0f;
        const FLOAT fY1 = fCrossY - 10.0f;
        const FLOAT fY2 = fCrossY + 10.0f;
        CGameRenderer::DrawLine(fX1, fY1, fX2, fY2, RGBA(0xFF, 0xFF, 0, 0xFF));
        CGameRenderer::DrawLine(fX1, fY2, fX2, fY1, RGBA(0xFF, 0xFF, 0, 0xFF));
    }
    CMain::ClearScissorTest();
    CSRBaseGraph::OnRender(fOffsetX, fOffsetY);
}

//VOID CSRChannel::PrepareChart(const CShiftedRenko& cSR, UINT uDepth) // oldest in back
//{
//    m_pcSR = &cSR;
//    m_uDepth = uDepth;
//    m_uUsed = 0;
//    m_uAreaUsed = 0;
//    m_uHistoryNum = cSR.GetSRNum(uDepth);
//    if (0 == m_uHistoryNum) {
//        m_uXOffset = m_uMaxXOffset = 0;
//        UpdateIndicator();
//        return;
//    }
//    const UINT uIterationNum = m_uMaxdisplayNum > m_uHistoryNum ? m_uHistoryNum : m_uMaxdisplayNum;
//    m_uMaxXOffset = m_uHistoryNum - uIterationNum;
//    if (m_uXOffset > m_uMaxXOffset) { // safety clamp
//        m_uXOffset = m_uMaxXOffset;
//    }
//    UpdateIndicator();
//    const UINT uLatestIndex = cSR.GetSRCurrentIndex(uDepth);
//    const UINT uCurrentIndex = (uLatestIndex + SHIFTED_RANKO_HISTORY_SIZE - m_uXOffset) % SHIFTED_RANKO_HISTORY_SIZE;
//    const Renko::SShiftedRenko* psSR = cSR.GetSR(uDepth);
//    ASSERT(psSR->byRankoNum <= m_uBlockNum);
//    const Renko::SShiftedRenko& sLatestSR = psSR[uLatestIndex];
//    // compute latest frame index actual Y
//    const INT nLatestFrameY = (m_uBlockNum - sLatestSR.byRankoNum) / 2;
//    const INT nMinFrameY = sLatestSR.nLowerIndex - nLatestFrameY;
//    const INT nMaxFrameY = nMinFrameY + m_uBlockNum;
//
//    INT nLowestIndex = sLatestSR.nLowerIndex;
//    INT nHighestIndex = nLowestIndex;
//    const UINT uScanNum = uIterationNum + m_uXOffset;
//    for (UINT uIndex = 0; uScanNum > uIndex; ++uIndex) {
//        const UINT uActualIndex = (uLatestIndex + SHIFTED_RANKO_HISTORY_SIZE - uIndex) % SHIFTED_RANKO_HISTORY_SIZE;
//        const Renko::SShiftedRenko& sSR = psSR[uActualIndex];
//        if (nLowestIndex > sSR.nLowerIndex) {
//            nLowestIndex = sSR.nLowerIndex;
//        }
//        else if (nHighestIndex < sSR.nLowerIndex) {
//            nHighestIndex = sSR.nLowerIndex;
//        }
//    }
//    nHighestIndex += psSR->byRankoNum;
//    m_nMaxYOffset = nMinFrameY - nLowestIndex;
//    if (0 > m_nMaxYOffset) {
//        m_nMaxYOffset = 0;
//    }
//    m_nMinYOffset = nMaxFrameY - nHighestIndex;
//    if (0 < m_nMinYOffset) {
//        m_nMinYOffset = 0;
//    }
//    // clamp
//    if (m_nMaxYOffset < m_nYOffset) {
//        m_nYOffset = 0;
//    }
//    else if (m_nMinYOffset > m_nYOffset) {
//        m_nYOffset = m_nMinYOffset;
//    }
//    const FLOAT fChannelHeight = (psSR->byRankoNum + 2) * m_fBoxPixel;
//    FLOAT fX = (uIterationNum - 1) * m_fBoxPixel;
//    const FLOAT fBottomY = GetLocalSize().y - CHANNEL_PADDING - m_fBoxPixel;
//    const Renko::SShiftedRenko& sCurrentSR = psSR[uCurrentIndex];
//    INT nCurrentFrameY = nLatestFrameY + (sCurrentSR.nLowerIndex - sLatestSR.nLowerIndex) + m_nYOffset;
//    BYTE byPrevBarType = 0;
//    FLOAT fPrevBottomY = 0.0f;
//    FLOAT fPrevAreaX = 0.0f;
//    for (UINT uIndex = 0; uIterationNum > uIndex; ++uIndex) {
//        const UINT uActualIndex = (uCurrentIndex + SHIFTED_RANKO_HISTORY_SIZE - uIndex) % SHIFTED_RANKO_HISTORY_SIZE;
//        const Renko::SShiftedRenko& sSR = psSR[uActualIndex];
//        
//        const UINT uAreaColor = (byPrevBarType == Renko::EType_Down) ? SR_DOWN_COLOR_2 : SR_DOWN_COLOR_1;
//        UINT uReplaceColor = RGBA(0x0, 0xFF, 0, 0xFF);
//        UINT uReplaceIndex = sSR.byLowRetraceIndex;;
//        if (sSR.byBarType == Renko::EType_Down) { // if it was a down bar
//            uReplaceColor = RGBA(0xFF, 0, 0, 0xFF);
//            uReplaceIndex = sSR.byHighRetraceIndex;
//        }
//        if (0 < uIndex) {
//            if (byPrevBarType == Renko::EType_Down) {
//                ++nCurrentFrameY;
//            }
//            else {
//                --nCurrentFrameY;
//            }
//        }
//        byPrevBarType = sSR.byBarType;
//        const FLOAT fAreaX = fX + m_fBoxPixel;
//        const FLOAT fAreaBottomY = fBottomY - nCurrentFrameY * m_fBoxPixel + m_fBoxPixel * 2.0f;
//        if (0 == uIndex) {
//            fPrevAreaX = fAreaX;
//            fPrevBottomY = fAreaBottomY;
//        }
//        else {
//            const FLOAT fPrevTopY = fPrevBottomY - fChannelHeight;
//            const FLOAT fNowTopY = fAreaBottomY - fChannelHeight;
//            AddArea(fAreaX, fNowTopY, fAreaX + m_fBoxPixel, fPrevTopY, fAreaBottomY, fPrevBottomY, uAreaColor);
//            fPrevBottomY = fAreaBottomY;
//        }
//        
//        if (0xFF != uReplaceIndex) {
//            const FLOAT fBoxY = fBottomY - (nCurrentFrameY + uReplaceIndex) * m_fBoxPixel;
//            AddBox(fX, fBoxY, m_fBoxPixel, m_fBoxPixel, uReplaceColor);
//        }
//        if (0 == uIndex && 0 == m_uXOffset) {
//            const FLOAT fBoxY = fBottomY - (nCurrentFrameY + sSR.byCurrentIndex) * m_fBoxPixel;
//            AddBox(fX, fBoxY, m_fBoxPixel, m_fBoxPixel, SR_UP_COLOR_3);
//        }
//        fX -= m_fBoxPixel;
//    }
//}

VOID CSRChannel::UpdateIndicator(VOID)
{
    CHAR szBuffer[64];
    snprintf(szBuffer, 64, "C(%d)", Renko::GetChannelValue(m_uDepth));
    SetIndicatorString(szBuffer);
}

BOOLEAN CSRChannel::OnTouchBegin(FLOAT fX, FLOAT fY)
{
    if (CSRBaseGraph::OnTouchBegin(fX, fY)) {
        m_bTouched = FALSE;
        return TRUE;
    }
    const SHLVector2D& sPos = { fX, fY };
    m_bTouched = HitTest(sPos);
    m_bScrollMode = FALSE;
    m_fPrevTouchX = fX;
    m_fPrevTouchY = fY;
    return m_bTouched;
}

BOOLEAN CSRChannel::OnTouchMove(FLOAT fX, FLOAT fY)
{
    if (!m_bTouched) {
        return CSRBaseGraph::OnTouchMove(fX, fY);
    }
    const FLOAT fDiffX = fX - m_fPrevTouchX;
    const FLOAT fDiffY = fY - m_fPrevTouchY;
    const FLOAT fDiffSquared = fDiffX * fDiffX + fDiffY * fDiffY;
    if (!m_bScrollMode) {
        if (fDiffSquared > 25.0f) {
            m_bScrollMode = TRUE;
            m_fPrevTouchX = fX;
            m_fPrevTouchY = fY;
        }
        return TRUE;
    }
    // scroll mode
    UINT uNewOffsetX = m_uXOffset;
    INT nNewOffsetY = m_nYOffset;
    if (m_fBoxPixel <= abs(fDiffX)) {
        INT nOffset = (INT)(fDiffX / m_fBoxPixel);
        m_fPrevTouchX += nOffset * m_fBoxPixel;
        if (0 < nOffset) {
            uNewOffsetX += nOffset;
            if (m_uMaxXOffset < uNewOffsetX) {
                uNewOffsetX = m_uMaxXOffset;
            }
        }
        else {
            ASSERT(0 != nOffset);
            if (uNewOffsetX >= (UINT)(-nOffset)) {
                uNewOffsetX += nOffset;
            }
        }
    }
    if (m_fBoxPixel <= abs(fDiffY)) {
        INT nOffset = (INT)(fDiffY / m_fBoxPixel);
        m_fPrevTouchY += nOffset * m_fBoxPixel;
        if (0 < nOffset) {
            nNewOffsetY -= nOffset;
            if (m_nMinYOffset > nNewOffsetY) {
                nNewOffsetY = m_nMinYOffset;
            }
        }
        else {
            ASSERT(0 != nOffset);
            nNewOffsetY -= nOffset;
            if (m_nMaxYOffset < nNewOffsetY) {
                nNewOffsetY = m_nMaxYOffset;
            }
        }
    }
    if (m_uXOffset != uNewOffsetX || m_nYOffset != nNewOffsetY) {
        m_uXOffset = uNewOffsetX;
        m_nYOffset = nNewOffsetY;
        if (NULL != m_pcSR) {
//            PrepareChart(*m_pcSR, m_uDepth);
        }
    }
    return TRUE;
}
BOOLEAN CSRChannel::OnTouchEnd(FLOAT fX, FLOAT fY)
{
    return CSRBaseGraph::OnTouchEnd(fX, fY);
}

VOID CSRChannel::OnReceiveEvent(CEvent& cEvent)
{
    // only 1 event
    if (0 != m_uXOffset) {
        m_uXOffset = 0;
        m_nYOffset = 0;
        if (NULL != m_pcSR) {
//            PrepareChart(*m_pcSR, m_uDepth);
        }
    }
}
