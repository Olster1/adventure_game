bool editorGui_isSameId(EditorGuiId a, EditorGuiId b) {
    return (a.a == b.a && a.b == b.b && a.type == b.type);
}

static void editorGui_clearInteraction(EditorGui *gui) {
    gui->currentInteraction.active = false;
}

void drawEditorGui(EditorState *state, Renderer *renderer, float x, float y, float windowWidth, float windowHeight) {

    float2 mouseP = make_float2(global_platformInput.mouseX, windowHeight - global_platformInput.mouseY);

    float2 mouseP_01 = make_float2(mouseP.x / windowWidth, mouseP.y / windowHeight);

    bool clicked = global_platformInput.keyStates[PLATFORM_MOUSE_LEFT_BUTTON].pressedCount > 0;

    TileSet *swampSet = &state->swampTileSet; 

    
	if(global_platformInput.keyStates[PLATFORM_KEY_ESCAPE].pressedCount > 0) {
        editorGui_clearInteraction(&state->editorGuiState);
	}

    if(swampSet->count > 0) {
        float16 orthoMatrix = make_ortho_matrix_bottom_left_corner(windowWidth, windowHeight, MATH_3D_NEAR_CLIP_PlANE, MATH_3D_FAR_CLIP_PlANE);

        pushMatrix(renderer, orthoMatrix);

        float aspectRatio = swampSet->tiles[0]->height / swampSet->tiles[0]->width;

        float size = 600;

        float scaleFactor = size / swampSet->tiles[0]->width;

        float tileSizeX = swampSet->tileSizeX * scaleFactor;
        float tileSizeY = swampSet->tileSizeY * scaleFactor;

        int xIndex = (mouseP.x - x) / tileSizeX; 
        int yIndex = (mouseP.y - y) / tileSizeY; 

        EditorGuiId thisId = {};

        thisId.a = xIndex;
        thisId.b = yIndex;
        thisId.type = TILE_SELECTION_SWAMP;

        //NOTE: Draw the backing
        pushShader(renderer, &textureShader);
        float2 scale = make_float2(size, size*aspectRatio);
        pushTexture(renderer, swampSet->tiles[0]->handle, make_float3(x + 0.5f*scale.x, y + 0.5f*scale.y, 2), scale, make_float4(1, 1, 1, 1), make_float4(0, 0, 1, 1));

        bool isActive = (state->editorGuiState.currentInteraction.active && state->editorGuiState.currentInteraction.id.type == TILE_SELECTION_SWAMP);

        bool inSelectionBounds = in_rect2f_bounds(make_rect2f_min_dim(x, y, scale.x, scale.y), mouseP);

        if(inSelectionBounds && clicked) {
            editorGui_clearInteraction(&state->editorGuiState);
	    }

        if((!state->editorGuiState.currentInteraction.active && inSelectionBounds) || isActive) {
            //NOTE: Get the tile mouse is hovering over
           
            pushShader(renderer, &rectOutlineShader);

            float4 selectColor = make_float4(0.3f, 0, 0.3f, 1);

            if(isActive) {
                selectColor = make_float4(0, 0.3f, 0.3f, 1);

                xIndex = state->editorGuiState.currentInteraction.id.a;
                yIndex = state->editorGuiState.currentInteraction.id.b;
            }

            Rect2f r = make_rect2f_min_dim(xIndex*tileSizeX + x, yIndex*tileSizeY + y, tileSizeX, tileSizeY);

            float2 tileP = get_centre_rect2f(r);
            float2 tileScaleP = get_scale_rect2f(r);

           
           if(!isActive) {
                if(global_platformInput.keyStates[PLATFORM_MOUSE_LEFT_BUTTON].isDown) {
                    selectColor = make_float4(0.3f, 0.3f, 0, 1);
                } else if(global_platformInput.keyStates[PLATFORM_MOUSE_LEFT_BUTTON].releasedCount > 0) {
                    state->editorGuiState.currentInteraction.id = thisId;
                    state->editorGuiState.currentInteraction.active = true;

                    selectColor = make_float4(0.3f, 0.3f, 0, 1);
                }
            }

            pushTexture(renderer, global_white_texture, make_float3(tileP.x, tileP.y, 1.0f), tileScaleP, selectColor, make_float4(0, 0, 1, 1));
        }


        //NOTE: See if user added tile to map
        if(!inSelectionBounds && global_platformInput.keyStates[PLATFORM_MOUSE_LEFT_BUTTON].pressedCount > 0) {
            //NOTE: Add tile to map
            assert(state->tileCount < arrayCount(state->tiles));
            MapTile *t = state->tiles + state->tileCount++;

            float worldX = lerp(-0.5f*state->planeSizeX, 0.5f*state->planeSizeX, make_lerpTValue(mouseP_01.x));
            float worldY = lerp(-0.5f*state->planeSizeY, 0.5f*state->planeSizeY, make_lerpTValue(mouseP_01.y));

            worldX += state->cameraPos.x;
            worldY += state->cameraPos.y;


            //NOTE: Make sure the tile goes in the cell you click
            if(worldX < 0) {
                worldX = floor(worldX);
            }

            if(worldY < 0) {
                worldY = floor(worldY);
            }
            
            
            t->x = worldX;
            t->y = worldY;
            
            t->xId = xIndex;
            t->yId = (swampSet->countY - 1) - yIndex;

            t->type = TILE_SET_SWAMP;
        }



    }

}