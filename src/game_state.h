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

	TileSet swampTileSet;

	float fontScale;

	bool draw_debug_memory_stats;

	Entity *player;	

	int entityCount;
	Entity entities[256];

	int colliderCount;
	Collider colliders;

	float3 cameraPos;

	float planeSizeX;
	float planeSizeY;

	Texture playerTexture;

	Texture pipeTexture;

	Texture pipeFlippedTexture;

	Texture backgroundTexture;

	Texture coinTexture;

	float coinRotation; //NOTE: Between 0 and 1

	int points;

	int tileCount;
	MapTile tiles[1028];

	bool hasInteratedYet;

	//NOTE: Resizeable array for the coins - if id in list, means user got it. 
	int *coinsGot;

	Animation playerIdleAnimation;
	Animation playerRunAnimation;
	Animation playerAttackAnimation;
	Animation playerHurtAnimation;
	Animation playerDieAnimation;
	Animation playerJumpAnimation;
	Animation playerFallingAnimation;


	//NOTE: Skeleton animation
	Animation skeletonIdleAnimation;
	Animation skeletonRunAnimation;
	Animation skeletonAttackAnimation;
	Animation skeletonHurtAnimation;
	Animation skeletonDieAnimation;
	Animation skeletonShieldAnimation;


	Animation fireballIdleAnimation;

	EasyAnimation_ListItem *animationItemFreeListPtr;

	EditorGui editorGuiState;

	bool movingCamera;

    int lightCount;
    GameLight lights[64];
} EditorState;