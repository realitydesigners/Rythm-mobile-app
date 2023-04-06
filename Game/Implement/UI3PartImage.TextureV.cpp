#include "stdafx.h"
#include "UI3PartImage.h"

VOID CUI3PartImage::Set3PartVerticalTexture(ETEX::ID eTexID)
{
    UINT uUpOffset = 0;
    UINT uDownOffset = 0;
    switch (eTexID) {
        case ETEX::ID::ui_9part_filter_off:
            uUpOffset = 6;
            uDownOffset = 6;
            break;

        default:
            ASSERT(FALSE);
            uUpOffset = 1;
            uDownOffset = 1;
            break;
    }
    Set3PartTexture(eTexID, uUpOffset, uDownOffset, TRUE);
}

