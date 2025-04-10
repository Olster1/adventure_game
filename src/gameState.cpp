void clearGameStatePerFrameValues(GameState *state) {
	state->lightCount = 0;
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

		gameState->textureAtlas = readTextureAtlas("../src/images/texture_atlas.json", "../src/images/texture_atlas.png");
		gameState->bannerTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/ui/banner.png");
		gameState->selectTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/ui/select.png");
		gameState->shadowUiTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/ui/shadow.png");
		gameState->kLogoText = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/ui/kLogo.png");
		gameState->gLogoText = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/ui/gLogo.png");
		gameState->blueText = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/ui/b.png");
		gameState->redText = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/ui/r.png");
		gameState->cloudText[0] = textureAtlas_getItem(&gameState->textureAtlas, "cloud.png");
		gameState->cloudText[1] = textureAtlas_getItem(&gameState->textureAtlas, "cloud2.png");
		gameState->cloudText[2] = textureAtlas_getItem(&gameState->textureAtlas, "cloud3.png");
		gameState->treeTexture = textureAtlas_getItem(&gameState->textureAtlas, "tree.png");

		DefaultEntityAnimations knightAnimations;
		DefaultEntityAnimations peasantAnimations;
		DefaultEntityAnimations archerAnimations;

		gameState->trees = initResizeArray(RenderObject);
		gameState->waterAnimations = initResizeArray(RenderObject);
		

		createAOOffsets(gameState);
		
		gameState->gameMode = PLAY_MODE;

		gameState->cameraFollowPlayer = true;
		//TODO: Probably save this each time we leave the app
		gameState->zoomLevel = 1.8f;

		// gameState->potPlantAnimations = loadEntityAnimations(gameState, backendRenderer, "pot_plant", 16);

		//NOTE: Init all animations for game

		loadImageStrip(&gameState->animationState.waterRocks[0], backendRenderer, "../src/images/Rocks_03.png", 128);
		loadImageStrip(&gameState->animationState.waterAnimation, backendRenderer, "../src/images/foam.png", 192);

	 	// peasantAnimations;
		// archerAnimations

		// Animation run;
		
		// //NOTE: ATTACK
		// Animation attackUp;
		// Animation attackDown;
		// Animation attackSide;
	
		// //NOTE: For the peasant
		// Animation work;
	
		// //NOTE: For the TNT barrel & Peasant
		// Animation scared;

		loadImageStripXY(&gameState->knightAnimations.idle, backendRenderer, "../src/images/knight.png", 192, 192, 6, 0, 0);
		loadImageStripXY(&gameState->knightAnimations.run, backendRenderer, "../src/images/knight.png", 192, 192, 6, 1, 0);
		loadImageStripXY(&gameState->knightAnimations.attackSide, backendRenderer, "../src/images/knight.png", 192, 192, 12, 2, 0);
		loadImageStripXY(&gameState->knightAnimations.attackDown, backendRenderer, "../src/images/knight.png", 192, 192, 12, 4, 0);
		loadImageStripXY(&gameState->knightAnimations.attackUp, backendRenderer, "../src/images/knight.png", 192, 192, 12, 6, 0);

		/////////////
		{
			int tileCount = 0;
			int countX = 0;
			int countY = 0;
			Texture ** tiles = loadTileSet(backendRenderer, "../src/images/tilemap.png", 64, 64, &global_long_term_arena, &tileCount, &countX, &countY);
			gameState->sandTileSet = buildTileSet(tiles, tileCount, TILE_SET_SAND, countX, countY, 64, 64);
		}

	#if DEBUG_BUILD
		DEBUG_runUnitTests(gameState);
	#endif
}