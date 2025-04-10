
void pushGameLight(GameState *state, float3 worldPos, float4 color, float perlinNoiseValue) {
	if(state->lightCount < arrayCount(state->lights)) {
		GameLight *l = &state->lights[state->lightCount++];

		l->worldPos = worldPos;
		l->color = scale_float4(perlinNoiseValue, color);
	}
}

char *makeEntityId(GameState *gameState) {
    u64 timeSinceEpoch = platform_getTimeSinceEpoch();
    char *result = easy_createString_printf(&globalPerEntityLoadArena, "%ld-%d-%d", timeSinceEpoch, gameState->randomIdStartApp, gameState->randomIdStart);

    //NOTE: This would have to be locked in threaded application
    gameState->randomIdStart++;

    return result;
}  

float16 getModelToWorldTransform(Entity *e_) {
    float16 result = float16_identity();

    Entity *e = e_;

    while(e) {
        //NOTE: ROTATION
        float16 local = float16_angle_aroundZ(e->rotation);

        if(e == e_) { //NOTE: Only scale by the orignal entity
            //NOTE: SCALE
            local = float16_scale(local, e->scale);
        }

        //NOTE: POS
        local = float16_set_pos(local, e->pos);

        //NOTE: Cocat to end matrix
        result = float16_multiply(local, result);

        e = e->parent;
    }

    return result;
}

float3 getWorldPosition(Entity *e_) {
    float3 result = e_->pos;

    Entity *e = e_->parent;

    while(e) {
        result = plus_float3(result, e->pos);

        e = e->parent;
    }

    return result;
}

float16 getModelToViewTransform(Entity *e_, float3 cameraPos) {

    float16 result = getModelToWorldTransform(e_);

    result = float16_multiply(float16_set_pos(float16_identity(), float3_negate(cameraPos)), result);

    return result;

}

Entity *makeNewEntity(GameState *state) {
    Entity *e = 0;
    if(state->entityCount < arrayCount(state->entities)) {
        e = &state->entities[state->entityCount++];

        memset(e, 0, sizeof(Entity));

        e->id = makeEntityId(state);
        e->idHash = get_crc32_for_string(e->id);

        e->velocity = make_float3(0, 0, 0);
        e->offsetP = make_float3(0, 0, 0);
        e->pos = make_float3(0, 0, 10);
        e->flags |= ENTITY_ACTIVE;
        e->scale = make_float3(4, 4, 1);
        e->speed = 3.0f;
    }
    return e;
}

Entity *addKnightEntity(GameState *state, float2 worldP) {
    Entity *e = makeNewEntity(state);
    if(e) {
        e->type = ENTITY_KNIGHT;
        e->pos.xy = worldP;
        e->offsetP.y = 0.16; //NOTE: Fraction of the scale
        easyAnimation_initController(&e->animationController);
		easyAnimation_addAnimationToController(&e->animationController, &state->animationState.animationItemFreeListPtr, &state->knightAnimations.idle, 0.08f);
		
    }
    return e;
} 


int compare_by_height(const void *a, const void *b) {
    const RenderObject *pa = (const RenderObject *)a;
    const RenderObject *pb = (const RenderObject *)b;
    int result = (int)(pb->sortIndex - pa->sortIndex);
    
    return result;
}

