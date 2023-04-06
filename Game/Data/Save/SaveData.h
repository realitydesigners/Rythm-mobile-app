#if !defined(_SAVE_DATA_H_)
#define _SAVE_DATA_H_

class CSaveData
{
public:
    static VOID CreateNewGame(VOID);
	static BOOLEAN LoadData(VOID);
    static BOOLEAN LoadData(const BYTE * pbyFileBuffer, UINT uFileSize);
    static BOOLEAN SaveData(BOOLEAN bOnlySaveIfDirty = TRUE);
    static BOOLEAN GetSaveData(BYTE ** pbyOutBuffer, UINT * puOutSize);

    static BOOLEAN IsSavable(VOID);

    static BOOLEAN IsSaveDataDirty(VOID);
    static VOID SetSaveDataDirty(VOID);
    static VOID SetSavable(BOOLEAN bSavable);
};

#endif // #if !defined(_SAVE_DATA_H_)
