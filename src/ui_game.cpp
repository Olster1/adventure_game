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

enum UiButtonType {
    UI_BUTTON_NONE,
    UI_BUTTON_NEXT_TURN,
};

void drawScrollText(char *text, GameState *gameState, Renderer *renderer, float2 percentOffset, UiAnchorPoint anchorPoint, float2 resolution, float2 mouseP, UiButtonType buttonType = UI_BUTTON_NONE) {
    DEBUG_TIME_BLOCK();
    float fontSize = 0.1;
	Rect2f bounds = getTextBounds(renderer, &gameState->font, text, 0, 0, fontSize); 
    float ar = gameState->bannerTexture.aspectRatio_h_over_w;
    float scalef = 25;
    float2 scale = make_float2(scalef, ar*scalef);
    float2 bScale = get_scale_rect2f(bounds);
    float sizeFactor = 1.25f;

    Texture *t = &gameState->bannerTexture;
    if(buttonType != UI_BUTTON_NONE) {
        t = &gameState->buttonTexture;
    }

    scale.x = math3d_maxfloat(sizeFactor*bScale.x, scale.x);
    scale.y = math3d_maxfloat(sizeFactor*bScale.y, scale.y);
    
    float2 pos = make_float2(0.5f*scale.x, 0.5f*scale.y);
    float2 pos1 = getUiPosition(percentOffset, anchorPoint, pos, resolution);

    float4 color = make_float4(1, 1, 1, 1);
    bool clicked = global_platformInput.keyStates[PLATFORM_MOUSE_LEFT_BUTTON].pressedCount > 0;
    bool inSelectionBounds = in_rect2f_bounds(make_rect2f_center_dim(pos1, scale), mouseP);
    if(inSelectionBounds) {
        color = make_float4(0.4f, 0.4f, 0.4f, 1);

        if(clicked) {
            if(buttonType == UI_BUTTON_NEXT_TURN) {
                gameState->gamePlay.turnOn = GAME_TURN_PLAYER_GOBLIN;
            }
        }
    }

    pushShader(renderer, &pixelArtShader);
    pushTexture(renderer, gameState->shadowUiTexture.handle, make_float3(pos1.x, pos1.y, UI_Z_POS), scale, make_float4(1, 1, 1, 0.3), gameState->shadowUiTexture.uvCoords);
    pushTexture(renderer, t->handle, make_float3(pos1.x, pos1.y, UI_Z_POS), scale, color, t->uvCoords);

    // pushTexture(renderer, global_white_texture, make_float3(pos1.x, pos1.y, UI_Z_POS), bScale, make_float4(1, 0, 0, 0.5f), make_float4(0, 0, 1, 1));


    // pos = getUiPosition(make_float2(0, 0), anchorPoint, pos, resolution);
    pushShader(renderer, &sdfFontShader);
    draw_text(renderer, &gameState->font, text, pos1.x - 0.5f*bScale.x, pos1.y + 0.5f*bScale.y, fontSize, make_float4(0, 0, 0, 1)); 
}

void drawResources(GameState *gameState, Renderer *renderer, float2 resolution, float dt) {
        float fontSize = 0.1;
        
        Texture *b = &gameState->logTexture;
        float2 s = make_float2(10, 10);
        float2 s1 = make_float2(5, 5);
        float2 p = getUiPosition(make_float2(0, 1), UI_ANCHOR_CENTER_LEFT, make_float2(0.5f*s.x, 0), resolution);

        pushShader(renderer, &pixelArtShader);
        pushTexture(renderer, gameState->bannerTexture.handle, make_float3(p.x, p.y, UI_Z_POS), s, make_float4(1, 1, 1, 1), gameState->bannerTexture.uvCoords);
        pushTexture(renderer, b->handle, make_float3(p.x, p.y, UI_Z_POS), s1, make_float4(1, 1, 1, 1), b->uvCoords);

         for(int i = 0; i < gameState->uiOnScreenItemCount; ) {
            UiOnScreenItem *ui = gameState->uiOnScreenItems + i;

            ui->tAt += 0.8f*dt;

            bool end = false;
            if(ui->tAt >= 1) {
                ui->tAt = 1;
                end = true;
            }

            float2 itemP = lerp_float2(ui->startP, make_float2(p.x/resolution.x, p.y/resolution.y), ui->tAt);
            pushTexture(renderer, b->handle, make_float3(itemP.x*resolution.x, itemP.y*resolution.y, UI_Z_POS), s1, make_float4(1, 1, 1, 1.0f - ui->tAt), b->uvCoords);

            if(end) {
                gameState->uiOnScreenItems[i] = gameState->uiOnScreenItems[--gameState->uiOnScreenItemCount];

            } else {
                i++;
            }
        }

        pushShader(renderer, &sdfFontShader);
        
        char *str = easy_createString_printf(&globalPerFrameArena, "%d", gameState->gamePlay.treeCount);
        float2 bounds = get_scale_rect2f(getTextBounds(renderer, &gameState->font, str, 0, 0, fontSize)); 
        draw_text(renderer, &gameState->font, str, (p.x - 0.5f*bounds.x) + 2, (p.y + 0.5f*bounds.y) - 2, fontSize, make_float4(0, 0, 0, 1)); 
    }

void drawGameUi(GameState *gameState, Renderer *renderer, float dt, float windowWidth, float windowHeight, float2 mouseP_01){
	DEBUG_TIME_BLOCK();
    GamePlay *play = &gameState->gamePlay;

    play->turnTime += dt;

    float aspectRatio_yOverX = windowHeight / windowWidth;
    float fuaxWidth = 100;
	float2 resolution = make_float2(fuaxWidth, fuaxWidth*aspectRatio_yOverX);
	float16 fovMatrix = make_ortho_matrix_bottom_left_corner(resolution.x, resolution.y, MATH_3D_NEAR_CLIP_PlANE, MATH_3D_FAR_CLIP_PlANE);
    pushMatrix(renderer, fovMatrix);

    float2 mouseP = make_float2(mouseP_01.x*resolution.x, mouseP_01.y*resolution.y);
    // drawScrollText("YOUR TURN", gameState, renderer, make_float2(1, 1));

    char *str = easy_createString_printf(&globalPerFrameArena, "TURN: %d/%d", play->turnCount, play->maxTurnCount);
    drawScrollText(str, gameState, renderer, make_float2(1, 1), UI_ANCHOR_TOP_LEFT, resolution, mouseP);
    
    // {
    //NOTE: Drawing the time left string
    //     int sec = (int)(play->maxTurnTime - play->turnTime);
    //     int min = sec / 60;
    //     sec -= min*60;
    
    //     str = easy_createString_printf(&globalPerFrameArena, "Time Left: %d:%d", min, sec);
    //     drawScrollText(str, gameState, renderer, make_float2(1, 1), UI_ANCHOR_TOP_LEFT, resolution);
    // }

    drawResources(gameState, renderer, resolution, dt);

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
    if(gameState->gamePlay.turnOn == GAME_TURN_PLAYER_KNIGHT){
        char *text = "FINISH MOVE";
        drawScrollText(text, gameState, renderer, make_float2(0, 1), UI_ANCHOR_CENTER_BOTTOM, resolution, mouseP, UI_BUTTON_NEXT_TURN);
    }
    // {
    //     char *text = "FINISH : MOVE 2 / 3\nHey";
    //     drawScrollText(text, gameState, renderer, make_float2(0, 0), UI_ANCHOR_CENTER, resolution);
    // }
}