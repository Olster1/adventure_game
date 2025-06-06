#include <time.h>
#include <stdlib.h>

#include "defines.h"
#include "wl_memory.h"
#include "save_settings.cpp"
#include "file_helper.cpp"
#include "lex_utf8.h"
#include "color.cpp"
#include "font.cpp"
#include "SimplexNoise.cpp"
#include "transform.cpp"
#include "animation.cpp"
#include "resize_array.cpp"
#include "particles.cpp"
#include "entity.h"
#include "tileMap.h"
#include "editor_gui.h"
#include "dialog.cpp"
#include "texture_atlas.cpp"
#include "terrain.hpp"
#include "gameplay.cpp"
#include "board_astar.h"
#include "game_state.h"
#include "terrain.cpp"
#include "assets.cpp"
#include "tileMap.cpp"
// #include "easy_text_io.h"
#include "save_load_level.h"
#include "editor_gui.cpp"
#include "board_astar.cpp"
#include "entity.cpp"
#include "cloudAlt.cpp"
#include "save_load_level.cpp"
#include "collision.cpp"
#include "entity_update.cpp"
#include "grid.cpp"

#if DEBUG_BUILD
#include "unit_tests.cpp"
#endif
#include "gameState.cpp"
#include "ui_game.cpp"
#include "easy_profiler_draw.h"
#include "debug.cpp"
#include "init_board.cpp"
#include "camera.cpp"

static GameState *updateEditor(BackendRenderer *backendRenderer, float dt, float windowWidth, float windowHeight, bool should_save_settings, char *save_file_location_utf8_only_use_on_inititalize, Settings_To_Save save_settings_only_use_on_inititalize) {
	DEBUG_TIME_BLOCK();
	GameState *gameState = (GameState *)global_platform.permanent_storage;
	assert(sizeof(GameState) < global_platform.permanent_storage_size);
	
	if(!gameState->initialized) {
		initGameState(gameState, backendRenderer);
	} else {
		releaseMemoryMark(&global_perFrameArenaMark);
		global_perFrameArenaMark = takeMemoryMark(&globalPerFrameArena);
	}

	if(!gameState->gamePlay.boardInited) {
		initPlayerBoard(gameState);
		gameState->gamePlay.boardInited = true;
	}

	Renderer *renderer = &gameState->renderer;

	//NOTE: Clear the renderer out so we can start again
	clearRenderer(renderer);
	clearGameStatePerFrameValues(gameState);

	float2 mouseP = make_float2(global_platformInput.mouseX, windowHeight - global_platformInput.mouseY);
    float2 mouseP_01 = make_float2(mouseP.x / windowWidth, mouseP.y / windowHeight);

	if(gameState->drawState->openState == EASY_PROFILER_DRAW_CLOSED) {
		updateZoomAndPan(gameState, dt, mouseP_01);
	}

	//NOTE: Get pointer to player - always at slot zero
	gameState->player = &gameState->entities[0];

	pushViewport(renderer, make_float4(0, 0, windowWidth, windowHeight));
	renderer_defaultScissors(renderer, windowWidth, windowHeight);

	pushRenderFrameBuffer(renderer, 0);
	pushClearColor(renderer, BACKGROUND_COLOR);

	float2 mouse_point_top_left_origin = make_float2(global_platformInput.mouseX, global_platformInput.mouseY);	
	float2 mouse_point_top_left_origin_01 = make_float2(global_platformInput.mouseX / windowWidth, global_platformInput.mouseY / windowHeight);

	float fauxDimensionY = 1000;
	float fauxDimensionX = fauxDimensionY * (windowWidth/windowHeight);

	// updatePlayerInput(gameState);
	updateCamera(gameState, dt);

	gameState->planeSizeY = (windowHeight / windowWidth) * gameState->planeSizeX;
	float16 fovMatrix = make_ortho_matrix_origin_center(gameState->planeSizeX*gameState->zoomLevel, gameState->planeSizeY*gameState->zoomLevel, MATH_3D_NEAR_CLIP_PlANE, MATH_3D_FAR_CLIP_PlANE);
	
	updateAndRenderEntities(gameState, renderer, dt, fovMatrix, windowWidth, windowHeight);
	drawGameUi(gameState, renderer, dt, windowWidth, windowHeight);

#if DEBUG_BUILD
	// drawDebugAndEditorText(gameState, renderer, fauxDimensionX, fauxDimensionY, windowWidth, windowHeight, dt, fovMatrix);
	
	EasyProfile_DrawGraph(renderer, gameState, gameState->drawState, dt, windowHeight/windowWidth, mouseP_01);
#endif

	return gameState;

}