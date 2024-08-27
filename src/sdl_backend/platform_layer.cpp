#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <cstdio>
#include <map>
#include <cstdio>

#include "../platform.h"

#define EASY_STRING_IMPLEMENTATION 1
#include "../easy_string_utf8.h"

#include "../debug_stats.h"

#include "../../libs/tinyfiledialogs.h"
#include "../../libs/tinyfiledialogs.c"


// #include "./win32/win32_threads.cpp"

static DEBUG_stats global_debug_stats = {};

static SDL_Window *global_wndHandle;
static SDL_GLContext renderContext;
static bool global_windowDidResize = false;

// static bool w32_got_system_info = false;
// SYSTEM_INFO w32_system_info = {}; 

//TODO:  From docs: Because the system cannot compact a private heap, it can become fragmented.
//TODO:  This means we don't want to use heap alloc, we would rather use a memory arena
static void *
platform_alloc_memory(size_t size, bool zeroOut)
{

    //TODO: Not sure if this should be different
    void *result = malloc(size);

    if(zeroOut) {
        memset(result, 0, size);
    }

    #if DEBUG_BUILD
        DEBUG_add_memory_block_size(&global_debug_stats, result, size);
    #endif

    return result;
}

//NOTE: Used by the game layer
static void platform_free_memory(void *data)
{
#if DEBUG_BUILD
    DEUBG_remove_memory_block_size(&global_debug_stats, data);
#endif

    free(data);

}

#include <unistd.h>
static u64 platform_get_memory_page_size() {
    long pagesize = sysconf(_SC_PAGE_SIZE);
    printf("%ld\n", pagesize);
    return pagesize;
}

static bool win32_isValidText(u16 wparam) {
    bool result = true;
    if(wparam < 9 || (14 <= wparam && wparam <= 27)) {
        
        result = false;
    }

     return result;
}

static char *platform_openFileDialog() {
    char const * result = tinyfd_openFileDialog (
	"Open File",
	"",
	0,
	NULL,
	NULL,
	0);

    return (char *)result;
}

static char *platform_saveFileDialog() {
    char const * result = tinyfd_saveFileDialog (
	"Save File",
	"",
	0,
	NULL,
	NULL);

    return (char *)result;
}



#include <sys/mman.h>
//NOTE: Used by the game layer
static void *platform_alloc_memory_pages(size_t size) {
    u64 page_size = platform_get_memory_page_size();

    size_t size_to_alloc = size + (page_size - 1);

    size_to_alloc -= size_to_alloc % page_size; 

#if DEBUG_BUILD
    global_debug_stats.total_virtual_alloc += size_to_alloc;
#endif

    //NOTE: According to the docs this just gets zeroed out
    void *result = mmap(0, size_to_alloc,
                                   PROT_READ | PROT_WRITE,
                                   MAP_ANON | MAP_PRIVATE,
                                   -1, 0);

    return result;
}

static u8 *platform_realloc_memory(void *src, u32 bytesToMove, size_t sizeToAlloc) {
    u8 *result = (u8 *)platform_alloc_memory(sizeToAlloc, true);

    memmove(result, src, bytesToMove);

    platform_free_memory(src);

    return result;
}

void updateKeyState(PlatformKeyType keyType, bool keyIsDown) {
    MouseKeyState state = global_platformInput.keys[keyType];
    int wasPressed = 0;
    int wasReleased = 0;

    if(keyIsDown) {
        if(state == MOUSE_BUTTON_NONE) {
          global_platformInput.keys[keyType] = MOUSE_BUTTON_PRESSED;
          
          wasPressed++;
        } else if(state == MOUSE_BUTTON_PRESSED) {
          global_platformInput.keys[keyType] = MOUSE_BUTTON_DOWN;
        }
    } else {
      if(global_platformInput.keys[keyType] == MOUSE_BUTTON_DOWN || global_platformInput.keys[keyType] == MOUSE_BUTTON_PRESSED) {
          global_platformInput.keys[keyType] = MOUSE_BUTTON_RELEASED;
          wasReleased++;
      } else {
        global_platformInput.keys[keyType] = MOUSE_BUTTON_NONE;
      }
    }

    global_platformInput.keyStates[keyType].pressedCount += wasPressed;
    global_platformInput.keyStates[keyType].releasedCount += wasReleased;

    global_platformInput.keyStates[keyType].isDown = keyIsDown;
  
}

