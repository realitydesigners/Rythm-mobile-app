#if !defined(FRACTAL_DATA_DEFS_H)
#define FRACTAL_DATA_DEFS_H
#include "MegaZZDef.h"
#define FRACTAL_NAME_LEN (32)

struct SFractal {
    CHAR  szName[FRACTAL_NAME_LEN];
    UINT  uBoxNum;
    FLOAT afPipSize[MEGAZZ_MAX_DEPTH];
};

#endif // #if !defined(FRACTAL_DATA_DEFS_H)