void drawClouds(GameState *gameState, Renderer *renderer, float dt) {
    DEBUG_TIME_BLOCK();

    TextureHandle *atlasHandle = gameState->textureAtlas.texture.handle;
    int cloudDistance = 10;
    for(int y = cloudDistance; y >= -cloudDistance; --y) {
        for(int x = -cloudDistance; x <= cloudDistance; ++x) {
            Chunk *c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, x, y, 0, true, false);
            if(c && (c->generateState == CHUNK_NOT_GENERATED || c->cloudFadeTimer >= 0)) {
                float maxTime = 1.5f;

                if(c->cloudCount == 0) {
                    int clouds = 10;
                    for(int i = 0; i < clouds; i++) {
                        for(int j = 0; j < clouds; j++) {
                            float2 p = {};
                            p.x += random_between_float(-1, CHUNK_DIM + 1);
                            p.y += random_between_float(-1, CHUNK_DIM + 1);
                            assert(c->cloudCount < arrayCount(c->clouds));
                            CloudData *d = &c->clouds[c->cloudCount++];
                            d->pos = p;
                            d->cloudIndex = random_between_int(0, 3);
                            d->fadePeriod = random_between_float(0.4f, maxTime);
                            d->scale = random_between_float(4, 10);
                            d->darkness = random_between_float(0.95f, 1.0f);
                            assert(d->cloudIndex < 3);

                        }
                    }
                }   
                
                for(int i = 0; i < c->cloudCount; ++i) {
                    CloudData *cloud = &c->clouds[i];
                    float3 worldP = make_float3(x*CHUNK_DIM, y*CHUNK_DIM, 10);
                    worldP.x += cloud->pos.x;
                    worldP.y += cloud->pos.y;
                    worldP.x -= gameState->cameraPos.x;
                    worldP.y -= gameState->cameraPos.y;
                    
                    float s = cloud->scale;

                    float tVal = c->cloudFadeTimer;
                    if(tVal < 0) {
                        tVal = 0;
                    }
                    float alpha = lerp(0.4f, 0, make_lerpTValue(tVal / cloud->fadePeriod));
                    
                    AtlasAsset *t = gameState->cloudText[cloud->cloudIndex];
                    
                    float2 scale = make_float2(s, s*t->aspectRatio_h_over_w);
                    float darkness = cloud->darkness;
                    pushTexture(renderer, atlasHandle, worldP, scale, make_float4(cloud->darkness, cloud->darkness, cloud->darkness, alpha), t->uv);
                }

                if(c->cloudFadeTimer >= 0) {
                    c->cloudFadeTimer += dt;

                    if(c->cloudFadeTimer >= maxTime) {
                        //NOTE: Turn fade timer off
                        c->cloudFadeTimer = -1;
                    }
                }
                
            }
        }
    }
}


