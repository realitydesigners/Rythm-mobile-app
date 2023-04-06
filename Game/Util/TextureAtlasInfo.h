//
//  TexturePackerObject.h
//  HappyLib
//
//  Created by Nyon on 7/12/17.
//  Copyright © 2017 Happy Labs Pte Ltd. All rights reserved.
//

#ifndef TexturePackerObject_h
#define TexturePackerObject_h

#if defined(MAC_PORT)
#include <string>
#include "json.h"
#include "Texture.h"

class CTextureAtlasInfo
{
public:
    struct STextureInfo {
        std::string     m_szPath;
        Json::Value     m_cRoot;
        BYTE *          m_pbyBuffer;
        UINT            m_uW;
        UINT            m_uH;
        BOOLEAN         m_bLoaded;  // if true but m_pbyBuffer is null, loading has failed and probably path is invalid
    };
    
    struct SFrame
    {
        std::string     szName;
        INT             x;
        INT             y;
        INT             w;
        INT             h;
    };
    
    CTextureAtlasInfo();
    CTextureAtlasInfo(const CTextureAtlasInfo& rhs);    // copy construct
    ~CTextureAtlasInfo();
    
    VOID Release();
    
    BOOLEAN LoadJson(const CHAR * szFilename);
    VOID RemoveJson(const INT nIndex);
    
    
    CTexture * GetTexture(const CHAR * szTextureName) const;
    BOOLEAN GetTextureArea(const CHAR * szTextureName, BYTE ** pbyBuffer, INT & nWidth, INT & nHeight) const;
    
    
    const std::vector<std::string> & GetTextureList() const { return m_cTextureNames; }
    
    CTextureAtlasInfo& operator=(const CTextureAtlasInfo& rhs);
    
    INT GetNumberTextures() const;
    std::string GetTexturePath(INT nIndex) const;
    const CTextureAtlasInfo::STextureInfo& GetTextureInfo(INT nIndex) const;
    
    BOOLEAN GetTextureFramesByIndex(INT nIndex, std::vector<SFrame>& vecOutput) const;
    
    static CTexture * GetTexture(const CHAR * szAtlasPath, const CHAR * szTextureName);
private:
    
    typedef std::vector<STextureInfo> CVecTextureInfo;
    
    BOOLEAN GetTextureArea(const STextureInfo& sInfo, INT nX, INT nY, INT nWidth, INT nHeight, BYTE * pbyBuffer, INT uBufSize) const;
    
    
    CVecTextureInfo             m_cTextureInfoList;
    
    std::vector<std::string>    m_cTextureNames;
};

inline INT CTextureAtlasInfo::GetNumberTextures() const
{
    return (INT)m_cTextureInfoList.size();
}

inline std::string CTextureAtlasInfo::GetTexturePath(INT nIndex) const
{
    return m_cTextureInfoList[nIndex].m_szPath;
}

inline const CTextureAtlasInfo::STextureInfo& CTextureAtlasInfo::GetTextureInfo(INT nIndex) const
{
    return m_cTextureInfoList[nIndex];
}

#endif // #if defined(MAC_PORT)
#endif /* TexturePackerObject_h */
