#include "wl_memory.h"
#include "save_settings.cpp"
#include "file_helper.cpp"
#include "lex_utf8.h"
#include "color.cpp"
#include "font.cpp"
#include "perlin.c"
#define EASY_ANIMATION_2D_IMPLEMENTATION 1
#include "animation.c"
#include "resize_array.cpp"
// #include "transform.cpp"
#include "easy_ai.h"
#include "entity.h"
#include "tileMap.h"
#include "editor_gui.h"

#include <time.h>
#include <stdlib.h>



inline char *easy_createString_printf(Memory_Arena *arena, char *formatString, ...) {

    va_list args;
    va_start(args, formatString);

    char bogus[1];
    int stringLengthToAlloc = vsnprintf(bogus, 1, formatString, args) + 1; //for null terminator, just to be sure
    
    char *strArray = pushArray(arena, stringLengthToAlloc, char);

    vsnprintf(strArray, stringLengthToAlloc, formatString, args); 

    va_end(args);

    return strArray;
}


#define MAX_WINDOW_COUNT 8
#define MAX_BUFFER_COUNT 256 //TODO: Allow user to open unlimited buffers


typedef enum {
	MODE_EDIT_BUFFER,
} EditorMode;

struct CollisionRect {
	Rect2f rect;

	CollisionRect(Rect2f rect) {

	}
};

#include "game_state.h"
#include "assets.cpp"
#include "tileMap.cpp"
#include "editor_gui.cpp"
#include "entity.cpp"
#include "collision.cpp"


static void DEBUG_draw_stats(EditorState *editorState, Renderer *renderer, Font *font, float windowWidth, float windowHeight, float dt) {

	float16 orthoMatrix = make_ortho_matrix_top_left_corner(windowWidth, windowHeight, MATH_3D_NEAR_CLIP_PlANE, MATH_3D_FAR_CLIP_PlANE);
	pushMatrix(renderer, orthoMatrix);

	//NOTE: Draw the backing
	pushShader(renderer, &textureShader);
	float2 scale = make_float2(200, 400);
	// pushTexture(renderer, global_white_texture, make_float3(100, -200, 1.0f), scale, make_float4(0.3f, 0.3f, 0.3f, 1), make_float4(0, 0, 1, 1));
	///////////////////////////


	//NOTE: Draw the name of the file
	pushShader(renderer, &sdfFontShader);
		
	float fontScale = 0.6f;
	float4 color = make_float4(1, 1, 1, 1);

	float xAt = 0;
	float yAt = -1.5f*font->fontHeight*fontScale;

	float spacing = font->fontHeight*fontScale;

#define DEBUG_draw_stats_MACRO(title, size, draw_kilobytes) { char *name_str = 0; if(draw_kilobytes) { name_str = easy_createString_printf(&globalPerFrameArena, "%s  %d %dkilobytes", title, size, size/1000); } else { name_str = easy_createString_printf(&globalPerFrameArena, "%s  %d", title, size); } draw_text(renderer, font, name_str, xAt, yAt, fontScale, color); yAt -= spacing; }
#define DEBUG_draw_stats_FLOAT_MACRO(title, f0, f1) { char *name_str = 0; name_str = easy_createString_printf(&globalPerFrameArena, "%s  %f  %f", title, f0, f1); draw_text(renderer, font, name_str, xAt, yAt, fontScale, color); yAt -= spacing; }
	
	DEBUG_draw_stats_MACRO("Total Heap Allocated", global_debug_stats.total_heap_allocated, true);
	DEBUG_draw_stats_MACRO("Total Virtual Allocated", global_debug_stats.total_virtual_alloc, true);
	DEBUG_draw_stats_MACRO("Render Command Count", global_debug_stats.render_command_count, false);
	DEBUG_draw_stats_MACRO("Draw Count", global_debug_stats.draw_call_count, false);
	DEBUG_draw_stats_MACRO("Heap Block Count ", global_debug_stats.memory_block_count, false);
	DEBUG_draw_stats_MACRO("Per Frame Arena Total Size", DEBUG_get_total_arena_size(&globalPerFrameArena), true);

	// WL_Window *w = &editorState->windows[editorState->active_window_index];
	// DEBUG_draw_stats_FLOAT_MACRO("Start at: ", editorState->selectable_state.start_pos.x, editorState->selectable_state.start_pos.y);
	// DEBUG_draw_stats_FLOAT_MACRO("Target Scroll: ", w->scroll_target_pos.x, w->scroll_target_pos.y);

	DEBUG_draw_stats_FLOAT_MACRO("mouse scroll x ", global_platformInput.mouseX / windowWidth, global_platformInput.mouseY / windowHeight);
	DEBUG_draw_stats_FLOAT_MACRO("dt for frame ", dt, dt);

}

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

static void shakeCamera(EditorState *editorState) {
	if(editorState->shakeTimer < 0 || editorState->shakeTimer > 0.5f) {
		editorState->shakeTimer = 0;
	}
}