void renderTileMap(GameState *gameState, Renderer *renderer, float dt) {
    DEBUG_TIME_BLOCK();

    pushShader(renderer, &terrainLightingShader);
    //NOTE: Draw the tile map
    int renderDistance = 3;
    float2 cameraBlockP = getChunkPosForWorldP(gameState->cameraPos.xy);
    int renderObjCount = 0;
    RenderObject objs[5] = {};
    float2 offset = make_float2(0, 0);
    clearResizeArray(gameState->trees);
    for(int tilez = 0; tilez <= CHUNK_DIM; ++tilez) {
        for(int y_ = renderDistance; y_ >= -renderDistance; --y_) {
            for(int x_ = -renderDistance; x_ <= renderDistance; ++x_) {
                Chunk *c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, x_ + offset.x, y_ + offset.y, 0, true, true);
                if(c) {
            
                    for(int tiley = 0; tiley <= CHUNK_DIM; ++tiley) {
                        for(int tilex = 0; tilex <= CHUNK_DIM; ++tilex) {
                            renderObjCount = 0; //NOTE: Clear the render queue
                            Tile *tile = c->getTile(tilex, tiley, tilez);

                            if(tile) {
                                float waterScale = 3;
                                float shadowScale = 3.1f;
                                float3 p = getTileWorldP(c, tilex, tiley, tilez);
                                float pX = (p.x + 0.5f) - gameState->cameraPos.x;
                                float pY = (p.y + 0.5f)  - gameState->cameraPos.y;
                                float3 defaultP = make_float3(pX, pY, 10);

                                u32 lightingMask = tile->lightingMask;

                                {
                                    if(tile->type == TILE_TYPE_BEACH) {
                                        assert(renderObjCount < arrayCount(objs));
                                        objs[renderObjCount++] = RenderObject(getTileTexture(&gameState->sandTileSet, tile->coords), defaultP, make_float2(1, 1), lightingMask);
                                    } else if(tile->type == TILE_TYPE_WATER_ROCK) {
                                        waterScale = 2;
                                        if(tile->animationController) {
                                            Texture *animationSprite = easyAnimation_updateAnimation_getTexture(tile->animationController, &gameState->animationState.animationItemFreeListPtr, dt);
                                            assert(renderObjCount < arrayCount(objs));
                                            objs[renderObjCount++] = RenderObject(animationSprite, defaultP, make_float2(waterScale, waterScale), lightingMask);
                                        }
                                    } else if(tile->type == TILE_TYPE_ROCK) {
                                        assert(renderObjCount < arrayCount(objs));
                                        objs[renderObjCount++] = RenderObject(getTileTexture(&gameState->sandTileSet, tile->coords), defaultP, make_float2(1, 1), lightingMask);

                                        if(tile->flags & TILE_FLAG_FRONT_FACE) {
                                            TileMapCoords coord = tile->coords;
                                            coord.y += 1;
                                            u32 lightingMask = tile->lightingMask >> 8; //NOTE: We want the top 8 bits so move them down
                                            
                                            assert(renderObjCount < arrayCount(objs));
                                            objs[renderObjCount++] = RenderObject(getTileTexture(&gameState->sandTileSet, coord), make_float3(pX, pY - 1, 10), make_float2(1, 1), lightingMask);
                                        }
                                        if(tile->flags & TILE_FLAG_TREE) {
                                            int sortingIndex = tiley;
                                            RenderObject r = RenderObject(0, make_float3(pX, pY + 1, 10), make_float2(3, 3), lightingMask, sortingIndex);
                                            pushArrayItem(&gameState->trees, r, RenderObject);
                                        }
                                    }

                                    if(tile->flags & TILE_FLAG_GRASSY_TOP) {
                                        assert(renderObjCount < arrayCount(objs));
                                        objs[renderObjCount++] = RenderObject(getTileTexture(&gameState->sandTileSet, tile->coordsSecondary), defaultP, make_float2(1, 1), lightingMask);
                                    }

                                    if((tile->flags & TILE_FLAG_FRONT_GRASS) || (tile->flags & TILE_FLAG_FRONT_BEACH)) {
                                        TileMapCoords coord = {};
                                        coord.x = 4;
                                        coord.y = 0;

                                        if(tile->flags & TILE_FLAG_FRONT_BEACH) {
                                            coord.x += 5;
                                        }

                                        assert(renderObjCount < arrayCount(objs));
                                        objs[renderObjCount++] = RenderObject(getTileTexture(&gameState->sandTileSet, coord), make_float3(pX, pY - 1, 10), make_float2(1, 1), lightingMask);
                                    }
                                } 
                                
                                if(tile->type != TILE_TYPE_WATER_ROCK) {
                                    if(tile->animationController) {
                                        Texture *animationSprite = easyAnimation_updateAnimation_getTexture(tile->animationController, &gameState->animationState.animationItemFreeListPtr, dt);
                                        //TODO: Add the water animation back in
                                        assert(renderObjCount < arrayCount(objs));
                                        RenderObject r = RenderObject(animationSprite, defaultP, make_float2(waterScale, waterScale), lightingMask);
                                        // pushArrayItem(&gameState->waterAnimations, r, RenderObject);
                                    }
                                }
                                
                                //NOTE: Render all the sprites now  
                                for(int i = 0; i < renderObjCount; ++i) {
                                    RenderObject b = objs[i];
                                    pushTexture(renderer, b.sprite->handle, b.pos, b.scale, make_float4(1, 1, 1, 1), b.sprite->uvCoords, b.lightingMask);
                                }
                                
                                // //NOTE: Render all the water animations now  
                                // for(int i = 0; i < getArrayLength(gameState->waterAnimations); ++i) {
                                //     RenderObject b = gameState->waterAnimations[i];
                                //     pushTexture(renderer, b.sprite->handle, b.pos, b.scale, make_float4(1, 1, 1, 1), b.sprite->uvCoords);
                                // }
                            }
                        }
                    }
                }
            }
        }
    }

    TextureHandle *atlasHandle = gameState->textureAtlas.texture.handle;
    {
        AtlasAsset *t = gameState->treeTexture;
        //NOTE: First sort
        qsort(gameState->trees, getArrayLength(gameState->trees), sizeof(RenderObject), compare_by_height);
        for(int i = 0; i < getArrayLength(gameState->trees); ++i) {
            RenderObject b = gameState->trees[i];
            pushTexture(renderer, atlasHandle, b.pos, b.scale, make_float4(1, 1, 1, 1), t->uv);
        }
    }

  
    
}


