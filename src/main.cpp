#include <time.h>
#include <stdlib.h>

#include "wl_memory.h"
#include "save_settings.cpp"
#include "file_helper.cpp"
#include "lex_utf8.h"
#include "color.cpp"
#include "font.cpp"
#include "perlin.c"
#include "animation.c"
#include "resize_array.cpp"
// #include "transform.cpp"
#include "easy_ai.h"
#include "entity.h"
#include "tileMap.h"
#include "editor_gui.h"
#include "game_state.h"
#include "assets.cpp"
#include "tileMap.cpp"
#include "editor_gui.cpp"
#include "entity.cpp"
#include "collision.cpp"
#include "entity_update.cpp"

static void drawGrid(EditorState *editorState) {
	float zPos = 10;
	float4 gridColor = make_float4(0.5f, 0.5f, 0.5f, 1.0f);

	Renderer *renderer = &editorState->renderer;

	float3 snappedCamera = editorState->cameraPos;
	snappedCamera.x = (int)snappedCamera.x;
	snappedCamera.y = (int)snappedCamera.y;

	//NOTE: Draw grid
	pushShader(renderer, &lineShader);

	int gridSize = 30;
	int halfGridSize = (int)(0.5f*gridSize);
		
	for(int x = -halfGridSize; x < halfGridSize; ++x) {
		float defaultY = halfGridSize;
		float3 posA = make_float3(x, -defaultY, zPos);
		float3 posB = make_float3(x, defaultY, zPos);

		posA = minus_float3(plus_float3(posA, snappedCamera), editorState->cameraPos);
		posB = minus_float3(plus_float3(posB, snappedCamera), editorState->cameraPos);
		
		pushLine(renderer, posA, posB, gridColor);
	}

	for(int y = -halfGridSize; y < halfGridSize; ++y) {
		float defaultX = halfGridSize;
		float3 posA = make_float3(-defaultX, y, zPos);
		float3 posB = make_float3(defaultX, y, zPos);

		posA = minus_float3(plus_float3(posA, snappedCamera), editorState->cameraPos);
		posB = minus_float3(plus_float3(posB, snappedCamera), editorState->cameraPos);
		
		pushLine(renderer, posA, posB, gridColor);
	}
}

#if DEBUG_BUILD
#include "unit_tests.cpp"
#endif

#include "gameState.cpp"
#include "debug.cpp"

