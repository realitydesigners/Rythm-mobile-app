#if !defined(_MESSAGE_DATA_H_)
#define _MESSAGE_DATA_H_
#include "MessageDataDefs.h"

class CMessageData
{
public:
    static VOID Release(VOID);
    static VOID OnChangeLanguage(UINT uLang);
    static const CHAR* GetMsgID(INT nID);
};
#endif // #if !defined(_MESSAGE_DATA_H_)
