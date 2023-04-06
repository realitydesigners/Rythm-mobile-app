#if !defined(AUTO_LOGIC_DATA_H)
#define AUTO_LOGIC_DATA_H
#include "AutoLogicDefs.h"
class CMegaZZ;
class CAutoLogicDataMgr {
public:
    static UINT GetTPEntryNum(VOID);
    static const STPEntry& GetTPEntry(UINT uIndex);
    
    static UINT GetLogicNum(VOID);
    static const SLogic& GetLogic(UINT uIndex);
    static const CHAR* GetLogicName(UINT uIndex);
    
    static UINT GetAlertNum(VOID);
    static const UINT* GetAlertPatterns(VOID);
};

#endif // #if !defined(AUTO_LOGIC_DATA_H)