void updateInput(SDL_Window *window, int *lastWindowWidth, int *lastWindowHeight, bool *running) {
      SDL_Event e;
      while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            *running = false;
          }
      }

    const Uint8* currentKeyStates = SDL_GetKeyboardState( NULL );

    updateKeyState(PLATFORM_KEY_UP, currentKeyStates[SDL_SCANCODE_UP] == 1 || currentKeyStates[SDL_SCANCODE_W] == 1);
    updateKeyState(PLATFORM_KEY_DOWN, currentKeyStates[SDL_SCANCODE_DOWN] == 1 || currentKeyStates[SDL_SCANCODE_S] == 1);
    updateKeyState(PLATFORM_KEY_LEFT, currentKeyStates[SDL_SCANCODE_LEFT] == 1 || currentKeyStates[SDL_SCANCODE_A] == 1);
    updateKeyState(PLATFORM_KEY_RIGHT, currentKeyStates[SDL_SCANCODE_RIGHT] == 1 || currentKeyStates[SDL_SCANCODE_D] == 1);
    updateKeyState(PLATFORM_KEY_SPACE, currentKeyStates[SDL_SCANCODE_SPACE] == 1);
    updateKeyState(PLATFORM_KEY_SHIFT, currentKeyStates[SDL_SCANCODE_LSHIFT] == 1);
    updateKeyState(PLATFORM_KEY_CTRL, currentKeyStates[SDL_SCANCODE_LCTRL] == 1);
    updateKeyState(PLATFORM_KEY_Z, currentKeyStates[SDL_SCANCODE_Z] == 1);
    updateKeyState(PLATFORM_KEY_S, currentKeyStates[SDL_SCANCODE_S] == 1);
    updateKeyState(PLATFORM_KEY_O, currentKeyStates[SDL_SCANCODE_O] == 1);
    updateKeyState(PLATFORM_KEY_1, currentKeyStates[SDL_SCANCODE_1] == 1);
    updateKeyState(PLATFORM_KEY_2, currentKeyStates[SDL_SCANCODE_2] == 1);
    updateKeyState(PLATFORM_KEY_3, currentKeyStates[SDL_SCANCODE_3] == 1);
    updateKeyState(PLATFORM_KEY_4, currentKeyStates[SDL_SCANCODE_4] == 1);
    updateKeyState(PLATFORM_KEY_5, currentKeyStates[SDL_SCANCODE_5] == 1);
    updateKeyState(PLATFORM_KEY_6, currentKeyStates[SDL_SCANCODE_6] == 1);
    updateKeyState(PLATFORM_KEY_7, currentKeyStates[SDL_SCANCODE_7] == 1);
    updateKeyState(PLATFORM_KEY_8, currentKeyStates[SDL_SCANCODE_8] == 1);
    int w; 
    int h;
    SDL_GetWindowSize(window, &w, &h);
    // gameState->screenWidth = (float)w;
    // gameState->aspectRatio_y_over_x = (float)h / (float)w;

    if(*lastWindowWidth != w || *lastWindowHeight != h) {
        global_windowDidResize = true;
    }

    *lastWindowWidth = w;
    *lastWindowHeight = h;

    int x; 
    int y;
    Uint32 mouseState = SDL_GetMouseState(&x, &y);
    
    global_platformInput.mouseX = (float)x;
    global_platformInput.mouseY = (float)(y);

    updateKeyState(PLATFORM_MOUSE_LEFT_BUTTON, mouseState && SDL_BUTTON(1));
}

#include "../memory_arena.cpp"
#include <time.h>
u64 platform_getTimeSinceEpoch() {
  time_t tIn;
    u64 t = (u64)time(&tIn);

    return (u64)t;
}

