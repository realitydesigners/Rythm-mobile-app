#if !defined(PATTERN_DATA_H)
#define PATTERN_DATA_H
#include "PatternDataDefs.h"

class CMegaZZ;
class CPatternDataMgr {
public:
    static UINT GetPatternRowNum(VOID);
    static SPatternRow* GetPatternRow(UINT uIndex);
    
    static UINT GetPatternNum(VOID);
    static SPattern& GetPattern(UINT uIndex);
    
    static BOOLEAN CanPassPattern(const CMegaZZ& cZZ, UINT uPatternIndex, BOOLEAN bFlip);
    static UINT GetSmallestDepth(UINT uPatternIndex);
};

#endif // #if !defined(PATTERN_DATA_H)
