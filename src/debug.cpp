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
	DEBUG_draw_stats_MACRO("Per Entity Load Arena Total Size", DEBUG_get_total_arena_size(&globalPerEntityLoadArena), true);

	// WL_Window *w = &editorState->windows[editorState->active_window_index];
	// DEBUG_draw_stats_FLOAT_MACRO("Start at: ", editorState->selectable_state.start_pos.x, editorState->selectable_state.start_pos.y);
	// DEBUG_draw_stats_FLOAT_MACRO("Target Scroll: ", w->scroll_target_pos.x, w->scroll_target_pos.y);

	DEBUG_draw_stats_FLOAT_MACRO("mouse position 01 ", global_platformInput.mouseX / windowWidth, global_platformInput.mouseY / windowHeight);
	DEBUG_draw_stats_FLOAT_MACRO("dt for frame ", dt, dt);

}

void drawDebugAndEditorText(EditorState *editorState, Renderer *renderer, float fauxDimensionX, float fauxDimensionY, float windowWidth, float windowHeight, float dt, float16 fovMatrix) {
	if(global_platformInput.keyStates[PLATFORM_KEY_1].pressedCount > 0) {
		editorState->gameMode = PLAY_MODE;
	} else if(global_platformInput.keyStates[PLATFORM_KEY_2].pressedCount > 0) {
		if(editorState->gameMode == TILE_MODE) {
			editorState->gameMode = PLAY_MODE;
		} else {
			editorState->gameMode = TILE_MODE;
		}
		
	} else if(global_platformInput.keyStates[PLATFORM_KEY_3].pressedCount > 0) {
		if(editorState->gameMode == SELECT_ENTITY_MODE) {
			editorState->gameMode = PLAY_MODE;
		} else {
			editorState->gameMode = SELECT_ENTITY_MODE;
		}
	} else if(global_platformInput.keyStates[PLATFORM_KEY_4].pressedCount > 0) {
		if(editorState->gameMode == A_STAR_MODE) {
			editorState->gameMode = PLAY_MODE;
		} else {
			editorState->gameMode = A_STAR_MODE;
		}
	}

	if(global_platformInput.keyStates[PLATFORM_KEY_5].pressedCount > 0) {
		editorState->draw_debug_memory_stats = !editorState->draw_debug_memory_stats;
	}
	
	if(editorState->gameMode == TILE_MODE) {
		drawAndUpdateEditorGui(editorState, renderer, 0, 0, windowWidth, windowHeight);
	} else if(editorState->gameMode == A_STAR_MODE) {
		pushShader(renderer, &textureShader);
		pushMatrix(renderer, fovMatrix);

		updateAStartEditor(editorState, renderer, windowWidth, windowHeight);
	}

    float16 orthoMatrix1 = make_ortho_matrix_bottom_left_corner(fauxDimensionX, fauxDimensionY, MATH_3D_NEAR_CLIP_PlANE, MATH_3D_FAR_CLIP_PlANE);
	pushMatrix(renderer, orthoMatrix1);
	pushShader(renderer, &sdfFontShader);
	{
		char *name_str = "PLAY MODE";
		if(editorState->gameMode == TILE_MODE) {
			name_str = "TILE MODE";
		} else if(editorState->gameMode == SELECT_ENTITY_MODE) {
			name_str = "SELECT ENTITY MODE";
		} else if(editorState->gameMode == A_STAR_MODE) {
			name_str = "A* MODE";
		}

		draw_text(renderer, &editorState->font, name_str, 50, fauxDimensionY - 50, 1, make_float4(0, 0, 0, 1)); 
	}

	if(editorState->draw_debug_memory_stats) {
		renderer_defaultScissors(renderer, windowWidth, windowHeight);
		DEBUG_draw_stats(editorState, renderer, &editorState->font, windowWidth, windowHeight, dt);
	}
}