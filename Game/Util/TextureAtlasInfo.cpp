//
//  TexturePackerObject.cpp
//  HappyLib
//
//  Created by Nyon on 7/12/17.
//  Copyright © 2017 Happy Labs Pte Ltd. All rights reserved.
//

#include "stdafx.h"
#if defined(MAC_PORT)
#include "TextureAtlasInfo.h"
#include "AutoPtr.h"
#include "PngFile.h"
#include "FileUtility.h"

#define JSON_KEY_FRAMES "frames"

CTextureAtlasInfo::CTextureAtlasInfo()
{
    
}

// copy constructor
CTextureAtlasInfo::CTextureAtlasInfo(const CTextureAtlasInfo& rhs)
{
    *this = rhs;
}

CTextureAtlasInfo::~CTextureAtlasInfo()
{
    CTextureAtlasInfo::Release();
}

VOID CTextureAtlasInfo::Release()
{
    for (auto& cTex: m_cTextureInfoList) {
        SAFE_DELETE_ARRAY(cTex.m_pbyBuffer);
    }
    m_cTextureInfoList.clear();
    m_cTextureNames.clear();
}

CTextureAtlasInfo& CTextureAtlasInfo::operator=(const CTextureAtlasInfo &rhs)
{
    if (this == &rhs) {
        return *this;
    }
    
    Release();
    
    for (auto& cTex: rhs.m_cTextureInfoList) {
        STextureInfo sNewInfo;
        sNewInfo.m_cRoot = cTex.m_cRoot;
        sNewInfo.m_uW = cTex.m_uW;
        sNewInfo.m_uH = cTex.m_uH;
        sNewInfo.m_szPath = cTex.m_szPath;
        
        if (NULL != cTex.m_pbyBuffer) {
            UINT uSize = sNewInfo.m_uW * sNewInfo.m_uH * 4;
            sNewInfo.m_pbyBuffer = new BYTE[uSize];
            if (NULL != sNewInfo.m_pbyBuffer) {
                memcpy(sNewInfo.m_pbyBuffer, cTex.m_pbyBuffer, uSize);
            }
        }
        
        m_cTextureInfoList.push_back(sNewInfo);
    }
    m_cTextureNames = rhs.m_cTextureNames;
    
    return *this;
}


BOOLEAN CTextureAtlasInfo::LoadJson(const CHAR * szFilename)
{
    if (NULL == szFilename) {
        return FALSE;
    }

    // texture should be same name as json file, except it's png extension
    std::string szTexturePath = szFilename;
    size_t pos = szTexturePath.rfind('.');
    if (pos == std::string::npos) {
        return false;
    }
    
    szTexturePath.replace(pos, szTexturePath.size() - pos, ".png");
    
    m_cTextureInfoList.push_back({});
    STextureInfo & sNewTexInfo = m_cTextureInfoList.back();
    sNewTexInfo.m_bLoaded = TRUE;
    sNewTexInfo.m_szPath = szFilename;
    sNewTexInfo.m_pbyBuffer = NULL;
    sNewTexInfo.m_uW = 0;
    sNewTexInfo.m_uH = 0;
    sNewTexInfo.m_cRoot.clear();
    
    
    {   // load json
        Json::Reader reader;
        BYTE * pbyBuf;
        UINT uSize = 0;
        if (!FileUtility::GetFileContent(szFilename, &pbyBuf, &uSize)) {
            sNewTexInfo.m_bLoaded = FALSE;
            return FALSE;
        }
        CAutoPtrArray<BYTE> cGuardBuf(pbyBuf);

        if (!reader.parse((CHAR*)pbyBuf, (CHAR*)pbyBuf + uSize, sNewTexInfo.m_cRoot)) {
            sNewTexInfo.m_bLoaded = FALSE;
            return FALSE;
        }
        
        if (!sNewTexInfo.m_cRoot.isMember(JSON_KEY_FRAMES) || !sNewTexInfo.m_cRoot[JSON_KEY_FRAMES].isObject()) {
            sNewTexInfo.m_bLoaded = FALSE;
            return FALSE;
        }
    }
    
    {   // load texture
        BYTE * pbyBuf = NULL;
        UINT uSize = 0;
        if (!FileUtility::GetFileContent(szTexturePath.c_str(), &pbyBuf, &uSize)) {
            sNewTexInfo.m_bLoaded = FALSE;
            return FALSE;
        }
        CAutoPtrArray<BYTE> cGuardBuf(pbyBuf);
        
        if (!CPngFile::ReadPngBuffer(&sNewTexInfo.m_pbyBuffer, sNewTexInfo.m_uW, sNewTexInfo.m_uH, pbyBuf, uSize)) {
            sNewTexInfo.m_bLoaded = FALSE;
            return FALSE;
        }
    }
    
    // add new names to list
    Json::Value::Members cNames = sNewTexInfo.m_cRoot[JSON_KEY_FRAMES].getMemberNames();
    m_cTextureNames.insert(m_cTextureNames.end(), cNames.begin(), cNames.end());
    return TRUE;
}

CTexture * CTextureAtlasInfo::GetTexture(const CHAR * szTextureName) const
{
    BYTE * pbyBuffer;
    INT nWidth, nHeight;
    if (FALSE == GetTextureArea(szTextureName, &pbyBuffer, nWidth, nHeight)) {
        return NULL;
    }
    
    CTexture * pcTexture = CTexture::CreateTexture(pbyBuffer, nWidth, nHeight, FALSE, TRUE);
    if (NULL == pcTexture) {
        delete [] pbyBuffer;
        ASSERT(FALSE);
        return NULL;
    }
    
    return pcTexture;
}

