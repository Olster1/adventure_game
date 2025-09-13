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

void initBuildingTextures(GameState *gameState) {
	gameState->houseTexture =  textureAtlas_getItemAsTexture(&gameState->textureAtlas, "house.png");
	easyAnimation_pushFrame(&gameState->houseAnimation, &gameState->houseTexture);

	gameState->castleTexture =  textureAtlas_getItemAsTexture(&gameState->textureAtlas, "castle.png");
	easyAnimation_pushFrame(&gameState->castleAnimation, &gameState->castleTexture);

	gameState->castleBurntTexture =  textureAtlas_getItemAsTexture(&gameState->textureAtlas, "castle_burnt.png");
	easyAnimation_pushFrame(&gameState->castleBurntAnimation, &gameState->castleBurntTexture);

	gameState->houseBurntTexture =  textureAtlas_getItemAsTexture(&gameState->textureAtlas, "house_burnt.png");
	easyAnimation_pushFrame(&gameState->houseBurntAnimation, &gameState->houseBurntTexture);

	gameState->towerTexture =  textureAtlas_getItemAsTexture(&gameState->textureAtlas, "tower.png");
	easyAnimation_pushFrame(&gameState->towerAnimation, &gameState->towerTexture);

	gameState->towerBurntTexture =  textureAtlas_getItemAsTexture(&gameState->textureAtlas, "tower_burnt.png");
	easyAnimation_pushFrame(&gameState->towerBurntAnimation, &gameState->towerBurntTexture);

	gameState->goblinHutTexture =  textureAtlas_getItemAsTexture(&gameState->textureAtlas, "goblinHut.png");
	easyAnimation_pushFrame(&gameState->goblinHutAnimation, &gameState->goblinHutTexture);

	gameState->goblinHutBurntTexture =  textureAtlas_getItemAsTexture(&gameState->textureAtlas, "goblinHut_burnt.png");
	easyAnimation_pushFrame(&gameState->goblinHutBurntAnimation, &gameState->goblinHutBurntTexture);

	gameState->goblinTowerBurntTexture =  textureAtlas_getItemAsTexture(&gameState->textureAtlas, "goblinTowerBurnt.png");
	easyAnimation_pushFrame(&gameState->goblinTowerBurntAnimation, &gameState->goblinTowerBurntTexture);

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

		gameState->textureAtlas = readTextureAtlas("../images/texture_atlas.json", "../images/texture_atlas.png");
		gameState->bannerTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../images/ui/banner.png");
		gameState->selectTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../images/ui/select.png");
		gameState->shadowUiTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../images/ui/shadow.png");
		gameState->kLogoText = backendRenderer_loadFromFileToGPU(backendRenderer, "../images/ui/kLogo.png");
		gameState->gLogoText = backendRenderer_loadFromFileToGPU(backendRenderer, "../images/ui/gLogo.png");
		gameState->blueText = backendRenderer_loadFromFileToGPU(backendRenderer, "../images/ui/b.png");
		gameState->buttonTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "../images/ui/button.png");
		gameState->redText = backendRenderer_loadFromFileToGPU(backendRenderer, "../images/ui/r.png");
		gameState->selectImage = backendRenderer_loadFromFileToGPU(backendRenderer, "../images/ui/select.png");
		gameState->cloudText[0] = textureAtlas_getItem(&gameState->textureAtlas, "cloud.png");
		gameState->cloudText[1] = textureAtlas_getItem(&gameState->textureAtlas, "cloud2.png");
		gameState->cloudText[2] = textureAtlas_getItem(&gameState->textureAtlas, "cloud3.png");
		gameState->treeTexture = textureAtlas_getItemAsTexture(&gameState->textureAtlas, "tree.png");
		gameState->stumpTexture = textureAtlas_getItemAsTexture(&gameState->textureAtlas, "stump.png");
		gameState->logTexture = textureAtlas_getItemAsTexture(&gameState->textureAtlas, "log.png");

		gameState->splatTexture = textureAtlas_getItemAsTexture(&gameState->textureAtlas, "exclamation1.png");
		gameState->smokeTexture =  textureAtlas_getItemAsTexture(&gameState->textureAtlas, "flame.png");

		gameState->arrows[0] = textureAtlas_getItemAsTexture(&gameState->textureAtlas, "arrowRight.png");
		gameState->arrows[1] = textureAtlas_getItemAsTexture(&gameState->textureAtlas, "arrowUp.png");
		gameState->arrows[2] = textureAtlas_getItemAsTexture(&gameState->textureAtlas, "arrowLeft.png");
		gameState->arrows[3] = textureAtlas_getItemAsTexture(&gameState->textureAtlas, "arrowDown.png");
		gameState->arrows[4] = textureAtlas_getItemAsTexture(&gameState->textureAtlas, "routeFinish.png");

		gameState->swordUiTexture = textureAtlas_getItemAsTexture(&gameState->textureAtlas, "sword.png");
		gameState->axeUiTexture = textureAtlas_getItemAsTexture(&gameState->textureAtlas, "axe.png");

		initBuildingTextures(gameState);
		loadImageStripFromAtlas(&gameState->goblinTowerAnimation, backendRenderer, &gameState->textureAtlas, textureAtlas_getItem(&gameState->textureAtlas, "goblinTower.png"), 256);

		gameState->selectedColor = make_float4(1, 1, 1, 1);
		createAOOffsets(gameState);
		gameState->gameMode = PLAY_MODE;
		gameState->cameraFollowPlayer = true;
		//TODO: Probably save this each time we leave the app
		gameState->zoomLevel = 1.8f;

		// gameState->potPlantAnimations = loadEntityAnimations(gameState, backendRenderer, "pot_plant", 16);

		//NOTE: Init all animations for game

		loadImageStrip(&gameState->animationState.waterRocks[0], backendRenderer, "../images/Rocks_03.png", 128);
		loadImageStrip(&gameState->animationState.waterAnimation, backendRenderer, "../images/foam.png", 192);

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

		loadImageStripXY(&gameState->knightAnimations.idle, backendRenderer, "../images/factions/knight.png", 192, 192, 6, 0, 0);
		loadImageStripXY(&gameState->knightAnimations.run, backendRenderer, "../images/factions/knight.png", 192, 192, 6, 0, 1);
		loadImageStripXY(&gameState->knightAnimations.attackSide, backendRenderer, "../images/factions/knight.png", 192, 192, 12, 0, 2);
		loadImageStripXY(&gameState->knightAnimations.attackDown, backendRenderer, "../images/factions/knight.png", 192, 192, 12, 0, 4);
		loadImageStripXY(&gameState->knightAnimations.attackUp, backendRenderer, "../images/factions/knight.png", 192, 192, 12, 0, 6);

		loadImageStripXY(&gameState->peasantAnimations.idle, backendRenderer, "../images/factions/peasant.png", 192, 192, 6, 0, 0);
		loadImageStripXY(&gameState->peasantAnimations.run, backendRenderer, "../images/factions/peasant.png", 192, 192, 6, 0, 1);
		loadImageStripXY(&gameState->peasantAnimations.work, backendRenderer, "../images/factions/peasant.png", 192, 192, 6, 0, 2);
		loadImageStripXY(&gameState->peasantAnimations.attackSide, backendRenderer, "../images/factions/peasant.png", 192, 192, 6, 0, 3);
		loadImageStripXY(&gameState->peasantAnimations.scared, backendRenderer, "../images/factions/peasant.png", 192, 192, 6, 0, 4);

		loadImageStripXY(&gameState->archerAnimations.idle, backendRenderer, "../images/factions/archer.png", 192, 192, 6, 0, 0);
		loadImageStripXY(&gameState->archerAnimations.run, backendRenderer, "../images/factions/archer.png", 192, 192, 6, 1, 0);
		loadImageStripXY(&gameState->archerAnimations.attackUp, backendRenderer, "../images/factions/archer.png", 192, 192, 8, 2, 0);

		loadImageStripXY(&gameState->goblinAnimations.idle, backendRenderer, "../images/factions/goblin.png", 192, 192, 7, 0, 0);
		loadImageStripXY(&gameState->goblinAnimations.run, backendRenderer, "../images/factions/goblin.png", 192, 192, 6, 0, 1);
		loadImageStripXY(&gameState->goblinAnimations.attackSide, backendRenderer, "../images/factions/goblin.png", 192, 192, 6, 0, 2);
		loadImageStripXY(&gameState->goblinAnimations.attackDown, backendRenderer, "../images/factions/goblin.png", 192, 192, 6, 0, 3);
		loadImageStripXY(&gameState->goblinAnimations.attackUp, backendRenderer, "../images/factions/goblin.png", 192, 192, 6, 0, 4);
		loadImageStripXY(&gameState->goblinAnimations.hurt, backendRenderer, "../images/factions/goblin_hit.png", 192, 192, 4, 0, 0);

		loadImageStripXY(&gameState->tntAnimations.idle, backendRenderer, "../images/factions/tnt.png", 192, 192, 6, 0, 0);
		loadImageStripXY(&gameState->tntAnimations.run, backendRenderer, "../images/factions/tnt.png", 192, 192, 6, 1, 0);
		loadImageStripXY(&gameState->tntAnimations.attackSide, backendRenderer, "../images/factions/tnt.png", 192, 192, 7, 2, 0);

		initAudioSpec(&gameState->audioSpec, 44100);
		initAudio(&gameState->audioSpec);
		loadOggVorbisFile(&gameState->soundAssets.woodChopSounds[0], "../sounds/woodChop1.ogg", &gameState->audioSpec);
		loadOggVorbisFile(&gameState->soundAssets.woodChopSounds[1], "../sounds/woodChop2.ogg", &gameState->audioSpec);
		loadOggVorbisFile(&gameState->soundAssets.medieval1, "../sounds/Medieval2.ogg", &gameState->audioSpec);
		loadOggVorbisFile(&gameState->soundAssets.footsteps[0], "../sounds/footstep_1.ogg", &gameState->audioSpec);
		loadOggVorbisFile(&gameState->soundAssets.footsteps[1], "../sounds/footstep_2.ogg", &gameState->audioSpec);
		loadOggVorbisFile(&gameState->soundAssets.footsteps[2], "../sounds/footstep_3.ogg", &gameState->audioSpec);
		loadOggVorbisFile(&gameState->soundAssets.swordAttack[0], "../sounds/sword1.ogg", &gameState->audioSpec);
		loadOggVorbisFile(&gameState->soundAssets.swordAttack[1], "../sounds/sword2.ogg", &gameState->audioSpec);
		
		playSound(&gameState->soundAssets.medieval1)->volume = 0.1f;
		
		/////////////
		{
			int tileCount = 0;
			int countX = 0;
			int countY = 0;
			Texture ** tiles = loadTileSet(backendRenderer, "../images/tilemap.png", 64, 64, &global_long_term_arena, &tileCount, &countX, &countY);
			gameState->sandTileSet = buildTileSet(tiles, tileCount, TILE_SET_SAND, countX, countY, 64, 64);
		}

	#if DEBUG_BUILD
		DEBUG_runUnitTests(gameState);
	#endif
}