static Platform_File_Handle platform_begin_file_write_utf8_file_path (char *path_utf8) {

    Platform_File_Handle Result = {};

    FILE *fileHandle = fopen(path_utf8, "w+");
    
    if(fileHandle)
    {
        Result.data = fileHandle;
    }
    else
    {
        Result.has_errors = true;
    }
    
    return Result;
}

static void platform_close_file(Platform_File_Handle handle)
{
    FILE *fileHandle = (FILE *)handle.data;
    if(fileHandle)
    {
        fclose(fileHandle);
    }
}

static bool platform_write_file_data(Platform_File_Handle handle, void *memory, size_t size_to_write, size_t offset) {   
    bool success = false;
    if(!handle.has_errors) {
        FILE *FileHandle = (FILE *)handle.data;
        if(FileHandle)
        {
            if(fseek(FileHandle, offset, SEEK_SET) == 0)
            {
                size_t elementsWritten = fwrite(memory, size_to_write, 1, FileHandle);
                if(elementsWritten == 1)
                {
                    //NOTE: Success
                    success = true;
                }
                else
                {
                    assert(!"Read file did not succeed");
                }
            }
        }
    } else {
        assert(!"File handle not correct");
    }
    return success;
}

//TODO: Change this to sdl specific functions
static bool Platform_LoadEntireFile_utf8(char *filename_utf8, void **data, size_t *data_size) {
    bool succeed = false;
    *data = 0;
    *data_size = 0;

    //NOTE: Open & get size of ttf file
    FILE *fileHandle = fopen(filename_utf8, "rb");

    if(fileHandle){
        fseek(fileHandle, 0, SEEK_END);
        size_t read_bytes = ftell(fileHandle);
        fseek(fileHandle, 0L, SEEK_SET);

        void *read_data = platform_alloc_memory(read_bytes + 1, false);
        size_t bytesRead = fread(read_data, 1, read_bytes, fileHandle); 

        //NOTE: Read the ttf file contents
        if(bytesRead == read_bytes) {
            //NOTE: Null terminate the buffer 
            ((u8 *)read_data)[read_bytes] = 0;
          
            *data = read_data;
            *data_size = (u64)bytesRead;
            
            succeed = true;
        } else {
          platform_free_memory(read_data);
        }

        fclose(fileHandle);
    }

    return succeed;

}

#include "../3DMaths.h"
#include "../render.c"
#include "../render_backend/opengl_render.cpp"
#include "../main.cpp"

int GetDpiForWindow() {
    float ddpi;
    float hdpi;
    float vdpi;
    SDL_GetDisplayDPI(0, &ddpi, &hdpi, &vdpi);

    return ddpi;
}

float getBestDt(float secondsElapsed) {
      float frameRates[] = {1.0f/15.0f, 1.0f/20.0f, 1.0f/30.0f, 1.0f/60.0f, 1.0f/120.0f};
        //NOTE: Clmap the dt so werid bugs happen if huge lag like startup
      float closestFrameRate = 0;
      float minDiff = FLT_MAX;
      for(int i = 0; i < arrayCount(frameRates); i++) {
        float dt_ = frameRates[i];
        float diff_ = get_abs_value(dt_ - secondsElapsed);

        if(diff_ < minDiff) {
          minDiff = diff_;
          closestFrameRate = dt_;
        }
      }
      // printf("frames per second: %f\n", closestFrameRate);              
      return closestFrameRate;
}

