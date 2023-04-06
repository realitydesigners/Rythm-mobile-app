#if !defined(AUTOMATION_DATA_DEFS_H)
#define AUTOMATION_DATA_DEFS_H
namespace AutoLogic {
    enum {
        EMAX_TARGET_NUM = 5,
    };
    struct STargets {
        FLOAT fTarget;          // target pip
        FLOAT fTrailingStop;    // trailing stop
    };
    struct STargetGroup {
        STargets asTargets[5];
    };
};
#endif // #if !defined(AUTOMATION_DATA_DEFS_H)
