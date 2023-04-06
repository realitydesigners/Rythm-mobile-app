#include "stdafx.h"
#include "MegaZZ.h"
#include "PlayerData.h"

CMegaZZ::CMegaZZ() :
m_eSource(ESOURCE_MARKET_NUM),
m_uTradePairIndex(0xFFFFFFFF),
m_uBoxNum(0),
m_uUsedFrameNum(0),
m_uFrameIndex(0),
m_bZZGenerated(FALSE),
m_uZZFrameNum(0),
m_fPrevPrice(0.0f),
m_fCurrentPrice(0.0f),
m_uLastUpdateTime(0),
m_ullLastUpdateTimeMillisec(0),
m_nPrevPriceIndex(-1)
{
    memset(m_afSize, 0, sizeof(m_afSize));
    memset(m_afDivisor, 0, sizeof(m_afDivisor));
    memset(m_afDisplayDivisor, 0, sizeof(m_afDisplayDivisor));
    memset(m_abyPrevChannelIndex, 0, sizeof(m_abyPrevChannelIndex));
}
CMegaZZ::~CMegaZZ()
{
}

VOID CMegaZZ::SetSource(ESOURCE_MARKET eSource, UINT uTradePairIndex)
{
    m_eSource = eSource;
    m_uTradePairIndex = uTradePairIndex;
}

BOOLEAN CMegaZZ::Initlialize(UINT uDepth, UINT uBoxNum, const FLOAT* pfSizes)
{
    if (MEGAZZ_MAX_DEPTH != uDepth) {
        ASSERT(FALSE);
        return FALSE;
    }
    m_uBoxNum = uBoxNum;
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        m_afSize[uIndex] = pfSizes[uIndex];
        FLOAT fSize = pfSizes[uIndex] / m_uBoxNum;
        if (0.0f == fSize) {
            TRACE("Warn: we have zero size for depth %d!\n", uIndex);
            fSize = 1.0f;
        }
        m_afDivisor[uIndex] = 1.0f / fSize;
        m_afDisplayDivisor[uIndex] = 10.0f / pfSizes[uIndex];
    }
    Clear();
    return TRUE;
}

VOID CMegaZZ::Release(VOID)
{
    Clear();
}

VOID CMegaZZ::Clear(VOID)
{
    m_uUsedFrameNum = 0;
    m_uFrameIndex = 0;
    m_bZZGenerated = FALSE;
    m_uZZFrameNum = 0;
    m_fPrevPrice = 0.0f;
    m_fCurrentPrice = 0.0f;
    m_uLastUpdateTime = 0;
    m_ullLastUpdateTimeMillisec = 0;
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        MegaZZ::SChannel& sCurrentChannel = m_asChannels[uIndex][0];
        sCurrentChannel.fBottom = 0.0f;
        sCurrentChannel.byType = MegaZZ::EType_Down;
        sCurrentChannel.byDisplayNumber = 0;
    }
}

VOID CMegaZZ::EndUpdate(VOID)
{
    m_bZZGenerated = FALSE; // reset ZZ generated
    m_uZZFrameNum = 0;
}

VOID CMegaZZ::SetupFirstPrice(UINT64 ullTimeMilliSec, FLOAT fPrice, BOOLEAN bUp)
{
    const UINT uTimeSec = (UINT)(ullTimeMilliSec / 1000);
    ASSERT(0 == m_uFrameIndex);
    ASSERT(0 == m_uUsedFrameNum);
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        MegaZZ::SChannel& sChannel = m_asChannels[uIndex][0];
        sChannel.byType = bUp ? MegaZZ::EType_Up : MegaZZ::EType_Down;
        sChannel.byDisplayNumber = 0;               // since channel is sticking to the price, this display number is 0
        sChannel.byRetracementNumber = 0;           // same
        sChannel.byRetracementPercent = 0;
        sChannel.uRetracementUpdateTime = uTimeSec;
        m_abyPrevChannelIndex[uIndex] = 0;
        const FLOAT fSize = m_afSize[uIndex];
        if (bUp) {
            sChannel.fTop = fPrice;
            sChannel.fBottom = fPrice - fSize;
        }
        else {
            sChannel.fTop = fPrice + fSize;
            sChannel.fBottom = fPrice;
        }
    }
    m_fCurrentPrice = fPrice;
    m_uLastUpdateTime = uTimeSec;
    m_ullLastUpdateTimeMillisec = ullTimeMilliSec;
    
    // update frame with smallest channel data
    const MegaZZ::SChannel& sChannel = m_asChannels[MEGAZZ_MAX_DEPTH-1][0];
    MegaZZ::SFrame& sCurrentFrame = m_asFrames[0];
    sCurrentFrame.fPrice = fPrice;
    sCurrentFrame.uUpdateTime = uTimeSec;
    sCurrentFrame.byMovementType = sChannel.byType;
    ++m_uFrameIndex;
    ++m_uUsedFrameNum;
    // do a frame copy for both channel and frame (to be used for next update)
    for (UINT uDepthIndex = 0; MEGAZZ_MAX_DEPTH > uDepthIndex; ++uDepthIndex) {
        m_asChannels[uDepthIndex][1] = m_asChannels[uDepthIndex][0];
    }
    m_asFrames[1] = sCurrentFrame;
    m_asFrames[1].byCopy = TRUE;
    ++m_uUsedFrameNum; // add 1 more used frame
}