int main(int argc, char **argv) {
  int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;
  if (SDL_Init(SDL_INIT_EVERYTHING)) {
    return 0;
  }
    Settings_To_Save settings_to_save = {};
    char *save_file_location_utf8 = 0;

    int windowWidth = 960;
    int windowHeight = 540;

    // Open a window
    {	
        //NOTE: Allocate stuff
        global_platform.permanent_storage_size = PERMANENT_STORAGE_SIZE;
        global_platform.permanent_storage = platform_alloc_memory_pages(global_platform.permanent_storage_size);
        
        global_long_term_arena = initMemoryArena_withMemory(((u8 *)global_platform.permanent_storage) + sizeof(EditorState), global_platform.permanent_storage_size - sizeof(EditorState));

        globalPerFrameArena = initMemoryArena(Kilobytes(100));
        global_perFrameArenaMark = takeMemoryMark(&globalPerFrameArena);

        //NOTE: Get the settings file we need
        // {
        //     save_file_location_utf8 = platform_get_save_file_location_utf8(&global_long_term_arena);

        //     char *settings_file_path = concatInArena(save_file_location_utf8, "user.settings", &globalPerFrameArena);
        //     Settings_To_Save settings_to_save = load_settings(settings_file_path);

        //     if(settings_to_save.is_valid) {
        //         window_width = (LONG)settings_to_save.window_width;
        //         window_height = (LONG)settings_to_save.window_height;

        //         window_xAt = (LONG)settings_to_save.window_xAt;
        //         window_yAt = (LONG)settings_to_save.window_yAt;
        //     }
        // }


        //Now create the actual window
        global_wndHandle = SDL_CreateWindow("Adventure Game",  SDL_WINDOWPOS_CENTERED,  SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, flags);

        assert(global_wndHandle);
    }

    
    //TODO: Change to using memory arena? 
    BackendRenderer *backendRenderer = (BackendRenderer *)platform_alloc_memory(sizeof(BackendRenderer), true); 
    backendRender_init(backendRenderer, global_wndHandle);

    global_platformInput.dpi_for_window = GetDpiForWindow();

    bool first_frame = true;

    //NOTE: Clear the keys states to NONE to start of with
    for(int i = 0; i < arrayCount(global_platformInput.keys); ++i) {
      global_platformInput.keys[i] = MOUSE_BUTTON_NONE;
    } 
  
    /////////////////////

    // Timing
    u64 startPerfCount = 0;
    u64 perfCounterFrequency = 0;
    {
        //NOTE: Get the current performance counter at this moment to act as our reference
        startPerfCount = SDL_GetPerformanceCounter();

        //NOTE: Get the Frequency of the performance counter to be able to convert from counts to seconds
        perfCounterFrequency = SDL_GetPerformanceFrequency();

    }

    //NOTE: To store the last time in
    double currentTimeInSeconds = 0.0;

    bool running = true;
    while(running) {

        //NOTE: Inside game loop
        float dt;
        {
            double previousTimeInSeconds = currentTimeInSeconds;
            u64 perfCount = SDL_GetPerformanceCounter();

            currentTimeInSeconds = (double)(perfCount - startPerfCount) / (double)perfCounterFrequency;
            dt = (float)(currentTimeInSeconds - previousTimeInSeconds);

            //NOTE: Round the dt to closest frame boundary for accuracy
            dt = getBestDt(dt);
        }

        //NOTE: Clear the input text buffer to empty
        global_platformInput.textInput_bytesUsed = 0;
        global_platformInput.textInput_utf8[0] = '\0';

        //NOTE: Clear our input command buffer
        global_platformInput.keyInputCommand_count = 0;

        global_platformInput.mouseScrollX = 0;
        global_platformInput.mouseScrollY = 0;
        //NOTE: Clear the key pressed and released count before processing our messages
        for(int i = 0; i < PLATFORM_KEY_TOTAL_COUNT; ++i) {
            global_platformInput.keyStates[i].pressedCount = 0;
            global_platformInput.keyStates[i].releasedCount = 0;
        }

    	updateInput(global_wndHandle, &windowWidth, &windowHeight, &running);

        bool resized_window = false;
        if(global_windowDidResize)
        {
            backendRender_release_and_resize_default_frame_buffer(backendRenderer);

            //NOTE: Make new ortho matrix here

            resized_window = true;
            global_windowDidResize = false;
        }

        EditorState *editorState = updateEditor(backendRenderer, dt, windowWidth, windowHeight, resized_window && !first_frame, save_file_location_utf8, settings_to_save);

        backendRender_processCommandBuffer(&editorState->renderer, backendRenderer);

        backendRender_presentFrame(backendRenderer);
        
        first_frame = false;
    }
    
    return 0;

}