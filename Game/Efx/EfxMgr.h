#if !defined(EFX_MGR_H)
#define EFX_MGR_H
#include "EfxDef.h"

namespace CEffect {
    class CEffectInstance;
    struct SEffectGroupInfo;
}
class CEfxMgr
{
public:
    static BOOLEAN Initialize(VOID);
    static VOID Release(VOID);
    
    static CEffect::CEffectInstance* LoadEffect(EEFFECT_ID eID);
    static const CEffect::SEffectGroupInfo* GetEffectInfo(EEFFECT_ID eID);
};

#endif // #if !defined(EFX_MGR_H)
