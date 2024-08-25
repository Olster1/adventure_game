void clearGameStatePerFrameValues(EditorState *state) {
	state->lightCount = 0;
}

void initGameState(EditorState *editorState, BackendRenderer *backendRenderer) {
		editorState->initialized = true;

		initRenderer(&editorState->renderer);

		editorState->mode = MODE_EDIT_BUFFER;

		#if DEBUG_BUILD
		editorState->font = initFont("../fonts/liberation-mono.ttf");
		#else
		editorState->font = initFont("./fonts/liberation-mono.ttf");
		#endif

		editorState->fontScale = 0.6f;

		editorState->draw_debug_memory_stats = false;
		editorState->hasInteratedYet = false;

		editorState->shakeTimer = -1;//NOTE: Turn the timer off

		srand(time(NULL));   // Initialization, should only be called once.

		//NOTE: Used to build the entity ids 
		editorState->randomIdStartApp = rand();
		editorState->randomIdStart = rand();

		editorState->planeSizeY = 20;
		editorState->planeSizeX = 20;

		// editorState->player.velocity = make_float3(0, 0, 0);
		// editorState->player.pos = make_float3(0, 0, 10);
		// editorState->playerTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "..\\src\\images\\helicopter.png");

		editorState->pipeTexture =  backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/pipe.png");
		editorState->pipeFlippedTexture =  backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/pipeRotated.png");

		editorState->backgroundTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/background_layer_1.png");//backgroundCastles.png");

		editorState->coinTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../src/images/coin.png");

		editorState->gameMode = PLAY_MODE;

		editorState->movingCamera = true;

		//NOTE: Init all animations for game
		easyAnimation_initAnimation(&editorState->fireballIdleAnimation, "fireball_idle");

		loadImageStrip(&editorState->fireballIdleAnimation, backendRenderer, "../src/images/fireball.png", 64);

		loadImageStripXY(&editorState->playerIdleAnimation, backendRenderer, "../src/images/char_blue.png", 56, 56, 0, 6);
		loadImageStripXY(&editorState->playerRunAnimation, backendRenderer, "../src/images/char_blue.png", 56, 56, 2, 8);
		loadImageStripXY(&editorState->playerAttackAnimation, backendRenderer, "../src/images/char_blue.png", 56, 56, 1, 6);
		loadImageStripXY(&editorState->playerJumpAnimation, backendRenderer, "../src/images/char_blue.png", 56, 56, 3, 16);
		loadImageStripXY(&editorState->playerHurtAnimation, backendRenderer, "../src/images/char_blue.png", 56, 56, 5, 3);
		loadImageStripXY(&editorState->playerDieAnimation, backendRenderer, "../src/images/char_blue.png", 56, 56, 5, 12);
		loadImageStripXY(&editorState->playerFallingAnimation, backendRenderer, "../src/images/char_blue.png", 56, 56, 4, 1);

		loadImageStrip(&editorState->skeletonIdleAnimation, backendRenderer, "../src/images/skeleton/SIdle_4.png", 150);
		loadImageStrip(&editorState->skeletonRunAnimation, backendRenderer, "../src/images/skeleton/SWalk_4.png", 150);
		loadImageStrip(&editorState->skeletonAttackAnimation, backendRenderer, "../src/images/skeleton/SAttack_8.png", 150);
		loadImageStrip(&editorState->skeletonHurtAnimation, backendRenderer, "../src/images/skeleton/SHit_4.png", 150);
		loadImageStrip(&editorState->skeletonDieAnimation, backendRenderer, "../src/images/skeleton/SDeath_4.png", 150);
		loadImageStrip(&editorState->skeletonShieldAnimation, backendRenderer, "../src/images/skeleton/SShield_4.png", 150);

		/////////////

		int tileCount = 0;
		int countX = 0;
		int countY = 0;
		Texture ** tiles = loadTileSet(backendRenderer, "../src/images/Tileset.png", 32, 32, &global_long_term_arena, &tileCount, &countX, &countY);

		editorState->swampTileSet = buildTileSet(tiles, tileCount, TILE_SET_SWAMP, countX, countY, 32, 32);

		editorState->coinsGot = initResizeArray(int);

		addPlayerEntity(editorState);

		Entity *e = addSkeletonEntity(editorState);

		e->pos.x = -3;

	#if DEBUG_BUILD
		DEBUG_runUnitTests(editorState);
	#endif
}