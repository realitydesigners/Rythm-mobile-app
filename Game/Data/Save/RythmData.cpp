#include "stdafx.h"
#include "RythmData.h"

#include <list>

#define RYTHM_DATA_ID           (0x12345678)
#define RYTHM_DATA_VERSION_OLD  (1)
#define RYTHM_DATA_VERSION      (2)

struct SRythmHeader {
    UINT uID;
    UINT uVersion;
    UINT uNum;
    UINT uBoxNum;
};
typedef std::list<SRythmParams> CSRythmParamList;

static CSRythmParamList s_cParamList;
static UINT             s_uBoxNum = 50;
static const FLOAT      s_afDefaultSize[9]      = { 1.6f, 1.4f, 1.2f, 1.0f, 0.8f, 0.6f, 0.4f, 0.2f, 0.1f };

const SRythmParams* CRythmData::NewParams(const CHAR* szTradePair)
{
    SRythmParams* psOld = GetParams(szTradePair);
    if (NULL != psOld) {
        ASSERT(FALSE);
        return psOld;
    }
    SRythmParams sTmp;
    ASSERT(sizeof(sTmp.szTradePair) == 16);
    strncpy(sTmp.szTradePair, szTradePair, sizeof(sTmp.szTradePair));
    for (UINT uIndex = 0; 9 > uIndex; ++uIndex) {
        sTmp.afSize[uIndex] = s_afDefaultSize[uIndex];
    }
    s_cParamList.push_back(sTmp);
    return &(*(s_cParamList.rbegin()));
}
SRythmParams* CRythmData::GetParams(const CHAR* szTradePair)
{
    for (auto& sParam : s_cParamList) {
        if (0 == strcmp(sParam.szTradePair, szTradePair)) {
            return &sParam;
        }
    }
    return NULL;
}
VOID CRythmData::SetParams(const SRythmParams& sParams)
{
    SRythmParams* psParam = GetParams(sParams.szTradePair);
    if (NULL != psParam) {
        if (&sParams == psParam) { // same pointer, do nothing
            return;
        }
        memcpy(psParam, &sParams, sizeof(SRythmParams));
        return;
    }
    // not found, so we insert new entry
    s_cParamList.push_back(sParams);
}
VOID CRythmData::ChangeBoxNum(UINT uBoxNum)
{
    s_uBoxNum = uBoxNum;
}
UINT CRythmData::GetBoxNum(VOID)
{
    return s_uBoxNum;
}

VOID CRythmData::NewGame(VOID)
{
    s_cParamList.clear();
}

BOOLEAN CRythmData::LoadData(const BYTE* pbyBuffer)
{
    s_cParamList.clear();

    ASSERT(0 == (((UINT64)pbyBuffer) % 8));
    const SRythmHeader* psHeader = (const SRythmHeader*)pbyBuffer;
    if (RYTHM_DATA_ID != psHeader->uID) {
        ASSERT(FALSE);
        return FALSE;
    }
    if (RYTHM_DATA_VERSION != psHeader->uVersion) {
        ASSERT(FALSE);
        return FALSE;
    }
    pbyBuffer += sizeof(SRythmHeader);
    ASSERT(RYTHM_DATA_VERSION == psHeader->uVersion);
    const SRythmParams* psParams = (const SRythmParams*)pbyBuffer;
    const UINT uNum = psHeader->uNum;
    for (UINT uIndex = 0; uNum > uIndex; ++uIndex, ++psParams) {
//        TRACE("\t{\n");
//        TRACE("\t\t\"%s\",\n", psParams->szTradePair);
//        TRACE("\t\t{ %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f },\n",
//              psParams->afSize[0],
//              psParams->afSize[1],
//              psParams->afSize[2],
//              psParams->afSize[3],
//              psParams->afSize[4],
//              psParams->afSize[5],
//              psParams->afSize[6],
//              psParams->afSize[7]);
//        TRACE("\t\t{ %d, %d, %d, %d, %d, %d, %d, %d },\n",
//        psParams->auBoxSize[0],
//        psParams->auBoxSize[1],
//        psParams->auBoxSize[2],
//        psParams->auBoxSize[3],
//        psParams->auBoxSize[4],
//        psParams->auBoxSize[5],
//        psParams->auBoxSize[6],
//        psParams->auBoxSize[7]);
//        TRACE("\t},\n");
        s_cParamList.push_back(*psParams);
    }
    return TRUE;
}

UINT CRythmData::GetSaveDataLength(VOID)
{
    return sizeof(SRythmHeader) + (UINT)s_cParamList.size() * sizeof(SRythmParams);
}

BOOLEAN CRythmData::SaveData(BYTE* pbyBuffer, UINT uLength)
{
    if (GetSaveDataLength() != uLength) {
        ASSERT(FALSE);
        return FALSE;
    }
    const UINT uNum = (UINT)s_cParamList.size();
    const SRythmHeader sHeader = {
        RYTHM_DATA_ID,      //        UINT uID;
        RYTHM_DATA_VERSION, //        UINT uVersion;
        uNum,               //        UINT uNum;
        s_uBoxNum           //        UINT uBoxNum;
    };
    memcpy(pbyBuffer, &sHeader, sizeof(SRythmHeader));
    pbyBuffer += sizeof(SRythmHeader);
    for (const auto& sParam : s_cParamList) {
        memcpy(pbyBuffer, &sParam, sizeof(SRythmParams));
        pbyBuffer += sizeof(SRythmParams);
    }
    return TRUE;
}
