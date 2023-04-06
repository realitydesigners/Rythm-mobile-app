#include "stdafx.h"
#include "Compressor.h"
#include "SaveData.h"
#include "SHA256.h"
#include "AutoPtr.h"

#include "SaveDataDef.h"
#include "PlayerData.h"
#include "RythmData.h"
#include "AppResourceManager.h"
#include <memory.h>

#if defined(DEBUG)
#define SAVE_STRING "SaveDebug.dat"
#else // #if defined(DEBUG)
#define SAVE_STRING "Save.dat"
#endif // #if defined(DEBUG)

#if defined(DEBUG)
#include <stddef.h>     /* offsetof */
static VOID DebugStruct(VOID)
{
    ASSERT(0 == offsetof(SSaveDataHeader, uVersion));
    ASSERT(4 == offsetof(SSaveDataHeader, uXORFlag));
    ASSERT(8 == sizeof(SSaveDataHeader));

    ASSERT(0 == offsetof(SSaveDataContents, uLastServerTime));
    ASSERT(4 == offsetof(SSaveDataContents, uOriginalSize));
    ASSERT(8 == offsetof(SSaveDataContents, abyHash));
    ASSERT(40 == offsetof(SSaveDataContents, uPadding1));
    ASSERT(44 == offsetof(SSaveDataContents, uPadding2));
    ASSERT(48 == offsetof(SSaveDataContents, ullPlayerOffset));
    ASSERT(48 == offsetof(SSaveDataContents, pbyPlayerChunk));
    ASSERT(56 == offsetof(SSaveDataContents, pbyRythmChunk));
    ASSERT(64 == offsetof(SSaveDataContents, auBuffer));
    ASSERT(96 == sizeof(SSaveDataContents));
}
#endif // #if defined(DEBUG)

static const UINT MAX_SAVE_BUFFER = 512 * 1024;

static BOOLEAN      s_bSaveDataDirty = FALSE;
static UINT         s_uLastServerSavedTime = 0;
static BOOLEAN      s_bSavable = FALSE;

VOID CSaveData::CreateNewGame(VOID)
{
#if defined(DEBUG)
    DebugStruct();
#endif // #if defiend(DEBUG)
    CPlayerData::NewGame();
    CRythmData::NewGame();
    s_bSavable       = TRUE;
    s_bSaveDataDirty = TRUE;
    CSaveData::SaveData();
}

BOOLEAN ProcessSaveFile( BYTE ** pbyOut, UINT * puOutSize, const BYTE * pbyInBuf, UINT uBufferSize )
{
    if ( NULL == pbyInBuf || 0 == uBufferSize || NULL == pbyOut || NULL == puOutSize || uBufferSize < sizeof(SSaveDataHeader) + sizeof(SSaveDataContents)) {
        ASSERT(FALSE);
        return FALSE;
    }
    
    BYTE * xorbuf = new BYTE[uBufferSize];
    if ( NULL == xorbuf ) {
        return FALSE;
    }
    CAutoPtrArray<BYTE> cGuardXorBuf(xorbuf);
    memcpy(xorbuf, pbyInBuf, uBufferSize);
    
    SSaveDataHeader* psHeader = reinterpret_cast<SSaveDataHeader*>(xorbuf);
    
    
    {   // un-xor the memory first
        const UINT uXOR = psHeader->uXORFlag;
        UINT* puData = reinterpret_cast<UINT*>(psHeader + 1);
        UINT uNum = (uBufferSize - sizeof(SSaveDataHeader)) / 4;
        UINT uIndex = 0;
        for ( ; uNum > uIndex; ++uIndex) {
            puData[uIndex] = puData[uIndex] ^ uXOR;
        }
    }
    
    if (psHeader->uVersion != SAVE_VERSION) {
        TRACE( "CSaveData::ProcessSaveFile] unsupported version: %d\n", psHeader->uVersion);
        ASSERT(FALSE);
        return FALSE;
    }
    
    const UINT uTotalHeaderSize = sizeof(SSaveDataHeader) + sizeof(SSaveDataContents);
    const SSaveDataContents * psContent = (SSaveDataContents*)(psHeader+1);
    BYTE * pbyTemp = NULL;
    const UINT uChunkSize = psContent->uOriginalSize;
    {
        // unzip buffer first
        if ( 0 == psContent->uOriginalSize ) {
            ASSERT(FALSE);
            return FALSE;
        }
		
        pbyTemp = new BYTE[ uTotalHeaderSize + psContent->uOriginalSize ];
        if ( NULL == pbyTemp ) {
            ASSERT(FALSE);
            return FALSE;
        }
        
        // copy over header information
        memcpy( pbyTemp, xorbuf, uTotalHeaderSize );
        
        // unzip the data
        UINT uUnCompressSize = CCompressor::UncompressData(pbyTemp + uTotalHeaderSize, psContent->uOriginalSize, (BYTE*)(psContent+1), uBufferSize - uTotalHeaderSize );
        if ( 0 == uUnCompressSize || uUnCompressSize != psContent->uOriginalSize ) {
            delete [] pbyTemp;
            ASSERT(FALSE);
            return FALSE;
        }
    }
    CAutoPtrArray<BYTE> cGuardTempBuf( pbyTemp );
    
    // check hash
    BYTE abyHash[32];
    CSHA256::GenerateHash(pbyTemp + uTotalHeaderSize, uChunkSize, abyHash);
    if ( 0 != memcmp(psContent->abyHash, abyHash, 32) ) {
#if defined( DEBUG )
        printf( "fileHash   : 0x" );
        for ( int i = 0; 32 > i; ++i ) {
            printf( "%02x", psContent->abyHash[i] );
        }
        printf( "\nBuffer hash: 0x");
        for ( int i = 0; 32 > i; ++i ) {
            printf( "%02x", abyHash[i] );
        }
        printf("\n");
#endif // #if defined( DEBUG )
        ASSERT(FALSE);
        return FALSE;
    }
    // compute and realign all pointer address
    SSaveDataContents * pTempContent = (SSaveDataContents*)( pbyTemp + sizeof(SSaveDataHeader) );
    pTempContent->pbyPlayerChunk    = pbyTemp + pTempContent->ullPlayerOffset;
    pTempContent->pbyRythmChunk     = pbyTemp + pTempContent->ullRythmOffset;
    cGuardTempBuf.RemoveLock();     // remove the auto delete functionality
    *pbyOut = pbyTemp;
    *puOutSize = uTotalHeaderSize + uChunkSize;
    
    return TRUE;
}

