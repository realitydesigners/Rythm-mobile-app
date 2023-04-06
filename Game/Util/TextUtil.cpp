#include "stdafx.h"
#include "TextUtil.h"
#include <string>

static const INT MAX_STRING_LEN = 1024;
static CHAR s_achBuffer[MAX_STRING_LEN];

static const CHAR* s_szTokens = NULL;
static const CHAR* s_szString = NULL;

static INT s_nTokenLength = 0;
static INT s_nStringLength = 0;
static INT s_nCurrentIndex = 0;

VOID CTextUtil::Tokenize(const CHAR* szString, const CHAR* szTokens)
{
	s_nTokenLength = (UINT)strlen( szTokens );
	s_nStringLength = (UINT)strlen( szString );
	s_nCurrentIndex = 0;
	if ( MAX_STRING_LEN <= s_nStringLength ) {
        TRACE("Failed to Tokenize %s", szString);
        exit(7);
		return;
	}
	s_szTokens = szTokens;
	s_szString = szString;
	strcpy( s_achBuffer, s_szString );
}

const CHAR* CTextUtil::NextToken(VOID)
{
	// skip tokens at beginning of string
    INT nTokenCount = 0;
	for ( ; s_nStringLength > s_nCurrentIndex; ++s_nCurrentIndex ) {
		INT nTokenIndex = 0;
		BOOLEAN bTokenFound = false;
		for ( ; s_nTokenLength > nTokenIndex; ++nTokenIndex ) {
			if ( s_szString[ s_nCurrentIndex ] == s_szTokens[ nTokenIndex ] ) {
				bTokenFound = true;
                ++nTokenCount;
				break;
			}
		}
		if ( !bTokenFound ) {
			break;
		}
        // not empty cell, just skipped
//        if (1 < nTokenCount) { // empty cell.
//            return NULL;
//        }
	}
	// if reached end of string
	if ( s_nCurrentIndex >= s_nStringLength ) {
		return NULL;
	}

	INT nIndex = s_nCurrentIndex + 1;
	// iterate until we find a token
	for ( ; s_nStringLength > nIndex; ++nIndex ) {
		INT nTokenIndex = 0;
		BOOLEAN bTokenFound = false;
		for ( ; s_nTokenLength > nTokenIndex; ++nTokenIndex ) {
			if ( s_szString[ nIndex ] == s_szTokens[ nTokenIndex ] ) {
				bTokenFound = true;
				break;
			}
		}
		if ( bTokenFound ) {
			break;
		}
	}
	if ( nIndex < s_nStringLength ) {
		s_achBuffer[ nIndex ] = 0;
	}
	const char* szToken = &s_achBuffer[ s_nCurrentIndex ];
	s_nCurrentIndex = nIndex;
	return szToken;
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

VOID CTextUtil::TrimString(CHAR* szString, UINT uMaxLen)
{
    std::string cString = szString;
    s_ltrim(cString);
    s_rtrim(cString);
    strncpy(szString, cString.c_str(), uMaxLen);
}