static float3 roundToGridBoard(float3 in, float tileSize) {
    float xMod = (in.x < 0) ? -tileSize : tileSize;
    float yMod = (in.y < 0) ? -tileSize : tileSize;
    
    float3 result = {};
    if(tileSize == 1) {
        result = make_float3((int)(in.x + xMod*0.5f), (int)(in.y + yMod*0.5f), (int)(in.z));
    } else {
        result = make_float3((int)(in.x + xMod*0.5f), (int)(in.y + yMod*0.5f), (int)(in.z));

        result.x -= ((int)result.x) % (int)tileSize;
        result.y -= ((int)result.y) % (int)tileSize;
    }
    
    return result;
}

void updateAStarEntity(GameState *gameState, Entity *e, float dt) {
    if(e->aStarController) {
        EasyAiController *controller = e->aStarController;
        
        float3 entP_inWorld_ = getWorldPosition(e);

        float3 playerInWorldP_ = getWorldPosition(gameState->player);

        float3 playerInWorldP = roundToGridBoard(playerInWorldP_, 1);
        float3 entP_inWorld = roundToGridBoard(entP_inWorld_, 1);
        entP_inWorld.z = 0;
        playerInWorldP.z = 0;
        entP_inWorld_.z = 0;
        playerInWorldP_.z = 0;
        
        float distance = float3_magnitude(minus_float3(playerInWorldP, entP_inWorld));
        //NOTE: See if we are far enough away from the player
        // if(distance > 0.5f) 
        {
            if(e->aStarController->searchBouysCount > 0) {
                playerInWorldP = playerInWorldP_ = controller->searchBouys[controller->bouyIndexAt];

                //NOTE: See if we have reached the bouy position
                float searchDist = 1.0f;
                if(float3_magnitude_sqr(minus_float3(playerInWorldP, entP_inWorld)) < (searchDist*searchDist)) {

                    // e->velocity.xy = make_float2(0, 0);
                    controller->bouyIndexAt++;

                    if(controller->bouyIndexAt >= controller->searchBouysCount) {
                        controller->bouyIndexAt = 0;
                    }
                }
            }
        }  
        
        {
            //NOTE: Update the A Start controller
            EasyAi_A_Star_Result aiResult = easyAi_update_A_star(controller, entP_inWorld,  playerInWorldP);

            //NOTE: Is in attacking animation
            // bool attack = easyAnimation_getCurrentAnimation(&entity->animationController, getAnimationForEnemy(gameState, ENTITY_ANIMATION_ATTACK, entity->enemyType));

            if(aiResult.found) {
                controller->lastSetPos = aiResult.nextPos;

                if(float3_equal(controller->lastSetPos, playerInWorldP)) {
                    controller->lastSetPos = playerInWorldP_; //get floating point version
                }

                float3 diff = minus_float3(controller->lastSetPos, e->pos);

                float2 dir = normalize_float2(diff.xy);

                e->velocity.xy = scale_float2(e->speed, dir);

                // if(getLengthSqrV3(diff) < 0.5f) { //if less than 1 metre away, try attacking
                // 	if(!attack) //NOTE: If not already attacking
                // 	{
                // 		//NOTE: Attack 
                // 		isAttackAnim = true;
                // 		animToAdd = getAnimationForEnemy(gameState, ENTITY_ANIMATION_ATTACK, entity->enemyType);
                // 	}
                // } else { //NOTE: More than 1m away
                // 	if(!attack) { //NOTE: Walk animation if not in the middle of attack animation
                // 		animToAdd = getAnimationForEnemy(gameState, ENTITY_ANIMATION_WALK, entity->enemyType);
                // 	}
                // }    
            }
        }
    }
}

void updateEntityVeclocity(Entity *e, float speed, float2 dir) {
    dir = normalize_float2(dir);
	dir = scale_float2(speed, dir); 

	e->velocity.xy = plus_float2(e->velocity.xy, dir);
}

