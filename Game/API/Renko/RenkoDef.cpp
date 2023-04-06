#include "stdafx.h"
#include "RenkoDef.h"
#include "PlayerData.h"

VOID Renko::GetPriceRange(DOUBLE& dHigh, DOUBLE& dLow, const SShiftedRenko& sSR)
{
    // formula
    // const INT64 nllPriceIndex = (dPrice * sRenko.uMultiplier) / sRenko.uDivisor;
    dHigh = (DOUBLE)(sSR.nLowerIndex + sSR.byRankoNum + 1) / sSR.dDivisor;
     dLow = (DOUBLE)(sSR.nLowerIndex) / sSR.dDivisor;
}

DOUBLE Renko::GetPrice(INT64 nllPriceIndex, const SShiftedRenko& sSR)
{
    return (DOUBLE)(nllPriceIndex) / sSR.dDivisor;
}

VOID Renko::AdjustChannelSize(FLOAT fTopChannelSize, FLOAT* pfSizes, UINT uSizeNum)
{
    ASSERT(MAX_DEPTH == uSizeNum);
    const USHORT* pushRatios = CPlayerData::GetChannelRatios();
    const FLOAT fDivisor = (FLOAT)pushRatios[0];
    for (UINT uIndex = 0; MAX_DEPTH > uIndex; ++uIndex) {
        const FLOAT fRatio = pushRatios[uIndex] / fDivisor;
        pfSizes[uIndex] = fTopChannelSize * fRatio;
    }    
}

VOID Renko::GetChannelDisplayLbl(CHAR* szBuffer, UINT uBufferLen, UINT uDepth)
{
    ASSERT(8 > uDepth);
    const USHORT* pushRatios = CPlayerData::GetChannelRatios();
    snprintf(szBuffer, uBufferLen, "%d", pushRatios[uDepth]);
}

UINT Renko::GetChannelValue(UINT uDepth)
{
    ASSERT(9 > uDepth);
    const USHORT* pushRatios = CPlayerData::GetChannelRatios();
    return pushRatios[uDepth];
}

