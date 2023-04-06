#include "stdafx.h"
#include "EfxMgr.h"
#include "CEffectDefs.h"
#include "EfxDef.h"
#include "CEffectManager.h"
#include "CEffectInstance.h"
#include "AppResourceManager.h"

static BYTE* s_pbyEffBuffer = NULL;

BOOLEAN CEfxMgr::Initialize(VOID)
{
    Release(); // safety
    UINT uFileSize = 0;
    s_pbyEffBuffer = CAppResourceManager::LoadBinary(CAppResourceManager::EFILE_EFX_BIN, uFileSize);
    if (NULL == s_pbyEffBuffer) {
        ASSERT(FALSE);
        return FALSE;
    }
#if defined(DEBUG)
    const CEffect::SEffectFileHeader* psHeader = (CEffect::SEffectFileHeader*)s_pbyEffBuffer;
    TRACE("%d efx loaded\n", psHeader->uEffectNum);
    ASSERT(EEFFECT_ID_NUM == psHeader->uEffectNum);
#endif // #if defined(DEBUG)
    return TRUE;
}

VOID CEfxMgr::Release(VOID)
{
    if (NULL != s_pbyEffBuffer) {
        delete [] s_pbyEffBuffer;
        s_pbyEffBuffer = NULL;
    }
}

const CEffect::SEffectGroupInfo* CEfxMgr::GetEffectInfo(EEFFECT_ID eID)
{
    if (NULL == s_pbyEffBuffer) {
        ASSERT(FALSE);
        return NULL;
    }
    const CEffect::SEffectFileHeader* psHeader = (CEffect::SEffectFileHeader*)s_pbyEffBuffer;
    if ((UINT)eID >= psHeader->uEffectNum) {
        ASSERT(FALSE);
        return NULL;
    }
    const CEffect::SEffectGroupInfo& sGrp = ((const CEffect::SEffectGroupInfo*)(psHeader+1))[eID];
    return &sGrp;
}

CEffect::CEffectInstance* CEfxMgr::LoadEffect(EEFFECT_ID eID)
{
    const CEffect::SEffectGroupInfo* psGrp = GetEffectInfo(eID);
    if (NULL == psGrp) {
        ASSERT(FALSE);
        return NULL;
    }
    const CEffect::SEffectSource* psSources = (const CEffect::SEffectSource*)(s_pbyEffBuffer + psGrp->uOffset);
    const CEffect::SEffectParticle* psParticles = (const CEffect::SEffectParticle*)(psSources + psGrp->uSourceNum);
    const CEffect::SEffectEmitter* psEmitters = (const CEffect::SEffectEmitter*)(psParticles + psGrp->uParticleNum);
    CEffect::CEffectInstance* pcInst = new CEffect::CEffectInstance();
    if (NULL == pcInst) {
        ASSERT(FALSE);
        return NULL;
    }
    if (!pcInst->Initialize(psSources, psGrp->uSourceNum, psParticles, psGrp->uParticleNum, psEmitters, psGrp->uEmitterNum)) {
        ASSERT(FALSE);
        pcInst->Release();
        delete pcInst;
        return NULL;
    }
    return pcInst;
}