VOID CMegaZZ::InitAllChannels(UINT uTimeSec)
{
    ASSERT(0 == m_uFrameIndex);
    ASSERT(0 == m_uUsedFrameNum);
    const BOOLEAN bUp = m_fPrevPrice < m_fCurrentPrice;
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        MegaZZ::SChannel& sChannel = m_asChannels[uIndex][0];
        sChannel.byType = bUp ? MegaZZ::EType_Up : MegaZZ::EType_Down;
        sChannel.byDisplayNumber = 0;               // since channel is sticking to the price, this display number is 0
        sChannel.byRetracementNumber = 0;           // same
        sChannel.byRetracementPercent = 0;
        sChannel.uRetracementUpdateTime = uTimeSec;
        m_abyPrevChannelIndex[uIndex] = 0;
        const FLOAT fSize = m_afSize[uIndex];
        if (bUp) {
            sChannel.fTop = m_fCurrentPrice;
            sChannel.fBottom = m_fCurrentPrice - fSize;
        }
        else {
            sChannel.fTop = m_fCurrentPrice + fSize;
            sChannel.fBottom = m_fCurrentPrice;
        }
    }
}
VOID CMegaZZ::UpdateAllChannels(UINT uFrameIndex, UINT uTimeSec, BOOLEAN bCopyFromPrev)
{
    UINT uPrevIndex;
    if (0 == uFrameIndex) {
        ASSERT(MEGAZZ_MAX_HISTORY == m_uUsedFrameNum);
        uPrevIndex = MEGAZZ_MAX_HISTORY - 1;
    }
    else {
        uPrevIndex = uFrameIndex - 1;
    }
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        MegaZZ::SChannel& sChannel = m_asChannels[uIndex][uFrameIndex];
        if (bCopyFromPrev) {
            const MegaZZ::SChannel& sPrev = m_asChannels[uIndex][uPrevIndex];
            sChannel = sPrev;
        }
        if (sChannel.fBottom > m_fCurrentPrice) { // if current price is lower
            sChannel.fTop = m_fCurrentPrice + m_afSize[uIndex];
            sChannel.fBottom = m_fCurrentPrice;
            sChannel.byDisplayNumber = 0;
            sChannel.byType = MegaZZ::EType_Down;
            sChannel.byRetracementNumber = 0;
            sChannel.byRetracementPercent = 0;
            sChannel.uRetracementUpdateTime = uTimeSec;
            m_abyPrevChannelIndex[uIndex] = 0;
        }
        else if (sChannel.fTop < m_fCurrentPrice) { // if current price is higher
            sChannel.fTop = m_fCurrentPrice;
            sChannel.fBottom = m_fCurrentPrice - m_afSize[uIndex];
            sChannel.byDisplayNumber = 0;
            sChannel.byType = MegaZZ::EType_Up;
            sChannel.byRetracementNumber = 0;
            sChannel.byRetracementPercent = 0;
            sChannel.uRetracementUpdateTime = uTimeSec;
            m_abyPrevChannelIndex[uIndex] = 0;
        }
        else {
            // just update display number
            FLOAT fDiffPrice;
            if (MegaZZ::EType_Up == sChannel.byType) {
                fDiffPrice = sChannel.fTop - m_fCurrentPrice;
            }
            else {
                fDiffPrice = m_fCurrentPrice - sChannel.fBottom;;
            }
            
            if (0.0f > fDiffPrice) {
                fDiffPrice = 0.0f;
            }
            else if (m_afSize[uIndex] < fDiffPrice) {
                fDiffPrice = m_afSize[uIndex];
            }
            UINT uBoxIndex = (UINT)(fDiffPrice * m_afDisplayDivisor[uIndex]);
            if (9 < uBoxIndex) {
                uBoxIndex = 9;
            }
            if (sChannel.byDisplayNumber != uBoxIndex) {
                m_abyPrevChannelIndex[uIndex] = sChannel.byDisplayNumber;
                sChannel.byDisplayNumber = (BYTE)uBoxIndex;
                if (sChannel.byRetracementNumber < sChannel.byDisplayNumber) {
                    sChannel.byRetracementNumber = sChannel.byDisplayNumber;
                    sChannel.uRetracementUpdateTime = uTimeSec;
                }
            }
            // compute retracement %
            BYTE byPercent = (BYTE)(fDiffPrice * 100 / m_afSize[uIndex]);
            if (100 == byPercent) {
                byPercent = 99;
            }
            if (sChannel.byRetracementPercent < byPercent) {
                sChannel.byRetracementPercent = byPercent;
            }
        }
    }
}
VOID CMegaZZ::UpdatePrice(UINT64 ullTimeMilliSec, FLOAT fPrice)
{
    const UINT uTimeSec = (UINT)(ullTimeMilliSec / 1000);
    if (m_fCurrentPrice == fPrice) { // same price, skip
        m_uLastUpdateTime = uTimeSec;
        m_ullLastUpdateTimeMillisec = ullTimeMilliSec;
        return;
    }
    m_fPrevPrice = m_fCurrentPrice;
    m_fCurrentPrice = fPrice;
    m_uLastUpdateTime = uTimeSec;
    m_ullLastUpdateTimeMillisec = ullTimeMilliSec;
    if (0.0f == m_fPrevPrice) { // skip
        return;
    }
    
    INT nCurrentPriceIndex = 0;
    {
        const INT64 nllPriceIndex = (INT64)((DOUBLE)fPrice * (DOUBLE)m_afDivisor[MEGAZZ_MAX_DEPTH-1]);
        if (0x7FFFFFFF <= nllPriceIndex || 0 > nllPriceIndex) {
            TRACE("Out of Bounds!\n");
            return;
        }
        nCurrentPriceIndex = (INT)nllPriceIndex;
    }
    if (m_nPrevPriceIndex == nCurrentPriceIndex) { // skip if same index.
        return;
    }
    m_nPrevPriceIndex = nCurrentPriceIndex;
    
    if (0 == m_uUsedFrameNum) { // if first frame
        ASSERT(0 == m_uFrameIndex);
        // initialize all channel data
        InitAllChannels(uTimeSec);
        // update frame with smallest channel data
        const MegaZZ::SChannel& sChannel = m_asChannels[MEGAZZ_MAX_DEPTH-1][0];
        MegaZZ::SFrame& sCurrentFrame = m_asFrames[0];
        sCurrentFrame.fPrice = fPrice;
        sCurrentFrame.uUpdateTime = uTimeSec;
        sCurrentFrame.byMovementType = sChannel.byType;
        ++m_uFrameIndex;
        ++m_uUsedFrameNum;
        // do a frame copy for both channel and frame (to be used for next update)
        for (UINT uDepthIndex = 0; MEGAZZ_MAX_DEPTH > uDepthIndex; ++uDepthIndex) {
            m_asChannels[uDepthIndex][1] = m_asChannels[uDepthIndex][0];
        }
        m_asFrames[1] = sCurrentFrame;
        m_asFrames[1].byCopy = TRUE;
        ++m_uUsedFrameNum; // add 1 more used frame 
        return;
    }
    ASSERT(0 < m_uUsedFrameNum);
    MegaZZ::SFrame& sFrame = m_asFrames[m_uFrameIndex];
    const BYTE byNowType = (sFrame.fPrice < fPrice) ? MegaZZ::EType_Up : MegaZZ::EType_Down;
    if (byNowType == sFrame.byMovementType) {
        sFrame.fPrice = fPrice;
        sFrame.uUpdateTime = uTimeSec;
        sFrame.byCopy = FALSE;
        UpdateAllChannels(m_uFrameIndex, uTimeSec, FALSE);
        return;
    }
    if (!sFrame.byCopy) { // if current frame is not a copy
        ++m_uFrameIndex;
        if (MEGAZZ_MAX_HISTORY == m_uFrameIndex) {
            m_uFrameIndex = 0;
        }
        if (MEGAZZ_MAX_HISTORY > m_uUsedFrameNum) {
            ++m_uUsedFrameNum;
        }
    }
    MegaZZ::SFrame& sUpdatedFrame = m_asFrames[m_uFrameIndex];
    sUpdatedFrame.fPrice = fPrice;
    sUpdatedFrame.uUpdateTime = uTimeSec;
    sUpdatedFrame.byMovementType = byNowType;
    sUpdatedFrame.byCopy = FALSE;
    UpdateAllChannels(m_uFrameIndex, uTimeSec, TRUE);
}