Animation *getBestWalkAnimation(Entity *e) {
    Animation *animation = &e->animations->idle;
    float margin = 0.3f;
    removeEntityFlag(e, ENTITY_SPRITE_FLIPPED);

    float2 impluse = e->velocity.xy;

    if((impluse.x > margin  && impluse.y < -margin) || (impluse.x < -margin && impluse.y > margin) || ((impluse.x > margin && impluse.y < margin && impluse.y > -margin))) { //NOTE: The extra check is becuase the front & back sideways animation aren't matching - should flip one in the aesprite
        addEntityFlag(e, ENTITY_SPRITE_FLIPPED);
    }

    if(impluse.y > margin) {
        if(impluse.x < -margin || impluse.x > margin) {
            animation = &e->animations->run;	
        } else {
            animation = &e->animations->run;
        }
    } else if(impluse.y < -margin) {
        if(impluse.x < -margin || impluse.x > margin) {
            animation = &e->animations->run;	

            //TODO: Remove this when the animations are coorect
            if((impluse.x > margin  && impluse.y < -margin) || (impluse.x < -margin && impluse.y > margin) || ((impluse.x > margin && impluse.y < margin && impluse.y > -margin))) { //NOTE: The extra check is becuase the front & back sideways animation aren't matching - should flip one in the aesprite
                removeEntityFlag(e, ENTITY_SPRITE_FLIPPED);
            } else {
                addEntityFlag(e, ENTITY_SPRITE_FLIPPED);
            }
        } else {
            animation = &e->animations->run;
        }
    } else if(impluse.x < -margin || impluse.x > margin) {
        animation = &e->animations->run;
    }

    return animation;
}

void updateEntity(GameState *gameState, Renderer *renderer, Entity *e, float dt, float16 fovMatrix) {
    DEBUG_TIME_BLOCK();
        {
        // float16 modelToViewT = getModelToViewTransform(e, gameState->cameraPos);
        
        // modelToViewT = float16_multiply(fovMatrix, modelToViewT); 

        // pushMatrix(renderer, modelToViewT);

        //NOTEL Draw the attack box
        // pushRect(renderer, make_float3(0, 0, 0), make_float2(1, 1), make_float4(1, 0, 0, 1));
    }

    updateAStarEntity(gameState, e, dt);

    
}

void renderEntity(GameState *gameState, Renderer *renderer, Entity *e, float16 fovMatrix, float dt) {
    float16 modelToViewT = getModelToViewTransform(e, gameState->cameraPos);

    if(hasEntityFlag(e, ENTITY_SPRITE_FLIPPED)) {
        modelToViewT.E[0] *= -1;
        modelToViewT.E[1] *= -1;
        modelToViewT.E[2] *= -1;
    }
    
    // modelToViewT = float16_multiply(fovMatrix, modelToViewT); 

    Texture *t = easyAnimation_updateAnimation_getTexture(&e->animationController, &gameState->animationState.animationItemFreeListPtr, dt);
    if(e->animationController.finishedAnimationLastUpdate) {
        //NOTE: Make not active anymore. Should Probably remove it from the list. 
        // e->flags &= ~ENTITY_ACTIVE;

        // if(e->animationController.lastAnimationOn == &gameState->playerAttackAnimation) {
        //     //NOTE: Turn off attack collider when attack finishes
        //     // e->colliders[ATTACK_COLLIDER_INDEX].flags &= ~COLLIDER_ACTIVE; 
	    // }
    }

    float3 worldP = e->pos;
    worldP.x += e->offsetP.x * e->scale.x;
    worldP.y += e->offsetP.y * e->scale.y;
    
    worldP.x -= gameState->cameraPos.x;
    worldP.y -= gameState->cameraPos.y;

    pushTexture(renderer, t->handle, worldP, e->scale.xy, make_float4(1, 1, 1, 1), t->uvCoords);
    // pushRect(renderer, make_float3(0, 0, 0), make_float2(1, 1), make_float4(1, 1, 1, 1));
}

