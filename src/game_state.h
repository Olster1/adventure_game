struct GameLight {
    float3 worldPos;
    float4 color;

};

typedef enum {
	EASY_PROFILER_DRAW_OVERVIEW,
	EASY_PROFILER_DRAW_ACCUMALTED,
} EasyProfile_DrawType;

typedef enum {
	EASY_PROFILER_DRAW_OPEN,
	EASY_PROFILER_DRAW_CLOSED,
} EasyDrawProfile_OpenState;

typedef struct {
	int hotIndex;

	EasyProfile_DrawType drawType;

	//For navigating the samples in a frame
	float zoomLevel;
	float xOffset; //for the scroll bar
	bool holdingScrollBar;
	float scrollPercent;
	float grabOffset;

	float2 lastMouseP;
	bool firstFrame;

	EasyDrawProfile_OpenState openState;
	float openTimer;

	int level;
} EasyProfile_ProfilerDrawState;

static EasyProfile_ProfilerDrawState *EasyProfiler_initProfilerDrawState() {
	EasyProfile_ProfilerDrawState *result = pushStruct(&global_long_term_arena, EasyProfile_ProfilerDrawState);
		
	result->hotIndex = -1;
	result->level = 0;
	result->drawType = EASY_PROFILER_DRAW_OVERVIEW;

	result->zoomLevel = 1;
	result->holdingScrollBar = false;
	result->xOffset = 0;
	result->lastMouseP =  make_float2(0, 0);
	result->firstFrame = true;
	result->grabOffset = 0;

	result->openTimer = -1;
	result->openState = EASY_PROFILER_DRAW_CLOSED;

	return result;

}

enum GameMode {
	PLAY_MODE,
	TILE_MODE,
	SELECT_ENTITY_MODE,
	A_STAR_MODE
};

struct RenderObject {
	Texture *sprite;
	float3 pos;
	float2 scale;
	u32 lightingMask;
	int sortIndex;

	RenderObject() {}

	RenderObject(Texture *sprite,
		float3 pos,
		float2 scale, u32 lightingMask, int sortIndex = 0) {
			this->sprite = sprite;
			this->pos = pos;
			this->scale = scale;
			this->lightingMask = lightingMask;
			this->sortIndex = sortIndex;
	}
};

typedef struct {
	bool initialized;

	GameMode gameMode;

	char *selectedEntityId;

	//NOTE: For creating unique entity ids like MongoDb ObjectIds
	int randomIdStartApp;
	int randomIdStart;

	Renderer renderer;

	Font font;
	Font pixelFont;

	float shakeTimer;

	float fontScale;

	bool draw_debug_memory_stats;

	EasyProfile_ProfilerDrawState *drawState;

	LightingOffsets lightingOffsets;

	Entity *player;	

	float scrollDp;

	int entityCount;
	Entity entities[256];

	float3 cameraPos;
	float cameraFOV;

	float planeSizeX;
	float planeSizeY;

	GamePlay gamePlay;

	Texture bannerTexture;
	Texture selectTexture;
	Texture shadowUiTexture;
	Texture kLogoText;
	Texture gLogoText;
	Texture blueText;
	Texture redText;
	AtlasAsset *cloudText[3];
	AtlasAsset *treeTexture;
	TextureAtlas textureAtlas;

	bool draggingCanvas;
	float2 startDragP;
	float2 canvasMoveDp;
	
	RenderObject *trees;
	RenderObject *waterAnimations;

	int tileCount;
	MapTile tiles[10000];

	DefaultEntityAnimations knightAnimations;
	DefaultEntityAnimations peasantAnimations;
	DefaultEntityAnimations archerAnimations;

	AnimationState animationState;

	GameDialogs dialogs;
	TileSet sandTileSet;

	Terrain terrain;

	EditorGui editorGuiState;

	bool cameraFollowPlayer;

	float zoomLevel;

    int lightCount;
    GameLight lights[64];
} GameState;