#if !defined(RYTHM_DATA_H)
#define RYTHM_DATA_H

struct SRythmParams_old {
    CHAR    szTradePair[16]; // 16 bytes of trade pair string
    FLOAT   afSize[8];
    UINT    auBoxSize[8];
};
struct SRythmParams {
    CHAR    szTradePair[16]; // 16 bytes of trade pair string
    FLOAT   afSize[9];
    UINT    uPad;            // 16 bit alignment
};

class CRythmData
{
    friend class CSaveData;
public:
    static SRythmParams*        GetParams(const CHAR* szTradePair);
    static const SRythmParams*  NewParams(const CHAR* szTradePair);
    static VOID                 SetParams(const SRythmParams& sParams);
    static VOID                 ChangeBoxNum(UINT uBoxBum);
    static UINT                 GetBoxNum(VOID);
private:
    static VOID             NewGame(VOID);
    static BOOLEAN          LoadData(const BYTE* pbyBuffer);
    
    static UINT             GetSaveDataLength(VOID);
    static BOOLEAN          SaveData(BYTE* pbyBuffer, UINT uLength);
};

#endif // #if !defined(RYTHM_DATA_H)
