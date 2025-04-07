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

	RenderObject() {}

	RenderObject(Texture *sprite,
		float3 pos,
		float2 scale, u32 lightingMask) {
			this->sprite = sprite;
			this->pos = pos;
			this->scale = scale;
			this->lightingMask = lightingMask;
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

	Texture playerTexture;
	Texture stoneTexture;
	Texture grassTexture;
	Texture dirtTexture;
	Texture waterTexture;
	Texture shadowTexture;
	Texture treeTexture;
	
	RenderObject *trees;

	int tileCount;
	MapTile tiles[10000];

	DefaultEntityAnimations potPlantAnimations;

	AnimationState animationState;

	Animation playerIdleAnimation;
	Animation playerRunForwardAnimation;
	Animation playerRunbackwardAnimation;
	Animation playerRunsidewardAnimation;
	
	Animation playerAttackAnimation;
	Animation playerHurtAnimation;
	Animation playerDieAnimation;
	Animation playerJumpAnimation;
	Animation playerFallingAnimation;

	Animation playerbackwardSidewardRun;
	Animation playerforwardSidewardRun;

	GameDialogs dialogs;
	TileSet sandTileSet;

	Terrain terrain;

	bool gravityOn;

	EditorGui editorGuiState;

	bool cameraFollowPlayer;


	float zoomLevel;

    int lightCount;
    GameLight lights[64];
} GameState;