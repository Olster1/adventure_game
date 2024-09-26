
void pushGameLight(EditorState *state, float3 worldPos, float4 color, float perlinNoiseValue) {
	if(state->lightCount < arrayCount(state->lights)) {
		GameLight *l = &state->lights[state->lightCount++];

		l->worldPos = worldPos;
		l->color = scale_float4(perlinNoiseValue, color);
	}
}

char *makeEntityId(EditorState *editorState) {
    u64 timeSinceEpoch = platform_getTimeSinceEpoch();
    char *result = easy_createString_printf(&globalPerEntityLoadArena, "%ld-%d-%d", timeSinceEpoch, editorState->randomIdStartApp, editorState->randomIdStart);

    //NOTE: This would have to be locked in threaded application
    editorState->randomIdStart++;

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

Entity *addPlayerEntity(EditorState *state) {
    Entity *e = 0;
    if(state->entityCount < arrayCount(state->entities)) {
        e = &state->entities[state->entityCount++];

        memset(e, 0, sizeof(Entity));

        e->id = makeEntityId(state);
        e->idHash = get_crc32_for_string(e->id);

        e->type = ENTITY_PLAYER;

        e->health = 10;

        e->velocity = make_float3(0, 0, 0);
        e->pos = make_float3(0, 0, 10);
        e->flags |= ENTITY_ACTIVE | ENTITY_FLAG_ONCE_OFF_ATTACK;
        e->scale = make_float3(2, 2, 1);
        e->speed = 1.0f;
        e->damage = 1;

        e->colliders[e->colliderCount++] = make_collider(make_float3(0, 0, 0), make_float3(1, 1, 0), COLLIDER_ACTIVE);
        
        //NOTE: Attack collider
        e->colliders[e->colliderCount++] = make_collider(make_float3(0, 0, 0), make_float3(1, 1, 0), COLLIDER_TRIGGER);

        //NOTE: Hurt collider
        e->colliders[e->colliderCount++] = make_collider(make_float3(0, 0, 0), make_float3(1.0f, 1.0f, 0), COLLIDER_ACTIVE | COLLIDER_TRIGGER);
        assert(e->colliders[HIT_COLLIDER_INDEX].flags & COLLIDER_ACTIVE);

        easyAnimation_initController(&e->animationController);
		easyAnimation_addAnimationToController(&e->animationController, &state->animationItemFreeListPtr, &state->playerIdleAnimation, 0.08f);
		
    }
    return e;
} 

Entity *addEnemyEntity(EditorState *state, DefaultEntityAnimations *animations) {
    Entity *e = 0;
    if(state->entityCount < arrayCount(state->entities)) {
        e = &state->entities[state->entityCount++];

        memset(e, 0, sizeof(Entity));

        e->id = makeEntityId(state);
        e->idHash = get_crc32_for_string(e->id);

        e->type = ENTITY_ENEMY;
        e->health = 10;

        e->animations = animations;

        e->velocity = make_float3(0, 0, 0);
        e->pos = make_float3(0, 0, 10);
        e->flags |= (ENTITY_ACTIVE | ENTITY_FLAG_ONCE_OFF_ATTACK);
        e->scale = make_float3(2, 2, 1);
        e->speed = 2;
        e->damage = 1;

        assert(e->colliderCount == 0);

        e->colliders[e->colliderCount++] = make_collider(make_float3(0, 0, 0), make_float3(1, 1, 0), COLLIDER_ACTIVE);
        
        //NOTE: Attack collider
        assert(e->colliderCount == ATTACK_COLLIDER_INDEX);
        e->colliders[e->colliderCount++] = make_collider(make_float3(0, 0, 0), make_float3(0.6f, 0.6f, 0), COLLIDER_TRIGGER);
        assert(!(e->colliders[ATTACK_COLLIDER_INDEX].flags & COLLIDER_ACTIVE));

        //NOTE: Hurt collider
        e->colliders[e->colliderCount++] = make_collider(make_float3(0, 0, 0), make_float3(1.5f, 1.5f, 0), COLLIDER_ACTIVE | COLLIDER_TRIGGER);

        easyAnimation_initController(&e->animationController);
		easyAnimation_addAnimationToController(&e->animationController, &state->animationItemFreeListPtr, &state->batAnimations.idle, 0.08f);

        e->aStarController = easyAi_initController(&globalPerEntityLoadArena);

        e->animations = &state->batAnimations;
		
    }
    return e;
} 

Entity *addFireballEnemy(EditorState *state) {
    Entity *e = 0;
    if(state->entityCount < arrayCount(state->entities)) {
        e = &state->entities[state->entityCount++];

        e->id = makeEntityId(state);
        e->idHash = get_crc32_for_string(e->id);
        
        e->type = ENTITY_FIREBALL;

        e->velocity = make_float3(-1, 0, 0);
        e->pos = make_float3(0, 0, 10);
        e->flags |= ENTITY_ACTIVE;
        e->respawnTimer = 3;
        e->scale = make_float3(2, 1, 1);

        e->colliders[e->colliderCount++] = make_collider(make_float3(0, 0, 0), make_float3(1, 1, 0), COLLIDER_ACTIVE | COLLIDER_TRIGGER);

        easyAnimation_initController(&e->animationController);
		easyAnimation_addAnimationToController(&e->animationController, &state->animationItemFreeListPtr, &state->fireballIdleAnimation, 0.3f);
		
    }
    return e;
} 

void renderTileMap(EditorState *editorState, Renderer *renderer) {
    //NOTE: Draw the tile map
	for(int i = 0; i < editorState->tileCount; ++i) {
		MapTile t = editorState->tiles[i];

		Texture *sprite = 0;

		//NOTE: Get the right texture
		switch(t.type) {
			case TILE_SET_SWAMP: {
				TileSet *set = &editorState->swampTileSet;

				int indexIntoArray = t.xId + (set->countX*t.yId);
				assert(indexIntoArray < set->count);
                assert(indexIntoArray >= 0);
				sprite = set->tiles[indexIntoArray];

			} break;
		}

		float pX = (t.x + 0.5f) - editorState->cameraPos.x;
		float pY = (t.y + 0.5f)  - editorState->cameraPos.y;

		pushTexture(renderer, sprite->handle, make_float3(pX, pY, 10), make_float2(1, 1), make_float4(1, 1, 1, 1), sprite->uvCoords);
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

void updateAStarEntity(EditorState *editorState, Entity *e, float dt) {
    float3 entP_inWorld_ = getWorldPosition(e);

    float3 playerInWorldP_ = getWorldPosition(editorState->player);

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

void updateEntity(EditorState *editorState, Renderer *renderer, Entity *e, float dt, float16 fovMatrix) {
        {
        float16 modelToViewT = getModelToViewTransform(e, editorState->cameraPos);
        
        modelToViewT = float16_multiply(fovMatrix, modelToViewT); 

        pushMatrix(renderer, modelToViewT);

        //NOTEL Draw the attack box
        pushRect(renderer, make_float3(0, 0, 0), make_float2(1, 1), make_float4(1, 0, 0, 1));
    }

    updateAStarEntity(editorState, e, dt);
    
    float hitForce = 30;

    if(e->type == ENTITY_PLAYER) {
        Collider c = e->colliders[HIT_COLLIDER_INDEX];
        assert(c.flags & COLLIDER_ACTIVE);
        assert(e->colliderCount > 1);

        if(c.collideEventsCount > 0) {
            for(int i = 0; i < c.collideEventsCount; ++i) {
                const CollideEvent event = c.events[i];  

                if((event.type == COLLIDE_STAY || event.type == COLLIDE_ENTER) && event.entityType == ENTITY_ENEMY && event.damage > 0) {
                    //NOTE: Hit by enemy
                    e->health -= event.damage;
                    
                    e->velocity.xy = plus_float2(e->velocity.xy, scale_float2(hitForce, event.hitDir));
                }
            }
        }
    }

    if(e->type == ENTITY_ENEMY) {

        Collider c = e->colliders[HIT_COLLIDER_INDEX];
        assert(c.flags & COLLIDER_ACTIVE);
        assert(e->colliderCount > 1);

        Animation *animToAdd = &e->animations->idle;

        if(c.collideEventsCount > 0) {
            for(int i = 0; i < c.collideEventsCount; ++i) {
                const CollideEvent event = c.events[i];  

                if((event.type == COLLIDE_STAY || event.type == COLLIDE_ENTER) && event.entityType == ENTITY_PLAYER && event.damage > 0) {
                    //NOTE: Hit by enemy
                    e->health -= event.damage;
                    e->velocity.xy = plus_float2(e->velocity.xy, scale_float2(hitForce, event.hitDir));
                    //NOTE: Play the hit animation
                    animToAdd = &e->animations->hurt;
                    e->aStarController->aiMode = EASY_AI_HURT;
                    e->aStarController->waitTimer = 0;

                    if(e->health <= 0) {
                        animToAdd = &e->animations->die;
                        e->aStarController->aiMode = EASY_AI_DIE;
                    }
                }
            }
        }
       
        
        e->aStarController->waitTimer += dt;
        //NOTE: Update the enemy 
        assert(e->aStarController);
        switch(e->aStarController->aiMode) {
            case(EASY_AI_IDLE): {
                //NOTE: Do nothing
                if(e->aStarController->waitTimer > 0.5f) {
                    e->aStarController->waitTimer = 0;
                    e->aStarController->aiMode = EASY_AI_MOVE_TOWARDS;
                }
            } break;
            case(EASY_AI_MOVE_TOWARDS): {
                if(e->aStarController->waitTimer > 10) {
                    e->aStarController->waitTimer = 0;
                    e->aStarController->aiMode = EASY_AI_IDLE;
                }

                 float2 dir = minus_float2(editorState->player->pos.xy, e->pos.xy);

                 float speed = 50*dt;
                
                if(float2_magnitude(dir) < 1.0f) {
                    //NOTE: Attack now
                    speed = 15; //NOTE: No *dt because it's an impulse not a acceleration - i.e. just added this frame
                    e->aStarController->waitTimer = 0;

                    e->aStarController->aiMode = EASY_AI_ATTACK;
                    
                    animToAdd = &e->animations->attack;

                    e->flags |= ENTITY_FLAG_ATTACKING;
                }

                updateEntityVeclocity(e, speed, dir);

                if(e->aStarController->aiMode != EASY_AI_ATTACK) {
                    animToAdd = getBestWalkAnimation(e);
                }
            } break;
            case(EASY_AI_ATTACK): {
                //NOTE: Once in the attack loop remove the flag because we want to be attacking just once
                //      This depends if the enemy is constantly attacking for the whole attack anaimation
                if(e->flags & ENTITY_FLAG_ONCE_OFF_ATTACK) {
                    e->flags &= ~(ENTITY_FLAG_ATTACKING);
                }
                
                animToAdd = &e->animations->attack;

                if(e->velocity.y > 0) {
                    animToAdd = &e->animations->attackBack;
                }

                if(e->animationController.finishedAnimationLastUpdate && (e->animationController.lastAnimationOn == &e->animations->attack || e->animationController.lastAnimationOn == &e->animations->attackBack)) {
                    animToAdd = &e->animations->idle;
                    e->aStarController->aiMode = EASY_AI_IDLE;
                    e->flags &= ~(ENTITY_FLAG_ATTACKING); //NOTE: Make sure not attacking anymore
                    
                }

            } break;
            case(EASY_AI_COOL_DOWN): {

            } break;
            case(EASY_AI_HURT): {
                animToAdd = &e->animations->hurt;
                //NOTE: Do nothing
                if(e->aStarController->waitTimer > 1.0f) {
                    e->aStarController->waitTimer = 0;
                    e->aStarController->aiMode = EASY_AI_MOVE_TOWARDS;
                }
            } break;
            case(EASY_AI_DIE): {
                animToAdd = &e->animations->die;
            } break;
            default: {
                assert(false);
            } break;
        } 

        if(animToAdd && e->animationController.lastAnimationOn != animToAdd) {
            easyAnimation_emptyAnimationContoller(&e->animationController, &editorState->animationItemFreeListPtr);
		    easyAnimation_addAnimationToController(&e->animationController, &editorState->animationItemFreeListPtr, animToAdd, 0.08f);	
        }
    }
}

void renderEntity(EditorState *editorState, Renderer *renderer, Entity *e, float16 fovMatrix, float dt) {
    float16 modelToViewT = getModelToViewTransform(e, editorState->cameraPos);

    if(e->spriteFlipped) {
        modelToViewT.E[0] *= -1;
        modelToViewT.E[1] *= -1;
        modelToViewT.E[2] *= -1;
    }
    
    modelToViewT = float16_multiply(fovMatrix, modelToViewT); 

    pushMatrix(renderer, modelToViewT);

    Texture *t = easyAnimation_updateAnimation_getTexture(&e->animationController, &editorState->animationItemFreeListPtr, dt);
    if(e->animationController.finishedAnimationLastUpdate) {
        //NOTE: Make not active anymore. Should Probably remove it from the list. 
        // e->flags &= ~ENTITY_ACTIVE;

        if(e->animationController.lastAnimationOn == &editorState->playerAttackAnimation) {
            //NOTE: Turn off attack collider when attack finishes
            // e->colliders[ATTACK_COLLIDER_INDEX].flags &= ~COLLIDER_ACTIVE; 
	    }
    }

    pushTexture(renderer, t->handle, make_float3(0, 0, 0), make_float2(1, 1), make_float4(1, 1, 1, 1), t->uvCoords);
    // pushRect(renderer, make_float3(0, 0, 0), make_float2(1, 1), make_float4(1, 1, 1, 1));
}

void pushAllEntityLights(EditorState *editorState, float dt) {
    //NOTE: Push all lights for the renderer to use
	for(int i = 0; i < editorState->entityCount; ++i) {
		Entity *e = &editorState->entities[i];

		if(e->flags & ENTITY_ACTIVE) {

			float3 worldPos = getWorldPosition(e);

			if(e->flags & LIGHT_COMPONENT) {
				//NOTE: Update the flicker
				e->perlinNoiseLight += dt;

				if(e->perlinNoiseLight > 1.0f) {
					e->perlinNoiseLight = 0.0f;
				}

				float value = perlin1d(e->perlinNoiseLight, 40, 3);

				//NOTE: Push light
				pushGameLight(editorState, worldPos, make_float4(1, 0.5f, 0, 1), value);
			}
		}
	}
}

void updateEntitySelection(EditorState *state, Entity *e, float windowWidth, float windowHeight, Renderer *renderer, float16 fovMatrix) {
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