static EditorState *updateEditor(BackendRenderer *backendRenderer, float dt, float windowWidth, float windowHeight, bool should_save_settings, char *save_file_location_utf8_only_use_on_inititalize, Settings_To_Save save_settings_only_use_on_inititalize) {
	EditorState *editorState = (EditorState *)global_platform.permanent_storage;
	assert(sizeof(EditorState) < global_platform.permanent_storage_size);
	if(!editorState->initialized) {

		editorState->initialized = true;

		initRenderer(&editorState->renderer);

		editorState->mode = MODE_EDIT_BUFFER;

		#if DEBUG_BUILD
		editorState->font = initFont("..\\fonts\\liberation-mono.ttf");
		#else
		editorState->font = initFont(".\\fonts\\liberation-mono.ttf");
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

		editorState->pipeTexture =  backendRenderer_loadFromFileToGPU(backendRenderer, "..\\src\\images\\pipe.png");
		editorState->pipeFlippedTexture =  backendRenderer_loadFromFileToGPU(backendRenderer, "..\\src\\images\\pipeRotated.png");

		editorState->backgroundTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "..\\src\\images\\background_layer_1.png");//backgroundCastles.png");

		editorState->coinTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "..\\src\\images\\coin.png");

		editorState->gameMode = PLAY_MODE;

		editorState->movingCamera = true;

		//NOTE: Init all animations for game
		easyAnimation_initAnimation(&editorState->fireballIdleAnimation, "fireball_idle");

		loadImageStrip(&editorState->fireballIdleAnimation, backendRenderer, "..\\src\\images\\fireball.png", 64);

		loadImageStripXY(&editorState->playerIdleAnimation, backendRenderer, "..\\src\\images\\char_blue.png", 56, 56, 0, 6);
		loadImageStripXY(&editorState->playerRunAnimation, backendRenderer, "..\\src\\images\\char_blue.png", 56, 56, 2, 8);
		loadImageStripXY(&editorState->playerAttackAnimation, backendRenderer, "..\\src\\images\\char_blue.png", 56, 56, 1, 6);
		loadImageStripXY(&editorState->playerJumpAnimation, backendRenderer, "..\\src\\images\\char_blue.png", 56, 56, 3, 16);
		loadImageStripXY(&editorState->playerHurtAnimation, backendRenderer, "..\\src\\images\\char_blue.png", 56, 56, 5, 3);
		loadImageStripXY(&editorState->playerDieAnimation, backendRenderer, "..\\src\\images\\char_blue.png", 56, 56, 5, 12);
		loadImageStripXY(&editorState->playerFallingAnimation, backendRenderer, "..\\src\\images\\char_blue.png", 56, 56, 4, 1);

		loadImageStrip(&editorState->skeletonIdleAnimation, backendRenderer, "..\\src\\images\\skeleton\\SIdle_4.png", 150);
		loadImageStrip(&editorState->skeletonRunAnimation, backendRenderer, "..\\src\\images\\skeleton\\SWalk_4.png", 150);
		loadImageStrip(&editorState->skeletonAttackAnimation, backendRenderer, "..\\src\\images\\skeleton\\SAttack_8.png", 150);
		loadImageStrip(&editorState->skeletonHurtAnimation, backendRenderer, "..\\src\\images\\skeleton\\SHit_4.png", 150);
		loadImageStrip(&editorState->skeletonDieAnimation, backendRenderer, "..\\src\\images\\skeleton\\SDeath_4.png", 150);
		loadImageStrip(&editorState->skeletonShieldAnimation, backendRenderer, "..\\src\\images\\skeleton\\SShield_4.png", 150);

		/////////////

		int tileCount = 0;
		int countX = 0;
		int countY = 0;
		Texture ** tiles = loadTileSet(backendRenderer, "..\\src\\images\\Tileset.png", 32, 32, &global_long_term_arena, &tileCount, &countX, &countY);

		editorState->swampTileSet = buildTileSet(tiles, tileCount, TILE_SET_SWAMP, countX, countY, 32, 32);

		editorState->coinsGot = initResizeArray(int);

		addPlayerEntity(editorState);

		Entity *e = addSkeletonEntity(editorState);

		e->pos.x = -3;

	#if DEBUG_BUILD
		DEBUG_runUnitTests(editorState);
	#endif

	} else {
		releaseMemoryMark(&global_perFrameArenaMark);
		global_perFrameArenaMark = takeMemoryMark(&globalPerFrameArena);
	}

	if(global_platformInput.keyStates[PLATFORM_KEY_F1].pressedCount > 0) {
		editorState->gameMode = PLAY_MODE;
	} else if(global_platformInput.keyStates[PLATFORM_KEY_F2].pressedCount > 0) {
		editorState->gameMode = TILE_MODE;
	} else if(global_platformInput.keyStates[PLATFORM_KEY_F3].pressedCount > 0) {
		editorState->gameMode = SELECT_ENTITY_MODE;
	} else if(global_platformInput.keyStates[PLATFORM_KEY_F4].pressedCount > 0) {
		editorState->gameMode = A_STAR_MODE;
	}

	



	if(global_platformInput.keyStates[PLATFORM_KEY_F5].pressedCount > 0) {
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
	pushClearColor(renderer, make_float4(0.9, 0.9, 0.9, 1));

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
		editorState->player->velocity.y = 5.0f;
		editorState->player->targetRotation = 0.5f*HALF_PI32;
		rotationPower = 15.0f;

		editorState->hasInteratedYet = true;

	} else if(editorState->hasInteratedYet) {
		editorState->player->velocity.y -= 0.3f;
		editorState->player->targetRotation = -0.5f*HALF_PI32;
	}

	bool playerMoved = false;

	

	if(global_platformInput.keyStates[PLATFORM_KEY_LEFT].isDown && editorState->player->animationController.lastAnimationOn != &editorState->playerAttackAnimation) {
		editorState->player->velocity.x = -5.0f;
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
		editorState->player->velocity.x = 5.0f;
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
		editorState->player->velocity.x = 5.0f;
		editorState->player->colliders[ATTACK_COLLIDER_INDEX].offset = make_float3(0.5f, 0, 0);

		if(editorState->player->spriteFlipped) {
			editorState->player->velocity.x = -5.0f;
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
	pushTexture(renderer, editorState->backgroundTexture.handle, make_float3(0, 0, 10), make_float2(fauxDimensionX, fauxDimensionY), make_float4(1, 1, 1, 1), make_float4(0, 0, 1, 1));

	editorState->planeSizeY = (windowHeight / windowWidth) * editorState->planeSizeX;
	float16 fovMatrix = make_ortho_matrix_origin_center(editorState->planeSizeX, editorState->planeSizeY, MATH_3D_NEAR_CLIP_PlANE, MATH_3D_FAR_CLIP_PlANE);
	// float16 fovMatrix = make_perspective_matrix_origin_center(60.0f, MATH_3D_NEAR_CLIP_PlANE, MATH_3D_FAR_CLIP_PlANE, windowWidth / windowHeight);

	pushMatrix(renderer, fovMatrix);

	drawGrid(editorState);

	//NOTE: Push all lights for the renderer to use
	pushAllEntityLights(editorState, dt);

	pushShader(renderer, &pixelArtShader);
	pushMatrix(renderer, fovMatrix);

	renderTileMap(editorState, renderer);

	//NOTE: Collision code - fill all colliders with info and move entities
	updateEntityCollisions(editorState, dt);

	//NOTE: Gameplay code
	for(int i = 0; i < editorState->entityCount; ++i) {
		Entity *e = &editorState->entities[i];

		// e->pos.xy = plus_float2(scale_float2(dt, e->velocity.xy),  e->pos.xy);
		// e->rotation = lerp(e->rotation, e->targetRotation, make_lerpTValue(rotationPower*0.05f)); 
		

		if(e->flags & ENTITY_ACTIVE) {
			
			updateEntitySelection(editorState, e, windowWidth, windowHeight);
			renderEntity(editorState, renderer, e, fovMatrix, dt);
		}
	}

	if(editorState->gameMode != PLAY_MODE) {
		drawEditorGui(editorState, renderer, 0, 0, windowWidth, windowHeight);
	}

	//NOTE: Draw the points
	float16 orthoMatrix1 = make_ortho_matrix_bottom_left_corner(fauxDimensionX, fauxDimensionY, MATH_3D_NEAR_CLIP_PlANE, MATH_3D_FAR_CLIP_PlANE);
	pushMatrix(renderer, orthoMatrix1);
	pushShader(renderer, &sdfFontShader);

#if DEBUG_BUILD
	{
		char *name_str = "PLAY MODE";
		if(editorState->gameMode == TILE_MODE) {
			name_str = "TILE MODE";
		} else if(editorState->gameMode == SELECT_ENTITY_MODE) {
			name_str = "SELECT ENTITY MODE";
		}

		draw_text(renderer, &editorState->font, name_str, 50, fauxDimensionY - 50, 1, make_float4(0, 0, 0, 1)); 
	}

#endif
	
	char *name_str = easy_createString_printf(&globalPerFrameArena, "%d points", editorState->points); 
	draw_text(renderer, &editorState->font, name_str, 50, 50, 1, make_float4(0, 0, 0, 1)); 

#if DEBUG_BUILD
	if(editorState->draw_debug_memory_stats) {
		renderer_defaultScissors(renderer, windowWidth, windowHeight);
		DEBUG_draw_stats(editorState, renderer, &editorState->font, windowWidth, windowHeight, dt);
	}
#endif

	return editorState;

}