BOOLEAN CTextureAtlasInfo::GetTextureArea(const CHAR * szTextureName, BYTE ** pbyBuffer, INT & nWidth, INT & nHeight) const
{
    if (NULL == szTextureName) {
        return FALSE;
    }
    
    for (auto& sTexInfo : m_cTextureInfoList) {
        if (FALSE == sTexInfo.m_bLoaded) {
            continue;
        }
        if (!sTexInfo.m_cRoot.isMember(JSON_KEY_FRAMES) || !sTexInfo.m_cRoot[JSON_KEY_FRAMES].isObject()) {
            continue;
        }
        
        const Json::Value& jFrames = sTexInfo.m_cRoot[JSON_KEY_FRAMES];
        if (!jFrames.isMember(szTextureName) || !jFrames[szTextureName].isObject()) {
            continue;
        }
        
        const Json::Value& jTexture = jFrames[szTextureName];
        if (!jTexture.isMember("frame") || !jTexture["frame"].isObject()) {
            continue;
        }
        
        const Json::Value& jFrame = jTexture["frame"];
        if (!jFrame.isMember("x") || !jFrame["x"].isInt() ||
            !jFrame.isMember("y") || !jFrame["y"].isInt() ||
            !jFrame.isMember("w") || !jFrame["w"].isInt() ||
            !jFrame.isMember("h") || !jFrame["h"].isInt()) {
            continue;
        }
        
        INT x = jFrame["x"].asInt();
        INT y = jFrame["y"].asInt();
        INT w = jFrame["w"].asInt();
        INT h = jFrame["h"].asInt();
        
        INT nSize = w * h * 4;
        if (nSize <= 0) {
            continue;
        }
        BYTE * pbyMem = new BYTE[nSize];
        if (NULL == pbyMem) {
            continue;
        }
        
        if (FALSE == GetTextureArea(sTexInfo, x, y, w, h, pbyMem, nSize)) {
            delete [] pbyMem;
            continue;
        }
        
        *pbyBuffer = pbyMem;
        nWidth = w;
        nHeight = h;
        return TRUE;
    }
    
    
    return FALSE;
}

BOOLEAN CTextureAtlasInfo::GetTextureArea(const STextureInfo& sInfo, INT nX, INT nY, INT nWidth, INT nHeight, BYTE * pbyBuffer, INT nBufSize) const
{
    if (nX < 0 || nY < 0 || nX + nWidth > sInfo.m_uW || nY + nHeight > sInfo.m_uH) {
        return FALSE;
    }
    
    for (INT y = 0; nHeight > y; ++y) {
        INT nDstOffset = y * nWidth * 4;
        INT nSrcOffset = ((nY + y) * sInfo.m_uW + nX) * 4;
        memcpy(pbyBuffer + nDstOffset, sInfo.m_pbyBuffer + nSrcOffset, nWidth * 4);
    }
    
    return TRUE;
}

BOOLEAN CTextureAtlasInfo::GetTextureFramesByIndex(INT nIndex, std::vector<SFrame>& vecOutput) const
{
    if (m_cTextureInfoList.size() <= nIndex || 0 > nIndex) {
        return FALSE;
    }
    
    if (FALSE == m_cTextureInfoList[nIndex].m_cRoot.isMember(JSON_KEY_FRAMES) || FALSE == m_cTextureInfoList[nIndex].m_cRoot[JSON_KEY_FRAMES].isObject()) {
        return FALSE;
    }
    
    const Json::Value & jFrames = m_cTextureInfoList[nIndex].m_cRoot[JSON_KEY_FRAMES];
    const Json::Value::Members list = jFrames.getMemberNames();
    for (auto& name : list) {
        const Json::Value& jTexture = jFrames[name];
        if (!jTexture.isMember("frame") || !jTexture["frame"].isObject()) {
            continue;
        }
        
        const Json::Value& jFrame = jTexture["frame"];
        if (!jFrame.isMember("x") || !jFrame["x"].isInt() ||
            !jFrame.isMember("y") || !jFrame["y"].isInt() ||
            !jFrame.isMember("w") || !jFrame["w"].isInt() ||
            !jFrame.isMember("h") || !jFrame["h"].isInt()) {
            continue;
        }
        
        vecOutput.push_back({
            name,
            jFrame["x"].asInt(),
            jFrame["y"].asInt(),
            jFrame["w"].asInt(),
            jFrame["h"].asInt(),
        });
    }
    
    return TRUE;
}

VOID CTextureAtlasInfo::RemoveJson(const INT nIndex)
{
    if (0 > nIndex || nIndex >= m_cTextureInfoList.size()) {
        return;
    }
    
    SAFE_DELETE_ARRAY(m_cTextureInfoList[nIndex].m_pbyBuffer);
    m_cTextureInfoList.erase(m_cTextureInfoList.begin() + nIndex);
}

CTexture * CTextureAtlasInfo::GetTexture(const CHAR * szAtlasPath, const CHAR * szTextureName)
{
    CTextureAtlasInfo cAtlas;
    if (FALSE == cAtlas.LoadJson(szAtlasPath)) {
        return NULL;
    }
    
    return cAtlas.GetTexture(szTextureName);
}

#endif // #if defined(MAC_PORT)

