#include "stdafx.h"
#include "UI9PartImage.h"

VOID CUI9PartImage::Set9PartTexture(ETEX::ID eTexID)
{
	UINT uLeft = 0;
	UINT uRight = 0;
	UINT uTop = 0;
	UINT uBottom = 0;
    switch (eTexID) {
        case ETEX::ID::ui_9part_filter_off:
            uLeft = 6;
            uRight = 6;
            uTop = 6;
            uBottom = 6;
            break;
        default:
			ASSERT(FALSE);
			break;
	}	
	Set9PartTexture(eTexID, uLeft, uRight, uTop, uBottom);
}
