struct GameLight {
    float3 pos;
    float4 color;

    float perlinNoiseT;
};

typedef struct {
	bool initialized;

	//NOTE: For creating unique entity ids like MongoDb ObjectIds
	int randomIdStartApp;
	int randomIdStart;

	Renderer renderer;

	EditorMode mode;

	Font font;

	float shakeTimer;

	TileSet swampTileSet;

	float fontScale;

	bool draw_debug_memory_stats;

	Entity player;	

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

	EasyAnimation_Controller playerAnimationController;
	Animation playerIdleAnimation;
	Animation fireballIdleAnimation;

	EasyAnimation_ListItem *animationItemFreeListPtr;

	EditorGui editorGuiState;

    int lightCount;
    GameLight lights[64];
} EditorState;
