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
#include "entity.cpp"
#include "collision.cpp"
#include "assets.cpp"
#include "tileMap.cpp"
#include "editor_gui.cpp"

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

		editorState->player.velocity = make_float3(0, 0, 0);
		editorState->player.pos = make_float3(0, 0, 10);
		editorState->playerTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "..\\src\\images\\helicopter.png");

		editorState->pipeTexture =  backendRenderer_loadFromFileToGPU(backendRenderer, "..\\src\\images\\pipe.png");
		editorState->pipeFlippedTexture =  backendRenderer_loadFromFileToGPU(backendRenderer, "..\\src\\images\\pipeRotated.png");

		editorState->backgroundTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "..\\src\\images\\bg.png");//backgroundCastles.png");

		editorState->coinTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "..\\src\\images\\coin.png");

		// for(int i = 0; i < arrayCount(editorState->rects); ++i) {
		// 	Rect2f r = make_rect2f_center_dim(make_float2(i, 3), make_float2(1, 2));
		// 	editorState->rects[i] = CollisionRect(r);
		// }

		//NOTE: Init all animations for game
		easyAnimation_initAnimation(&editorState->playerIdleAnimation, "flappy_bird_idle");
		easyAnimation_initAnimation(&editorState->fireballIdleAnimation, "fireball_idle");
		//////////////////

		easyAnimation_initController(&editorState->playerAnimationController);
		easyAnimation_addAnimationToController(&editorState->playerAnimationController, &editorState->animationItemFreeListPtr, &editorState->playerIdleAnimation, 0.08f);

		loadImageStrip(&editorState->playerIdleAnimation, backendRenderer, "..\\src\\images\\Flappy_bird.png", 64);
		loadImageStrip(&editorState->fireballIdleAnimation, backendRenderer, "..\\src\\images\\fireball.png", 64);

		int tileCount = 0;
		int countX = 0;
		int countY = 0;
		Texture ** tiles = loadTileSet(backendRenderer, "..\\src\\images\\Tileset.png", 32, 32, &global_long_term_arena, &tileCount, &countX, &countY);

		editorState->swampTileSet = buildTileSet(tiles, tileCount, TILE_SET_SWAMP, countX, countY, 32, 32);

		editorState->coinsGot = initResizeArray(int);

		Entity *e = addFireballEnemy(editorState);

		e->pos.x = -3;
		// e->rotation = 0.5f*HALF_PI32;
		e->targetRotation = 0.5f*HALF_PI32;
		// e->velocity = make_float3(0, 10, 0);

		Entity *e1 = addFireballEnemy(editorState);

		e1->pos.x = 1;
		e1->parent = e;
		e1->velocity = make_float3(0, 0, 0);
		
	#if DEBUG_BUILD
		DEBUG_runUnitTests(editorState);
	#endif

	} else {
		releaseMemoryMark(&global_perFrameArenaMark);
		global_perFrameArenaMark = takeMemoryMark(&globalPerFrameArena);
	}


	if(global_platformInput.keyStates[PLATFORM_KEY_F5].pressedCount > 0) {
		editorState->draw_debug_memory_stats = !editorState->draw_debug_memory_stats;
	}

	Renderer *renderer = &editorState->renderer;

	//NOTE: Clear the renderer out so we can start again
	clearRenderer(renderer);


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
		editorState->player.velocity.y = 5.0f;
		editorState->player.targetRotation = 0.5f*HALF_PI32;
		rotationPower = 15.0f;

		if(!editorState->hasInteratedYet) {
			editorState->player.velocity.x = 2;
		}

		editorState->hasInteratedYet = true;

	} else if(editorState->hasInteratedYet) {
		editorState->player.velocity.y -= 0.3f;
		editorState->player.targetRotation = -0.5f*HALF_PI32;
	}

	editorState->player.pos.xy = plus_float2(scale_float2(dt, editorState->player.velocity.xy),  editorState->player.pos.xy);

	editorState->player.rotation = lerp(editorState->player.rotation, editorState->player.targetRotation, make_lerpTValue(rotationPower*0.05f)); 

	editorState->cameraPos.x = editorState->player.pos.x + cameraOffset.x;
	editorState->cameraPos.y = cameraOffset.y;

	pushShader(renderer, &textureShader);

	//NOTE: Background texture
	pushTexture(renderer, editorState->backgroundTexture.handle, make_float3(0, 0, 10), make_float2(fauxDimensionX, fauxDimensionY), make_float4(1, 1, 1, 1), make_float4(0, 0, 1, 1));

	editorState->planeSizeY = (windowHeight / windowWidth) * editorState->planeSizeX;
	float16 fovMatrix = make_ortho_matrix_origin_center(editorState->planeSizeX, editorState->planeSizeY, MATH_3D_NEAR_CLIP_PlANE, MATH_3D_FAR_CLIP_PlANE);
	// float16 fovMatrix = make_perspective_matrix_origin_center(60.0f, MATH_3D_NEAR_CLIP_PlANE, MATH_3D_FAR_CLIP_PlANE, windowWidth / windowHeight);

	pushMatrix(renderer, fovMatrix);

	editorState->coinRotation += dt*0.6f;

	if(editorState->coinRotation > 1.0f) {
		editorState->coinRotation = 0;
	}

	float2 playerSize = make_float2(1, 1);

	float gapSize = 2.5f;

	drawGrid(editorState);

	for(int i = 0; i < 128; ++i) {

		pushMatrix(renderer, fovMatrix);

		float yPos = 3.5f*perlin1d(i + 1,  10, 10);

		Rect2f r = make_rect2f_center_dim(make_float2(i*2, yPos + gapSize), make_float2(1, 2));

		float3 pos = {};
		pos.xy = get_centre_rect2f(r);
		pos.z = 9;

		pushShader(renderer, &textureShader);
		pushTexture(renderer, editorState->pipeFlippedTexture.handle, minus_float3(pos, editorState->cameraPos), get_scale_rect2f(r), make_float4(1, 1, 1, 1), make_float4(0, 0, 1, 1));
		

		Rect2f minowskiPlus = rect2f_minowski_plus(r, make_rect2f_center_dim(editorState->player.pos.xy, playerSize), pos.xy);
		if(in_rect2f_bounds(minowskiPlus, editorState->player.pos.xy)) {
			// editorState->player.pos.y = 0;
			// editorState->player.velocity.y = 0;
			//NOTE: Reset player

			//NOTE: Shake the camera
			if(editorState->shakeTimer < 0 || editorState->shakeTimer > 0.5f) {
				editorState->shakeTimer = 0;
			}
			
		}

		pushShader(renderer, &textureShader);

		////////////
		/// This is the ones below
		/// 
		r = make_rect2f_center_dim(make_float2(i*2, yPos - gapSize), make_float2(1, 2));

		pos = {};
		pos.xy = get_centre_rect2f(r);
		pos.z = 9;

		float3 pipePos = minus_float3(pos, editorState->cameraPos);

		//pushRect(renderer, minus_float3(pos, editorState->player.cameraPos), get_scale_rect2f(r), make_float4(1, 0, 0, 1));

		pushTexture(renderer, editorState->pipeTexture.handle, pipePos, get_scale_rect2f(r), make_float4(1, 1, 1, 1), make_float4(0, 0, 1, 1));

		minowskiPlus = rect2f_minowski_plus(r, make_rect2f_center_dim(editorState->player.pos.xy, playerSize), pos.xy);
		if(in_rect2f_bounds(minowskiPlus, editorState->player.pos.xy)) {
			// editorState->player.pos.y = 0;
			// editorState->player.velocity.y = 0;
			//NOTE: Reset player

			if(editorState->shakeTimer < 0 || editorState->shakeTimer > 0.5f) {
				editorState->shakeTimer = 0;
			}
		}

		{
			bool gotCoin = false;

			for(int j = 0; j < getArrayLength(editorState->coinsGot); j++) {
				if(editorState->coinsGot[j] == i) {
					gotCoin = true;
					break;
				}
			}

			if(!gotCoin) {

				//NOTE: Draw and update coins
				float3 coinPos = {};
				coinPos.xy = make_float2(i*2, yPos);
				coinPos.z = 10;

				//NOTE: MOve from model to view space
				float3 coinPosViewSpace = minus_float3(coinPos, editorState->cameraPos);

				float16 coinMatrix = float16_angle_aroundY(lerp(0, 2*PI32, make_lerpTValue(editorState->coinRotation)));

				coinMatrix = float16_set_pos(coinMatrix, coinPosViewSpace);

				coinMatrix = float16_multiply(fovMatrix, coinMatrix); 

				pushMatrix(renderer, coinMatrix);

				float2 coinSize = make_float2(1, 1);

				pushTexture(renderer, editorState->coinTexture.handle, make_float3(0, 0, 0), coinSize, make_float4(1, 1, 1, 1), make_float4(0, 0, 1, 1));
				
				//NOTE: Player collision

				r = make_rect2f_center_dim(coinPos.xy, scale_float2(0.7f, coinSize));

				minowskiPlus = rect2f_minowski_plus(r, make_rect2f_center_dim(editorState->player.pos.xy, playerSize), coinPos.xy);
				if(in_rect2f_bounds(minowskiPlus, editorState->player.pos.xy)) {
					editorState->points += 10;
					//NOTE: Make coin dissapear
					int val = i;
					editorState->coinsGot = pushArrayItem(editorState->coinsGot, val, int);
				}
			}
		}

	}	

	pushShader(renderer, &textureShader);
	pushMatrix(renderer, fovMatrix);

	//NOTE: Draw the tile map
	for(int i = 0; i < editorState->tileCount; ++i) {
		MapTile t = editorState->tiles[i];

		Texture *sprite = 0;

		//NOTE: Get the right texture
		switch(t.type) {
			case TILE_SET_SWAMP: {
				TileSet *set = &editorState->swampTileSet;

				int indexIntoArray = t.xId + (set->countX*t.yId);
				assert(indexIntoArray < set->count);
				sprite = set->tiles[indexIntoArray];

			} break;
		}

		float pX = (t.x + 0.5f) - editorState->cameraPos.x;
		float pY = (t.y + 0.5f)  - editorState->cameraPos.y;

		pushTexture(renderer, sprite->handle, make_float3(pX, pY, 10), make_float2(1, 1), make_float4(1, 1, 1, 1), sprite->uvCoords);
	}

	//NOTE: Collision code - fill all colliders with info and move entities
	updateEntityCollisions(editorState, dt);

	//NOTE: Gameplay code
	for(int i = 0; i < editorState->entityCount; ++i) {
		Entity *e = &editorState->entities[i];

		// e->pos.xy = plus_float2(scale_float2(dt, e->velocity.xy),  e->pos.xy);
		e->rotation = lerp(e->rotation, e->targetRotation, make_lerpTValue(rotationPower*0.05f)); 

		if(e->flags & ENTITY_ACTIVE) {

			float16 modelToViewT = getModelToViewTransform(e, editorState->cameraPos);

			if(e->spriteFlipped) {
				modelToViewT.E[0] *= -1;
				modelToViewT.E[1] *= -1;
				modelToViewT.E[2] *= -1;
			}
			
			modelToViewT = float16_multiply(fovMatrix, modelToViewT); 

			pushMatrix(renderer, modelToViewT);

			Texture *t = easyAnimation_updateAnimation_getTexture(&e->animationController, &editorState->animationItemFreeListPtr, dt);
			if(e->animationController.finishedAnimationLastUpdate) {
				//NOTE: Make not active anymore. Should Probably remove it from the list. 
				// e->flags &= ~ENTITY_ACTIVE;
			}
			pushTexture(renderer, t->handle, make_float3(0, 0, 0), make_float2(1, 1), make_float4(1, 1, 1, 1), t->uvCoords);
			
		}
	}

	//NOTE: Draw player
	float16 playerMatrix = float16_angle_aroundZ(editorState->player.rotation);

	float3 playerP = minus_float3(editorState->player.pos, editorState->cameraPos);

	playerMatrix = float16_set_pos(playerMatrix, playerP);

	playerMatrix = float16_multiply(fovMatrix, playerMatrix); 

	pushMatrix(renderer, playerMatrix);

	Texture *t = easyAnimation_updateAnimation_getTexture(&editorState->playerAnimationController, &editorState->animationItemFreeListPtr, dt);
	
	pushTexture(renderer, t->handle, make_float3(0, 0, 0), playerSize, make_float4(1, 1, 1, 1), t->uvCoords);

	drawEditorGui(editorState, renderer, 0, 0, windowWidth, windowHeight);

	//NOTE: Draw the points
	float16 orthoMatrix1 = make_ortho_matrix_bottom_left_corner(fauxDimensionX, fauxDimensionY, MATH_3D_NEAR_CLIP_PlANE, MATH_3D_FAR_CLIP_PlANE);
	pushMatrix(renderer, orthoMatrix1);


	pushShader(renderer, &sdfFontShader);
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