BOOLEAN CSaveData::LoadData(const BYTE * pbyFileBuffer, UINT uFileSize)
{
    BYTE * pbyBuffer = NULL;
    UINT uBufferSize = 0;
    if ( FALSE == ProcessSaveFile( &pbyBuffer, &uBufferSize, pbyFileBuffer, uFileSize ) ) {
        ASSERT(FALSE);
        return FALSE;
    }
    CAutoPtrArray<BYTE> cGuardBuffer( pbyBuffer );
    SSaveDataContents * psContents = (SSaveDataContents*)(pbyBuffer + sizeof(SSaveDataHeader));
    // load server related data
    s_uLastServerSavedTime      = psContents->uLastServerTime;
    // load each data chunk
    {
        // load player data chunk
        if (!CPlayerData::LoadData(psContents->pbyPlayerChunk)) {
            TRACE("[CSaveData::LoadData] failed to load player data\n");
            ASSERT(FALSE);
            return FALSE;
        }
    }
    {
            // load rythm data chunk
            if (!CRythmData::LoadData(psContents->pbyRythmChunk)) {
                TRACE("[CSaveData::LoadData] failed to load rythm data\n");
                ASSERT(FALSE);
                return FALSE;
            }
        }

    s_bSavable          = TRUE;
    s_bSaveDataDirty    = FALSE;
	return TRUE;
}

BOOLEAN CSaveData::LoadData(VOID)
{
    UINT uDataSize = 0;
    BYTE* pbyBuffer = CAppResourceManager::ReadFromLocalStorage(SAVE_STRING, uDataSize);
    if (NULL == pbyBuffer) {
        TRACE("Save Data not found\n");
        return FALSE;
    }
    TRACE("Found Save File of size:%d\n", uDataSize);
    CAutoPtrArray<BYTE> cGuardBuffer(pbyBuffer);
    return LoadData(pbyBuffer, uDataSize);
}

