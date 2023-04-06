#if !defined(RENKO_H)
#define RENKO_H
#include "RenkoDef.h"

class CRenko {
public:
    CRenko();
    ~CRenko();
    
    BOOLEAN Initlialize(DOUBLE dPipSize);
    VOID Release(VOID);
    
    DOUBLE GetPipSize(VOID) const { return m_dPipSize; }
    const Renko::SRenko& GetRenko(VOID) const { return m_sRenko; }
    
    VOID Update(DOUBLE dPrice);
private:
    DOUBLE          m_dPipSize;
    UINT            m_uMaxHistoryNum;
    Renko::SRenko   m_sRenko;
};
#endif // #if !defined(RENKO_H)

