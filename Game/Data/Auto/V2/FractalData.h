#if !defined(FRACTAL_DATA_H)
#define FRACTAL_DATA_H
#include "FractalDataDefs.h"

class CFractalDataMgr {
public:
    static UINT GetNum(VOID);
    static SFractal& GetFractal(UINT uIndex);
};

#endif // #if !defined(FRACTAL_DATA_H)
