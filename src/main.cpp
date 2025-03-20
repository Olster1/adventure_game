#include <time.h>
#include <stdlib.h>

#include "wl_memory.h"
#include "save_settings.cpp"
#include "file_helper.cpp"
#include "lex_utf8.h"
#include "color.cpp"
#include "font.cpp"
#include "SimplexNoise.cpp"
#include "animation.cpp"
#include "resize_array.cpp"
// #include "transform.cpp"
#include "easy_ai.h"
#include "entity.h"
#include "tileMap.h"
#include "editor_gui.h"
#include "dialog.cpp"
#include "terrain.cpp"
#include "game_state.h"
#include "assets.cpp"
#include "tileMap.cpp"
// #include "easy_text_io.h"
#include "save_load_level.h"
#include "editor_gui.cpp"
#include "entity.cpp"
#include "save_load_level.cpp"
#include "collision.cpp"
#include "entity_update.cpp"
#include "grid.cpp"
#if DEBUG_BUILD
#include "unit_tests.cpp"
#endif
#include "gameState.cpp"
#include "debug.cpp"
#include "player.cpp"
#include "camera.cpp"

static EditorState *updateEditor(BackendRenderer *backendRenderer, float dt, float windowWidth, float windowHeight, bool should_save_settings, char *save_file_location_utf8_only_use_on_inititalize, Settings_To_Save save_settings_only_use_on_inititalize) {
	EditorState *editorState = (EditorState *)global_platform.permanent_storage;
	assert(sizeof(EditorState) < global_platform.permanent_storage_size);
	if(!editorState->initialized) {
		initGameState(editorState, backendRenderer);
	} else {
		releaseMemoryMark(&global_perFrameArenaMark);
		global_perFrameArenaMark = takeMemoryMark(&globalPerFrameArena);
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

	updatePlayerInput(editorState);
	updateCamera(editorState, dt);

	editorState->planeSizeY = (windowHeight / windowWidth) * editorState->planeSizeX;
	float16 fovMatrix = make_ortho_matrix_origin_center(editorState->planeSizeX*editorState->zoomLevel, editorState->planeSizeY*editorState->zoomLevel, MATH_3D_NEAR_CLIP_PlANE, MATH_3D_FAR_CLIP_PlANE);
	// float16 fovMatrix = make_perspective_matrix_origin_center(60.0f, MATH_3D_NEAR_CLIP_PlANE, MATH_3D_FAR_CLIP_PlANE, windowWidth / windowHeight);

	pushMatrix(renderer, fovMatrix);
	drawGrid(editorState);
	updateAndRenderEntities(editorState, renderer, dt, fovMatrix, windowWidth, windowHeight);

#if DEBUG_BUILD
	drawDebugAndEditorText(editorState, renderer, fauxDimensionX, fauxDimensionY, windowWidth, windowHeight, dt, fovMatrix);
#endif

	return editorState;

}