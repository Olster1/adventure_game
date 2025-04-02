void clearGameStatePerFrameValues(GameState *state) {
	state->lightCount = 0;
}


DefaultEntityAnimations loadEntityAnimations(GameState *gameState, BackendRenderer *backendRenderer, char *folder, int sizePerWidth) {
	DefaultEntityAnimations result = {};

	char *formatStr = "../src/images/%s/%s.png";

	#define DEFINE_loadImageForEntity(animPtr, name) loadImageStrip(animPtr, backendRenderer,  easy_createString_printf(&globalPerFrameArena, formatStr, folder, name), sizePerWidth);

	DEFINE_loadImageForEntity(&result.idle, "idle");
	DEFINE_loadImageForEntity(&result.runForward, "run_forward");
	DEFINE_loadImageForEntity(&result.runBack, "run_back");
	DEFINE_loadImageForEntity(&result.runSideward, "run_sideways");
	DEFINE_loadImageForEntity(&result.runSidewardBack, "run_sideways_back");
	DEFINE_loadImageForEntity(&result.attack, "attack");
	DEFINE_loadImageForEntity(&result.hurt, "hurt");
	DEFINE_loadImageForEntity(&result.suprised, "suprised");
	DEFINE_loadImageForEntity(&result.die, "die");
	DEFINE_loadImageForEntity(&result.attackBack, "attack_back");
	

	#undef DEFINE_loadImageForEntity

	return result;
}

void initGameState(GameState *gameState, BackendRenderer *backendRenderer) {
		gameState->initialized = true;

		initRenderer(&gameState->renderer);
		gameState->animationState.animationItemFreeListPtr = 0;

		#if DEBUG_BUILD
		gameState->font = initFont("../fonts/liberation-mono.ttf");
		#else
		gameState->font = initFont("./fonts/liberation-mono.ttf");
		#endif

		gameState->fontScale = 0.6f;

		gameState->terrain = Terrain();

		gameState->draw_debug_memory_stats = false;

		initDialogTrees(&gameState->dialogs);

		gameState->shakeTimer = -1;//NOTE: Turn the timer off

		srand(time(NULL));   // Initialization, should only be called once.

		//NOTE: Used to build the entity ids 
		gameState->randomIdStartApp = rand();
		gameState->randomIdStart = rand();

		gameState->planeSizeY = 20;
		gameState->planeSizeX = 20;

		// gameState->player.velocity = make_float3(0, 0, 0);
		// gameState->player.pos = make_float3(0, 0, 10);
		// gameState->playerTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "..\\src\\images\\helicopter.png");

		gameState->stoneTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/stone.png");
		gameState->grassTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/grass.png");
		gameState->dirtTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/dirt.png");
		gameState->waterTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/water.png");

		gameState->gameMode = PLAY_MODE;

		gameState->cameraFollowPlayer = true;
		gameState->zoomLevel = 1;

		// gameState->potPlantAnimations = loadEntityAnimations(gameState, backendRenderer, "pot_plant", 16);

		//NOTE: Init all animations for game

		loadImageStrip(&gameState->playerIdleAnimation, backendRenderer, "../src/images/player/Man_forward_idle_6.png", 64);
		loadImageStrip(&gameState->playerRunForwardAnimation, backendRenderer, "../src/images/player/Man_forward_4.png", 64);
		loadImageStrip(&gameState->playerRunbackwardAnimation, backendRenderer, "../src/images/player/Man_back_4.png", 64);
		loadImageStrip(&gameState->playerRunsidewardAnimation, backendRenderer, "../src/images/player/Man_left_walk_4.png", 64);
		
		
		loadImageStrip(&gameState->playerAttackAnimation, backendRenderer, "../src/images/player/Man_forward_attack_4.png", 64);
		loadImageStrip(&gameState->playerJumpAnimation, backendRenderer, "../src/images/player/Man_forward_idle_6.png", 64);
		loadImageStrip(&gameState->playerHurtAnimation, backendRenderer, "../src/images/player/Man_forward_idle_6.png", 64);
		loadImageStrip(&gameState->playerDieAnimation, backendRenderer, "../src/images/player/Man_forward_idle_6.png", 64);
		loadImageStrip(&gameState->playerFallingAnimation, backendRenderer, "../src/images/player/Man_forward_idle_6.png", 64);

		loadImageStrip(&gameState->playerbackwardSidewardRun, backendRenderer, "../src/images/player/Man_back_sideways_4.png", 64);
		loadImageStrip(&gameState->playerforwardSidewardRun, backendRenderer, "../src/images/player/Man_forward_sideways_4.png", 64);

		loadImageStrip(&gameState->animationState.waterAnimation, backendRenderer, "../src/images/foam.png", 192);

		/////////////

		int tileCount = 0;
		int countX = 0;
		int countY = 0;
		Texture ** tiles = loadTileSet(backendRenderer, "../src/images/tileMapFlat.png", 64, 64, &global_long_term_arena, &tileCount, &countX, &countY);

		gameState->sandTileSet = buildTileSet(tiles, tileCount, TILE_SET_SAND, countX, countY, 64, 64);

		addPlayerEntity(gameState);

		gameState->gravityOn = false;

	#if DEBUG_BUILD
		DEBUG_runUnitTests(gameState);
	#endif
}