BOOLEAN CSaveData::GetSaveData(BYTE** pbyOutBuffer, UINT* puOutSize)
{
    // calculate how much memory is needed to save data
    const UINT uTotalHeaderSize = sizeof(SSaveDataHeader) + sizeof(SSaveDataContents);
    
    const UINT uPlayerChunkSize     = CPlayerData::GetSaveDataLength();
    const UINT uRythmChunkSize      = CRythmData::GetSaveDataLength();
    
    const UINT uChunkSize = uPlayerChunkSize + uRythmChunkSize;
    const UINT uBufferSize = uTotalHeaderSize + uChunkSize;
    
    if ( MAX_SAVE_BUFFER < uBufferSize ) {
        ASSERT(FALSE);
        return FALSE;
    }
    
    BYTE* pbySaveBuffer = new BYTE[uBufferSize];
    if (NULL == pbySaveBuffer) {
        ASSERT(FALSE);
        return FALSE;
    }
    CAutoPtrArray<BYTE> cGuardSaveBuffer(pbySaveBuffer);  // to ensure this memory gets deleted when the function exits
    
    SSaveDataHeader* psHeader = (SSaveDataHeader*)pbySaveBuffer;
    SSaveDataContents* psContent = (SSaveDataContents*)(psHeader + 1);
    
    UINT uDataSize = uTotalHeaderSize;
    
    memset(psHeader,  0, sizeof(SSaveDataHeader));
    memset(psContent, 0, sizeof(SSaveDataContents));
    
    // update save version
    psHeader->uVersion = SAVE_VERSION;
    
    // randomize xor flag (non 0)
    do {
        psHeader->uXORFlag = rand();
    } while (0 == psHeader->uXORFlag);
    
    // update server related data
    psContent->uLastServerTime      = s_uLastServerSavedTime;
    psContent->uOriginalSize        = uChunkSize;
    
    {   // player data
        psContent->ullPlayerOffset = uDataSize;
        BYTE* pbyPlayerData = pbySaveBuffer + uDataSize;
        uDataSize += uPlayerChunkSize;
        if (!CPlayerData::SaveData(pbyPlayerData, uPlayerChunkSize)) {
            ASSERT(FALSE);
            return FALSE;
        }
    }
    {   // rythm data
        psContent->ullRythmOffset = uDataSize;
        BYTE* pbyPlayerData = pbySaveBuffer + uDataSize;
        uDataSize += uRythmChunkSize;
        if (!CRythmData::SaveData(pbyPlayerData, uRythmChunkSize)) {
            ASSERT(FALSE);
            return FALSE;
        }
    }

    // generate hash
    if ( uDataSize != uBufferSize ) {
        ASSERT(FALSE);
        return FALSE;
    }
    ASSERT( uChunkSize == uDataSize - uTotalHeaderSize );
    
    CSHA256::GenerateHash(pbySaveBuffer+uTotalHeaderSize, uChunkSize, psContent->abyHash);
    
    BYTE * pbyCompressSave = new BYTE[ uBufferSize ];
    if ( NULL == pbyCompressSave ) {
        ASSERT(FALSE);
        return FALSE;
    }
    CAutoPtrArray<BYTE> cGuardCompressSave( pbyCompressSave );
    
    // copy header information
    memcpy( pbyCompressSave, pbySaveBuffer, uTotalHeaderSize );
    
    UINT uCompressedSize = CCompressor::CompressData(pbyCompressSave + uTotalHeaderSize,
                                                     uChunkSize,
                                                     pbySaveBuffer + uTotalHeaderSize,
                                                     uChunkSize);
    
    if ( uCompressedSize == 0 ) {
        ASSERT(FALSE);
        return FALSE;
    }
    
    // perform xor
    UINT* puData = (UINT*)(pbyCompressSave + sizeof(SSaveDataHeader));
    UINT uNum = (uCompressedSize + sizeof(SSaveDataContents)) / 4;
    UINT uIndex = 0;
    for ( ; uNum > uIndex; ++uIndex) {
        puData[uIndex] = puData[uIndex] ^ psHeader->uXORFlag;
    }
    
    cGuardCompressSave.RemoveLock();        // remove auto delete
    *pbyOutBuffer = pbyCompressSave;
    *puOutSize = uTotalHeaderSize + uCompressedSize;

    return TRUE;
}

BOOLEAN CSaveData::SaveData(BOOLEAN bOnlySaveIfDirty)
{
#if defined(DEBUG)
    DebugStruct();
#endif // #if defined(DEBUG)
    if (!s_bSavable) {
        ASSERT(FALSE);
        return FALSE;
    }
    if (bOnlySaveIfDirty && !s_bSaveDataDirty) {
        return FALSE;
    }
    BYTE * pbyDataBuffer;
    UINT uDataSize;
    if (FALSE == GetSaveData( &pbyDataBuffer, &uDataSize ) ) {
        ASSERT(FALSE);
        return FALSE;
    }
    CAutoPtrArray<BYTE> cGuardDataBuffer( pbyDataBuffer );
    
    if (!CAppResourceManager::WriteToLocalStorage(SAVE_STRING, pbyDataBuffer, uDataSize)) {
        ASSERT(FALSE);
        return FALSE;
    }
    s_bSaveDataDirty = FALSE;
	return TRUE;
}

BOOLEAN CSaveData::IsSaveDataDirty(VOID)
{
	return s_bSaveDataDirty;
}

VOID CSaveData::SetSaveDataDirty(VOID)
{
    s_bSaveDataDirty = TRUE;
}

BOOLEAN CSaveData::IsSavable(VOID)
{
    return s_bSavable;
}
    
VOID CSaveData::SetSavable(BOOLEAN bSavable)
{
    s_bSavable = bSavable;
}
