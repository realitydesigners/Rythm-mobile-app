#include "stdafx.h"
#include "AutoLogicData.h"
#include "PatternData.h"

static const STPEntry s_asTPEntry[] = {
#include "TPEntry.inc"
};
static const UINT s_uTPEntryNum = sizeof(s_asTPEntry)/sizeof(STPEntry);

UINT CAutoLogicDataMgr::GetTPEntryNum(VOID)
{
    return s_uTPEntryNum;
}

const STPEntry& CAutoLogicDataMgr::GetTPEntry(UINT uIndex)
{
    ASSERT(s_uTPEntryNum > uIndex);
    return s_asTPEntry[uIndex];
}

static const SLogic s_asLogic[] = {
#include "TradeOrder.inc"
};
static const UINT s_uLogicNum = sizeof(s_asLogic)/sizeof(SLogic);

UINT CAutoLogicDataMgr::GetLogicNum(VOID)
{
    return s_uLogicNum;
}

const SLogic& CAutoLogicDataMgr::GetLogic(UINT uIndex)
{
    ASSERT(s_uLogicNum > uIndex);
    return s_asLogic[uIndex];
}

const CHAR* CAutoLogicDataMgr::GetLogicName(UINT uIndex)
{
    if (s_uLogicNum <= uIndex) {
        return "OOB";
    }
    return CPatternDataMgr::GetPattern(s_asLogic[uIndex].uCategoryPatternIndex).szName;
}

static const UINT s_auAlerts[] = {
#include "Alert.inc"
};
static const UINT s_uAlertNum = sizeof(s_auAlerts)/sizeof(UINT);

UINT CAutoLogicDataMgr::GetAlertNum(VOID)
{
    return s_uAlertNum;
}

const UINT* CAutoLogicDataMgr::GetAlertPatterns(VOID)
{
    return s_auAlerts;
}
