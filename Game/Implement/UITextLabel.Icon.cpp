#include "stdafx.h"
#include "UITextLabel.h"
#include "TexID.h"

// check UITextLabel.cpp for list of reserved alphabets

#define ESCAPE_DRAW_PLUS_ICON    '+'

struct STextLabelIconInfo {
    ETEX::ID eTex;
    BYTE byW;
    BYTE byH;
    BYTE bySpacing; // will be applied to the left and right. unless the line starts with the icon, then only apply to right
};

static const STextLabelIconInfo s_asIconInfo[] = {
    { ETEX::efx_heal, 20, 20, 1 }, // ESCAPE_DRAW_PLUS_ICON
};

BOOLEAN CUITextLabel::GetLabelIcon(USHORT ushCode, UINT& uTexID, BYTE& byW, BYTE& byH, BYTE& bySpacing)
{
    UINT uIndex = 0;
    switch (ushCode) {
        case ESCAPE_DRAW_PLUS_ICON:
            uIndex = 0;
            break;
            
        default:
            TRACE("unknow escape code: %c(%d)", (CHAR)ushCode, ushCode);
            ASSERT(FALSE);
            return FALSE;
    }
    const STextLabelIconInfo& sIcon = s_asIconInfo[uIndex];
    uTexID = sIcon.eTex;
    byW = sIcon.byW;
    byH = sIcon.byH;
    bySpacing = sIcon.bySpacing;
    return TRUE;
}

