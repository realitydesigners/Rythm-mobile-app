#if !defined(SAVE_DATA_DEF_H)
#define SAVE_DATA_DEF_H

#define SAVE_VERSION (1)  // version at game released was 1

struct SSaveDataHeader {
    UINT        uVersion;
    UINT        uXORFlag; // for xor against entire save data
};

struct SSaveDataContents {
    UINT        uLastServerTime;        // last server updated time
    UINT        uOriginalSize;          // original size before zip. only start after SSaveDataContents
    
    BYTE        abyHash[32];            // hash of the rest of the payload
    
    UINT        uPadding1;
    UINT        uPadding2;
    
    union {
        UINT64  ullPlayerOffset;      // player info offset
        BYTE*   pbyPlayerChunk;
    };
    union {
        UINT64  ullRythmOffset;      // Rythm info offset
        BYTE*   pbyRythmChunk;
    };
    
    UINT        auBuffer[8];
};

#endif // #if !defined(SAVE_DATA_DEF_H)
