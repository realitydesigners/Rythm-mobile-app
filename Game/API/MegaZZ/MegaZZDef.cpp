#include "stdafx.h"
#include "MegaZZDef.h"
#include "PlayerData.h"

DOUBLE MegaZZ::GetPrice(INT64 nllPriceIndex, DOUBLE dDivisor)
{
    return (DOUBLE)(nllPriceIndex) / dDivisor;
}

VOID MegaZZ::AdjustChannelSize(FLOAT fTopChannelSize, FLOAT* pfSizes, UINT uSizeNum)
{
    ASSERT(MEGAZZ_MAX_DEPTH == uSizeNum);
    const USHORT* pushRatios = CPlayerData::GetChannelRatios();
    const FLOAT fDivisor = (FLOAT)pushRatios[0];
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        const FLOAT fRatio = pushRatios[uIndex] / fDivisor;
        pfSizes[uIndex] = fTopChannelSize * fRatio;
    }    
}

VOID MegaZZ::GetChannelDisplayLbl(CHAR* szBuffer, UINT uBufferLen, UINT uDepth)
{
    ASSERT(8 > uDepth);
    const USHORT* pushRatios = CPlayerData::GetChannelRatios();
    snprintf(szBuffer, uBufferLen, "%d", pushRatios[uDepth]);
}

UINT MegaZZ::GetChannelValue(UINT uDepth)
{
    ASSERT(9 > uDepth);
    const USHORT* pushRatios = CPlayerData::GetChannelRatios();
    return pushRatios[uDepth];
}
