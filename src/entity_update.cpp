
void updateAndRenderEntities(GameState *gameState, Renderer *renderer, float dt, float16 fovMatrix, float windowWidth, float windowHeight){
	DEBUG_TIME_BLOCK();

    //NOTE: Push all lights for the renderer to use
	pushAllEntityLights(gameState, dt);

	pushMatrix(renderer, fovMatrix);

	renderTileMap(gameState, renderer, dt);

	//NOTE: Collision code - fill all colliders with info and move entities
	updateEntityCollisions(gameState, dt);

	pushShader(renderer, &pixelArtShader);
	//NOTE: Gameplay code
	for(int i = 0; i < gameState->entityCount; ++i) {
		Entity *e = &gameState->entities[i];

		// e->pos.xy = plus_float2(scale_float2(dt, e->velocity.xy),  e->pos.xy);
		// e->rotation = lerp(e->rotation, e->targetRotation, make_lerpTValue(rotationPower*0.05f)); 

		if(e->flags & ENTITY_ACTIVE) {
			#if DEBUG_BUILD
			updateEntitySelection(gameState, e, windowWidth, windowHeight, renderer, fovMatrix);
			#endif
			updateEntity(gameState, renderer, e, dt, fovMatrix);
			renderEntity(gameState, renderer, e, fovMatrix, dt);
		}
	}

	drawClouds(gameState, renderer, dt);
	
}