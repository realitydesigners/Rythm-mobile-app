#include "stdafx.h"
#include "CMain.h"
#include "FontManager.h"

// TODO: Remember to uncheck Device Orientation in ios settings for project.
#define LANDSCAPE_MODE TRUE
//#define LANDSCAPE_MODE FALSE

#if defined(MAC_PORT) || defined(IPHONE)
#define FONT_NAME "Segoe UI"
#else // #if defined(MAC_PORT) || defined(IPHONE)
#define FONT_NAME "font/SegoeUI.ttf"
#endif // #if defined(MAC_PORT) || defined(IPHONE)

static const FLOAT FONT_SIZES[EGAMEFONT_NUM] = {
    40.0f,  // EGAMEFONT_SIZE_40
    38.0f,  // EGAMEFONT_SIZE_38
    34.0f,  // EGAMEFONT_SIZE_34
    32.0f,  // EGAMEFONT_SIZE_32
    30.0f,  // EGAMEFONT_SIZE_30
    28.0f,  // EGAMEFONT_SIZE_28
    26.0f,  // EGAMEFONT_SIZE_26
    24.0f,  // EGAMEFONT_SIZE_24
    22.0f,  // EGAMEFONT_SIZE_22
    20.0f,  // EGAMEFONT_SIZE_20
    18.0f,  // EGAMEFONT_SIZE_18
    16.0f,  // EGAMEFONT_SIZE_16
    14.0f,  // EGAMEFONT_SIZE_14
    12.0f,  //    EGAMEFONT_SIZE_12,
    10.0f,  //    EGAMEFONT_SIZE_10,
    8.0f,   //    EGAMEFONT_SIZE_08,
};

BOOLEAN CMain::IsLandScapeMode(VOID) {
    return LANDSCAPE_MODE;
}

#if defined(IPHONE) || defined(MAC_PORT)
const CHAR* CFontManager::GetFontName(EFONT_LANGUAGE eLanguage) {
    return FONT_NAME;
}
#endif // #if defined(IPHONE) || defined(MAC_PORT)

FLOAT CFontManager::GetFontSize(EGAMEFONT eFont) {
    return FONT_SIZES[eFont];
}
UINT CFontManager::GetFontIndexFromGameFont(EGAMEFONT eFont)
{
    return 0;
//    UINT uIndex = FONT_DATA[eFont].eLang;
//    if (EFONT_LANGUAGE_DEFAULT == uIndex) {
//        uIndex = s_uDefaultFontIndex;
//    }
//    return uIndex;
}

