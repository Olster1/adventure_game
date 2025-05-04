
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
		float3 worldP = convertRealWorldToBlockCoords(worldMouseP);
		float3 p = worldP;

		{
			//NOTE: See if the selection is on a tree tile
			float2 chunkP = getChunkPosForWorldP(worldMouseP.xy);
			Chunk *c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, &gameState->textureAtlas, chunkP.x, chunkP.y, 0, true, true);
			float3 tileP = getChunkLocalPos(p.x, p.y, p.z);
			Tile *tile = c->getTile(tileP.x, tileP.y, tileP.z);
			if(tile && !(tile->flags & TILE_FLAG_WALKABLE)) {
				gameState->selectedColor = make_float4(1, 0, 0, 1);
			}
		}

		p = getRenderWorldP(worldP);
		p.z = RENDER_Z;

		p.x -= gameState->cameraPos.x; //NOTE: Offset for middle of the tile
		p.y -= gameState->cameraPos.y; //NOTE: Offset for middle of the tile

		//NOTE: P is now in camera space
		
		float scale = 1;//lerp(0.8f, 1.2f, make_lerpTValue(sin01(5*gameState->selectHoverTimer)));
		float3 sortP = worldP;
		pushEntityTexture(renderer, gameState->selectImage.handle, p, make_float2(scale, scale), gameState->selectedColor, gameState->selectImage.uvCoords, getSortIndex(sortP, RENDER_LAYER_2));

		float3 tileP = convertRealWorldToBlockCoords(make_float3(worldMouseP.x, worldMouseP.y, worldMouseP.z));
		 //NOTE: Draw position above player
		 char *str = easy_createString_printf(&globalPerFrameArena, "(%d %d %d)", (int)tileP.x, (int)tileP.y, (int)tileP.z);
		 pushShader(renderer, &sdfFontShader);
		 draw_text(renderer, &gameState->font, str, p.x, p.y, 0.02, make_float4(0, 0, 0, 1)); 
	 
    }
}


//NOTE: If this function returns a positive number it means it should swap a & b, making a come after b
// int compare_by_height(const void *a, const void *b) {
// 	InstanceEntityData *pa = (InstanceEntityData *)a; 
// 	InstanceEntityData *pb = (InstanceEntityData *)b; 
int compare_by_height(InstanceEntityData *pa, InstanceEntityData *pb) {
    float a_yz = (float)pa->sortIndex.worldP.y + (float)pa->sortIndex.worldP.z;
    float b_yz = (float)pb->sortIndex.worldP.y + (float)pb->sortIndex.worldP.z;

	if (a_yz != b_yz) {
		return (b_yz - a_yz) > 0 ? 1 : -1; 
	}

	if(pa->sortIndex.worldP.z != pb->sortIndex.worldP.z) {
		return (pa->sortIndex.worldP.z - pb->sortIndex.worldP.z)  > 0 ? 1 : -1;
	}

	if (pa->sortIndex.layer != pb->sortIndex.layer) {
		return (pa->sortIndex.layer - pb->sortIndex.layer);
	}

    if (pa->sortIndex.worldP.x != pb->sortIndex.worldP.x) {
		return pa->sortIndex.worldP.x - pb->sortIndex.worldP.x;
	}
    return 0;
}

void sortAndRenderEntityQueue(Renderer *renderer) {
	DEBUG_TIME_BLOCK();

	// qsort(renderer->entityRenderData, renderer->entityRenderCount, sizeof(InstanceEntityData), compare_by_height);
	
	{
		DEBUG_TIME_BLOCK_NAMED("SORT RENDER ENTITIES");
		//NOTE: First sort the list. This is an insert sort
		for (int i = 1; i < renderer->entityRenderCount; i++) {
			InstanceEntityData temp = renderer->entityRenderData[i];
			int j = i - 1;
			while (j >= 0 && compare_by_height(&renderer->entityRenderData[j], &temp) > 0) {
				renderer->entityRenderData[j + 1] = renderer->entityRenderData[j];
				j--;
			}
			renderer->entityRenderData[j + 1] = temp;
		}
	}

	pushShader(renderer, &terrainLightingShader);
	//NOTE: Draw the list
	for(int i = 0; i < renderer->entityRenderCount; ++i) {
		InstanceEntityData *d = renderer->entityRenderData + i; 
		pushTexture(renderer, d->textureHandle, d->pos, d->scale, d->color, d->uv, d->aoMask);
	}
}

void updateParticlers(Renderer *renderer, GameState *gameState, ParticlerParent *parent, float dt) {
	if(parent->particlerCount > 0) {
		// pushBlendMode(renderer, RENDER_BLEND_MODE_ADD);
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
		// pushBlendMode(renderer, RENDER_BLEND_MODE_DEFAULT);
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

	sortAndRenderEntityQueue(renderer);

	drawClouds(gameState, renderer, dt);
	
}