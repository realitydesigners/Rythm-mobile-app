#include "stdafx.h"
#include "SoundManager.h"

static const INT s_anCacheID[] = {
    SE_SFX_PING,
};
const INT* CSoundManager::GetDefaultCachedSEID(VOID)
{
    return s_anCacheID;
}

UINT CSoundManager::GetDefaultCachedSEIDNum(VOID)
{
    return sizeof(s_anCacheID)/sizeof(INT);
}

