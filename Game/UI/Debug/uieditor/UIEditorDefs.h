#if !defined(UI_EDITOR_DEFS_H)
#define UI_EDITOR_DEFS_H
#include "HLMath.h"
#include "FontDefs.h"
#include "NibDefs.h"
#include <string>

class CTexture;

struct SNibEditImageInfo {
    std::string     cImgPath;
    CTexture*       pcTexture;
    UINT            uColor;
    FLOAT           fRotation;
    BYTE            abyOffset[4]; // offsets used by 3 part and 9 part
    BOOLEAN         bVertical;
};
struct SNibEditTextInfo {
    std::string     cDefaultString;
    EGAMEFONT       eFontSize;
    UINT            uColor;
    UINT            uTextAlignment;
    UINT            uEfxType;
    FLOAT           afEfxParams[2];
    UINT            uEfxColor;
};

struct SNibEditBtnInfo;
struct SNibEditInfo {
    EBaseWidgetType     eType;
    SNibSizeInfo        sSize;      // size information
    SNibPosInfo         sPos;       // position information
    SHLVector2D         sAnchor;    // anchor of this widget
    std::string         cName;      // name of this widget
    
    SNibEditImageInfo*  psImg;
    SNibEditTextInfo*   psText;
    SNibEditBtnInfo*    psBtn;
};

struct SNibEditBtnInfo {
    SNibEditInfo   sIdleInfo;
    SNibEditInfo   sPressedInfo;
    BOOLEAN        bToggleBtn;
    FLOAT          fPressedOffset;
};

#endif // #if !defined(UI_EDITOR_DEFS_H)

