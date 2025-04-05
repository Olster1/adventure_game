
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

        e->type = ENTITY_PLAYER;

        e->velocity = make_float3(0, 0, 0);
        e->pos = make_float3(0, 0, 10);
        e->flags |= ENTITY_ACTIVE;
        e->scale = make_float3(2, 2, 1);
        e->speed = 3.0f;
    }
    return e;
}

Entity *addPlayerEntity(GameState *state) {
    Entity *e = makeNewEntity(state);
    if(e) {
        easyAnimation_initController(&e->animationController);
		easyAnimation_addAnimationToController(&e->animationController, &state->animationState.animationItemFreeListPtr, &state->playerIdleAnimation, 0.08f);
		
    }
    return e;
} 



Entity *addPotPlantEntity(GameState *state, DefaultEntityAnimations *animations) {
    Entity *e = makeNewEntity(state);
    if(e) {
        e->type = ENTITY_OBJECT;
        e->scale = make_float3(1, 2, 1);

        easyAnimation_initController(&e->animationController);
		easyAnimation_addAnimationToController(&e->animationController, &state->animationState.animationItemFreeListPtr, &animations->idle, 5.0f);
        e->animations = animations;

    }
    return e;
} 

void renderTileMap(GameState *gameState, Renderer *renderer, float dt) {
    //NOTE: Draw the tile map
    int renderDistance = 3;

    float2 cameraBlockP = getChunkPosForWorldP(gameState->cameraPos.xy);
    int renderObjCount = 0;
    RenderObject objs[5] = {};
    float2 offset = make_float2(0, 0);
    for(int tilez = 0; tilez <= CHUNK_DIM; ++tilez) {
        const int RENDER_PASS_COUNT = 2;
        for(int pass = 0; pass <= RENDER_PASS_COUNT; ++pass) {
            for(int y_ = renderDistance; y_ >= -renderDistance; --y_) {
                for(int x_ = -renderDistance; x_ <= renderDistance; ++x_) {

                    Chunk *c = gameState->terrain.getChunk(&gameState->animationState, x_ + offset.x, y_ + offset.y, 0);
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

                                    u8 lightingMask = tile->lightingMask;

                                    if(pass == 1) {
                                        if(tile->type == TILE_TYPE_BEACH) {
                                            assert(renderObjCount < arrayCount(objs));
                                            objs[renderObjCount++] = RenderObject(getTileTexture(&gameState->sandTileSet, tile->coords), defaultP, make_float2(1, 1));
                                        } else if(tile->type == TILE_TYPE_WATER_ROCK) {
                                            waterScale = 2;
                                            if(tile->animationController) {
                                                Texture *animationSprite = easyAnimation_updateAnimation_getTexture(tile->animationController, &gameState->animationState.animationItemFreeListPtr, dt);
                                                assert(renderObjCount < arrayCount(objs));
                                                objs[renderObjCount++] = RenderObject(animationSprite, defaultP, make_float2(waterScale, waterScale));
                                            }
                                        } else if(tile->type == TILE_TYPE_ROCK) {
                                            assert(renderObjCount < arrayCount(objs));
                                            objs[renderObjCount++] = RenderObject(getTileTexture(&gameState->sandTileSet, tile->coords), defaultP, make_float2(1, 1));

                                            if(tile->flags & TILE_FLAG_FRONT_FACE) {
                                                TileMapCoords coord = tile->coords;
                                                coord.y += 1;
                                                u8 lightingMask = tile->lightingMask >> 4; //NOTE: We want the top 4 bits so move them down
                                                
                                                assert(renderObjCount < arrayCount(objs));
                                                objs[renderObjCount++] = RenderObject(getTileTexture(&gameState->sandTileSet, coord), make_float3(pX, pY - 1, 10), make_float2(1, 1));
                                            }
                                        }

                                        if(tile->flags & TILE_FLAG_GRASSY_TOP) {
                                            assert(renderObjCount < arrayCount(objs));
                                            objs[renderObjCount++] = RenderObject(getTileTexture(&gameState->sandTileSet, tile->coordsSecondary), defaultP, make_float2(1, 1));
                                        }

                                        if((tile->flags & TILE_FLAG_FRONT_GRASS) || (tile->flags & TILE_FLAG_FRONT_BEACH)) {
                                            TileMapCoords coord = {};
                                            coord.x = 4;
                                            coord.y = 0;

                                            if(tile->flags & TILE_FLAG_FRONT_BEACH) {
                                                coord.x += 5;
                                            }

                                            assert(renderObjCount < arrayCount(objs));
                                            objs[renderObjCount++] = RenderObject(getTileTexture(&gameState->sandTileSet, coord), make_float3(pX, pY - 1, 10), make_float2(1, 1));
                                        }
                                    } else if(pass == 0 && tile->type != TILE_TYPE_WATER_ROCK) {
                                        if(tile->animationController) {
                                            Texture *animationSprite = easyAnimation_updateAnimation_getTexture(tile->animationController, &gameState->animationState.animationItemFreeListPtr, dt);
                                            assert(renderObjCount < arrayCount(objs));
                                            objs[renderObjCount++] = RenderObject(animationSprite, defaultP, make_float2(waterScale, waterScale));
                                        }
                                        // if(tile->flags & TILE_FLAG_SHADOW) {
                                        //     Texture *t = &gameState->shadowTexture;
                                        //     assert(renderObjCount < arrayCount(objs));
                                        //     objs[renderObjCount++] = RenderObject(t, make_float3(pX, pY - 1, 10), make_float2(shadowScale, shadowScale));
                                        // }
                                        
                                    }

                                    //NOTE: Render all the sprites now
                                    for(int i = 0; i < renderObjCount; ++i) {
                                        RenderObject b = objs[i];
                                        pushTexture(renderer, b.sprite->handle, b.pos, b.scale, make_float4(1, 1, 1, 1), b.sprite->uvCoords);
                                    }
                                }
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
    float3 entP_inWorld_ = getWorldPosition(e);

    float3 playerInWorldP_ = getWorldPosition(gameState->player);

    float3 playerInWorldP = roundToGridBoard(playerInWorldP_, 1);
	float3 entP_inWorld = roundToGridBoard(entP_inWorld_, 1);
    entP_inWorld.z = 0;
    playerInWorldP.z = 0;
    entP_inWorld_.z = 0;
    playerInWorldP_.z = 0;

    if(e->aStarController) {
        float distance = float3_magnitude(minus_float3(playerInWorldP, entP_inWorld));
        //NOTE: See if we are far enough away from the player
        // if(distance > 0.5f) 
        {
            if(e->aStarController->searchBouysCount > 0) {
                playerInWorldP = playerInWorldP_ = e->aStarController->searchBouys[e->aStarController->bouyIndexAt];

                //NOTE: See if we have reached the bouy position
                float searchDist = 1.0f;
                if(float3_magnitude_sqr(minus_float3(playerInWorldP, entP_inWorld)) < (searchDist*searchDist)) {

                    // e->velocity.xy = make_float2(0, 0);
                    e->aStarController->bouyIndexAt++;

                    if(e->aStarController->bouyIndexAt >= e->aStarController->searchBouysCount) {
                        e->aStarController->bouyIndexAt = 0;
                    }
                }
            }
        }  
        
        {
            //NOTE: Update the A Start controller
            EasyAi_A_Star_Result aiResult = easyAi_update_A_star(e->aStarController, entP_inWorld,  playerInWorldP);

            //NOTE: Is in attacking animation
            // bool attack = easyAnimation_getCurrentAnimation(&entity->animationController, getAnimationForEnemy(gameState, ENTITY_ANIMATION_ATTACK, entity->enemyType));

            if(aiResult.found) {
                e->lastSetPos = aiResult.nextPos;

                if(float3_equal(e->lastSetPos, playerInWorldP)) {
                    e->lastSetPos = playerInWorldP_; //get floating point version
                }

                float3 diff = minus_float3(e->lastSetPos, e->pos);

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
    e->spriteFlipped = false;

    float2 impluse = e->velocity.xy;

    if((impluse.x > margin  && impluse.y < -margin) || (impluse.x < -margin && impluse.y > margin) || ((impluse.x > margin && impluse.y < margin && impluse.y > -margin))) { //NOTE: The extra check is becuase the front & back sideways animation aren't matching - should flip one in the aesprite
        e->spriteFlipped = true;
    }

    if(impluse.y > margin) {
        if(impluse.x < -margin || impluse.x > margin) {
            animation = &e->animations->runSidewardBack;	
        } else {
            animation = &e->animations->runBack;
        }
    } else if(impluse.y < -margin) {
        if(impluse.x < -margin || impluse.x > margin) {
            animation = &e->animations->runSideward;	

            //TODO: Remove this when the animations are coorect
            if((impluse.x > margin  && impluse.y < -margin) || (impluse.x < -margin && impluse.y > margin) || ((impluse.x > margin && impluse.y < margin && impluse.y > -margin))) { //NOTE: The extra check is becuase the front & back sideways animation aren't matching - should flip one in the aesprite
                e->spriteFlipped = false;
            } else {
                e->spriteFlipped = true;
            }
        } else {
            animation = &e->animations->runForward;
        }
    } else if(impluse.x < -margin || impluse.x > margin) {
        animation = &e->animations->runSideward;
    }

    return animation;
}

void updateEntity(GameState *gameState, Renderer *renderer, Entity *e, float dt, float16 fovMatrix) {
        {
        float16 modelToViewT = getModelToViewTransform(e, gameState->cameraPos);
        
        modelToViewT = float16_multiply(fovMatrix, modelToViewT); 

        pushMatrix(renderer, modelToViewT);

        //NOTEL Draw the attack box
        // pushRect(renderer, make_float3(0, 0, 0), make_float2(1, 1), make_float4(1, 0, 0, 1));
    }

    updateAStarEntity(gameState, e, dt);
    
    if(e->type == ENTITY_PLAYER) {
    }
}

void renderEntity(GameState *gameState, Renderer *renderer, Entity *e, float16 fovMatrix, float dt) {
    float16 modelToViewT = getModelToViewTransform(e, gameState->cameraPos);

    if(e->spriteFlipped) {
        modelToViewT.E[0] *= -1;
        modelToViewT.E[1] *= -1;
        modelToViewT.E[2] *= -1;
    }
    
    modelToViewT = float16_multiply(fovMatrix, modelToViewT); 

    pushMatrix(renderer, modelToViewT);

    Texture *t = easyAnimation_updateAnimation_getTexture(&e->animationController, &gameState->animationState.animationItemFreeListPtr, dt);
    if(e->animationController.finishedAnimationLastUpdate) {
        //NOTE: Make not active anymore. Should Probably remove it from the list. 
        // e->flags &= ~ENTITY_ACTIVE;

        if(e->animationController.lastAnimationOn == &gameState->playerAttackAnimation) {
            //NOTE: Turn off attack collider when attack finishes
            // e->colliders[ATTACK_COLLIDER_INDEX].flags &= ~COLLIDER_ACTIVE; 
	    }
    }

    pushTexture(renderer, t->handle, make_float3(0, 0, 0), make_float2(1, 1), make_float4(1, 1, 1, 1), t->uvCoords);
    // pushRect(renderer, make_float3(0, 0, 0), make_float2(1, 1), make_float4(1, 1, 1, 1));
}

void pushAllEntityLights(GameState *gameState, float dt) {
    //NOTE: Push all lights for the renderer to use
	for(int i = 0; i < gameState->entityCount; ++i) {
		Entity *e = &gameState->entities[i];

		if(e->flags & ENTITY_ACTIVE) {

			float3 worldPos = getWorldPosition(e);

			if(e->flags & LIGHT_COMPONENT) {
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

