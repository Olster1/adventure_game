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



typedef struct {
	bool initialized;

	//NOTE: For creating unique entity ids like MongoDb ObjectIds
	int randomIdStartApp;
	int randomIdStart;

	Renderer renderer;

	EditorMode mode;

	Font font;

	float shakeTimer;

	float fontScale;

	bool draw_debug_memory_stats;

	Entity player;	

	int entityCount;
	Entity entities[256];

	float3 cameraPos;

	Texture playerTexture;

	Texture pipeTexture;

	Texture pipeFlippedTexture;

	Texture backgroundTexture;

	Texture coinTexture;

	float coinRotation; //NOTE: Between 0 and 1

	int points;

	bool hasInteratedYet;

	//NOTE: Resizeable array for the coins - if id in list, means user got it. 
	int *coinsGot;

	EasyAnimation_Controller playerAnimationController;
	Animation playerIdleAnimation;
	Animation fireballIdleAnimation;

	EasyAnimation_ListItem *animationItemFreeListPtr;
} EditorState;

#include "entity.cpp"


static void loadImageStrip(Animation *animation, BackendRenderer *backendRenderer, char *filename_full_utf8, int widthPerImage) {
	Texture texOnStack = backendRenderer_loadFromFileToGPU(backendRenderer, filename_full_utf8);
	int count = 0;

	float xAt = 0;

	float widthTruncated = ((int)(texOnStack.width / widthPerImage))*widthPerImage;
	while(xAt < widthTruncated) {
		Texture *tex = pushStruct(&global_long_term_arena, Texture);
		easyPlatform_copyMemory(tex, &texOnStack, sizeof(Texture));

		tex->uvCoords.x = xAt / texOnStack.width;

		xAt += widthPerImage;

		tex->uvCoords.z = xAt / texOnStack.width;

		tex->aspectRatio_h_over_w = ((float)texOnStack.height) / ((float)(tex->uvCoords.z - tex->uvCoords.x)*(float)texOnStack.width);

		easyAnimation_pushFrame(animation, tex);

		count++;
	}
}

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

		editorState->player.velocity = make_float3(0, 0, 0);
		editorState->player.pos = make_float3(0, 0, 10);
		editorState->playerTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "..\\src\\images\\helicopter.png");

		editorState->pipeTexture =  backendRenderer_loadFromFileToGPU(backendRenderer, "..\\src\\images\\pipe.png");
		editorState->pipeFlippedTexture =  backendRenderer_loadFromFileToGPU(backendRenderer, "..\\src\\images\\pipeRotated.png");

		editorState->backgroundTexture = backendRenderer_loadFromFileToGPU(backendRenderer, "..\\src\\images\\backgroundCastles.png");

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

		editorState->coinsGot = initResizeArray(int);

		Entity *e = addFireballEnemy(editorState);

		e->rotation = 0.5f*HALF_PI32;
		e->velocity = make_float3(0, 10, 0);


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

	editorState->player.rotation = lerp(editorState->player.rotation, editorState->player.targetRotation, rotationPower*0.05f); 

	editorState->cameraPos.x = editorState->player.pos.x + cameraOffset.x;
	editorState->cameraPos.y = cameraOffset.y;

	pushShader(renderer, &textureShader);

	//NOTE: Background texture
	pushTexture(renderer, editorState->backgroundTexture.handle, make_float3(0, 0, 10), make_float2(fauxDimensionX, fauxDimensionY), make_float4(1, 1, 1, 1), make_float4(0, 0, 1, 1));
	
	float planeSize = 10.0f;
	float16 fovMatrix = make_ortho_matrix_origin_center((windowWidth / windowHeight)*planeSize, planeSize, MATH_3D_NEAR_CLIP_PlANE, MATH_3D_FAR_CLIP_PlANE);
	// float16 fovMatrix = make_perspective_matrix_origin_center(60.0f, MATH_3D_NEAR_CLIP_PlANE, MATH_3D_FAR_CLIP_PlANE, windowWidth / windowHeight);

	editorState->coinRotation += dt*0.6f;

	if(editorState->coinRotation > 1.0f) {
		editorState->coinRotation = 0;
	}

	float2 playerSize = make_float2(1, 1);

	float gapSize = 2.5f;

	

	for(int i = 0; i < 128; ++i) {

		pushMatrix(renderer, fovMatrix);

		float yPos = 3.5f*perlin1d(i + 1,  10, 10);

		Rect2f r = make_rect2f_center_dim(make_float2(i*2, yPos + gapSize), make_float2(1, 2));

		float3 pos = {};
		pos.xy = get_centre_rect2f(r);
		pos.z = 9;

		pushShader(renderer, &textureShader);
		pushTexture(renderer, editorState->pipeFlippedTexture.handle, minus_float3(pos, editorState->cameraPos), get_scale_rect2f(r), make_float4(1, 1, 1, 1), make_float4(0, 0, 1, 1));
		
		pushShader(renderer, &lineShader);
		pushLine(renderer, pos, plus_float3(pos, make_float3(2, 2, 0)), make_float4(1, 0, 0, 1));
		

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

				float16 coinMatrix = float16_angle_aroundY(lerp(0, 2*PI32, editorState->coinRotation));

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

	for(int i = 0; i < editorState->entityCount; ++i) {
		Entity *e = &editorState->entities[i];

		e->pos.xy = plus_float2(scale_float2(dt, e->velocity.xy),  e->pos.xy);
		e->rotation = lerp(e->rotation, e->targetRotation, rotationPower*0.05f); 

		if(e->flags & ENTITY_ACTIVE) {
			
				//NOTE: Draw the fireball
				float16 entityMatrix = float16_angle_aroundZ(e->rotation);

				entityMatrix.E[0] *= -1;

				float3 entityP = minus_float3(e->pos, editorState->cameraPos);

				entityMatrix = float16_set_pos(entityMatrix, entityP);

				entityMatrix = float16_multiply(fovMatrix, entityMatrix); 

				pushMatrix(renderer, entityMatrix);

				Texture *t = easyAnimation_updateAnimation_getTexture(&e->animationController, &editorState->animationItemFreeListPtr, dt);
				if(e->animationController.finishedAnimationLastUpdate) {
					//NOTE: Make not active anymore. Should Probably remove it from the list. 
					e->flags &= ~ENTITY_ACTIVE;
				}
				pushTexture(renderer, t->handle, make_float3(0, 0, 0), make_float2(1, 0.5f), make_float4(1, 1, 1, 1), t->uvCoords);

				Rect2f r = make_rect2f_center_dim(e->pos.xy, scale_float2(e->collisionScale, e->scale.xy));

				//NOTE: See if it hit anything
				Rect2f minowskiPlus = rect2f_minowski_plus(r, make_rect2f_center_dim(editorState->player.pos.xy, playerSize), e->pos.xy);
				if(in_rect2f_bounds(minowskiPlus, editorState->player.pos.xy)) {
					// editorState->player.pos.y = 0;
					// editorState->player.velocity.y = 0;
					//NOTE: Reset player

					//NOTE: Shake the camera
					if(editorState->shakeTimer < 0 || editorState->shakeTimer > 0.5f) {
						editorState->shakeTimer = 0;
					}
					
				}
			
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