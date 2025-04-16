
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
    result.x += e_->offsetP.x * e_->scale.x;
    result.y += e_->offsetP.y * e_->scale.y;

    Entity *e = e_->parent;

    while(e) {
        float3 localP = e->pos;
        localP.x += e->offsetP.x * e->scale.x;
        localP.y += e->offsetP.y * e->scale.y;
        
        result = plus_float3(result, localP);

        e = e->parent;
    }

    return result;
}

float16 getModelToViewTransform(Entity *e_, float3 cameraPos) {

    float16 result = getModelToWorldTransform(e_);

    result = float16_multiply(float16_set_pos(float16_identity(), float3_negate(cameraPos)), result);

    return result;

}



Entity *makeNewEntity(GameState *state, float3 worldP) {
    Entity *e = 0;
    if(state->entityCount < arrayCount(state->entities)) {
        e = &state->entities[state->entityCount++];

        memset(e, 0, sizeof(Entity));

        e->id = global_entityId++;

        e->velocity = make_float3(0, 0, 0);
        e->offsetP = make_float3(0, 0, 0);
        e->maxMoveDistance = MAX_MOVE_DISTANCE; //NOTE: Default move distance per turn
        e->pos = make_float3(worldP.x, worldP.y, worldP.z);
        e->flags |= ENTITY_ACTIVE;
        e->scale = make_float3(4, 4, 1);
        e->speed = 3.0f;

        assert(e->maxMoveDistance <= MAX_MOVE_DISTANCE);

    }
    return e;
}

Entity *addKnightEntity(GameState *state, float3 worldP) {
    Entity *e = makeNewEntity(state, worldP);
    if(e) {
        e->type = ENTITY_KNIGHT;
        e->offsetP.y = 0.16; //NOTE: Fraction of the scale
        e->scale = make_float3(2.5f, 2.5f, 1);
        easyAnimation_initController(&e->animationController);
		easyAnimation_addAnimationToController(&e->animationController, &state->animationState.animationItemFreeListPtr, &state->knightAnimations.idle, 0.08f);
    }
    return e;
} 

Entity *addHouseEntity(GameState *state, float3 worldP) {
    Entity *e = makeNewEntity(state, worldP);
    if(e) {
        e->type = ENTITY_HOUSE;
        e->offsetP.y = 0.08f;
        e->scale = make_float3(2, 3, 1);
        easyAnimation_initController(&e->animationController);
		easyAnimation_addAnimationToController(&e->animationController, &state->animationState.animationItemFreeListPtr, &state->houseAnimation, 0.08f);
    }
    return e;
} 

Entity *addCastleEntity(GameState *state, float3 worldP) {
    Entity *e = makeNewEntity(state, worldP);
    if(e) {
        e->type = ENTITY_CASTLE;
        e->offsetP.y = 0.07;
        e->scale = make_float3(6, 5, 1);
        easyAnimation_initController(&e->animationController);
		easyAnimation_addAnimationToController(&e->animationController, &state->animationState.animationItemFreeListPtr, &state->castleAnimation, 0.08f);
    }
    return e;
} 

Entity *addPeasantEntity(GameState *state, float3 worldP) {
    Entity *e = makeNewEntity(state, worldP);
    if(e) {
        e->type = ENTITY_PEASANT;
        e->offsetP.y = 0.16; //NOTE: Fraction of the scale
        easyAnimation_initController(&e->animationController);
		easyAnimation_addAnimationToController(&e->animationController, &state->animationState.animationItemFreeListPtr, &state->peasantAnimations.idle, 0.08f);
    }
    return e;
} 

Entity *addArcherEntity(GameState *state, float3 worldP) {
    Entity *e = makeNewEntity(state, worldP);
    if(e) {
        e->type = ENTITY_ARCHER;
        e->offsetP.y = 0.16; //NOTE: Fraction of the scale
        easyAnimation_initController(&e->animationController);
		easyAnimation_addAnimationToController(&e->animationController, &state->animationState.animationItemFreeListPtr, &state->archerAnimations.idle, 0.08f);
    }
    return e;
} 

