#if !defined(HL_AWS_FUNCTION_NAME_H)
#define HL_AWS_FUNCTION_NAME_H
#include "HLAWSDefs.h"

static const SAWSFunction s_asFunctions[EAWSFunction_ID_NUM] = {
    { "ms_new_id",          -1 },   //    EAWSFunction_GetNewID=0,
    { "ms_write_user",      -1 },   //    EAWSFunction_WriteUser,
};
#endif //#if !defined(HL_AWS_FUNCTION_NAME_H)
