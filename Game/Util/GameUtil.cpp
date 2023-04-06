#include <string>
#include "stdafx.h"
#include "ConfirmationWindow.h"
#include "EventManager.h"
#include "GameUtil.h"
#include "stdio_ext.h"


VOID GameUtil::GetHourMinSec(INT & nDay, INT & nHour, INT & nMins, INT & nSec)
{
    nDay = nSec / (24 * 60 * 60);
    nSec -= nDay * (24 * 60 * 60);
    nHour = nSec / (60 * 60);
    nSec -= nHour * (60 * 60);
    nMins = nSec / 60;
    nSec -= nMins * 60;
    ASSERT(nSec < 60);
}

VOID GameUtil::GetMMSSString(CHAR* szBuffer, UINT uBufferLen, INT nSeconds)
{
    INT nDay, nHour, nMin;
    GameUtil::GetHourMinSec(nDay, nHour, nMin, nSeconds);
    nMin += (nDay * 24 + nHour) * 60;
    snprintf(szBuffer, uBufferLen, "%02d:%02d", nMin, nSeconds);
}

static VOID s_ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}
// trim from end (in place)
static VOID s_rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch) && (',' != ch);
    }).base(), s.end());
}

VOID GameUtil::TrimString(CHAR* szString, UINT uMaxLen)
{
    std::string cString = szString;
    s_ltrim(cString);
    s_rtrim(cString);
    strncpy(szString, cString.c_str(), uMaxLen);
}

BOOLEAN GameUtil::ValidateEmailAddress(const CHAR* szEmailAddress)
{
    //TrimString(szEmailAddress, uMaxLen);
    // there are a lot of rules to what constitute to be a valid email
    // so for starters, we're just going to check on some simple rules

    // 1) no empty spaces on start nor end of email
    // 2) exactly 1 '@' sign, none or more than 1 is invalid
    // 3) there is at least a username part and domain part

    if (NULL == szEmailAddress) {
        return FALSE;
    }

    const INT nLength = (INT)strlen(szEmailAddress);

    // check for empty space before or after email;
    if (nLength == 0 || szEmailAddress[0] == ' ' || szEmailAddress[nLength - 1] == ' ') {
        return FALSE;
    }

    

    // search for '@'
    INT nAtSignIndex = -1;
    for (INT i = 0; nLength > i; ++i) {
        // check for valid charset
        
        // valid range of ascii chars are from ' ' (32) to '~' (126)
        if (szEmailAddress[i] < 32 || szEmailAddress[i] > 126) {
            return FALSE;
        }

        if (szEmailAddress[i] == '@') {
            if (nAtSignIndex >= 0) {
                // more than 1 '@' sign
                // TODO: in some cases, '@' sign in double quote username is considered valid
                //       eg, "me@home"@home.com
                //       but this is rare and uncommon
                return FALSE;
            }
            nAtSignIndex = i;
        }
    }

    if (nAtSignIndex < 0 || nAtSignIndex == (nLength - 1) || 0 == nAtSignIndex) {
        // missing '@', at the end (no domain) or at the start (no username)
        return FALSE;
    }
    
    // TODO: we could dive deep and verify username part and domain part separately

    return TRUE;
}

CConfirmationWindow* GameUtil::ShowConfirmationDialog(const CHAR * szTitle, const CHAR * szMesg, CEventHandler * handler, UINT uEventID, BOOLEAN bHideCancelButton)
{
    CConfirmationWindow * pcWin = new CConfirmationWindow(handler);
    if (NULL == pcWin) {
        ASSERT(FALSE);
        return NULL;
    }
    if (NULL != szTitle) {
        pcWin->SetTitle(szTitle);
    }
    if (NULL != szMesg) {
        pcWin->SetMessage(szMesg);
    }
    if (bHideCancelButton) {
        pcWin->HideCancelButton();
    }
    pcWin->SetOKButton(NULL, uEventID);
    pcWin->DoModal();
    
    return pcWin;
}

VOID GameUtil::FormatNumbers(CHAR * szBuffer, UINT uBufferLen, INT nValue)
{
    std::string strNum;
    const BOOLEAN bIsNegative = nValue < 0;
    
    if (bIsNegative) {
        nValue *= -1;
        strNum.append("-");
    }
    
    std::vector<INT> stack;
    
    while (nValue > 1000) {
        stack.push_back(nValue % 1000);
        nValue /= 1000;
    }
    
    CHAR buf[16];
    snprintf(buf, 16, "%d", nValue);
    strNum.append(buf);
    
    std::vector<INT>::reverse_iterator itr = stack.rbegin();
    for (; stack.rend() != itr; ++itr) {
        strNum.append(",");
        snprintf(buf, 16, "%03d", *itr);
        strNum.append(buf);
    }
    
    const INT nSize = (INT)strNum.size();
    if (nSize+1 >= uBufferLen) {
        return;
    }
    
    strncpy(szBuffer, strNum.c_str(), nSize);
    szBuffer[nSize] = 0;
}