UINT CMegaZZ::GetOldestFrameIndex(VOID) const
{
    if (MEGAZZ_MAX_HISTORY > m_uUsedFrameNum) { // if havent maxed out
        return 0;
    }
    UINT uNextIndex = m_uFrameIndex + 1;
    if (MEGAZZ_MAX_HISTORY == uNextIndex) {
        uNextIndex = 0;
    }
    return uNextIndex;
}
const MegaZZ::SChannel& CMegaZZ::GetOldestChannel(UINT uDepthIndex) const
{
    return m_asChannels[uDepthIndex][GetOldestFrameIndex()];
}

UINT CMegaZZ::GenerateMegaZZ(ZZUtil::SFloatValue** ppsList, UINT uListSize)
{
    if (5 > m_uUsedFrameNum) { // less than 5 frames, skip
        return 0;
    }
    const UINT uSmallestDepthIndex = MEGAZZ_MAX_DEPTH - 1;
    // compute all the zz levels
    const FLOAT fSmallestSize = GetChannelSize(uSmallestDepthIndex);
    FLOAT afDeviations[MEGAZZ_MAX_DEPTH+3]; // add smallest depth, and 3 more level from sema settings
    const FLOAT fBiggestSema = CPlayerData::GetSemaPercent(3);
    const USHORT* psuChannelRatios = CPlayerData::GetChannelRatios();
    FLOAT fChannelRatio;
    if (0 < psuChannelRatios[0]) {
        fChannelRatio = (FLOAT)psuChannelRatios[1] / (FLOAT)psuChannelRatios[0];
    }
    else {
        fChannelRatio = 89.0f/144.0f;
    }
    afDeviations[2] = fBiggestSema * fChannelRatio;
    afDeviations[1] = afDeviations[2] * fChannelRatio;
    afDeviations[0] = afDeviations[1] * fChannelRatio;
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        ASSERT(uSmallestDepthIndex >= uIndex);
        const UINT uCurrentDepthIndex = uSmallestDepthIndex - uIndex;
        afDeviations[uIndex+3] = fBiggestSema * GetChannelSize(uCurrentDepthIndex) / fSmallestSize;
    }
    const UINT uStartIndex = GetOldestFrameIndex();
    UINT uActualIndex = uStartIndex;
    for (UINT uIndex = 0; m_uUsedFrameNum > uIndex; ++uIndex) { // for each frame
        ASSERT(0xFFFF > uActualIndex);
        const MegaZZ::SFrame& sFrame = m_asFrames[uActualIndex];
        ZZUtil::SFloatValue& sZZ = m_asZZs[uIndex];
        sZZ.ushActualIndex = (USHORT)uActualIndex;
        sZZ.fValue = sFrame.fPrice;
        sZZ.byFrameType = 0; // not in use
        sZZ.byZZIndicator = 0xFF; // not set
        ++uActualIndex;
        ASSERT(MEGAZZ_MAX_HISTORY >= uActualIndex);
        if (MEGAZZ_MAX_HISTORY == uActualIndex) { // to revent using % as it is heavy operation
            ASSERT(MEGAZZ_MAX_HISTORY == m_uUsedFrameNum);
            uActualIndex = 0;
        }
    }
    for (UINT uIndex = 0; (MEGAZZ_MAX_DEPTH+3) > uIndex; ++uIndex) {    // for each zz level
        const FLOAT fDeviation = afDeviations[uIndex] * 0.01f * fSmallestSize;
        ZZUtil::GenerateZZ(ppsList, uListSize, m_asZZs, m_uUsedFrameNum, fDeviation, uIndex);
    }
    // now we pass the zz back to the caller thru the list
    UINT uAddedNum = 0;
    for (UINT uIndex = 0; m_uUsedFrameNum > uIndex; ++uIndex) { // for each zz
        ZZUtil::SFloatValue& sZZ = m_asZZs[uIndex];
        if ((sZZ.byZZIndicator != 0xFF) || ((m_uUsedFrameNum - 1) == uIndex)) {
            if (uListSize == uAddedNum) { // already maxed out
                return uAddedNum;
            }
            ppsList[uAddedNum] = &sZZ;
            ++uAddedNum;
        }
    }
    return uAddedNum;
}

#include "BinanceMgr.h"
#include "OandaMgr.h"
const CHAR* CMegaZZ::GetName(VOID) const
{
    if (ESOURCE_MARKET_OANDA == m_eSource) {
        const COandaMgr::SOandaPair* psPair = COandaMgr::GetInstance().GetTradePair(m_uTradePairIndex);
        if (NULL == psPair) {
            return "Unknown";
        }
        return psPair->szTradePair;
    }
    if (ESOURCE_MARKET_BINANCE == m_eSource) {
        const STradePair* psPair = CBinanceMgr::GetInstance().GetTradePair(m_uTradePairIndex);
        if (NULL == psPair) {
            return "Unknown";
        }
        return psPair->szBase;
    }
    return "Unknown source";
}

