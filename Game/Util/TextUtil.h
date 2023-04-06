#if !defined(TEXT_UTIL_H)
#define TEXT_UTIL_H


class CTextUtil {
public:
	static VOID Tokenize(const CHAR* szString, const CHAR* szTokens);
	static const CHAR* NextToken(VOID);
    
    static VOID TrimString(CHAR* szString, UINT uMaxLen);
};


#endif // TEXT_UTIL_H
