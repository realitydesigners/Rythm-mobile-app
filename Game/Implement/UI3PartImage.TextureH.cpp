#include "stdafx.h"
#include "UI3PartImage.h"

VOID CUI3PartImage::Set3PartTexture(ETEX::ID eTexID)
{
	UINT uLeftOffset = 0;
	UINT uRightOffset = 0;
	switch (eTexID) {
        case ETEX::ID::ui_9part_filter_off:
            uLeftOffset = 6;
            uRightOffset = 6;
            break;
        default:
            ASSERT(FALSE);
            uLeftOffset = 1;
            uRightOffset = 1;
			break;
	}
	Set3PartTexture(eTexID, uLeftOffset, uRightOffset, FALSE);
}

