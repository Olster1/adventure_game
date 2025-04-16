
float3 getMouseWorldP(GameState *state, float windowWidth, float windowHeight) {
	float2 mouseP = make_float2(global_platformInput.mouseX, windowHeight - global_platformInput.mouseY);
    float2 mouseP_01 = make_float2(mouseP.x / windowWidth, mouseP.y / windowHeight);

	float worldX = lerp(-0.5f*state->planeSizeX*state->zoomLevel, 0.5f*state->planeSizeX*state->zoomLevel, make_lerpTValue(mouseP_01.x));
    float worldY = lerp(-0.5f*state->planeSizeY*state->zoomLevel, 0.5f*state->planeSizeY*state->zoomLevel, make_lerpTValue(mouseP_01.y));

    worldX += state->cameraPos.x;
    worldY += state->cameraPos.y;

	float3 tileP = convertRealWorldToBlockCoords(make_float3(worldX, worldY, 0));
	float worldZ = getMapHeight(tileP.x, tileP.y);

	bool wasStandAlone = true;

	float3 oneIn = convertRealWorldToBlockCoords(make_float3(worldX, worldY - 1, 0));
	float oneInHeight = getMapHeight(oneIn.x, oneIn.y);

	//NOTE: Check now if the worldZ is what it says it is
	{
		float3 twoIn = convertRealWorldToBlockCoords(make_float3(worldX, worldY - 2, 0));
		float twoInHeight = getMapHeight(twoIn.x, twoIn.y);

		if(twoInHeight == 2) {
			//NOTE: Height is actually 2 because of our werid rendering perspective
			worldZ = 2;
			worldY -= 2;
			wasStandAlone = false;
		} else {
			

			if(oneInHeight == 1) {
				//NOTE: Height is actually 1 because of our werid rendering perspective
				worldZ = 1;
				worldY -= 1;
				wasStandAlone = false;
			}
		}
	}

	if(wasStandAlone && worldZ > 0) {
		if(worldZ == 1) {
			if(oneInHeight > 0) {
				tileP = convertRealWorldToBlockCoords(make_float3(worldX, worldY - 1, 0));
				worldZ = getMapHeight(tileP.x, tileP.y);
			}
		}
		worldY -= (worldZ - 1);
	}

	if(worldZ < 0) {
		//NOTE: Nothign is below 0 and we don't want to render the selectable offset
		worldZ = 0;
	}

	return make_float3(worldX, worldY, worldZ);

}

void drawSelectionHover(GameState *gameState, Renderer *renderer, float dt, float3 worldMouseP) {
	// if(gameState->selectedEntityCount > 0) 
	{

		gameState->selectHoverTimer += dt;
		float3 p = convertRealWorldToBlockCoords(worldMouseP);

		{
			float2 chunkP = getChunkPosForWorldP(worldMouseP.xy);
			Chunk *c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, chunkP.x, chunkP.y, 0, true, true);
			float3 tileP = getChunkLocalPos(p.x, p.y, p.z);
			Tile *tile = c->getTile(tileP.x, tileP.y, tileP.z);
			if(tile && !(tile->flags & TILE_FLAG_WALKABLE)) {
				gameState->selectedColor = make_float4(1, 0, 0, 1);
			}
		}

		p = getRenderWorldP(p);

		p.z = 10;

		p.x -= gameState->cameraPos.x; //NOTE: Offset for middle of the tile
		p.y -= gameState->cameraPos.y; //NOTE: Offset for middle of the tile

		//NOTE: P is now in camera space
		
		float scale = 1;//erp(0.8f, 1.2f, make_lerpTValue(sin01(5*gameState->selectHoverTimer)));
        pushTexture(renderer, gameState->selectImage.handle, p, make_float2(scale, scale), gameState->selectedColor, gameState->selectImage.uvCoords);

		float3 tileP = convertRealWorldToBlockCoords(make_float3(worldMouseP.x, worldMouseP.y, worldMouseP.z));
		 //NOTE: Draw position above player
		 char *str = easy_createString_printf(&globalPerFrameArena, "(%d %d %d)", (int)tileP.x, (int)tileP.y, (int)tileP.z);
		 pushShader(renderer, &sdfFontShader);
		 draw_text(renderer, &gameState->font, str, p.x, p.y, 0.02, make_float4(0, 0, 0, 1)); 
	 
    }
}

void updateParticlers(Renderer *renderer, GameState *gameState, ParticlerParent *parent, float dt) {
    for(int i = 0; i < parent->particlerCount; ) {
        int addend = 1;
        Particler *p = &parent->particlers[i];

        bool shouldRemove = updateParticler(renderer, p, gameState->cameraPos, dt);

        if(shouldRemove) {
            //NOTE: Move from the end
            parent->particlers[i] = parent->particlers[--parent->particlerCount];
            addend = 0;
        } 

        i += addend;
    }
    
}

void updateAndRenderEntities(GameState *gameState, Renderer *renderer, float dt, float16 fovMatrix, float windowWidth, float windowHeight){
	DEBUG_TIME_BLOCK();

	//NOTE: reset the selected color, becuase we change it in the update entity loop
	gameState->selectedColor = make_float4(1, 1, 1, 1);

    //NOTE: Push all lights for the renderer to use
	pushAllEntityLights(gameState, dt);

	pushMatrix(renderer, fovMatrix);

	renderTileMap(gameState, renderer, dt);

	//NOTE: Collision code - fill all colliders with info and move entities
	updateEntityCollisions(gameState, dt);
	pushShader(renderer, &pixelArtShader);

	float3 worldMouseP = getMouseWorldP(gameState, windowWidth, windowHeight);

	updateEntitySelection(renderer, gameState, worldMouseP.xy);

	drawTrees(gameState, renderer);

	//NOTE: Gameplay code
	for(int i = 0; i < gameState->entityCount; ++i) {
		Entity *e = &gameState->entities[i];

		if(e->flags & ENTITY_ACTIVE) {
			updateEntity(gameState, renderer, e, dt, worldMouseP);
			renderEntity(gameState, renderer, e, fovMatrix, dt);
		}
	}

	updateParticlers(renderer, gameState, &gameState->particlers, dt);

	drawSelectionHover(gameState, renderer, dt, worldMouseP);

	// drawClouds(gameState, renderer, dt);
	
}