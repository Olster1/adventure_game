
void updateAndRenderEntities(EditorState *editorState, Renderer *renderer, float dt, float16 fovMatrix, float windowWidth, float windowHeight){
    	//NOTE: Push all lights for the renderer to use
	pushAllEntityLights(editorState, dt);

	pushShader(renderer, &pixelArtShader);
	pushMatrix(renderer, fovMatrix);

	renderTileMap(editorState, renderer);

	//NOTE: Collision code - fill all colliders with info and move entities
	updateEntityCollisions(editorState, dt);

	// if(global_platformInput.keyStates[PLATFORM_MOUSE_LEFT_BUTTON].pressedCount > 0 && editorState->selectedEntityId) {
	// 	editorGui_clearInteraction(&editorState->editorGuiState);
	// 	editorState->selectedEntityId = 0;
	// } 


	//NOTE: Gameplay code
	for(int i = 0; i < editorState->entityCount; ++i) {
		Entity *e = &editorState->entities[i];

		// e->pos.xy = plus_float2(scale_float2(dt, e->velocity.xy),  e->pos.xy);
		// e->rotation = lerp(e->rotation, e->targetRotation, make_lerpTValue(rotationPower*0.05f)); 
		

		if(e->flags & ENTITY_ACTIVE) {
			#if DEBUG_BUILD
			updateEntitySelection(editorState, e, windowWidth, windowHeight, renderer, fovMatrix);
			#endif
			updateEntity(editorState, renderer, e, dt, fovMatrix);
			renderEntity(editorState, renderer, e, fovMatrix, dt);
		}
	}
}