void pushAllEntityLights(GameState *gameState, float dt) {
    //NOTE: Push all lights for the renderer to use
	for(int i = 0; i < gameState->entityCount; ++i) {
		Entity *e = &gameState->entities[i];

		if(hasEntityFlag(e, ENTITY_ACTIVE) && hasEntityFlag(e, LIGHT_COMPONENT)) {
            float3 worldPos = getWorldPosition(e);
            //NOTE: Update the flicker
            e->perlinNoiseLight += dt;

            if(e->perlinNoiseLight > 1.0f) {
                e->perlinNoiseLight = 0.0f;
            }

            float value = SimplexNoise_fractal_1d(40, e->perlinNoiseLight, 3);

            //NOTE: Push light
            pushGameLight(gameState, worldPos, make_float4(1, 0.5f, 0, 1), value);
		}
	}
}

void updateEntitySelection(GameState *state, Entity *e, float windowWidth, float windowHeight, Renderer *renderer, float16 fovMatrix) {
     //NOTE: Update entity selection
#if DEBUG_BUILD

    if(state->gameMode == SELECT_ENTITY_MODE) 
    {

        float2 mouseP = make_float2(global_platformInput.mouseX, windowHeight - global_platformInput.mouseY);
        float2 mouseP_01 = make_float2(mouseP.x / windowWidth, mouseP.y / windowHeight);

        bool clicked = global_platformInput.keyStates[PLATFORM_MOUSE_LEFT_BUTTON].pressedCount > 0;
        bool released = global_platformInput.keyStates[PLATFORM_MOUSE_LEFT_BUTTON].releasedCount > 0;

        float worldX = lerp(-0.5f*state->planeSizeX*state->zoomLevel, 0.5f*state->planeSizeX*state->zoomLevel, make_lerpTValue(mouseP_01.x));
        float worldY = lerp(-0.5f*state->planeSizeY*state->zoomLevel, 0.5f*state->planeSizeY*state->zoomLevel, make_lerpTValue(mouseP_01.y));

        worldX += state->cameraPos.x;
        worldY += state->cameraPos.y;

        float3 entityWorldPos = getWorldPosition(e);

        float2 mouseWorldP = make_float2(worldX, worldY);

        bool inSelectionBounds = in_rect2f_bounds(make_rect2f_center_dim(entityWorldPos.xy, e->scale.xy), mouseWorldP);

        if(clicked && inSelectionBounds) {
            editorGui_clearInteraction(&state->editorGuiState);
            state->selectedEntityId = 0;
        } 

        EditorGuiId thisId = {};

        thisId.a = e->idHash;
        thisId.c = e->id;
        thisId.type = EDITOR_PLAYER_SELECT;

        if(state->selectedEntityId && easyString_stringsMatch_nullTerminated(state->selectedEntityId, e->id)) {
            //NOTE: Outline entity
            float16 modelToViewT = getModelToViewTransform(e, state->cameraPos);

            modelToViewT = float16_multiply(fovMatrix, modelToViewT); 

            pushMatrix(renderer, modelToViewT);

            pushRectOutlineWorldSpace(renderer,  make_float4(1, 0.5f, 0, 1));

        }

        if(!state->editorGuiState.currentInteraction.active && inSelectionBounds && clicked) {
            
            state->editorGuiState.currentInteraction.id = thisId;
            state->editorGuiState.currentInteraction.active = true;

            state->selectedEntityId = e->id;

            state->cameraFollowPlayer = false;
            
        } else if(state->editorGuiState.currentInteraction.active && sameEntityId(state->editorGuiState.currentInteraction.id, thisId)) {
            e->pos.x = mouseWorldP.x;
            e->pos.y = mouseWorldP.y;
        }

        if(released) {
            editorGui_clearInteraction(&state->editorGuiState);
            state->cameraFollowPlayer = true;
        }
    } else if(state->gameMode == A_STAR_MODE) {
        if(state->selectedEntityId) {
            void easyAi_pushNode(EasyAiController *controller, float3 pos);
            EasyAi_Node *easyAi_removeNode(EasyAiController *controller, float3 pos);
        }
    }

#endif
}

