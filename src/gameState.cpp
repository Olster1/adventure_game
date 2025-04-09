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

void createAOOffsets(GameState *gameState) {
    for(int i = 0; i < arrayCount(global_cubeData); ++i) {
        assert(i < arrayCount(gameState->lightingOffsets.aoOffsets));

        CubeVertex v = global_cubeData[i];
        float3 normal = v.normal;
        float3 sizedOffset = scale_float3(2, v.pos);

        float3 masks[2] = {};
        int maskCount = 0;

        for(int k = 0; k < 3; k++) {
            if(normal.E[k] == 0) {
                assert(maskCount < arrayCount(masks));
                float3 m = make_float3(0, 0, 0);
                m.E[k] = -sizedOffset.E[k];

                masks[maskCount++] = m;
            }
        }

        gameState->lightingOffsets.aoOffsets[i].offsets[0] = plus_float3(sizedOffset, masks[0]);
        gameState->lightingOffsets.aoOffsets[i].offsets[1] = sizedOffset; 
        gameState->lightingOffsets.aoOffsets[i].offsets[2] = plus_float3(sizedOffset, masks[1]);
    }
}


void initGameState(GameState *gameState, BackendRenderer *backendRenderer) {
		gameState->initialized = true;

		initRenderer(&gameState->renderer);
		gameState->animationState.animationItemFreeListPtr = 0;

		gameState->font = initFont("../fonts/liberation-mono.ttf");
		gameState->pixelFont = initFont("../fonts/Pixelify.ttf");
		
		gameState->fontScale = 0.6f;
		gameState->scrollDp = 0;

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

		gameState->gamePlay = init_gameplay();

		gameState->drawState = EasyProfiler_initProfilerDrawState();
		

		gameState->stoneTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/stone.png");
		gameState->grassTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/grass.png");
		gameState->dirtTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/dirt.png");
		gameState->waterTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/water.png");
		gameState->shadowTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/shadow.png");
		gameState->treeTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/tree.png");
		gameState->bannerTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/ui/banner.png");
		gameState->selectTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/ui/select.png");
		gameState->shadowUiTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/ui/shadow.png");
		gameState->kLogoText = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/ui/kLogo.png");
		gameState->gLogoText = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/ui/gLogo.png");
		gameState->blueText = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/ui/b.png");
		gameState->redText = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/ui/r.png");
		gameState->cloudText[0] = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/cloud.png");
		gameState->cloudText[1] = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/cloud2.png");
		gameState->cloudText[2] = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/cloud3.png");

		gameState->trees = initResizeArray(RenderObject);
		gameState->waterAnimations = initResizeArray(RenderObject);
		

		createAOOffsets(gameState);
		
		gameState->gameMode = PLAY_MODE;

		gameState->cameraFollowPlayer = true;
		//TODO: Probably save this each time we leave the app
		gameState->zoomLevel = 2.25;

		// gameState->potPlantAnimations = loadEntityAnimations(gameState, backendRenderer, "pot_plant", 16);

		//NOTE: Init all animations for game

		loadImageStrip(&gameState->playerIdleAnimation, backendRenderer, "../src/images/player/Man_forward_idle_6.png", 64);
		loadImageStrip(&gameState->playerRunForwardAnimation, backendRenderer, "../src/images/player/Man_forward_4.png", 64);
		loadImageStrip(&gameState->playerRunbackwardAnimation, backendRenderer, "../src/images/player/Man_back_4.png", 64);
		loadImageStrip(&gameState->playerRunsidewardAnimation, backendRenderer, "../src/images/player/Man_left_walk_4.png", 64);

		loadImageStrip(&gameState->animationState.waterRocks[0], backendRenderer, "../src/images/Rocks_03.png", 128);
		
		
		loadImageStrip(&gameState->playerAttackAnimation, backendRenderer, "../src/images/player/Man_forward_attack_4.png", 64);
		loadImageStrip(&gameState->playerJumpAnimation, backendRenderer, "../src/images/player/Man_forward_idle_6.png", 64);
		loadImageStrip(&gameState->playerHurtAnimation, backendRenderer, "../src/images/player/Man_forward_idle_6.png", 64);
		loadImageStrip(&gameState->playerDieAnimation, backendRenderer, "../src/images/player/Man_forward_idle_6.png", 64);
		loadImageStrip(&gameState->playerFallingAnimation, backendRenderer, "../src/images/player/Man_forward_idle_6.png", 64);

		loadImageStrip(&gameState->playerbackwardSidewardRun, backendRenderer, "../src/images/player/Man_back_sideways_4.png", 64);
		loadImageStrip(&gameState->playerforwardSidewardRun, backendRenderer, "../src/images/player/Man_forward_sideways_4.png", 64);

		loadImageStrip(&gameState->animationState.waterAnimation, backendRenderer, "../src/images/foam.png", 192);

		/////////////
		{
			int tileCount = 0;
			int countX = 0;
			int countY = 0;
			Texture ** tiles = loadTileSet(backendRenderer, "../src/images/tilemap.png", 64, 64, &global_long_term_arena, &tileCount, &countX, &countY);
			gameState->sandTileSet = buildTileSet(tiles, tileCount, TILE_SET_SAND, countX, countY, 64, 64);
		}

		// addPlayerEntity(gameState);

		gameState->gravityOn = false;

	#if DEBUG_BUILD
		DEBUG_runUnitTests(gameState);
	#endif
}