struct GameLight {
    float3 worldPos;
    float4 color;

};

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

	RenderObject() {}

	RenderObject(Texture *sprite,
		float3 pos,
		float2 scale) {
			this->sprite = sprite;
			this->pos = pos;
			this->scale = scale;
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