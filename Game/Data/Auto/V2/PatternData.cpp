#include "stdafx.h"
#include "MegaZZ.h"
#include "PatternData.h"

static SPatternRow s_asRow[] = {
#include "PatternRow.inc"
};
static const UINT s_uRowNum = sizeof(s_asRow)/sizeof(SPatternRow);

static SPattern s_asPattern[] = {
#include "PatternIndex.inc"
};
static const UINT s_uNum = sizeof(s_asPattern)/sizeof(SPattern);

UINT CPatternDataMgr::GetPatternRowNum(VOID)
{
    return s_uRowNum;
}

SPatternRow* CPatternDataMgr::GetPatternRow(UINT uIndex)
{
    ASSERT(s_uRowNum > uIndex);
    return s_asRow + uIndex;
}

UINT CPatternDataMgr::GetPatternNum(VOID)
{
    return s_uNum;
}

SPattern& CPatternDataMgr::GetPattern(UINT uIndex)
{
    ASSERT(s_uNum > uIndex);
    return s_asPattern[uIndex];
}


BOOLEAN CPatternDataMgr::CanPassPattern(const CMegaZZ& cZZ, UINT uPatternIndex, BOOLEAN bFlip)
{
    if (s_uNum <= uPatternIndex) {
        ASSERT(FALSE);
        return FALSE;
    }
    const SPattern& sPattern = GetPattern(uPatternIndex);
    if ((sPattern.uIndexOffset + sPattern.uPatternNum) > s_uRowNum) {
        ASSERT(FALSE);
        return FALSE;
    }
    if (0 == sPattern.uPatternNum) {
        ASSERT(FALSE);
        return FALSE;
    }
    const SPatternRow* psRow = GetPatternRow(sPattern.uIndexOffset);
    for (UINT uIndex = 0; sPattern.uPatternNum > uIndex; ++uIndex) {
        const SPatternRow& sRow = psRow[uIndex];
        const MegaZZ::SChannel& sChannel = cZZ.GetCurrentChannel(sRow.byDepthIndex);
        const BOOLEAN bCheckUP = (bFlip ? !sRow.bUp : sRow.bUp);
        const BOOLEAN bChannelUp = (MegaZZ::EType_Up == sChannel.byType);
        if (bCheckUP != bChannelUp) {
            return FALSE;
        }
        if (PATTERN_UNUSED != sRow.byWasLow) {
            if (sChannel.byRetracementNumber < sRow.byWasLow) {
                return FALSE;
            }
        }
        if (PATTERN_UNUSED != sRow.byWasHigh) {
            if (sChannel.byRetracementNumber > sRow.byWasHigh) {
                return FALSE;
            }
        }
        // check is
        if (PATTERN_UNUSED != sRow.byIsLow) {
            if (sChannel.byDisplayNumber < sRow.byIsLow) {
                return FALSE;
            }
        }
        if (PATTERN_UNUSED != sRow.byIsHigh) {
            if (sChannel.byDisplayNumber > sRow.byIsHigh) {
                return FALSE;
            }
        }
        EPATTERN_MATCH eMatch = sRow.eMatch;
        if (bFlip) {
            if (EPATTERN_MATCH_BOTTOM == eMatch) {
                eMatch = EPATTERN_MATCH_TOP;
            }
            else if (EPATTERN_MATCH_TOP == eMatch) {
                eMatch = EPATTERN_MATCH_BOTTOM;
            }
        }
        switch (eMatch) {
            case EPATTERN_MATCH_NONE:
                break;
            case EPATTERN_MATCH_BOTTOM:
                {
                    const MegaZZ::SChannel& sTgtChannel = cZZ.GetCurrentChannel(sRow.byMatchTargetDepthIndex);
                    if (sTgtChannel.fBottom != sChannel.fBottom) {
                        return FALSE;
                    }
                }
                break;
            case EPATTERN_MATCH_TOP:
                {
                    const MegaZZ::SChannel& sTgtChannel = cZZ.GetCurrentChannel(sRow.byMatchTargetDepthIndex);
                    if (sTgtChannel.fTop != sChannel.fTop) {
                        return FALSE;
                    }
                }
                break;
            default:
                ASSERT(FALSE);
                return FALSE;
        }
    }
    return TRUE;
}

UINT CPatternDataMgr::GetSmallestDepth(UINT uPatternIndex)
{
    if (s_uNum <= uPatternIndex) {
        ASSERT(FALSE);
        return FALSE;
    }
    const SPattern& sPattern = GetPattern(uPatternIndex);
    if ((sPattern.uIndexOffset + sPattern.uPatternNum) > s_uRowNum) {
        ASSERT(FALSE);
        return FALSE;
    }
    if (0 == sPattern.uPatternNum) {
        ASSERT(FALSE);
        return FALSE;
    }
    UINT uBiggest = 0;
    const SPatternRow* psRow = GetPatternRow(sPattern.uIndexOffset);
    for (UINT uIndex = 0; sPattern.uPatternNum > uIndex; ++uIndex) {
        const SPatternRow& sRow = psRow[uIndex];
        if (uBiggest < sRow.byDepthIndex) {
            uBiggest = sRow.byDepthIndex;
        }
    }
    return uBiggest;
}
