//
//  FileUtility.cpp
//  HappyLib
//
//  Created by Nyon on 12/12/17.
//  Copyright © 2017 Happy Labs Pte Ltd. All rights reserved.
//

#include "stdafx.h"
#include "FileUtility.h"
#include "AutoPtr.h"



BOOLEAN FileUtility::GetFileContent(const CHAR * szFilename, BYTE ** pbyBuffer, UINT * pSize)
{
    if (NULL == szFilename || NULL == pbyBuffer || NULL == pSize) {
        return FALSE;
    }
    
    FILE * fp = fopen(szFilename, "r");
    if (NULL == fp) {
        return FALSE;
    }
    CAutoFile cGuardFP(fp);
    
    if (0 != fseek(fp, 0, SEEK_END)) {
        return FALSE;
    }
    
    size_t kSize = ftell(fp);
    if (kSize <= 0) {
        return FALSE;
    }
    
    if (0 != fseek(fp, 0, SEEK_SET)) {
        return FALSE;
    }
    
    BYTE * pbyBuf = new BYTE[kSize];
    if (NULL == pbyBuf) {
        return FALSE;
    }
    
    if (kSize != fread(pbyBuf, 1, kSize, fp)) {
        delete [] pbyBuf;
        return FALSE;
    }
    
    *pbyBuffer = pbyBuf;
    *pSize = (UINT)kSize;
    
    return TRUE;
}

BOOLEAN FileUtility::WriteStringToFile(const CHAR * szFilename, const std::string& data)
{
    if (NULL == szFilename || data.empty()) {
        return FALSE;
    }
    
    FILE * fp = fopen(szFilename, "w");
    if (NULL == fp) {
        return FALSE;
    }
    CAutoFile cGuardFP(fp);
    
    const size_t kLen = data.length();
    if (kLen != fwrite(data.c_str(), 1, kLen, fp)) {
        return FALSE;
    }

    return TRUE;
}
