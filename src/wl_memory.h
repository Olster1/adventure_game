////NOTE: MEMORY FUNCTIONS
#ifndef EASY_PLATFORM_H
/*
Code for platform depedent functions 
*/

typedef enum {
    EASY_PLATFORM_MEMORY_NONE,
    EASY_PLATFORM_MEMORY_ZERO,
} EasyPlatform_MemoryFlag;

static void *easyPlatform_allocateMemory(u32 sizeInBytes, EasyPlatform_MemoryFlag flags) {
    
    void *result = 0;
    
#if __WIN32__
    result = HeapAlloc(GetProcessHeap(), 0, sizeInBytes);
#else 
    result = malloc(sizeInBytes);
#endif
    
    if(!result) {
        // easyLogger_addLog("Platform out of memory on heap allocate!");
    }
    
    if(flags & EASY_PLATFORM_MEMORY_ZERO) {
        memset(result, 0, sizeInBytes);
    }
    
    return result;
}

static void easyPlatform_freeMemory(void *memory) {
#if __WIN32__
    HeapFree(GetProcessHeap(), 0, memory);
#else 
    free(memory);
#endif
    
}


static inline void easyPlatform_copyMemory(void *to, void *from, u32 sizeInBytes) {
    memcpy(to, from, sizeInBytes);
}

static inline u8 * easyPlatform_reallocMemory(void *from, u32 oldSize, u32 newSize) {
    u8 *result = (u8 *)easyPlatform_allocateMemory(newSize, EASY_PLATFORM_MEMORY_ZERO);

    easyPlatform_copyMemory(result, from, oldSize);

    easyPlatform_freeMemory(from);

    return result;
}

#define EASY_PLATFORM_H 1
#endif


/////////////////////////

