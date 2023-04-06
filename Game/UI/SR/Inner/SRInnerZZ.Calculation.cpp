#include "stdafx.h"
#include "EventManager.h"
#include "HLTime.h"
#include "MegaZZ.h"
#include "SRGraphDef.h"
#include "SRInnerZZ.h"
#include <list>
#include <math.h>

BOOLEAN CSRInnerZZ::ResizeWorkBuffer(UINT uNum)
{
    if (m_uZZValueNum < uNum) {
        SAFE_DELETE_ARRAY(m_ppsZZFloatValueList);
        SAFE_DELETE_ARRAY(m_psZZFloatValues);
        m_ppsZZFloatValueList = new ZZUtil::SFloatValue*[uNum];
        m_psZZFloatValues = new ZZUtil::SFloatValue[uNum];
        if (NULL == m_ppsZZFloatValueList || NULL == m_psZZFloatValues) {
            ASSERT(FALSE);
            return FALSE;
        }
        m_uZZValueNum = uNum;
    }
    return TRUE;
}

BOOLEAN CSRInnerZZ::GenerateZZ(UINT uNum, const CMegaZZ& cMegaZZ, UINT uDepth)
{
    memset(m_psZZFloatValues, 0,      sizeof(ZZUtil::SFloatValue) * uNum);
    ASSERT(uNum == cMegaZZ.GetFrameUsedNum());
    const UINT uStartIndex = (cMegaZZ.GetFrameIndex() + MEGAZZ_MAX_HISTORY - (uNum - 1)) % MEGAZZ_MAX_HISTORY;
    const MegaZZ::SFrame* psFrames = cMegaZZ.GetFrames();
    const MegaZZ::SChannel* psChannels = cMegaZZ.GetChannels(uDepth);
    for (UINT uIndex = 0; uNum > uIndex; ++uIndex) {
        const UINT uActualIndex = (uStartIndex + uIndex) % MEGAZZ_MAX_HISTORY;
        ZZUtil::SFloatValue& sValue = m_psZZFloatValues[uIndex];
        sValue.fValue = psFrames[uActualIndex].fPrice;
        sValue.ushActualIndex = uActualIndex;
        sValue.ushIndex = uIndex;
        sValue.byFrameType = psChannels[uActualIndex].byType;
        sValue.byZZIndicator = 0xFF; // not set
    }
    // now we do the ZZ line generation
    const UINT uTotalZZLevels = ZZUtil::GetZZLevelNum();
    const FLOAT fChannelSize = cMegaZZ.GetChannelSize(uDepth);
    for (UINT uZZLevel = 0; uTotalZZLevels > uZZLevel; ++uZZLevel) {
        const FLOAT fDeviation = ZZUtil::GetInfo(uZZLevel).fDeviationPercent * 0.01f * fChannelSize;
        ZZUtil::GenerateZZ(m_ppsZZFloatValueList, m_uZZValueNum, m_psZZFloatValues, uNum, fDeviation, uZZLevel);
    }
    return TRUE;
}
VOID CSRInnerZZ::PrepareChart(const CMegaZZ& cMegaZZ, UINT uDepth)
{
    ClearTimeLines();
    m_pcMegaZZ = &cMegaZZ;
    m_uDepth = uDepth;
    m_uUsedArea = 0;
    m_uUsedZZ = 0;
    m_uUsedLine = 0;
    UpdateIndicator(cMegaZZ.GetChannelSize(uDepth));
    const UINT uTotalFrameNum = cMegaZZ.GetFrameUsedNum();
    if (5 > uTotalFrameNum) { // not enough frames, skip
        m_fXOffset = 0.0f;
        return;
    }
    // resize working buffer if needed
    if (!ResizeWorkBuffer(uTotalFrameNum)) {
        ASSERT(FALSE);
        return;
    }
    ASSERT(NULL != m_psZZFloatValues);
    ASSERT(NULL != m_ppsZZFloatValueList);
    ASSERT(uTotalFrameNum <= m_uZZValueNum);

    if (!GenerateZZ(uTotalFrameNum, cMegaZZ, uDepth)) {
        ASSERT(FALSE);
        return;
    }
    // generate final zz list
    UINT uZZNum = 0;
    for (UINT uIndex = 0; uTotalFrameNum > uIndex; ++uIndex) {
        ZZUtil::SFloatValue& sZZ = m_psZZFloatValues[uIndex];
        ASSERT(uTotalFrameNum > sZZ.ushActualIndex);
        ASSERT(uIndex == sZZ.ushIndex);
        const BOOLEAN bInsert = (0xFF != sZZ.byZZIndicator || uIndex == (uTotalFrameNum - 1));
        if (bInsert) {
            ASSERT(m_uZZValueNum > uZZNum);
            m_ppsZZFloatValueList[uZZNum] = &sZZ;
            ++uZZNum;
        }
    }
    if (0 == uZZNum) { // no zz points, skip
        return;
    }
    const SHLVector2D& sSize = GetLocalSize();
    const FLOAT fTopY = m_bHideUI ? SR_INNER_ZZ_PADDING : (SR_INNER_ZZ_HEIGHT_OFFSET + SR_INNER_ZZ_PADDING);
    const FLOAT fBottomY = sSize.y - SR_INNER_ZZ_PADDING;
    const FLOAT fHeight = (fBottomY - fTopY);
    const FLOAT fPixelPerPrice = fHeight / cMegaZZ.GetChannelSize(uDepth);
    const MegaZZ::SChannel* psChannels = cMegaZZ.GetChannels(uDepth);
    // compute total width
    FLOAT fWidth = 0.0f;
    const ZZUtil::SFloatValue* psPrev = NULL;
    FLOAT fPrevY = 0.0f;
    for (UINT uIndex = 0; uZZNum > uIndex; ++uIndex) { // for each zz
        const ZZUtil::SFloatValue* psCurrent = m_ppsZZFloatValueList[uIndex];
        ASSERT(uTotalFrameNum > psCurrent->ushActualIndex);
        const MegaZZ::SChannel& sChannel = psChannels[psCurrent->ushActualIndex];
        ASSERT(psCurrent->fValue >= sChannel.fBottom);
        const FLOAT fDiffPrice = psCurrent->fValue - sChannel.fBottom;
        const FLOAT fY = fDiffPrice * fPixelPerPrice;
        if (NULL == psPrev) {
            psPrev = psCurrent;
            fPrevY = fY;
            continue;
        }
        fWidth += abs(fY - fPrevY);
        fPrevY = fY;
        psPrev = psCurrent;
    }
    const FLOAT fEndX = sSize.x - 20.0f;
    const FLOAT fOffsetX = fEndX - fWidth;
    const UINT uNowSec = CHLTime::GetTimeSecs();
    const FLOAT fMidY = GetLocalSize().y * 0.5f;
    const BOOLEAN bShowTimeline = TRUE;//CPlayerData::IsTimelineEnabled();
    const MegaZZ::SFrame* psFrames = cMegaZZ.GetFrames();
    FLOAT fPrevX = fOffsetX + m_fXOffset;
    psPrev = NULL;
    fPrevY = 0.0f;
    for (UINT uIndex = 0; uZZNum > uIndex; ++uIndex) { // for each zz
        const ZZUtil::SFloatValue* psCurrent = m_ppsZZFloatValueList[uIndex];
        ASSERT(uTotalFrameNum > psCurrent->ushActualIndex);
        const MegaZZ::SChannel& sChannel = psChannels[psCurrent->ushActualIndex];
        ASSERT(psCurrent->fValue >= sChannel.fBottom);
        const FLOAT fDiffPrice = psCurrent->fValue - sChannel.fBottom;
        const FLOAT fY = fBottomY - (fDiffPrice * fPixelPerPrice);
        if (NULL == psPrev) {
            psPrev = psCurrent;
            fPrevY = fY;
            continue;
        }
        const FLOAT fMoveX = abs(fY - fPrevY);
        const FLOAT fX = fPrevX + fMoveX;
        if (0.0f < fX) {
            const FLOAT fBaseY = (psPrev->byFrameType == MegaZZ::EType_Up) ? fTopY : fBottomY;
            AddArea(fPrevX, fPrevY, fX, fY, fBaseY, SR_DOWN_COLOR_1);
            if (0xFF != psCurrent->byZZIndicator && sSize.x > fX) {
                const ZZUtil::SInfo& sZZInfo = ZZUtil::GetInfo(psCurrent->byZZIndicator);
                AddZZ(fX, fY, sZZInfo.fThickness, psCurrent->byZZIndicator, sZZInfo.uColor);
                if (bShowTimeline) {
                    const UINT uTimeDiff = uNowSec - psFrames[psCurrent->ushActualIndex].uUpdateTime;
                    if (5 < uTimeDiff) {
                        FLOAT fTimeY;
                        if (fMidY < fY) {
                            fTimeY = fTopY + 16.0f;
                        }
                        else {
                            fTimeY = fBottomY - 20.0f;
                        }
                        if (AddTimeLine(fX, fTimeY, uTimeDiff)) {
                            AddLine(fX, fTimeY, fX, fY, 0xFFFFFFFF, TRUE);
                        }
                    }
                }
            }
        }
        fPrevX = fX;
        fPrevY = fY;
        psPrev = psCurrent;
        if (fX >= sSize.x) { // reach end of screen
            break;
        }
    }
}
