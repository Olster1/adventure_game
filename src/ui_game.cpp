enum UiAnchorPoint {
    UI_ANCHOR_BOTTOM_LEFT,
    UI_ANCHOR_BOTTOM_RIGHT,
    UI_ANCHOR_TOP_LEFT,
    UI_ANCHOR_TOP_RIGHT,
    UI_ANCHOR_CENTER,

    UI_ANCHOR_CENTER_LEFT,
    UI_ANCHOR_CENTER_RIGHT,
    UI_ANCHOR_CENTER_TOP,
    UI_ANCHOR_CENTER_BOTTOM,
    
};

float2 getUiPosition(float2 percentOffset, UiAnchorPoint anchorPoint, float2 pos, float2 resolution) {
    float inverse100 = 1.0f / 100.0f;
    float2 t = make_float2(resolution.x*percentOffset.x*inverse100, resolution.y*percentOffset.y*inverse100);

    switch(anchorPoint) {
        case UI_ANCHOR_BOTTOM_LEFT: {
            pos.x += t.x;
            pos.y += t.y;
        } break;
        case UI_ANCHOR_BOTTOM_RIGHT: {
            pos.x = (resolution.x - pos.x) - t.x;
            pos.y += t.y;
        } break;
        case UI_ANCHOR_TOP_LEFT: {
            pos.x += t.x;
            pos.y = (resolution.y - pos.y) - t.y;
        } break;
        case UI_ANCHOR_TOP_RIGHT: {
            pos.x = (resolution.x - pos.x) - t.x;
            pos.y = (resolution.y - pos.y) - t.y;
        } break;
        case UI_ANCHOR_CENTER: {
            pos.x = (0.5f*resolution.x) + t.x;
            pos.y = (0.5f*resolution.y) + t.y;
        } break;
        case UI_ANCHOR_CENTER_LEFT: {
            pos.x += t.x;
            pos.y = 0.5f*resolution.y + t.y;
        } break;
        case UI_ANCHOR_CENTER_RIGHT: {
            pos.x = (resolution.x - pos.x) - t.x;
            pos.y = 0.5f*resolution.y + t.y;
        } break;
        case UI_ANCHOR_CENTER_TOP: {
            pos.x = (0.5f*resolution.x) + t.x;
            pos.y = (resolution.y - pos.y) - t.y;
        } break;
        case UI_ANCHOR_CENTER_BOTTOM: {
            pos.x = (0.5f*resolution.x) + t.x;
            pos.y += t.y;
        } break;
        default: {

        };
    }
    return pos;
}

void drawScrollText(char *text, GameState *gameState, Renderer *renderer, float2 percentOffset, UiAnchorPoint anchorPoint, float2 resolution) {
    DEBUG_TIME_BLOCK();
	pushShader(renderer, &pixelArtShader);
    float ar = gameState->bannerTexture.aspectRatio_h_over_w;
    float scalef = 25;
    float2 scale = make_float2(scalef, ar*scalef);


    Rect2f bounds = getTextBounds(renderer, &gameState->pixelFont, text, 0, 0, 0.1); 
    float2 bScale = get_scale_rect2f(bounds);

    scale.x = math3d_maxfloat(1.1f*bScale.x, scale.x);
    scale.y = math3d_maxfloat(bScale.y, scale.y);

    float2 pos = make_float2(0.5f*scale.x, 0.5f*scale.y);

    pos = getUiPosition(percentOffset, anchorPoint, pos, resolution);
    
    float sOffset = 0;
    pushTexture(renderer, gameState->shadowUiTexture.handle, make_float3(pos.x + sOffset, pos.y - sOffset, UI_Z_POS), scale, make_float4(1, 1, 1, 0.3), gameState->shadowUiTexture.uvCoords);
    pushTexture(renderer, gameState->bannerTexture.handle, make_float3(pos.x, pos.y, UI_Z_POS), scale, make_float4(1, 1, 1, 1), gameState->bannerTexture.uvCoords);

    pushShader(renderer, &sdfFontShader);
    draw_text(renderer, &gameState->pixelFont, text, pos.x - bScale.x*0.5f, pos.y + bScale.y*0.5f, 0.1, make_float4(0, 0, 0, 1)); 
}

