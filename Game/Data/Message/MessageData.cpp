#include "stdafx.h"
#include "MessageData.h"
#include "AppResourceManager.h"
#include "FontDefs.h"

static BYTE* s_pData = NULL;

static const CAppResourceManager::EFILE LANG_FILE[EFONT_LANGUAGE_NUM] = {
    CAppResourceManager::EFILE_LANGUAGE_ENGLISH,        //  EFONT_LANGUAGE_ENGLISH=0,
    CAppResourceManager::EFILE_LANGUAGE_ENGLISH,           //  EFONT_LANGUAGE_HANS,
    CAppResourceManager::EFILE_LANGUAGE_ENGLISH,           //  EFONT_LANGUAGE_HANT,
    CAppResourceManager::EFILE_LANGUAGE_ENGLISH,       //  EFONT_LANGUAGE_JP,
    CAppResourceManager::EFILE_LANGUAGE_ENGLISH,        //  EFONT_LANGUAGE_THAI
    CAppResourceManager::EFILE_LANGUAGE_ENGLISH,        //  EFONT_LANGUAGE_INDO
    
};

VOID CMessageData::Release(VOID)
{
    SAFE_DELETE_ARRAY(s_pData);
}

VOID CMessageData::OnChangeLanguage(UINT uLang)
{
    Release();
    UINT uFileSize = 0;
    s_pData = CAppResourceManager::LoadBinary(LANG_FILE[uLang], uFileSize);
    if (NULL == s_pData) {
        ASSERT(FALSE);
        return;
    }
    UINT* pNumMessage = (UINT*)s_pData;
    if (*pNumMessage != MSG_MAX) {
        TRACE("Incorrect number of messages for lang %d, expected %d read %d\n", uLang, MSG_MAX, *pNumMessage );
        ASSERT(FALSE);
        Release();
        return;
    }
    // converter writes int of 4 bytes
    ASSERT(sizeof(UINT) == 4);
    const UINT kBaseOffset = sizeof(UINT) + sizeof(UINT) * (*pNumMessage);
    UINT uIndex = 0;
    for ( ; MSG_MAX > uIndex; ++uIndex ) {
        pNumMessage[1 + uIndex] = (kBaseOffset + pNumMessage[1 + uIndex]);
    }
}

const CHAR* CMessageData::GetMsgID(INT nID)
{
    if (0 > nID || MSG_MAX <= nID) {
        ASSERT(FALSE);
        return "";
    }
    if (NULL == s_pData) {
        ASSERT(FALSE);
        return "";
    }
    UINT* paOffsets = (UINT*)(s_pData + 4);
    return (const CHAR *)(s_pData + paOffsets[nID]);
}
