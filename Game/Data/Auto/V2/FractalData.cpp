#include "stdafx.h"
#include "FractalData.h"

static SFractal s_asFractals[] = {
#include "FractalData.inc"
};
static const UINT s_uFractalNum = sizeof(s_asFractals)/sizeof(SFractal);
UINT CFractalDataMgr::GetNum(VOID)
{
    return s_uFractalNum;
}
    
SFractal& CFractalDataMgr::GetFractal(UINT uIndex)
{
    ASSERT(s_uFractalNum > uIndex);
    return s_asFractals[uIndex % s_uFractalNum];
}
