#if !defined(APP_MAIN_H)
#define APP_MAIN_H

namespace CAppMain {
    BOOLEAN Initialize(VOID);
    VOID    Release(VOID);
    
    VOID    Start(VOID);
    
    VOID    ReleaseGPUResources(VOID);
    VOID    RestoreGPUResources(VOID);
    
    VOID    OnPause(VOID);
    VOID    OnResume(VOID);
    VOID    OnStop(VOID);
    
    VOID    OnUpdate(FLOAT fLapsed);
    VOID    OnRender(VOID);
    
    FLOAT   GetGlobalSine(VOID); // get a global sine wave (-1 to 1) with a fixed frequency
    FLOAT   GetGlobalSineAdjusted(FLOAT fSpeed); // get a global sine wave (-1 to 1) with adjusted frequency

    FLOAT   GetGlobalLapsedTime(VOID);
#if defined(DEBUG)
    VOID    ToggleFPS(VOID);
#endif // #if defined(DEBUG)
#if defined(MAC_PORT)
    VOID    OnAppMinimized(VOID);
    VOID    OnAppDeMinimized(VOID);
#endif // #if defined(MAC_PORT)
};

#endif // #if !defined(APP_MAIN_H)
