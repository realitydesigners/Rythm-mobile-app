#ifndef GAMEUTIL_H
#define GAMEUTIL_H
#include <time.h>

class CEventHandler;
class CConfirmationWindow;
class CUITextLabel;

namespace GameUtil {
    VOID GetHourMinSec(INT & nDay, INT & nHour, INT & nMin, INT & nSec);
    VOID GetMMSSString(CHAR* szBuffer, UINT uBufferLen, INT nSeconds);

    BOOLEAN ValidateEmailAddress(const CHAR* szEmailAddress);
    CConfirmationWindow* ShowConfirmationDialog(const CHAR * szTitle, const CHAR * szMesg, CEventHandler * handler, UINT uEventID, BOOLEAN bHideCancelButton);
    
    VOID FormatNumbers(CHAR * szBuffer, UINT uBufferLen, INT uValue);
    
    VOID TrimString(CHAR* szString, UINT uMaxLen);

    BOOLEAN IsValidEmail(CHAR * szEmail);
};

#endif /* GAMEUTIL_H */