void drawGameUi(GameState *gameState, Renderer *renderer, float dt, float windowWidth, float windowHeight){
	DEBUG_TIME_BLOCK();
    GamePlay *play = &gameState->gamePlay;

    play->turnTime += dt;

    float aspectRatio_yOverX = windowHeight / windowWidth;
    float fuaxWidth = 100;
	float2 resolution = make_float2(fuaxWidth, fuaxWidth*aspectRatio_yOverX);
	float16 fovMatrix = make_ortho_matrix_bottom_left_corner(resolution.x, resolution.y, MATH_3D_NEAR_CLIP_PlANE, MATH_3D_FAR_CLIP_PlANE);
    pushMatrix(renderer, fovMatrix);
	
    // drawScrollText("YOUR TURN", gameState, renderer, make_float2(1, 1));

    char *str = easy_createString_printf(&globalPerFrameArena, "TURN: %d/%d", play->turnCount, play->maxTurnCount);
    drawScrollText(str, gameState, renderer, make_float2(1, 1), UI_ANCHOR_TOP_LEFT, resolution);
    
    // {
    //NOTE: Drawing the time left string
    //     int sec = (int)(play->maxTurnTime - play->turnTime);
    //     int min = sec / 60;
    //     sec -= min*60;
    
    //     str = easy_createString_printf(&globalPerFrameArena, "Time Left: %d:%d", min, sec);
    //     drawScrollText(str, gameState, renderer, make_float2(1, 1), UI_ANCHOR_TOP_LEFT, resolution);
    // }

    //NOTE: Draw the player logo
    {
        Texture *l = 0;
        Texture *b = 0;
        if(play->turnOn == GAME_TURN_PLAYER_KNIGHT) {
            l = &gameState->kLogoText;
            b = &gameState->blueText;
        } else {
            l = &gameState->gLogoText;
            b = &gameState->redText;
        }

        pushShader(renderer, &pixelArtShader);
        float2 s = make_float2(10, 10);
        float2 s1 = make_float2(7, 7);
        float2 pos = getUiPosition(make_float2(0, 1), UI_ANCHOR_CENTER_TOP, make_float2(0, 0.5f*s.y), resolution);
        float3 p = make_float3(pos.x, pos.y, UI_Z_POS);
        float3 p1 = p;
        p1.y += 0.5f;
        // pushTexture(renderer, gameState->shadowUiTexture.handle, p, s, make_float4(1, 1, 1, 0.3), gameState->shadowUiTexture.uvCoords);
        pushTexture(renderer, b->handle, p, s, make_float4(1, 1, 1, 1), b->uvCoords);
        pushTexture(renderer, l->handle, p1, s1, make_float4(1, 1, 1, 1), l->uvCoords);

    }
    {
        char *text = "";
        switch (play->phase) {
            case GAME_TURN_PHASE_COMMAND: {
                text = "COMMAND";
            } break;
            case GAME_TURN_PHASE_MOVE: {
                text = "MOVE";
            } break;
            case GAME_TURN_PHASE_SHOOT: {
                text = "SHOOT";
            } break;
            case GAME_TURN_PHASE_CHARGE: {
                text = "CHARGE";
            } break;
            case GAME_TURN_PHASE_FIGHT: {
                text = "FIGHT";
            } break;
            case GAME_TURN_PHASE_BATTLESHOCK: {
                text = "BATTLESHOCK";
            } break;
            default: {

            } break;
        }

        drawScrollText(text, gameState, renderer, make_float2(0, 1), UI_ANCHOR_CENTER_BOTTOM, resolution);
    }
    

    
    
}