static EditorState *updateEditor(BackendRenderer *backendRenderer, float dt, float windowWidth, float windowHeight, bool should_save_settings, char *save_file_location_utf8_only_use_on_inititalize, Settings_To_Save save_settings_only_use_on_inititalize) {
	EditorState *editorState = (EditorState *)global_platform.permanent_storage;
	assert(sizeof(EditorState) < global_platform.permanent_storage_size);
	if(!editorState->initialized) {
		initGameState(editorState, backendRenderer);
	} else {
		releaseMemoryMark(&global_perFrameArenaMark);
		global_perFrameArenaMark = takeMemoryMark(&globalPerFrameArena);
	}

	if(global_platformInput.keyStates[PLATFORM_KEY_1].pressedCount > 0) {
		editorState->gameMode = PLAY_MODE;
	} else if(global_platformInput.keyStates[PLATFORM_KEY_2].pressedCount > 0) {
		editorState->gameMode = TILE_MODE;
	} else if(global_platformInput.keyStates[PLATFORM_KEY_3].pressedCount > 0) {
		editorState->gameMode = SELECT_ENTITY_MODE;
	} else if(global_platformInput.keyStates[PLATFORM_KEY_4].pressedCount > 0) {
		editorState->gameMode = A_STAR_MODE;
	}

	if(global_platformInput.keyStates[PLATFORM_KEY_5].pressedCount > 0) {
		editorState->draw_debug_memory_stats = !editorState->draw_debug_memory_stats;
	}

	Renderer *renderer = &editorState->renderer;

	//NOTE: Clear the renderer out so we can start again
	clearRenderer(renderer);
	clearGameStatePerFrameValues(editorState);

	//NOTE: Get pointer to player - always at slot zero
	editorState->player = &editorState->entities[0];

	pushViewport(renderer, make_float4(0, 0, 0, 0));
	renderer_defaultScissors(renderer, windowWidth, windowHeight);
	pushClearColor(renderer, make_float4(1, 0.9, 0.9, 1));

	float2 mouse_point_top_left_origin = make_float2(global_platformInput.mouseX, global_platformInput.mouseY);	
	float2 mouse_point_top_left_origin_01 = make_float2(global_platformInput.mouseX / windowWidth, global_platformInput.mouseY / windowHeight);

	float fauxDimensionY = 1000;
	float fauxDimensionX = fauxDimensionY * (windowWidth/windowHeight);

	float2 cameraOffset = make_float2(0, 0);

	if(editorState->shakeTimer >= 0) {

		float offset = perlin1d(editorState->shakeTimer, 40, 3)*(1.0f - editorState->shakeTimer);
		//NOTE: Update the camera position offset
		cameraOffset.x = offset;
		cameraOffset.y = offset;

		editorState->shakeTimer += dt;

		if(editorState->shakeTimer >= 1.0f) {
			editorState->shakeTimer = -1.0f;
		}
	}


	float16 orthoMatrix = make_ortho_matrix_origin_center(fauxDimensionX, fauxDimensionY, MATH_3D_NEAR_CLIP_PlANE, MATH_3D_FAR_CLIP_PlANE);
	pushMatrix(renderer, orthoMatrix);
	pushShader(renderer, &rectOutlineShader);


	float rotationPower = 1;
	if(global_platformInput.keyStates[PLATFORM_KEY_UP].pressedCount > 0) {
		// editorState->player.pos.y += 1.0f;
		editorState->player->velocity.y = editorState->player->speed;
		editorState->player->targetRotation = 0.5f*HALF_PI32;
		rotationPower = 15.0f;

		editorState->hasInteratedYet = true;

	} else if(editorState->hasInteratedYet) {
		editorState->player->velocity.y -= 0.3f;
		editorState->player->targetRotation = -0.5f*HALF_PI32;
	}

	bool playerMoved = false;

	

	if(global_platformInput.keyStates[PLATFORM_KEY_LEFT].isDown && editorState->player->animationController.lastAnimationOn != &editorState->playerAttackAnimation) {
		editorState->player->velocity.x = -editorState->player->speed;
		editorState->hasInteratedYet = true;

		editorState->player->spriteFlipped = true;

		playerMoved = true;

		//NOTE: RUN ANIAMTION
		if(editorState->player->animationController.lastAnimationOn != &editorState->playerJumpAnimation && editorState->player->animationController.lastAnimationOn != &editorState->playerRunAnimation)  {
			easyAnimation_emptyAnimationContoller(&editorState->player->animationController, &editorState->animationItemFreeListPtr);
			easyAnimation_addAnimationToController(&editorState->player->animationController, &editorState->animationItemFreeListPtr, &editorState->playerRunAnimation, 0.08f);	
		}
	}

	if(global_platformInput.keyStates[PLATFORM_KEY_RIGHT].isDown && editorState->player->animationController.lastAnimationOn != &editorState->playerAttackAnimation) {
		editorState->player->velocity.x = editorState->player->speed;
		editorState->hasInteratedYet = true;

		editorState->player->spriteFlipped = false;

		playerMoved = true;

		if(editorState->player->animationController.lastAnimationOn != &editorState->playerJumpAnimation && editorState->player->animationController.lastAnimationOn != &editorState->playerRunAnimation)  {
			easyAnimation_emptyAnimationContoller(&editorState->player->animationController, &editorState->animationItemFreeListPtr);
			easyAnimation_addAnimationToController(&editorState->player->animationController, &editorState->animationItemFreeListPtr, &editorState->playerRunAnimation, 0.08f);	
		}
	} 

	//NOTE: IDLE ANIMATION
	if(!playerMoved && editorState->player->animationController.lastAnimationOn != &editorState->playerJumpAnimation && editorState->player->animationController.lastAnimationOn != &editorState->playerIdleAnimation && editorState->player->animationController.lastAnimationOn != &editorState->playerAttackAnimation)  {
		easyAnimation_emptyAnimationContoller(&editorState->player->animationController, &editorState->animationItemFreeListPtr);
		easyAnimation_addAnimationToController(&editorState->player->animationController, &editorState->animationItemFreeListPtr, &editorState->playerIdleAnimation, 0.08f);	
	}
	
	//NOTE: JUMP ANIMATION
	if(global_platformInput.keyStates[PLATFORM_KEY_SPACE].pressedCount > 0 && editorState->player->grounded) {
		editorState->player->velocity.y = 10.0f;
		easyAnimation_emptyAnimationContoller(&editorState->player->animationController, &editorState->animationItemFreeListPtr);
		easyAnimation_addAnimationToController(&editorState->player->animationController, &editorState->animationItemFreeListPtr, &editorState->playerJumpAnimation, 0.08f);	
		easyAnimation_addAnimationToController(&editorState->player->animationController, &editorState->animationItemFreeListPtr, &editorState->playerFallingAnimation, 0.08f);	
	} else {
		if(editorState->player->animationController.lastAnimationOn == &editorState->playerJumpAnimation && editorState->player->grounded)  {
			//NOTE: LANDED ANIMATION
			easyAnimation_emptyAnimationContoller(&editorState->player->animationController, &editorState->animationItemFreeListPtr);
			easyAnimation_addAnimationToController(&editorState->player->animationController, &editorState->animationItemFreeListPtr, &editorState->playerIdleAnimation, 0.08f);	
		} else if(editorState->player->animationController.lastAnimationOn != &editorState->playerJumpAnimation && !editorState->player->grounded && editorState->player->animationController.lastAnimationOn != &editorState->playerAttackAnimation) {
			//NOTE: FALLING ANIMATION
			easyAnimation_emptyAnimationContoller(&editorState->player->animationController, &editorState->animationItemFreeListPtr);
			easyAnimation_addAnimationToController(&editorState->player->animationController, &editorState->animationItemFreeListPtr, &editorState->playerFallingAnimation, 0.08f);	

		}
	}

	if(global_platformInput.keyStates[PLATFORM_KEY_X].pressedCount > 0 && editorState->player->animationController.lastAnimationOn != &editorState->playerAttackAnimation) {
		//NOTE: SpriteFlipped = false
		editorState->player->velocity.x = editorState->player->speed;
		editorState->player->colliders[ATTACK_COLLIDER_INDEX].offset = make_float3(0.5f, 0, 0);

		if(editorState->player->spriteFlipped) {
			editorState->player->velocity.x = -editorState->player->speed;
			editorState->player->colliders[ATTACK_COLLIDER_INDEX].offset = make_float3(-0.5f, 0, 0);
		} 

		//NOTE: Make active
		editorState->player->colliders[ATTACK_COLLIDER_INDEX].flags |= ENTITY_ACTIVE;

		playerMoved = true;

		easyAnimation_emptyAnimationContoller(&editorState->player->animationController, &editorState->animationItemFreeListPtr);
		easyAnimation_addAnimationToController(&editorState->player->animationController, &editorState->animationItemFreeListPtr, &editorState->playerAttackAnimation, 0.08f);	
		easyAnimation_addAnimationToController(&editorState->player->animationController, &editorState->animationItemFreeListPtr, &editorState->playerIdleAnimation, 0.08f);	

	} 

	// editorState->player->pos.xy = plus_float2(scale_float2(dt, editorState->player->velocity.xy),  editorState->player->pos.xy);
	// editorState->player->rotation = lerp(editorState->player.rotation, editorState->player.targetRotation, make_lerpTValue(rotationPower*0.05f)); 

	if(editorState->movingCamera) {
		editorState->cameraPos.x = lerp(editorState->cameraPos.x, editorState->player->pos.x + cameraOffset.x, make_lerpTValue(0.4f));
		editorState->cameraPos.y = lerp(editorState->cameraPos.y, editorState->player->pos.y + cameraOffset.y, make_lerpTValue(0.4f));
	}

	pushShader(renderer, &textureShader);

	//NOTE: Background texture
	assert(editorState->backgroundTexture.handle);
	pushTexture(renderer, editorState->backgroundTexture.handle, make_float3(0, 0, 10), make_float2(fauxDimensionX, fauxDimensionY), make_float4(1, 1, 1, 1), make_float4(0, 0, 1, 1));
	// return editorState;
	
	editorState->planeSizeY = (windowHeight / windowWidth) * editorState->planeSizeX;
	float16 fovMatrix = make_ortho_matrix_origin_center(editorState->planeSizeX, editorState->planeSizeY, MATH_3D_NEAR_CLIP_PlANE, MATH_3D_FAR_CLIP_PlANE);
	// float16 fovMatrix = make_perspective_matrix_origin_center(60.0f, MATH_3D_NEAR_CLIP_PlANE, MATH_3D_FAR_CLIP_PlANE, windowWidth / windowHeight);

	pushMatrix(renderer, fovMatrix);

	drawGrid(editorState);

	updateAndRenderEntities(editorState, renderer, dt, fovMatrix, windowWidth, windowHeight);


#if DEBUG_BUILD
	drawDebugAndEditorText(editorState, renderer, fauxDimensionX, fauxDimensionY, windowWidth, windowHeight, dt, fovMatrix);
#endif

	return editorState;

}