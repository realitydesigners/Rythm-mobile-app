#if !defined(PATTERN_DATA_DEFS_H)
#define PATTERN_DATA_DEFS_H

#define PATTERN_NAME_LEN    (32)
#define PATTERN_UNUSED      (0xFF)

enum EPATTERN_MATCH {
    EPATTERN_MATCH_TOP=0,
    EPATTERN_MATCH_BOTTOM,
    EPATTERN_MATCH_NONE,
};
struct SPatternRow {
    BOOLEAN         bUp;            // up or dn
    BYTE            byDepthIndex;    // depth index
    BYTE            byWasLow;
    BYTE            byWasHigh;
    BYTE            byIsLow;
    BYTE            byIsHigh;
    EPATTERN_MATCH  eMatch;
    BYTE            byMatchTargetDepthIndex;
};
struct SPattern {
    CHAR szName[PATTERN_NAME_LEN];
    UINT uIndexOffset;
    UINT uPatternNum;
};

#endif // #if !defined(PATTERN_DATA_DEFS_H)
