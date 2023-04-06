#if !defined(MESSAGE_LOG_H)
#define MESSAGE_LOG_H
#include <list>

struct SMessageLog {
    UINT        uTimeStamp; // UNIX time
    const CHAR* szMessage;
};
typedef std::list<SMessageLog> CMessageLogList;

class CMessageLog
{
public:
    static BOOLEAN Initialize(VOID);
    static VOID Release(VOID);
    static VOID Update(FLOAT fLapsed);

    static VOID Clear(VOID);
    static VOID AddLog(const CHAR* szMessage);
    static const CMessageLogList& GetLogs(VOID);
    
};

#endif // #if !defined(RYTHM_DATA_H)
