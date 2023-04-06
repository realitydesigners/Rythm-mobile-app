#include "stdafx.h"
#include "AutoPtr.h"
#include "RenkoDef.h"
#include "PlayerData.h"
#include "ZZUtil.h"
#include <math.h>

static ZZUtil::SInfo ZZ_LEVELS[] = {
    { RGBA(255,   0,   0, 0xFF),  5,  4.0f  },
    { RGBA(255, 100, 255, 0xFF),  21, 6.0f  },
    { RGBA(105, 145, 255, 0xFF),  34, 8.0f  },
    { RGBA(0,   255, 125, 0xFF),  55, 10.0f  },
};
static const UINT ZZ_LEVEL_NUM = sizeof(ZZ_LEVELS)/sizeof(ZZUtil::SInfo);

VOID ZZUtil::LoadZZInfo(VOID)
{
    for (UINT uIndex = 0; ZZ_LEVEL_NUM > uIndex; ++uIndex) {
        ZZUtil::SInfo& sInfo = ZZ_LEVELS[uIndex];
        sInfo.fDeviationPercent = CPlayerData::GetSemaPercent(uIndex);
        sInfo.uColor = CPlayerData::GetSemaColor(uIndex);
    }
}
UINT ZZUtil::GetZZLevelNum(VOID)
{
    return ZZ_LEVEL_NUM;
}

const ZZUtil::SInfo& ZZUtil::GetInfo(UINT uLevel)
{
    if (ZZ_LEVEL_NUM <= uLevel) {
        ASSERT(FALSE);
        return ZZ_LEVELS[0];
    }
    return ZZ_LEVELS[uLevel];
}
