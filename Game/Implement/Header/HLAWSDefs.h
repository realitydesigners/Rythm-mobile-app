#if !defined(HL_AWS_FUNCTION_DEFS_H)
#define HL_AWS_FUNCTION_DEFS_H

enum EAWS_ErrorCode {
    EAWS_Error_None = 0,
    EAWS_Error_NotConnected,
    EAWS_Error_Invalid_Params,
    EAWS_Error_Invalid_Result,
    EAWS_Error_Generic,
    EAWS_Error_DeviceTimeSkewed,
};

enum EAWSFunction_ID {
    EAWSFunction_GetNewID=0,
    EAWSFunction_WriteUser,
    EAWSFunction_ID_NUM
};

struct SAWSFunction {
    const CHAR* szFunctionName;
    INT         nCacheDurationSec; // -1 for never to cache
};

#define AWS_IDENTITY_POOL_ID    "us-east-1:b7cdb7e2-cdc0-499a-91ab-e65e3504c87e"
#define AWS_IDENTITY_REGION     AWSRegionUSEast1
#define AWS_S3_BUCKET_NAME      "happylib"
#define VISIT_LOCAL_FILENAME    "visit.bin"
#define RESTORE_LOCAL_FILENAME  "restore.bin"
#endif //#if !defined(HL_AWS_FUNCTION_DEFS_H)