Entity *addGoblinEntity(GameState *state, float3 worldP) {
    Entity *e = makeNewEntity(state, worldP);
    if(e) {
        e->type = ENTITY_GOBLIN;
        e->offsetP.y = 0.16; //NOTE: Fraction of the scale
        easyAnimation_initController(&e->animationController);
		easyAnimation_addAnimationToController(&e->animationController, &state->animationState.animationItemFreeListPtr, &state->goblinAnimations.idle, 0.08f);
    }
    return e;
} 

Entity *addGoblinBarrelEntity(GameState *state, float3 worldP) {
    Entity *e = makeNewEntity(state, worldP);
    if(e) {
        e->type = ENTITY_GOBLIN_BARREL;
        e->offsetP.y = 0.16; //NOTE: Fraction of the scale
        easyAnimation_initController(&e->animationController);
		easyAnimation_addAnimationToController(&e->animationController, &state->animationState.animationItemFreeListPtr, &state->barrellAnimations.idle, 0.08f);
    }
    return e;
} 

Entity *addGoblinTntEntity(GameState *state, float3 worldP) {
    Entity *e = makeNewEntity(state, worldP);
    if(e) {
        e->type = ENTITY_GOBLIN_TNT;
        e->offsetP.y = 0.16; //NOTE: Fraction of the scale
        easyAnimation_initController(&e->animationController);
		easyAnimation_addAnimationToController(&e->animationController, &state->animationState.animationItemFreeListPtr, &state->tntAnimations.idle, 0.08f);
    }
    return e;
} 


int compare_by_height(const void *a, const void *b) {
    const RenderObject *pa = (const RenderObject *)a;
    const RenderObject *pb = (const RenderObject *)b;
    int result = (int)(pb->sortIndex - pa->sortIndex);
    
    return result;
}

void drawTrees(GameState *gameState, Renderer *renderer) {
    DEBUG_TIME_BLOCK();
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
                    
                    for(int i = 0; i < MAX_CLOUD_DIM; i++) {
                        for(int j = 0; j < MAX_CLOUD_DIM; j++) {
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
                    float3 worldP = make_float3(x*CHUNK_DIM, y*CHUNK_DIM, 2);
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

float3 getRenderWorldP(float3 p) {
    p.y += p.z;
    return p;
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
                                float pX = (p.x) - gameState->cameraPos.x;
                                float pY = (p.y)  - gameState->cameraPos.y;
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

bool isEntitySelected(GameState *gameState, Entity *e) {
    bool result = false;
    for(int i = 0; i < gameState->selectedEntityCount && !result; ++i) {
        if(gameState->selectedEntityIds[i] == e->id) {
            result = true;
            break;
        }
    }
    return result;
}

void updateEntity(GameState *gameState, Renderer *renderer, Entity *e, float dt, float3 mouseWorldP) {
    DEBUG_TIME_BLOCK();
    updateAStarEntity(gameState, e, dt);

    bool clicked = global_platformInput.keyStates[PLATFORM_MOUSE_LEFT_BUTTON].pressedCount > 0;

    float3 p = getWorldPosition(e);
    float2 chunkP =  getChunkPosForWorldP(p.xy);
    //NOTE: Make sure the chunk the player is on is available
    int chunkRadius = 0;
    //TODO: Maybe just if they get close to the edge
    for(int i = -chunkRadius; i <= chunkRadius; i++) {
        for(int j = -chunkRadius; j <= chunkRadius; j++) {
            Chunk *c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, chunkP.x + j, chunkP.y + i, 0, true, true);
            assert(c);
        }
    }

    if(isEntitySelected(gameState, e)) {
        //NOTE: Draw the path 
        MemoryArenaMark mark = takeMemoryMark(&globalPerFrameArena);

        FloodFillEvent *foundNode = floodFillSearch(gameState, convertRealWorldToBlockCoords(p), convertRealWorldToBlockCoords(mouseWorldP));

        if(foundNode) {
        } else {
            gameState->selectedColor = make_float4(1, 0, 0, 1);
        }

        
            

	    releaseMemoryMark(&mark);
        if(clicked) {
            //NOTE: active the path

        }
    }
}


void renderEntity(GameState *gameState, Renderer *renderer, Entity *e, float16 fovMatrix, float dt) {

    Texture *t = 0;

    if(easyAnimation_isControllerValid(&e->animationController)) {

        t = easyAnimation_updateAnimation_getTexture(&e->animationController, &gameState->animationState.animationItemFreeListPtr, dt);
        if(e->animationController.finishedAnimationLastUpdate) {
            //NOTE: Make not active anymore. Should Probably remove it from the list. 
            // e->flags &= ~ENTITY_ACTIVE;

            // if(e->animationController.lastAnimationOn == &gameState->playerAttackAnimation) {
            //     //NOTE: Turn off attack collider when attack finishes
            //     // e->colliders[ATTACK_COLLIDER_INDEX].flags &= ~COLLIDER_ACTIVE; 
            // }
        }
    } 

    
    float3 renderWorldP = getRenderWorldP(e->pos);
    renderWorldP.x += e->offsetP.x * e->scale.x;
    renderWorldP.y += e->offsetP.y * e->scale.y;
    
    renderWorldP.x -= gameState->cameraPos.x;
    renderWorldP.y -= gameState->cameraPos.y;

    float4 color = make_float4(1, 1, 1, 1);

    // NOTE: color the entity that you have selected
    if(isEntitySelected(gameState, e)) {
        color.y = 0;
    }
    

    //NOTE: Draw position above player
    float3 tileP = convertRealWorldToBlockCoords(e->pos);
    char *str = easy_createString_printf(&globalPerFrameArena, "(%d %d %d)", (int)tileP.x, (int)tileP.y, (int)tileP.z);
    pushShader(renderer, &sdfFontShader);
	// draw_text(renderer, &gameState->font, str, renderWorldP.x, renderWorldP.y, 0.02, make_float4(0, 0, 0, 1)); 

    if(t) {
        pushShader(renderer, &pixelArtShader);
        pushTexture(renderer, t->handle, renderWorldP, e->scale.xy, color, t->uvCoords);
    }
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

void updateEntitySelection(Renderer *renderer, GameState *gameState, float2 mouseWorldP) {
    //NOTE: Update entity selection
    bool clicked = global_platformInput.keyStates[PLATFORM_MOUSE_LEFT_BUTTON].pressedCount > 0;
    bool released = global_platformInput.keyStates[PLATFORM_MOUSE_LEFT_BUTTON].releasedCount > 0;

    int entityClickCount = 0;
    for(int i = 0; i < gameState->entityCount; ++i) {
		Entity *e = &gameState->entities[i];

		if(e->flags & ENTITY_ACTIVE) {
          
            float3 entityWorldPos = getWorldPosition(e);
            bool inSelectionBounds = in_rect2f_bounds(make_rect2f_center_dim(entityWorldPos.xy, scale_float2(0.5f, e->scale.xy)), mouseWorldP);

            if(clicked) {
                if(inSelectionBounds) {
                    assert(gameState->selectedEntityCount < arrayCount(gameState->selectedEntityIds));
                    if(gameState->selectedEntityCount < arrayCount(gameState->selectedEntityIds)) {
                        gameState->selectedEntityIds[gameState->selectedEntityCount++] = e->id;
                    }
                    entityClickCount++;
                }
            } 
		}
	}

    if(clicked && entityClickCount == 0) {
        gameState->selectedEntityCount = 0;
        gameState->selectHoverTimer = 0;
    }
    
}

